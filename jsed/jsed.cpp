#include <iostream>

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

void usage(){
    printf("Usage: jsed <script> | -f <scriptFile>\n");
}

int main(int argc, const char *argv[])
{
    const char *script;
    if (argc == 1) {
        usage();
        return -1;
    }
    if  (argc == 2) {
        script=argv[1];
    }

    JSInterpreter js;
    Function transformation = js.evaluateScript(script);
    Transformer transformer(js, transformation);
    std::string input = readStdIn();
    std::string result = transformer(input);
    std::cout << result;
    return 0;
}

