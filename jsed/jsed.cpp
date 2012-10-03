#include <iostream>

#include "js.cpp"
#include "jsedjs.h"
using namespace std;


class Transformer {

    private:
    JSInterpreter interpreter;

    public:

    Transformer(JSInterpreter interpreter) : interpreter(interpreter){}


    std::string transform(std::string jsonInput){
        return std::string("");
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

    std::string input = readStdIn();
    Function transformation=js.evaluateScript(script);
    Function transformationWrapper = js.evaluateScript(jsSource);
    std::string result = transformationWrapper.invoke(input, transformation);
    std::cout << result;
    return 0;
}

