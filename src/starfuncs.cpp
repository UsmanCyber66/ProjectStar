#pragma once
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
#include "tokenizer.cpp"
using namespace std;



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
            str_memory[var_name] = str_val;
            memory.erase(var_name);
        } else {
            long double val = tier1_expr(tokens);
            string literal = to_string(val);
            memory[var_name] = stod(literal);
            str_memory.erase(var_name);
            emitted_code += "    double " + var_name + " = " + literal + ";\n";
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
    ostringstream shown; // formatted the way std::cout would print it
    if (token_index < tokens.size() && tokens[token_index].type == TokenType::STRING_LITERAL) {
        print_arg = "\"" + tokens[token_index].value + "\"";
        shown << tokens[token_index].value;
        token_index++;
    } 
    else if (token_index < tokens.size() && tokens[token_index].type == TokenType::IDENTIFIER) {
        print_arg = tokens[token_index].value;
        if (str_memory.count(print_arg)) shown << str_memory[print_arg];
        else if (memory.count(print_arg)) shown << memory[print_arg];
        else throw runtime_error("Error: Variable '" + print_arg + "' is not defined (line " + to_string(tokens[token_index].line) + ")");
        token_index++;
    } 
    else {
        long double val = tier1_expr(tokens);
        print_arg = to_string(val);
        shown << stod(print_arg);
    }

    if (token_index >= tokens.size() || tokens[token_index].type != TokenType::RPAREN) {
        throw runtime_error("Syntax Error: Expected ')' in print statement");
    }
    token_index++; // Consume ')'

    if (!has_error) {
        emitted_code += "    std::cout << " + print_arg + " << std::endl;\n";
        program_output += shown.str() + "\n";
    }
}

// ==========================================
// Transpiler: Star source (imports already resolved) -> C++ source
// ==========================================
string transpile_source(const string& combined_source) {
    token_index = 0;
    has_error = false;
    emitted_code = "";
    program_output = "";
    memory.clear();
    str_memory.clear();

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
    return header_code + "int main() {\n" + body_code + "    return 0;\n}\n";
}
