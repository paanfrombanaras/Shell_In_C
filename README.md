# lsh — a simple Unix shell in C

lsh is a small educational Unix shell implemented in C. It implements a minimal command loop that reads a command line, parses it into arguments, and launches programs using fork/exec. It also provides a few built-in commands.

## Features
- Read/execute simple commands with arguments (e.g. ls -la /tmp)
- Built-in commands:
    - `cd <dir>` — change working directory
    - `help` — show short help text
    - `exit` — exit the shell
- Simple loop with prompt
- Uses fork() and execvp() to run external programs
- Basic error reporting

## Requirements
- POSIX-compatible system (Linux, macOS)
- C compiler (gcc/clang)
- Make (optional)

## Build
Typical Makefile target:

- Build with gcc:
    gcc -Wall -Wextra -std=c99 -o lsh lsh.c

Or if a Makefile is included:
- make

## Usage
Run the shell:
- ./lsh

Basic session:
- lsh> ls -la
- lsh> cd /tmp
- lsh> ./my_program arg1 arg2
- lsh> help
- lsh> exit

The shell prints a prompt, reads a line, tokenizes by whitespace, and executes the command.

## Design (functions commonly found in lsh.c)
- `int main(int argc, char **argv)`  
    Initializes the shell and enters the main loop.
- `void lsh_loop(void)`  
    Main read–eval–print loop: print prompt, read line, parse, execute.
- `char *lsh_read_line(void)`  
    Read a full line of input (typically using getline or a custom routine).
- `char **lsh_split_line(char *line)`  
    Split the input line into tokens (arguments). Returns a NULL-terminated array of strings.
- `int lsh_launch(char **args)`  
    Fork and execvp the command specified in args. Parent waits for the child to finish.
- `int lsh_execute(char **args)`  
    Check for built-ins, otherwise call lsh_launch.
- Built-ins are registered as a list of names and function pointers (e.g., `builtin_str[]`, `builtin_func[]`).

## Extending lsh
- Add job control (background processes & `&` handling).
- Implement input/output redirection (`>`, `<`) and pipes (`|`).
- Add command history and line editing (readline or custom).
- Improve parsing to handle quotes and escapes.

## Notes
- This shell is intended for learning and small experiments. It is not secure or feature-complete like bash or zsh.
- Be careful when running programs that modify your system from a simple shell.

## Credits
- [Typical implementations of lsh are based on tutorials such as "Write a shell in C" by Stephen Brennan and other learning resources.](https://brennan.io/2015/01/16/write-a-shell-in-c/)

## License
- Use, modify, and redistribute as needed. Add a LICENSE file to specify terms.
