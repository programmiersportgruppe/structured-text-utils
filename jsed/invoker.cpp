#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "fdstream.hpp"

using namespace std;

/*
 *
 * Filters a string through an external process
 *
 * TODO
 *  - Add timeout functionality (nasty)
 *  - Pass in command
 *  - Pass in arguments (std::list?)
 *  - Deal with non-zero exit code
 *  - Deal with standard-error
 */

std::string filter(std::string input) {//char* command, char** args, istream input, ostream output){
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

         execlp( "grep", "grep", "hello", NULL );
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
    std::string  output = filter("hello world\nwhat is going on\nhello hello?");
    printf("output:\n%s", output.c_str());
    return 0;
}
