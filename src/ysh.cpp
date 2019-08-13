#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include "rang.hpp"

#define DEBUG false

using namespace std;
using namespace rang;

typedef int (*builtin_func)(vector<string>);


char *convert(const string& );
int executeCommand(vector<string>);
int launchCommand(vector<string>);
void prompt(string*);
void processArgs(vector<string>*, string);

// ysh built-ins
map<string, builtin_func> builtins;
int ysh_cd(vector<string>);
int ysh_pwd(vector<string>);
int ysh_exit(vector<string>);
int ysh_help(vector<string>);


int main() {
    string command;
    vector<string> args;
    int status = true;

    builtins = {
        {"cd", &ysh_cd},
        {"pwd", &ysh_pwd},
        {"help", &ysh_help},
        {"exit", &ysh_exit},
    };

    while (status) {
        prompt(&command);

        processArgs(&args, command);

        status = executeCommand(args);

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
            interim.push_back('\0');
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


int executeCommand(vector<string> args) {
    if (args[0][0] == '\0') {
        // empty input
        return 1;
    }

    for(auto b: builtins) {
        if (strcmp(b.first.c_str(), args[0].c_str()) == 0) {
            return (*b.second)(args);
        }
    }

    return launchCommand(args);
}


int launchCommand(vector<string> args) {
    pid_t pid, wpid;
    int status;

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
        // wait() unless command has &
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


char *convert(const std::string & s)
{
   char *pc = new char[s.size()+1];
   std::strcpy(pc, s.c_str());
   return pc;
}


int ysh_cd(vector<string> args) {
    if (args[1] == "\0") {
        if (chdir("~") != 0) {
            perror("error changing directory to ~");
        }
    }
    else {
        if (chdir(convert(args[1])) != 0) {
            perror("error changing directory");
        }
    }

    return 1;
}


int ysh_pwd(vector<string> args) {
    string pwd = getcwd(NULL, 0);
    cout << pwd << endl;

    return 1;
}


int ysh_exit(vector<string> args) {
    if (DEBUG) cerr << "exit called\n";
    return 0;
}


int ysh_help(vector<string> args) {
    cout << fg::yellow << "ysh" << style::reset << ": yet another shell\n";
    cout << fg::yellow << "author" << style::reset << ": @say4n\n";
    cout << fg::yellow <<"builtins" << style::reset << ": ";

    for (auto b: builtins) {
        cout << b.first << ", ";
    }
    cout << "\b\b.\n";

    return 1;
}

