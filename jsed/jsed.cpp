#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include "js.cpp"
#include "jsedjs.h"
using namespace std;


class Transformer {

    private:
    const JSInterpreter &interpreter;
    const Function transformationWrapper;
    const Function &transformation;
    const bool rawMode;
    const bool pretty;

    public:
    Transformer(JSInterpreter &interpreter, Function &transformation, bool rawMode, bool pretty) :
        interpreter(interpreter),
        transformationWrapper(interpreter.evaluateScript(jsSource)),
        transformation(transformation),
        rawMode(rawMode),
        pretty(pretty)
    {}

    std::string operator()(const std::string &jsonInput) const {
        return transformationWrapper.invoke(jsonInput, transformation, rawMode, pretty);
    }
};

std::string readStdIn() {
    std::stringstream buffer;
    buffer << cin.rdbuf();
    return buffer.str();
}

std::string readFile(const char* filename){
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}


void usage() {
    printf("Usage: jsed [options] (transformation | -f file) \n");
    printf("   transformation   Transformation function\n");
    printf("   file             File containing transformation function\n");
    printf("Options:\n");
    printf("   -m, --multi-docs Expects input to be multiple documents\n");
    printf("                    formatted on a single line and separated\n");
    printf("                    by new lines\n");
    printf("   -r, --raw        Produces raw string output\n");
    printf("   -p, --pretty     Pretty print json output\n");
    printf("   -h, --help       Display help message\n");
    printf("   -d, --debug      Print debug information\n");
}

class Line {
    std::string data;
    public:
    friend std::istream &operator>>(std::istream &is, Line &line) {
        std::getline(is, line.data);
        return is;
    }
    operator std::string() const {
        return data;
    }
};



int main(int argc, const char *argv[])
{
    std::string script = "";
    bool multiple = false;
    bool debug = false;
    bool raw = false;
    bool pretty = false;

    for (int i=1; i < argc; i++){
        string next(argv[i]);
        if (next == "-m" || next == "--multi-docs") {
            multiple = true;
            continue;
        }
        if (next == "-h" || next == "--help") {
            usage();
            return 0;
        }
        if (next == "-d" || next == "--debug") {
            debug = true;
            continue;
        }
        if (next == "-r" || next == "--raw"){
            raw = true;
            continue;
        }
        if (next == "-p" || next == "--pretty"){
            pretty = true;
            continue;
        }
        if (next == "-f") {
            i++;
            if (i==argc){
                fprintf(stderr, "missing argument value for -f");
                return 1;
            }
            script = readFile(argv[i]);
            continue;
        }
        script = next;
    }

    if (script == "") {
        fprintf(stderr, "Missing script parameter\n");
        usage();
        return 1;
    }

    if (debug) {
        fprintf(stderr, "Script:   '%s'\n", script.c_str());
        fprintf(stderr, "Multiple: '%s'\n", multiple ? "true" : "false" );
        fprintf(stderr, "Raw:      '%s'\n", raw ? "true" : "false" );
        fprintf(stderr, "Pretty:   '%s'\n", pretty ? "true" : "false" );
    }

    JSInterpreter js;
    Function transformation = js.evaluateScript(script);
    Transformer transformer(js, transformation, raw, pretty);

    if (multiple) {
        std::transform(istream_iterator<Line>(cin), istream_iterator<Line>(), ostream_iterator<std::string>(cout, "\n"), transformer);
    } else {
        std::string input = readStdIn();
        std::string result = transformer(input);
        std::cout << result;
    }
    std::cout << "\n";
    return 0;
}
