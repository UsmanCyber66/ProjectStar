// WebAssembly entry point for the browser playground (web/).
// Exposes transpile(source) -> { cpp, output, error } to JavaScript.
#include <emscripten/bind.h>
#include "starfuncs.cpp"
using namespace std;

emscripten::val transpile(const string& source) {
    emscripten::val result = emscripten::val::object();
    try {
        // There is no file system in the browser, so imports can't be resolved
        istringstream lines(source);
        string line;
        int line_no = 0;
        while (getline(lines, line)) {
            line_no++;
            size_t first = line.find_first_not_of(" \t");
            if (first == string::npos) continue;
            if (line.compare(first, 10, "starimport") == 0 || line.compare(first, 9, "cppimport") == 0) {
                throw runtime_error("Error: imports aren't supported in the web playground (line " + to_string(line_no) + ")");
            }
        }

        result.set("cpp", transpile_source(source));
        result.set("output", program_output);
        result.set("error", string(""));
    } catch (const exception& e) {
        result.set("cpp", string(""));
        result.set("output", program_output);
        result.set("error", string(e.what()));
    }
    return result;
}

EMSCRIPTEN_BINDINGS(star) {
    emscripten::function("transpile", &transpile);
}
