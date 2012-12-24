#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "filter.h"
using namespace std;
// test function for our filter function
int main(int argc, const char *argv[]){
    std::vector<std::string> args;

    for (int i=2; i<argc;i++){

        args.push_back(argv[i]);
    }
    try {

        std::stringstream buffer;
        buffer << cin.rdbuf();
        std::string input = buffer.str();

        std::string output = filter(input, argv[1], args);
        printf("%s", output.c_str());
    }
    catch(std::runtime_error& ex) {
        std::cerr << ex.what() ;
        return 1;
    }
    return 0;
}

