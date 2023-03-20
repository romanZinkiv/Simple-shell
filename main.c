#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//Function declaration for shell builtins:
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

//List of built-in commands followed by their respective functions
char* builtin_str[] = {
    "cd", "help", "exit"
};

int (*builtin_func[])(char**) = {
    &shell_cd, &shell_help, &shell_exit
};

int shell_num_builtins() {
    return sizeof(builtin_str)/sizeof(char*);
}

//Built-in Function Implementations
int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "shell: expected argument for \"cd\"\n");
    } else if (chdir(args[1]) != 0)
    {
        perror("shell");
    }

    return 1;
}

int shell_help(char **args)
{
    printf("Type the name of the program and its arguments and hit enter.\n");
    printf("Here is a list of built-in commands:\n");

    for (int i = 0; i < shell_num_builtins(); i++)
    {
        printf("    %s\n", builtin_str[i]);
    }
    
    printf("Use the man command for information on other programs.\n");

    return 1;
}

int shell_exit(char **args)
{
    return 0;
}

int shell_execute(char **args)
{
    if (args[0] == NULL)
    {
        // An empty command has been entered
        return 1;
    }

    for (int i = 0; i < shell_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }
    
    return shell_launch(args);
}

int shell_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        //Child process
        //We use one of the variants of the exec system call, execvp.
        //This particular variant takes a program name and an array 
        //(also called a vector, hence 'v') of string arguments 
        //(the program name must come first). 'p' means that instead of 
        //providing the full path to the program file to run, 
        //we will only provide its name, and also tell 
        //the operating system to look for it on its own.
        if(execvp(args[0], args) == -1)
        {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid == -1)
    {
        //Forking error
        perror("shell");
    } else {
        //Parent process
        do
        {
             wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

#define SHELL_BUFFSIZE 1024
char *shell_read_line()
{
    int buffsize = SHELL_BUFFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "shell: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();

        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        if (position >= buffsize)
        {
            buffsize += SHELL_BUFFSIZE;
            buffer = realloc(buffer, buffsize);
        }
        if (!buffer)
        {
            fprintf(stderr, "shell: memory allocation error\n");
            exit(EXIT_FAILURE);
        }

        // char *line = NULL;
        // int buffsize = 0;
        // getline(&line, &buffsize, stdin);
        // return line;
    }
}

#define SHELL_TOK_BUFFSIZE 64
#define SHELL_TOK_DELIMETER " \t\r\n\a"
char **shell_split_line(char *line)
{
    int buffsize = SHELL_TOK_BUFFSIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char) * buffsize);
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "shell: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHELL_TOK_DELIMETER);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= buffsize)
        {
            buffsize += SHELL_TOK_BUFFSIZE;
            tokens = realloc(tokens, buffsize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "shell: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHELL_TOK_DELIMETER);
    }
    tokens[position] = NULL;
    return tokens;
}

void shell_loop()
{
    char *line;
    char **argc;
    char status;

    do
    {
        printf("> ");
        line = shell_read_line();
        argc = shell_split_line(line);
        status = shell_execute(argc);

        free(line);
        free(argc);
    } while (status);
}


int main(int argc, char **argv)
{

    shell_loop();

    return EXIT_SUCCESS;
}
