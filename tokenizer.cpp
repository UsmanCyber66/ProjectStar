#pragma once
#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <sstream>

using namespace std;
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


size_t token_index = 0;
bool has_error = false;
string emitted_code = "";
unordered_map<string, double> memory;




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

        // symbols
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
