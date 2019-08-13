#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include "rang.hpp"

#define DEBUG false

using namespace std;
using namespace rang;


char *convert(const string& );
void executeCommand(vector<string>);
void prompt(string*);
void processArgs(vector<string>*, string);


int main() {
    string command;
    vector<string> args;

    while (true) {
        prompt(&command);

        processArgs(&args, command);

        executeCommand(args);

        command.clear();
        args.clear();
    }

    return 0;
}


void prompt(string* command) {
    char ch;
    cout << fg::cyan << style::bold << "ysh" << style::reset << "> ";
    fflush(stdout);

    while ((ch = getchar()) != '\n' ) {
        command->push_back(ch);
    }
}


void processArgs(vector<string>* args, string command) {
    string interim;
    if (DEBUG) cerr << "processing command :: " << command << endl;

    for(auto ch: command) {
        if (ch == ' ' | ch == '\0' | ch == '\n') {
            args->push_back(interim);
            interim.clear();
        }
        else {
            interim.push_back(ch);
        }
    }

    interim.push_back('\0');
    args->push_back(interim);

    for(auto arg: *args){
        if (DEBUG) cerr << "arg :: " << arg << endl;
    }
}


void executeCommand(vector<string> args) {
    pid_t pid, wpid;
    int status;
    bool should_wait = args.at(args.size() - 1) != "&";
    if (!should_wait) {
        args.pop_back();
    }

    vector<char*> cargs;
    transform(args.begin(), args.end(), std::back_inserter(cargs), convert);
    char **argv = &cargs[0];

    // fork() a process
    pid = fork();

    if (pid == 0) {
        // inside child process
        // invoke execvp() from child

        for(auto agv: cargs)
            if (DEBUG) cerr << "argv :: " << agv << endl;

        if (execvp(argv[0], argv) == -1) {
            perror("error in forked process");
        }

        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("error forking");
    }
    else {
        // parent process
        if (should_wait) {
            // wait() unless command has &
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
}


char *convert(const std::string & s)
{
   char *pc = new char[s.size()+1];
   std::strcpy(pc, s.c_str());
   return pc;
}

