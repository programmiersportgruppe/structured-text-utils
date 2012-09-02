#include <iostream>

#include "js.cpp"

using namespace std;


const char *readStdIn() {
  string result("");
  string lf("\n");
  while(cin) {
    string input_line("");
    getline(cin, input_line);
    result+= input_line;
    result+=lf;
  };

  char *rv = (char*)malloc(strlen(result.c_str()));
  strcpy(rv, result.c_str());
  return rv;
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

    const char *input = readStdIn();
    jsval rval=js.evaluateScript(script);
    js.setProperty("transformation", &rval);
    js.setProperty("input", input);
    rval = js.executeScriptFile("jsed.js");
    printf("%s\n", js.jsvalToString(rval));
   return 0;
}

