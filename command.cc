
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command &currentCommand = Command::_currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    for(size_t i=0; i< tokens.size(); i++)
    {
        Token token = tokens[i];

        switch (token.type)
        {
            case TOKEN_COMMAND:
            case TOKEN_ARGUMENT:
                currentSimpleCommand->insertArgument(strdup(token.value.c_str()));
                break;

            case TOKEN_PIPE: //Add the current command and start a new one
                if(currentSimpleCommand->_numberOfArguments>0){
                    currentCommand.insertSimpleCommand(currentSimpleCommand);
                }
                currentSimpleCommand = new SimpleCommand();
                break;

            case TOKEN_REDIRECT: //">"
                if(i+1<tokens.size())
                {
                    currentCommand._outFile = strdup(tokens[++i].value.c_str());
                    currentCommand._append = 0;
                }
                else{
                    fprintf(stderr, "Syntax error: expected filename after '>' \n");
                }
                break;

            case TOKEN_APPEND: //">>"
                if(i+1<tokens.size())
                {
                currentCommand._outFile = strdup(tokens[++i].value.c_str());
                currentCommand._append = 1;
                }
                else{
                    fprintf(stderr, "Syntax error: expected filename after '>>' \n");
                }
                break;

            case TOKEN_INPUT: //"<"
                if(i+1<tokens.size())
                {
                    currentCommand._inputFile =strdup(tokens[++i].value.c_str());
                }
                else{
                    fprintf(stderr, "Syntax error: expected filename after '<' \n");
                }
                break;
            
            case TOKEN_ERROR: //"2>"
                if(i+1<tokens.size())
                {
                    currentCommand._errFile = strdup(tokens[++i].value.c_str());
                }
                else{
                    fprintf(stderr, "Syntax error: expected filename after '2>' \n");
                }  
                break;

            case TOKEN_REDIRECT_AND_ERROR: //">>&"
                if(i+1<tokens.size())
                {
                    char *filename =strdup(tokens[++i].value.c_str());
                    currentCommand._outFile = filename;
                    currentCommand._errFile = filename;
                    currentCommand._out_error = 1;
                }
                else{
                    fprintf(stderr, "Syntax error: expected filename after '>>&' \n");
                }
                break;

            case TOKEN_BACKGROUND:
                currentCommand._background=1;
                break;

            default:
                break;
        }
    }
    //insert last command
    if(currentSimpleCommand->_numberOfArguments>0)
       {
        currentCommand.insertSimpleCommand(currentSimpleCommand);
       }
    else{
        if(currentCommand._numberOfSimpleCommands == 0)
        {
            free(currentSimpleCommand->_arguments);
            free(currentSimpleCommand);
            currentCommand.clear();
            return;
        }
    }

    //call execute
    currentCommand.execute();
    
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    wordexp_t p;
    if(wordexp(argument, &p, 0) == 0 && p.we_wordc > 0) //wildcard expansion succeeded
    {
        for( size_t i=0; i<p.we_wordc; i++){
            if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }
    _arguments[_numberOfArguments] = strdup(p.we_wordv[i]);
    _numberOfArguments++;
}
    _arguments[_numberOfArguments] = NULL;
    wordfree(&p); //free memory AFTER the loop

    } else{
        if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }
    _arguments[_numberOfArguments] = argument;
    _numberOfArguments++;
    _arguments[_numberOfArguments] = NULL;

    }
    }
   

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _numberOfSimpleCommands = 0;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _append = 0;
    _out_error = 0;
    _background = 0;
    
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }
    free(_simpleCommands);

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();
    // Handle Built-in Commands
    if(_numberOfSimpleCommands == 1)
    {
        SimpleCommand *sc = _simpleCommands[0];
        const char *cmd = sc->_arguments[0];

        //exit
        if(strcmp(cmd, "exit") == 0){
            printf("Good bye!\n");
            exit(0);
        }

        //cd
        if(strcmp(cmd, "cd")==0){
            const char *path;
            if(sc->_numberOfArguments>1){ //cd with arguments
                path = sc->_arguments[1];
            }
            else{   //cd without arguments
                path =getenv("HOME");
            }

            if(chdir(path) != 0){
                perror("cd");
            }

            clear();
            prompt();
            return;
        }
    }

    // set up pipes
    std::vector<pid_t> pids;
    int num=_numberOfSimpleCommands;
    int pipefd[num > 1 ? num - 1 : 1][2];

    if(num>1){

    for(int i=0; i<num-1;i++)
    {
        if(pipe(pipefd[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    }
    
    
    
    // Fork for each SimpleCommand
    for(int i=0; i<num;i++)
    {
        pid_t pid=fork();
        if(pid<0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if(pid==0) //child process
        {
          if(i==0 && _inputFile){ //input redirection for first command or pipe
                    int fd=open(_inputFile, O_RDONLY);
                    if(fd<0){
                        perror("open inputFile");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                else if(i>0){
                    //not first get input from last command or pipe
                    dup2(pipefd[i-1][0], STDIN_FILENO);
                }

                if(i==num-1){
                    if(_outFile){
                        int flags = O_WRONLY | O_CREAT | (_append ? O_APPEND : O_TRUNC);
                        int fd = open(_outFile, flags, 0666);
                        if(fd<0){
                            perror("open outFile");
                            exit(EXIT_FAILURE);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }
                    if(_errFile){
                        int flags = O_WRONLY | O_CREAT | (_append ? O_APPEND : O_TRUNC);
                        int fd = open(_errFile, flags, 0666);
                        if(fd<0){
                            perror("open errFile");
                            exit(EXIT_FAILURE);
                        }
                        if(_out_error){
                            dup2(fd, STDOUT_FILENO);
                            dup2(fd, STDERR_FILENO); //Redirect both stdout and stderr
                        }
                        else{
                           dup2(fd, STDERR_FILENO); 
                        }
                        close(fd);
                    }
                }
                    else{
                        dup2(pipefd[i][1], STDOUT_FILENO); //not last connect output to next pipe
                    }
                    for(int j=0;j<num-1;j++) //close all pipe ends in child
                    {
                        close(pipefd[j][0]);
                        close(pipefd[j][1]);
                    }
                    execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments); //Execute command

                    perror(_simpleCommands[i]->_arguments[0]);
                    exit(EXIT_FAILURE);


                }
                pids.push_back(pid);
           }
            for(int i=0; i<num-1; i++) // parent closes all pipes
            {
                close(pipefd[i][0]);
                close(pipefd[i][1]);
            }

            if(!_background){
                for(pid_t pid : pids) {
                    waitpid(pid,nullptr,0);
                }
            } else{
                printf("[Running in background]\n");
            }
        //Cleanup and prompt again
        clear(); 
        prompt();
        }
    


void Command::prompt()
{
    while (true)
    {
    printf("myshell>");
    fflush(stdout);
    std::string input;
    if(!std::getline(std::cin, input))
        break; //EOF
    
    std::vector<Token> tokens = tokenize(input);
    parse(tokens);
    }
}
    


Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

void sigchld_handler(int sig) //create a log file that contains logs when every child is terminated
{
    FILE *log = fopen("log.txt", "a"); //append mode
    if(log)
    {
        time_t now = time(NULL);
        fprintf(log, "Child process terminated at %s", ctime(&now));
        fclose(log);
    }
}

int main()
{
    signal(SIGINT, SIG_IGN); //ignore ctrl+c so the shell doesn"t quit
    signal(SIGCHLD, sigchld_handler); //handle child process termination (logging)
    Command::_currentCommand.prompt();
    return 0;
}
