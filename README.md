# MyShell - Mini UNIX Shell
===========================

## Description
This project implements a simple UNIX-like shell in C++.
The shell supports executing commands, pipes, I/O redirection, wildcard expansion, background execution, and logging child process termination.

## Files
- command.cc      → Main logic for parsing and executing commands
- command.h       → Header file for Command and SimpleCommand classes
- tokenizer.cc    → Responsible for breaking input into tokens
- tokenizer.h     → Header file for tokenizer functions
- Makefile        → Builds the myshell program
- README.txt      → Project documentation

## Compilation
To build the project, simply run:
    make

This will generate the executable:
    myshell

## Usage
Run the shell:
    ./myshell

You can then type UNIX-like commands such as:
    ls
    ls -l | grep ".cc"
    cat < input.txt > output.txt
    sort < unsorted.txt >> sorted.txt
    sleep 5 &    (run in background)
    cd /path/to/directory
    exit

The shell supports:
- Input redirection: `<`
- Output redirection: `>` and `>>`
- Error redirection: `2>`
- Combined output & error redirection: `>>&`
- Pipes: `|`
- Background processes: `&`
- Wildcard expansion (e.g., `*.txt`)

## Logging
Whenever a child process terminates, a log entry is written to `log.txt`
containing the termination timestamp.

## Cleanup
To remove all generated files:
    make clean
