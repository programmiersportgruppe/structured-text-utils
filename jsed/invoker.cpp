#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>

#include "fdstream.hpp"

using namespace std;

/*
 *
 * Filters a string through an external process
 *
 * TODO
 *  - Deal with non-zero exit code
 *  - Deal with standard-error
 *  - Add timeout functionality (nasty)
 */

std::string filter(std::string input, std::string command, std::vector<std::string> args) {//char* command, char** args, istream input, ostream output){
    int Input[2], Output[2];

    pipe( Input );
    pipe( Output );

    if( fork() )
    {
        // We're in the parent here.
        // Close the reading end of the input pipe.
        close( Input[ 0 ] );
        // Close the writing end of the output pipe
        close( Output[ 1 ] );
    }
    else
    {    // We're in the child here.
        close( Input[ 1 ] );
        dup2( Input[ 0 ], STDIN_FILENO );
        close( Output[ 0 ] );
        dup2( Output[ 1 ], STDOUT_FILENO );

        // The types and casts in the following paragraph
        // constitute a severe loss of control.
        char *c_args[args.size() + 2];
        c_args[0] = (char*)command.c_str();

        for (int i=0; i < args.size() ; i++)
            c_args[i + 1] = (char*) args[i].c_str();

        c_args[args.size() + 1] = NULL;

        execvp("grep", c_args);
        exit (0);
    }

    boost::fdistream out(Output[0]);
    boost::fdostream in(Input[1]);

    in << input ;
    close(Input[1]);

    std::stringstream buffer;
    buffer << out.rdbuf();
    return buffer.str();
}

int main(int argc, const char *argv[]){
    std::vector<std::string> args;
    args.push_back("hello");

    std::string output = filter("hello world\nwhat is going on\nhello hello?", "grep", args);
    printf("output:\n%s", output.c_str());
    return 0;
}
