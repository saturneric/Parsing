//
//  main.cpp
//  parsing
//
//  Created by 胡宇 on 2021/4/11.
//

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <map>

using std::printf;
using std::fgetc;
using std::fprintf;
using std::fopen;
using std::putchar;
using std::string;
using std::stringstream;
using std::vector;
using std::ofstream;
using std::ios;
using std::map;



class Automata {
public:
    
    Automata(string path) {
        fp = fopen(path.c_str(), "r");
    }
    
    ~Automata() {
        if(fp != nullptr) {
            fclose(fp);
        }
    }
    
    void parse() {
        while(!ifeof) {
            tokens.push_back(nextToken());
        }
    }
    
    void output(string path) {
        
        ofstream stream(path, ios::out | ios::trunc);
        int line = 1;
        stream << "1 ";
        
        for(auto token : tokens) {
            if(token.line > line) {
                stream << '\n' << line << ' ';
                line = token.line;
            }
            
            if(token.token == ERROR) {
                stream << tokenTypeStrMap.find(token.token)->second << '{' << token.str << ", " << stateTypeStrMap.find(token.state)->second << '}' << ' ';
            } else {
                stream << tokenTypeStrMap.find(token.token)->second << '(' << token.str << ')' << ' ';
            }
            
        }
        
        stream.close();
    }
    
private:
    
    using TokenType = enum {
        /* Reserve Words */
        STRUCT, BOOLEAN, SHORT, LONG,
        DOUBLE, FLOAT,
        INT8, INT16, INT32, INT64,
        UINT8, UINT16, UINT32, UINT64,
        CHAR,
        UNSIGNED,
        /* Special Symbols */
        OPENING_BRACE, CLOSING_BRACE,
        SEMICOLON,
        LEFT_BRACKET, RIGHT_BRACKET,
        MULT, PLUS, SUB, TILDE, SLASH,
        PERCENT, LEFT_SHIFT, RIGHT_SHIFT, AND, INSERT,
        DELIMITER, COMMA,
        /* Multicharacter Tokens */
        ID, LETTER, DIGIT, UNDERLINE, T_TRUE, T_FALSE,
        INTEGER, INTEGER_TYPE_SUFFIX, STRING, T_BOOLEAN,
        /* None & Error & EOF */
        NONE, ERROR, T_EOF
    };
    

    using StateType = enum {
        START,
        S_LETTER, S_UNDERLINE, S_DIGIT,
        INT_0, INT_NOT_0, INT_TYPE_SUFFIX,
        STRING_START, STRING_END,
        S_SIGN,
        DONE,
        S_NONE
    };
    
    using ReservedWord = struct {
        string str;
        TokenType token;
    };
    
    struct TokenInfo {
        
        const int line;
        const TokenType token;
        const string str;
        const StateType state;
        
        TokenInfo(const int line, const TokenType token, const string str, StateType state) : line(line), token(token), str(str), state(state)  {}
        
    };
    
    FILE *fp = nullptr;
    
    bool ifeof = false;
    
    vector<TokenInfo> tokens;
    
    int line = 1;

    
    const static vector<ReservedWord> reservedWords;
    
    const static map<TokenType, string> tokenTypeStrMap;

    const static map<StateType, string> stateTypeStrMap;
    
    char nextChar() {
        char c;
        if((c = fgetc(fp)) == EOF ) {
            this->ifeof = true;
        }
        return c;
    }
    
    void pushBackChar() {
        fseek(fp, -1, SEEK_CUR);
    }

    TokenInfo nextToken(void) {
        TokenType currentToken = NONE;
        StateType state = START, last_state = S_NONE;
        stringstream ss;
        while (state != DONE) {
            
            last_state = state;
            
            char c = nextChar();
            bool save = true;
            if(ifeof){
                currentToken = T_EOF;
                break;
            }
            switch (state) {
                case START:
                    if(isdigit(c)) {
                        if(c == '0'){
                            state = INT_0;
                        } else {
                            state = INT_NOT_0;
                        }
                    } else if(isalpha(c)) {
                        state = S_LETTER;
                    } else if(c == '\"') {
                        state = STRING_START;
                    } else if ((c == ' ') || (c == '\t') || (c == '\r')) {
                        save = false;
                    }
                    else if (c == '\n') {
                        this->line++;
                        save = false;
                    } else {
                        
                        state = S_SIGN;
                        switch (c) {
                            case '{':
                                currentToken = OPENING_BRACE;
                                break;
                            case '}':
                                currentToken = CLOSING_BRACE;
                                break;
                            case ';':
                                currentToken = SEMICOLON;
                                break;
                            case '[':
                                currentToken = LEFT_BRACKET;
                                break;
                            case ']':
                                currentToken = RIGHT_BRACKET;
                                break;
                            case '*':
                                currentToken = MULT;
                                break;
                            case '+':
                                currentToken = PLUS;
                                break;
                            case '-':
                                currentToken = SUB;
                                break;
                            case '~':
                                currentToken = TILDE;
                                break;
                            case '/':
                                currentToken = SLASH;
                                break;
                            case '%':
                                currentToken = PERCENT;
                                break;
                            case '>':
                                c = nextChar();
                                if(c == '>') {
                                    currentToken = RIGHT_SHIFT;
                                    ss << c;
                                }
                                else {
                                    pushBackChar();
                                    currentToken = ERROR;
                                }
                                break;
                            case '<':
                                c = nextChar();
                                if(c == '<') {
                                    currentToken = LEFT_SHIFT;
                                    ss << c;
                                }
                                else {
                                    pushBackChar();
                                    currentToken = ERROR;
                                }
                                break;
                            case '&':
                                currentToken = AND;
                                break;
                            case '^':
                                currentToken = INSERT;
                                break;
                            case '|':
                                currentToken = DELIMITER;
                                break;
                            case ',':
                                currentToken = COMMA;
                                break;
                            default:
                                currentToken = ERROR;
                                break;
                        }
                    }
                    break;
                
                case S_LETTER:
                    if(c == '_') {
                        state = S_UNDERLINE;
                    } else if(isdigit(c)) {
                        state = S_DIGIT;
                    } else if(isalpha(c)) {
                        state = S_LETTER;
                    } else {
                        currentToken = ID;
                        pushBackChar();
                        state = DONE;
                        save = false;
                    }
                    break;
                    
                case S_DIGIT:
                    if(isalpha(c)) {
                        state = S_LETTER;
                    } if(isdigit(c)) {
                        state = S_DIGIT;
                    } else {
                        currentToken = ID;
                        pushBackChar();
                        state = DONE;
                        save = false;
                    }
                    break;
                
                case S_UNDERLINE:
                    if(isdigit(c)) {
                        state = S_DIGIT;
                    } if(isalpha(c)) {
                        state = S_LETTER;
                    } else {
                        pushBackChar();
                        currentToken = ERROR;
                        save = false;
                    }
                    break;
                
                case INT_0:
                    if(c == 'l' or c == 'L') {
                        state = INT_TYPE_SUFFIX;
                    } else {
                        currentToken = INTEGER;
                        pushBackChar();
                        state = DONE;
                        save = false;
                    }
                    break;
                
                case INT_NOT_0:
                    if(c == 'l' or c == 'L') {
                        state = INT_TYPE_SUFFIX;
                    } else if (isdigit(c)) {
                        state = INT_NOT_0;
                    } else {
                        currentToken = INTEGER;
                        pushBackChar();
                        state = DONE;
                        save = false;
                    }
                    break;
                    
                case INT_TYPE_SUFFIX:
                    state = DONE;
                    currentToken = INTEGER;
                    pushBackChar();
                    save = false;
                    break;
                    
                case STRING_START:
                    if(c == '\\') {
                        char buff_c = c;
                        c = nextChar();
                        if((c == 'b') || (c == 't') || (c == 'n') || (c == 'f') || (c == 'r')
                           || (c == '"') || (c == '\\')) {
                            ss << buff_c;
                            state = STRING_START;
                        } else {
                            pushBackChar();
                            currentToken = ERROR;
                        }
                    } else if (c == '\"') {
                        state = STRING_END;
                    } else {
                        state = STRING_START;
                    }
                    break;
                    
                case STRING_END:
                    state = DONE;
                    currentToken = STRING;
                    pushBackChar();
                    save = false;
                    break;
                    
                case S_SIGN:
                    state = DONE;
                    pushBackChar();
                    save = false;
                    break;
                    
                case DONE:
                    ;
                    
                default:
                    state = DONE;
                    currentToken = ERROR;
                    break;
            }
            
            if(save) {
                ss << c;
            }
            
            if(state == DONE) {
                const string token = ss.str();
                if(currentToken == ID) {
                    currentToken = reservedLookup(token);
                }
            }
            
        }
        
        return TokenInfo(this->line, currentToken, ss.str(), last_state);
    }
    
    TokenType reservedLookup (const string &s) {
        for (auto word : reservedWords)
            if (word.str == s)
                return word.token;
        return ID;
    }


    
};


const vector<Automata::ReservedWord> Automata::reservedWords = {
    {"struct", STRUCT},
    {"boolean", BOOLEAN},
    {"short", SHORT},
    {"long", LONG},
    {"double", DOUBLE},
    {"float", FLOAT},
    {"int8", INT8},
    {"int16", INT16},
    {"int32", INT32},
    {"int64", INT64},
    {"uint8", UINT8},
    {"uint16", UINT16},
    {"uint32", UINT32},
    {"uint64", UINT64},
    {"char", CHAR},
    {"unsigned", UNSIGNED},
    {"TRUE", T_TRUE},
    {"FLASE", T_FALSE}
};

const map<Automata::StateType, string> Automata::stateTypeStrMap = {
    {START, "START"},
    {S_LETTER, "LETTER"},
    {S_UNDERLINE, "UNDERLINE"},
    {S_DIGIT, "DIGIT"},
    {INT_0, "INT_0"},
    {INT_NOT_0, "INT_NOT_0"},
    {INT_TYPE_SUFFIX, "INT_TYPE_SUFFIX"},
    {STRING_START, "STRING_START"},
    {STRING_END, "STRING_END"},
    {DONE, "DONE"},
    {S_SIGN, "SIGN"}
};

const map<Automata::TokenType, string> Automata::tokenTypeStrMap = {
    {STRUCT, "STRUCT"},
    {BOOLEAN, "BOOLEAN"},
    {SHORT, "SHORT"},
    {LONG, "LONG"},
    {FLOAT, "FLOAT"},
    {DOUBLE, "DOUBLE"},
    {INT8, "INT8"},
    {INT16, "INT16"},
    {INT32, "INT32"},
    {INT64, "INT64"},
    {UINT8, "UINT8"},
    {UINT16, "UINT16"},
    {UINT32, "UINT32"},
    {UINT64, "UINT64"},
    {CHAR, "CHAR"},
    {UNSIGNED, "UNSIGNED"},
    {OPENING_BRACE, "OPENING_BRACE"},
    {CLOSING_BRACE, "CLOSING_BRACE"},
    {SEMICOLON, "SEMICOLON"},
    {LEFT_BRACKET, "LEFT_BRACKET"},
    {RIGHT_BRACKET, "RIGHT_BRACKET"},
    {MULT, "MULT"},
    {PLUS, "PLUS"},
    {SUB, "SUB"},
    {TILDE, "TILDE"},
    {SLASH, "SLASH"},
    {PERCENT, "PERCENT"},
    {LEFT_SHIFT, "LEFT_SHIFT"},
    {RIGHT_SHIFT, "RIGHT_SHIFT"},
    {AND, "AND"},
    {INSERT, "INSERT"},
    {DELIMITER, "DELIMITER"},
    {COMMA, "COMMA"},
    {ID, "ID"},
    {LETTER, "LETTER"},
    {DIGIT, "DIGIT"},
    {UNDERLINE, "UNDERLINE"},
    {T_TRUE, "TRUE"},
    {T_FALSE, "FALSE"},
    {INTEGER, "INTEGER"},
    {INTEGER_TYPE_SUFFIX, "INTEGER_TYPE_SUFFIX"},
    {T_BOOLEAN, "BOOLEAN"},
    {STRING, "STRING"},
    {COMMA, "COMMA"},
    {NONE, "NONE"},
    {ERROR, "ERROR"},
    {T_EOF, "EOF"}
};


int main(int argc, const char * argv[]) {
    
    printf("Parsing Program Written By Saturneric\n");
    
    if(argc < 3) {
        printf("Usage: <Input Path> <Output Path>\n");
        return -1;
    }
    
    Automata atm(argv[1]);
    
    atm.parse();
    
    atm.output(argv[2]);
    
    return 0;
}
