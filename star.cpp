#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <cctype>
#include <chrono>
using namespace std;

// ==========================================
// Token Types & Structure
// ==========================================
enum class TokenType {
    INT,
    FLOAT,
    STRING,
    BOOLEAN,
    NUMBER_LITERAL,
    STRING_LITERAL,
    BOOL_LITERAL,
    IDENTIFIER,
    ASSIGN,
    PLUS,
    MINUS,
    MUL,
    DIV,
    MOD,
    POWER,
    LPAREN,
    RPAREN,
    PRINT,
    STARIMPORT,
    CPPIMPORT,
    EOF_TOKEN,
    UNKNOWN
};

struct Token {
    TokenType type;
    string value;
    int line;
};

// ==========================================
// Global Variables & State
// ==========================================
size_t token_index = 0;
bool has_error = false;
string emitted_code = "";
unordered_map<string, double> memory;

// ==========================================
// Utility Functions
// ==========================================
bool file_exists(const string& filename) {
    ifstream f(filename.c_str());
    return f.good();
}

string cat(const string& filename) {
    ifstream f(filename);
    if (!f.is_open()) return "";
    stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

// ==========================================
// Lexer / Tokenizer
// ==========================================
vector<Token> tokenize(const string& source) {
    vector<Token> tokens;
    size_t i = 0;
    int line = 1;

    while (i < source.length()) {
        char current = source[i];

        if (current == '\n') {
            line++;
            i++;
            continue;
        }

        if (isspace(current)) {
            i++;
            continue;
        }

        // Single-line comment: //
        if (current == '/' && i + 1 < source.length() && source[i + 1] == '/') {
            while (i < source.length() && source[i] != '\n') {
                i++;
            }
            continue;
        }

        // Numbers
        if (isdigit(current)) {
            string num_str = "";
            bool is_float = false;
            while (i < source.length() && (isdigit(source[i]) || source[i] == '.')) {
                if (source[i] == '.') is_float = true;
                num_str += source[i];
                i++;
            }
            tokens.push_back({TokenType::NUMBER_LITERAL, num_str, line});
            continue;
        }

        // Strings
        if (current == '"') {
            i++; // skip open quote
            string str_val = "";
            while (i < source.length() && source[i] != '"') {
                if (source[i] == '\\' && i + 1 < source.length()) {
                    i++;
                }
                str_val += source[i];
                i++;
            }
            if (i < source.length()) i++; // skip close quote
            tokens.push_back({TokenType::STRING_LITERAL, str_val, line});
            continue;
        }

        // Identifiers and Keywords
        if (isalpha(current) || current == '_') {
            string ident = "";
            while (i < source.length() && (isalnum(source[i]) || source[i] == '_')) {
                ident += source[i];
                i++;
            }

            if (ident == "int") tokens.push_back({TokenType::INT, ident, line});
            else if (ident == "float") tokens.push_back({TokenType::FLOAT, ident, line});
            else if (ident == "string") tokens.push_back({TokenType::STRING, ident, line});
            else if (ident == "bool") tokens.push_back({TokenType::BOOLEAN, ident, line});
            else if (ident == "true" || ident == "false") tokens.push_back({TokenType::BOOL_LITERAL, ident, line});
            else if (ident == "print") tokens.push_back({TokenType::PRINT, ident, line});
            else if (ident == "starimport") tokens.push_back({TokenType::STARIMPORT, ident, line});
            else if (ident == "cppimport") tokens.push_back({TokenType::CPPIMPORT, ident, line});
            else tokens.push_back({TokenType::IDENTIFIER, ident, line});

            continue;
        }

        // Operators & Symbols
        switch (current) {
            case '=': tokens.push_back({TokenType::ASSIGN, "=", line}); break;
            case '+': tokens.push_back({TokenType::PLUS, "+", line}); break;
            case '-': tokens.push_back({TokenType::MINUS, "-", line}); break;
            case '*': tokens.push_back({TokenType::MUL, "*", line}); break;
            case '/': tokens.push_back({TokenType::DIV, "/", line}); break;
            case '%': tokens.push_back({TokenType::MOD, "%", line}); break;
            case '^': tokens.push_back({TokenType::POWER, "^", line}); break;
            case '(': tokens.push_back({TokenType::LPAREN, "(", line}); break;
            case ')': tokens.push_back({TokenType::RPAREN, ")", line}); break;
            default:  tokens.push_back({TokenType::UNKNOWN, string(1, current), line}); break;
        }
        i++;
    }

    tokens.push_back({TokenType::EOF_TOKEN, "", line});
    return tokens;
}

// ==========================================
// Preprocessor / Star Imports
// ==========================================
void starimports(const string& filepath, vector<string>& source_lines, unordered_set<string>& visited) {
    if (visited.count(filepath)) return;
    visited.insert(filepath);

    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filepath << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        string trimmed = line;
        size_t first = trimmed.find_first_not_of(" \t");
        if (first != string::npos) trimmed = trimmed.substr(first);

        if (trimmed.rfind("starimport", 0) == 0) {
            size_t start_q = trimmed.find('"');
            size_t end_q = trimmed.rfind('"');
            if (start_q != string::npos && end_q != string::npos && end_q > start_q) {
                string imported_path = trimmed.substr(start_q + 1, end_q - start_q - 1);
                starimports(imported_path, source_lines, visited);
            }
        } else {
            source_lines.push_back(line);
        }
    }
}

// ==========================================
// Expression Parsing (AST / Tier Evaluation)
// ==========================================
long double tier1_expr(const vector<Token>& tokens); // Forward declaration

long double primary_expr(const vector<Token>& tokens) {
    if (token_index >= tokens.size()) {
        throw runtime_error("Syntax Error: Unexpected end of tokens");
    }

    Token cur = tokens[token_index];

    if (cur.type == TokenType::NUMBER_LITERAL) {
        token_index++;
        return stold(cur.value);
    } else if (cur.type == TokenType::IDENTIFIER) {
        token_index++;
        if (memory.find(cur.value) != memory.end()) {
            return memory[cur.value];
        }
        return 0.0;
    } else if (cur.type == TokenType::LPAREN) {
        token_index++; // Consume '('
        long double val = tier1_expr(tokens);
        if (token_index < tokens.size() && tokens[token_index].type == TokenType::RPAREN) {
            token_index++; // Consume ')'
        } else {
            throw runtime_error("Syntax Error: Expected closing parenthesis");
        }
        return val;
    }

    throw runtime_error("Syntax Error: Invalid primary expression");
}

long double tier1_expr(const vector<Token>& tokens) {
    long double left = primary_expr(tokens);

    while (token_index < tokens.size()) {
        TokenType op = tokens[token_index].type;
        if (op == TokenType::PLUS || op == TokenType::MINUS || op == TokenType::MUL || op == TokenType::DIV) {
            token_index++;
            long double right = primary_expr(tokens);
            if (op == TokenType::PLUS) left += right;
            else if (op == TokenType::MINUS) left -= right;
            else if (op == TokenType::MUL) left *= right;
            else if (op == TokenType::DIV) {
                if (right == 0) throw runtime_error("Runtime Error: Division by zero");
                left /= right;
            }
        } else {
            break;
        }
    }
    return left;
}

// ==========================================
// Statement Parsers
// ==========================================
void VarDecl(const vector<Token>& tokens) {
    if (token_index >= tokens.size()) return;

    string var_name = tokens[token_index].value;
    token_index++; // Consume variable identifier

    if (token_index < tokens.size() && tokens[token_index].type == TokenType::ASSIGN) {
        token_index++; // Consume '='

        if (token_index < tokens.size() && tokens[token_index].type == TokenType::STRING_LITERAL) {
            string str_val = tokens[token_index].value;
            token_index++;
            emitted_code += "    std::string " + var_name + " = \"" + str_val + "\";\n";
        } else {
            long double val = tier1_expr(tokens);
            memory[var_name] = val;
            emitted_code += "    double " + var_name + " = " + to_string(val) + ";\n";
        }
    }
}
void cppimport(const vector<Token>& tokens) {
    token_index++; // Consume 'cppimport' keyword

    if (token_index >= tokens.size() || tokens[token_index].type != TokenType::STRING_LITERAL) {
        throw runtime_error("Syntax Error: Expected File Name after cppimport!");
    }

    string file_path = tokens[token_index].value;

    if (file_exists(file_path)) {
        emitted_code += cat(file_path) + "\n";
        token_index++;
    } else {
        throw runtime_error("Error: File " + tokens[token_index].value + " was not found in this directory!");
    }
}

void PrintStatement(const vector<Token>& tokens) {
    token_index++; // Consume 'print'

    if (token_index >= tokens.size() || tokens[token_index].type != TokenType::LPAREN) {
        throw runtime_error("Syntax Error: Expected '(' after 'print'");
    }
    token_index++; // Consume '('

    string print_arg = "";
    if (token_index < tokens.size() && tokens[token_index].type == TokenType::STRING_LITERAL) {
        print_arg = "\"" + tokens[token_index].value + "\"";
        token_index++;
    } 
    else if (token_index < tokens.size() && tokens[token_index].type == TokenType::IDENTIFIER) {
        print_arg = tokens[token_index].value;
        token_index++;
    } 
    else {
        long double val = tier1_expr(tokens);
        print_arg = to_string(val);
    }

    if (token_index >= tokens.size() || tokens[token_index].type != TokenType::RPAREN) {
        throw runtime_error("Syntax Error: Expected ')' in print statement");
    }
    token_index++; // Consume ')'

    if (!has_error) {
        emitted_code += "    std::cout << " + print_arg + " << std::endl;\n";
    }
}

// ==========================================
// Main Entry Point
// ==========================================
int main(int argc, char* argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    if (argc < 2) {
        cerr << "Usage: star <filename.star>\n";
        return 1;
    }

    try {
        vector<string> source_lines;
        unordered_set<string> visited;
        starimports(argv[1], source_lines, visited);

        string combined_source = "";
        for (const auto& line : source_lines) {
            combined_source += line + "\n";
        }

        vector<Token> tokens = tokenize(combined_source);

        string header_code = "#include <iostream>\n";
        string body_code = "";

        while (token_index < tokens.size() && tokens[token_index].type != TokenType::EOF_TOKEN) {
            if (tokens[token_index].type == TokenType::PRINT) {
                PrintStatement(tokens);
                body_code += emitted_code;
                emitted_code = "";
            } else if (tokens[token_index].type == TokenType::CPPIMPORT) {
                cppimport(tokens);
                header_code += emitted_code + "\n";
                emitted_code = "";
            } else if (tokens[token_index].type == TokenType::IDENTIFIER) {
                VarDecl(tokens);
                body_code += emitted_code;
                emitted_code = "";
            } else {
                token_index++;
            }
        }

        // Assembly of valid C++ source structure
        emitted_code = header_code + "int main() {\n" + body_code + "    return 0;\n}\n";

        cout << emitted_code;
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Star completed transpiling in " << duration.count() << " Seconds!\n";

    std::ofstream out_file("staroutput.cpp");
    out_file << emitted_code;
    out_file.close();

    // Compile and run safely
    int compile_res = system("g++ staroutput.cpp -o staroutput");
    if (compile_res == 0) {
        system("./staroutput");
        system("rm staroutput");
    } else {
        cerr << "Compilation of generated C++ code failed!" << endl;
    }

    return 0;
}
