# Eggshell

Eggshell is a minimal Unix-style shell written in C as part of an operating systems coursework project.  
The goal of the project is to demonstrate how a basic command-line shell works internally, rather than to replicate the full functionality of existing shells such as Bash.

The shell provides an interactive prompt, executes external programs using Unix system calls, and supports simple command piping.

## Features

- Interactive prompt displaying the current working directory
- Execution of external commands using fork() and execv()
- Built-in commands:
    - cd (with support for \$HOME)
    - exit
- Support for a single pipe between two commands (command1 | command2)
- Proper process synchronisation using waitpid()
- Basic error handling for invalid commands and pipe syntax

## Design Decisions

This implementation was intentionally kept small and focused to match the project specification.

- Commands are executed from /usr/bin only
- No PATH searching is performed
- Quoting, redirection, job control, and background execution are not implemented
- Only one pipe is supported at a time

These constraints keep the emphasis on core operating-system concepts such as process creation, inter-process communication, and file descriptor management.

## How It Works

1. The shell prints a prompt containing the current working directory
2. User input is read and tokenised into command arguments
3. Built-in commands (cd and exit) are handled directly by the shell
4. For external commands:
    - A child process is created using fork()
    - The command is executed using execv()
    - The parent process waits for completion using waitpid()
5. For piped commands:
    - A pipe is created using pipe()
    - Two child processes are forked
    - Standard input and output are redirected using dup2()

## Building and Running

Compile the shell using gcc:

```bash
gcc -o eggshell eggshell.c
