#include <iostream>
#include <iterator>
#include <algorithm>
#include "js.cpp"
#include "jsedjs.h"
using namespace std;


class Transformer {

    private:
    const JSInterpreter &interpreter;
    const Function transformationWrapper;
    const Function &transformation;

    public:
    Transformer(JSInterpreter &interpreter, Function &transformation) :
        interpreter(interpreter),
        transformationWrapper(interpreter.evaluateScript(jsSource)),
        transformation(transformation)
    {}

    std::string operator()(const std::string &jsonInput) const {
        return transformationWrapper.invoke(jsonInput, transformation);
    }
};

/* Yes, I am suitably embarressed, needs fixing */
std::string readStdIn() {
  string result("");
  string lf("\n");
  while(cin) {
    string input_line("");
    getline(cin, input_line);
    result+= input_line;
    result+=lf;
  };

  return result;
}

void usage() {
    printf("Usage: jsed [options] <script>\n");
    printf("Options:\n");
    printf("   -m, --multi-docs Expects input to be multiple documents\n");
    printf("                    formatted on a single line and separated\n");
    printf("                    by new lines\n");
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
    const char *script = NULL;
    bool multiple;
    bool debug;

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

        script = argv[i];
    }

    if (script == NULL) {
        fprintf(stderr, "Missing script parameter\n");
        usage();
        return 1;
    }

    if (debug) {
        fprintf(stderr, "Script:   '%s'\n", script);
        fprintf(stderr, "Multiple: '%s'\n", multiple ? "true" : "false" );
    }

    JSInterpreter js;
    Function transformation = js.evaluateScript(script);
    Transformer transformer(js, transformation);

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
