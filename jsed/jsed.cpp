#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <exception>
#include <stdexcept>
#include "js/js.h"
#include "jsedjs.h"
using namespace std;

class Transformer {

    private:
    JSInterpreter &interpreter;
    const js::Function transformationWrapper;
    const js::Function transformation;
    const bool rawMode;
    const bool pretty;

    public:
    Transformer(JSInterpreter &interpreter, js::Function transformation, bool rawMode, bool pretty) :
        interpreter(interpreter),
        transformationWrapper(interpreter.evaluateScript(jsSource)),
        transformation(transformation),
        rawMode(rawMode),
        pretty(pretty)
    {}


    std::string operator()(const std::string &jsonInput) const {
        // this is a bit awkward, but once we have switched to C++11 we can instead write:
        // -std=c++11
        // return interpreter.invoke(transformationWrapper, {jsonInput, (js::ValueRef)transformation, rawMode, pretty});

        std::vector<js::ValueRef> args;
        args.push_back(jsonInput);
        args.push_back(transformation);
        args.push_back(rawMode);
        args.push_back(pretty);
        return interpreter.invoke(transformationWrapper, args);
    }
};

std::string readStdIn() {
    std::stringstream buffer;
    buffer << cin.rdbuf();
    return buffer.str();
}

std::string readFile(const char* filename) throw (runtime_error) {
    std::ifstream t(filename);
    if (!t) {
        throw * new runtime_error(std::string("Could not open input file '") + filename  + "'");
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

void writeFile(std::string filename, std::string content) {
    std::ofstream outputStream(filename.c_str());
    outputStream << content;
}

class OptionParser {
    public:

    std::string script;
    bool showHelp;
    bool multiple;
    bool debug;
    bool raw;
    bool pretty;
    std::string inPlaceFile;

    OptionParser(int argc, const char *argv[]) throw (runtime_error) {
        for (int i=1; i < argc; i++){
            string next(argv[i]);
            if (next == "-m" || next == "--multi-docs") {
                multiple = true;
                continue;
            }
            if (next == "-h" || next == "--help") {
                usage();
                showHelp = true;
                return;
            }
            if (next == "-d" || next == "--debug") {
                debug = true;
                continue;
            }
            if (next == "-r" || next == "--raw") {
                raw = true;
                continue;
            }
            if (next == "-p" || next == "--pretty") {
                pretty = true;
                continue;
            }
            if (next == "-f") {
                i++;
                if (i==argc) {
                    throw * new runtime_error("Missing argument value for -f");
                }
                script = readFile(argv[i]);
                continue;
            }
            if (next == "-i" || next == "--in-place") {
                i++;
                if (i==argc) {
                    throw * new runtime_error("Missing filename argument for in place editing");
                }
                inPlaceFile = argv[i];
                continue;
            }
            script = next;
        }

        if (debug) {
            fprintf(stderr, "Script:   '%s'\n", script.c_str());
            fprintf(stderr, "Multiple: '%s'\n", multiple ? "true" : "false" );
            fprintf(stderr, "Raw:      '%s'\n", raw ? "true" : "false" );
            fprintf(stderr, "Inplace:  '%s'\n", inPlaceFile.c_str());
            fprintf(stderr, "Pretty:   '%s'\n", pretty ? "true" : "false" );
        }

        if (script == "" && !showHelp) {
            throw  * new runtime_error("Missing transformation function");
        }

        if (multiple && inPlaceFile != "") {
            throw  * new runtime_error("Multi document mode and in place editing of files are mutually exclusive");
        }
    }

    void usage() {
        printf("Usage: jsed [options] [--in-place filename] (transformation | -f file) \n");
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
};

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

class FooCallback: public CFunc {
    virtual js::ValueRef operator() (std::vector<js::ValueRef> args) const {
        std::string rv("Lovely foo-return value");
        return rv;
    }
};

class BarCallback: public CFunc {
    virtual js::ValueRef operator() (std::vector<js::ValueRef> args) const {
        std::string rv("Lovely bar-return value");
        return rv;
    }
};

int main(int argc, const char *argv[])
{
    try {
        OptionParser parser(argc, argv);

        if (parser.showHelp)
            return 0;

        JSInterpreter js;

        js.registerNativeFunction("foo", new FooCallback());
        js.registerNativeFunction("bar", new BarCallback());
        js::Function transformation = js.evaluateScript(parser.script);
        Transformer transformer(js, transformation, parser.raw, parser.pretty);

        if (parser.multiple) {
            std::transform(istream_iterator<Line>(cin), istream_iterator<Line>(), ostream_iterator<std::string>(cout, "\n"), transformer);
        } else {
            if (parser.inPlaceFile != "") {
                std::string input = readFile(parser.inPlaceFile.c_str());
                std::string result = transformer(input);
                writeFile(parser.inPlaceFile.c_str(), result);
            } else {
                std::string input = readStdIn();
                std::string result = transformer(input);
                std::cout << result;
                std::cout << "\n";
            }
        }
    }
    catch(runtime_error& ex) {
        cerr << ex.what() << endl;
        cerr << "See 'jsed --help' for more information." << endl;
        return 1;
    }
}
