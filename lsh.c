// writing a shell in C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define LSH_RL_BUFSIZE 1024

// token bufsize
// i.e. expected intial number of tokens in a line
#define LSH_TOK_BUFSIZE 64
// delimiters
#define LSH_TOK_DELIM " \t\r\n\a"

// these are exit codes
// ./a.out
// echo $?
// this prints the exit code
// 0 for success ,1 for failure
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

// there are some commands which need the current state of the
// parent process to also change
// like cd ,exit
// if i use just lsh_launch it will change the properties
// in the child process and exit out to the parent process
// where these properties remain unchanged
// so we have to write some functions for these beforehand
// by using sys calls directly instead of lsh_launch
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */

// to add more ,just add here and code its function
// usefull instead of editing a large swithc statement
// remember that the order in both builtin_str and builtin_func should be the same
char *builtin_str[] = {
    "cd",
    "help",
    "exit"};

// declares that there are such function which
// take char ** as input and return an int
// stores the pointer to that funciton

int (*builtin_func[])(char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit};

int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int lsh_cd(char **args)
{
    if (args[1] == NULL)
    {
        // no argument given to cd
        // hence print the error that it expects some arguments
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }
    else
    {
        // use the function call chdir()
        // to change the current directory to the specified location
        if (chdir(args[1]) != 0)
        {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args)
{
    // you can basically write anything here its your own choice
    int i;
    printf("n00b69master's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char **args)
{
    // exit out of the shell
    // hence returning 0
    return 0;
}

char *lsh_read_line(void)
{
    // this function already exists in C

    int bufsize = LSH_RL_BUFSIZE;
    char *buffer = malloc(sizeof(char) * bufsize);

    if (!buffer)
    {
        // error in allocation
        fprintf(stderr, "lsh: allocation error\n");
        return NULL;
    }

    int position = 0;

    // input charecter is taken as int
    // to test for EOF
    int c;

    // not really infinite because EOF or new line charecter
    // will come at some point
    while (1)
    {
        // take input of the charecter
        c = getchar();

        // if we hit EOF or new line
        // use null charecter to indicate end of string/line
        // and return the final buffer
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
            buffer[position] = c;

        // going to next position
        position++;

        if (position >= bufsize)
        {
            // increasing the size of buffer
            bufsize += LSH_RL_BUFSIZE;

            // reallocating more space to buffer
            // this allocates new space,copies the old one
            // returns the new pointer
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                // error in allocation
                fprintf(stderr, "lsh: allocation error\n");
                return NULL;
            }
        }
    }
}

char **lsh_split_line(char *line)
{
    // splits the line into
    // numerous tokes to be executed
    // we assume that the arguments are seperated by
    // a space or a tab or a new line etc
    // these are called delimiters
    int bufsize = LSH_TOK_BUFSIZE;
    int position = 0;

    // initialing the array of strings of tokens
    char **tokens = malloc(bufsize * sizeof(char *));
    if (!tokens)
    {
        fprintf(stderr, "lsh: allocation error\n");
        return NULL;
    }

    // this function is used to split the string into words
    // splitting words via delimiters provided
    // puts first word in token or NULL when end
    // replaces the space by null and stores where it left off
    // for the next call it continues from there
    char *token = strtok(line, LSH_TOK_DELIM);

    // example if "ls -l /home" is 'line'
    // 'line' becomes "ls\0-l\0/home\0"
    // and tokens[0]= "ls"
    // tokens[1]= "-l"
    // and so on..

    while (token != NULL)
    {
        tokens[position] = token;
        position++;
        if (position >= bufsize)
        {
            // increasing size of array to accomodate more strings
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));

            if (!tokens)
            {
                fprintf(stderr, "lsh: allocation error\n");
                return NULL;
            }
        }
        // starts from where it left off
        token = strtok(NULL, LSH_TOK_DELIM);
    }

    // indicating end of tokens by a null cahrecter
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args)
{
    // process id and the id which waits for the child to get executed
    __pid_t pid, wpid;
    int status;

    // system call to fork
    pid = fork();

    if (pid == 0)
    {
        // we are inside the child process
        // we perform the command here instead of the parent part
        // exec() call is used
        // it does not return as in it does not return to the code where it was called
        // it terminates there
        // execvp() is just a method to call it in a different way
        // if there was an error doing this log it,otherwise continue
        // execvp() searches for the program in the directories in $PATH,
        // and tries each one until it finds an executable file matching args[0].

        // example
        // line = "ls -l /home"
        // sends the call to /bin/ls
        // if it doesnt exists there then returns -1
        // else executes it
        if (execvp(args[0], args) == -1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // there was an error forking
        // mostly due to memmory

        // log the error and continue the shell
        perror("lsh");
    }
    else
    {
        // Parent process: wait for the child to finish.
        //
        // waitpid(pid, &status, WUNTRACED) suspends the parent until the child
        // changes state. This happens if the child:
        //   • exits normally (return from main or exit())
        //   • is terminated by a signal (e.g., Ctrl+C -> SIGINT)
        //   • is stopped by a signal (e.g., Ctrl+Z -> SIGTSTP), because of WUNTRACED
        //
        // The returned child's state information is stored in 'status'.
        // The macros WIFEXITED(status)   -> child exited normally
        //           WIFSIGNALED(status)  -> child was killed by a signal
        //           WIFSTOPPED(status)   -> child was stopped (not used here)
        //
        // We loop until the child either:
        //   1. exits normally  (WIFEXITED)
        //   2. is killed by a signal (WIFSIGNALED)
        //
        // If the child is stopped (Ctrl+Z), waitpid returns, but neither WIFEXITED
        // nor WIFSIGNALED is true, so the loop continues and we call waitpid again.
        // (Full job control requires more logic, but this simple shell does not implement it.)
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    // signal to the calling function
    // to prompt for the next input
    return 1;
}

int lsh_execute(char **args)
{

    if (args[0] == NULL)
    {
        // nothing was given to the shell
        // prompt for input again
        return 1;
    }

    // check if the builtin function for this is already made
    // from the builtin function string list
    for (int i = 0; i < lsh_num_builtins(); i++)
    {
        // compares the strings
        // returns 0 if both are completey the same
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            // return the function made for this
            return (*builtin_func[i])(args);
        }
    }

    // not found then execute the launch function
    return lsh_launch(args);
}
void lsh_loop(void)
{
    // passing void specifies that this function takes no params
    // lsh_loop() would mean the same thing but when calling
    // you can pass it something and the compiler would give
    // undefined behaviour
    // void would give error

    // line is 1 line input
    // it is taken input
    // args is 'line' split into args
    // split into args by splitting function
    // status is returned by execute function

    char *line;
    char **args;
    int status;

    // do while to atleast do once
    do
    {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        // freeing the space everytime after execution of one line
        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    // argc is the number of arguments the main function takes
    // **argv is a char
    // *argv is a string of char
    // argv is an array of strings of the arguments
    // printf("Number of arguments is %d\n", argc);
    // for (int i = 0; i < argc; i++)
    // {
    //     printf("Arg %d: %s\n", i, argv[i]);
    // }

    lsh_loop();

    return EXIT_SUCCESS;
}