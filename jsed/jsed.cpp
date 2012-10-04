#include <iostream>
#include <iterator>

#include "js.cpp"
#include "jsedjs.h"
using namespace std;


class Transformer {

    private:
    JSInterpreter &interpreter;
    Function transformationWrapper;
    Function &transformation;

    public:
    Transformer(JSInterpreter &interpreter, Function &transformation) :
        interpreter(interpreter),
        transformationWrapper(interpreter.evaluateScript(jsSource)),
        transformation(transformation)
    {}

    std::string operator()(const std::string &jsonInput) {
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
    printf("Usage: jsed <script> | -f <scriptFile>\n");
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
    const char *script;
    bool multiple;

    if (argc == 2) {
        script=argv[1];
    } else if (argc == 3 && argv[1] == string("-m")) {
        multiple = true;
        script=argv[2];
    } else {
        usage();
        return -1;
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
    return 0;
}

