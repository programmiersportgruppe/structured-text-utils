#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>
#include "fdstream.hpp"
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

/*
 *
 * Filters a string through an external process
 *
 * TODO
 *  - Return exit code in exception
 *  - Add timeout functionality
 *       this seems to be nasty according to:
 *       http://stackoverflow.com/questions/282176/waitpid-equivalent-with-timeout
 */

std::string filter(std::string input, std::string command, std::vector<std::string> args) {//char* command, char** args, istream input, ostream output){
    int Input[2],
        Output[2],
        ErrorOutput[2];


    pipe( Input );
    pipe( Output );
    pipe( ErrorOutput );

    int pid = fork();
    if(pid)
    {
        // We're in the parent here.
        // Close the reading end of the input pipe.
        close(Input[0]);
        // Close the writing end of the output pipe
        close(Output[1]);
        close(ErrorOutput[1]);

    }
    else
    {    // We're in the child here.
        close(Input[1]);
        dup2(Input[0], STDIN_FILENO );
        close(Output[0] );
        dup2(Output[1], STDOUT_FILENO);
        close(ErrorOutput[0]);
        dup2(ErrorOutput[1], STDERR_FILENO);

        // The types and casts in the following paragraph
        // constitute a severe loss of control.
        char *c_args[args.size() + 2];
        c_args[0] = (char*)command.c_str();

        for (int i=0; i < args.size() ; i++)
            c_args[i + 1] = (char*) args[i].c_str();

        c_args[args.size() + 1] = NULL;

        int retVal = execvp(command.c_str(), c_args);
        exit (0); //this not needed, after calling execve we won't return, will we?
    }

    boost::fdistream out(Output[0]);
    boost::fdistream err(ErrorOutput[0]);
    boost::fdostream in(Input[1]);

    in << input;
    close(Input[1]);

    std::stringstream buffer;
    buffer << out.rdbuf();
    std::string output = buffer.str();

    std::stringstream errBuffer;
    errBuffer << err.rdbuf();

    int stat_loc;
    waitpid(pid, &stat_loc, 0);
    int exitCode = stat_loc >> 8;
    if (exitCode != 0)
        throw * new runtime_error(std::string("Error executing '") + command + "'\n" + errBuffer.str().c_str());

    return output;
}

