/// Käyttöjärjestelmät ja systeemiohjelmointi
/// wish.c 
/// Jericho Koskinen
/// https://medium.com/@santiagobedoa/coding-a-shell-using-c-1ea939f10e7e

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_INPUT_SIZE 1024
#define INITIAL_PATH_SIZE 1
#define PATH_DELIM ":"

// Global variables for path management
char **search_paths;
int num_paths;

// Error message
const char *error_message = "An error has occurred\n";

// Function prototypes
char **split_to_args(char *line, int *argc);
void execute_command(char **args);
void change_directory(char *dir);
void set_path(char *new_path);
void handle_redirection(char **args);
void handle_parallel_commands(char *line);

int main(int argc, char *argv[]) {
    // Initialize the default search path
    num_paths = INITIAL_PATH_SIZE;
    search_paths = malloc(num_paths * sizeof(char *));
    search_paths[0] = strdup("/bin");

    char input[MAX_INPUT_SIZE];
    char **args;
    int arg_count;
    FILE *input_file = stdin;

    // Handle batch mode
    if (argc > 2) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    } else if (argc == 2) {
        input_file = fopen(argv[1], "r");
        if (input_file == NULL) {
            perror("fopen");
            exit(1);
        }
    }

    while (1) {
        if (input_file == stdin) {
            // Print the prompt only in interactive mode
            printf(">wish ");
            fflush(stdout);
        }

        // Read user input
        if (fgets(input, sizeof(input), input_file) == NULL) {
            if (feof(input_file)) {
                // End of file, exit gracefully
                break;
            } else {
                perror("fgets");
                continue;
            }
        }

        // Remove the newline character from input
        input[strcspn(input, "\n")] = '\0';

        // Handle parallel commands if '&' is found
        if (strchr(input, '&') != NULL) {
            handle_parallel_commands(input);
        } else {
            args = split_to_args(input, &arg_count);

            if (args[0] == NULL) {
                free(args);
                continue;
            }

            // Handle built-in commands
            if (strcmp(args[0], "exit") == 0) {
                if (args[1] != NULL) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                } else {
                    free(args);
                    for (int i = 0; i < num_paths; i++) {
                        free(search_paths[i]);
                    }
                    free(search_paths);
                    if (input_file != stdin) fclose(input_file);
                    exit(0);
                }
            } else if (strcmp(args[0], "cd") == 0) {
                if (args[1] == NULL) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                } else {
                    change_directory(args[1]);
                }
            } else if (strcmp(args[0], "path") == 0) {
                if (args[1] == NULL) {
                    // Handle empty path case
                    num_paths = INITIAL_PATH_SIZE;
                    search_paths = realloc(search_paths, num_paths * sizeof(char *));
                    search_paths[0] = strdup("/bin");
                } else {
                    set_path(args[1]);
                }
            } else {
                handle_redirection(args);
                execute_command(args);
            }
            free(args);
        }
    }

    for (int i = 0; i < num_paths; i++) {
        free(search_paths[i]);
    }
    free(search_paths);
    if (input_file != stdin) fclose(input_file);
    return 0;
}

// Function to split the input line into arguments
char **split_to_args(char *line, int *argc) {
    int bufsize = 64;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(1);
    }

    token = strtok(line, " \t");
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "allocation error\n");
                exit(1);
            }
        }

        token = strtok(NULL, " \t");
    }
    tokens[position] = NULL;
    *argc = position;
    return tokens;
}

// Function to execute commands
void execute_command(char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        char *cmd = args[0];
        char full_path[1024];

        // Search for the command in the search paths
        for (int i = 0; i < num_paths; i++) {
            snprintf(full_path, sizeof(full_path), "%s/%s", search_paths[i], cmd);
            if (access(full_path, X_OK) == 0) {
                // Command found, execute it
                execv(full_path, args);
                // If execv returns, an error occurred
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
        }

        // Command not found in any path
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    } else if (pid < 0) {
        // Error forking
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else {
        // Parent process
        wait(NULL);
    }
}

// Function to change directory
void change_directory(char *dir) {
    if (chdir(dir) != 0) {
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

// Function to set path
void set_path(char *new_path) {
    // Free old paths
    for (int i = 0; i < num_paths; i++) {
        free(search_paths[i]);
    }
    free(search_paths);

    // Parse new path
    num_paths = 1;
    char *path_copy = strdup(new_path);
    char *token = strtok(path_copy, PATH_DELIM);

    // Count number of new paths
    while (token != NULL) {
        num_paths++;
        token = strtok(NULL, PATH_DELIM);
    }

    free(path_copy);

    // Allocate memory for new paths
    search_paths = malloc(num_paths * sizeof(char *));
    if (!search_paths) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    // Parse new path again to store paths
    token = strtok(new_path, PATH_DELIM);
    for (int i = 0; i < num_paths - 1; i++) {
        search_paths[i] = strdup(token);
        if (!search_paths[i]) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        token = strtok(NULL, PATH_DELIM);
    }
}



// Function to handle output redirection
void handle_redirection(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL || args[i + 2] != NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                return;
            }

            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            if (fd == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                return;
            }

            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);

            args[i] = NULL;
            break;
        }
        i++;
    }
}

// Function to handle parallel commands
void handle_parallel_commands(char *line) {
    char *commands[64];
    int i = 0;

    char *command = strtok(line, "&");
    while (command != NULL && i < 64 - 1) {
        commands[i++] = command;
        command = strtok(NULL, "&");
    }
    commands[i] = NULL;

    for (int j = 0; j < i; j++) {
        int arg_count;
        char **args = split_to_args(commands[j], &arg_count);
        if (args[0] != NULL) {
            if (fork() == 0) {
                handle_redirection(args);
                execute_command(args);
                exit(0);
            }
        }
        free(args);
    }

    for (int j = 0; j < i; j++) {
        wait(NULL);
    }
}
