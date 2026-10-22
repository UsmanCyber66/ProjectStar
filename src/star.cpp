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
#include "starfuncs.cpp"
using namespace std;
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
