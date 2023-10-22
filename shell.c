#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_INPUT_LENGTH 2048
#define HISTORY_SIZE 100

// Global array to store command history
char command_history[HISTORY_SIZE][MAX_INPUT_LENGTH];
int history_count = 0;

//function to add to history
void add_to_history(const char *command) {
    if (history_count < HISTORY_SIZE) {
        strcpy(command_history[history_count], command);
        history_count++;
    } else {
        // Shift the history to make room for the new command
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            strcpy(command_history[i], command_history[i + 1]);
        }
        strcpy(command_history[HISTORY_SIZE - 1], command);
    }
}

//function to print history
void print_history2(int n) {
     if (n <= 0) {
        return;
    }
    int start_index;
    if(n > HISTORY_SIZE ) {
        if(history_count > HISTORY_SIZE){
            start_index=1;
        }
    }
    start_index = history_count - n +1;
    if (start_index <= 0) {
        start_index = 1;
    }
    
    for (int i = start_index; i <= history_count; i++) {
        printf("%d %s\n", i - start_index + 1, command_history[i-1]);
    }
    exit(EXIT_SUCCESS);
}
void print_history1(int n) {
     if (n <= 0) {
        printf("Invalid argument: history count must be greater than 0\n");
        return;
    }
    int start_index;
    if(n > HISTORY_SIZE ) {
        if(history_count > HISTORY_SIZE){
            printf("Only last executed 100 commands will be displayed due to large input n\n");
            start_index=1;
        }
    }
    start_index = history_count - n +1;
    if (start_index <= 0) {
        printf("Only %d commands are executed\n", history_count);
        start_index = 1;
    }
    
    for (int i = start_index; i <= history_count; i++) {
        printf("%d %s\n", i - start_index + 1, command_history[i-1]);
    }
    exit(EXIT_SUCCESS);
}

//function to execute cd command
void execute_cd_command(char *directory) {
    if (chdir(directory) == -1) {
        perror("chdir");
    }
}

//function to execute inbuilt commands
void execute_command(char *command) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        if (strncmp(command, "history", 7) == 0) {
            int n=-1;
            sscanf(command + 7, "%d", &n); // Parse the number after "history"
            if(n == -1) {printf("Inavalid command: no argument after history\n");
            exit(EXIT_FAILURE);
            }
            else print_history1(n); // Print the last n commands from history
            //add_to_history(input);
        } 
        else{
        char *args[MAX_INPUT_LENGTH]; // Array to hold command and its arguments
        char *token; // Tokenize command string

        int i = 0;
        token = strtok(command, " "); // Tokenize command string by space
        while (token != NULL) {
            args[i++] = token; // Store tokens in args array
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // NULL-terminate the args array

        // Execute the command
        execvp(args[0], args);

        // If execvp returns, there was an error
        perror("execvp");
        exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}


void execute_piped_commands(char *command1, char *command2) {
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // Child process
        close(pipe_fd[0]); // Close unused read end of the pipe
        dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to the pipe write end
        close(pipe_fd[1]);

        if (strncmp(command1, "history", 7) == 0) {
            int n = -1;
            sscanf(command1 + 7, "%d", &n); // Parse the number after "history"
            if (n == -1) {
                printf("Invalid command: history count missing\n");
                exit(EXIT_FAILURE);
            }
            print_history2(n);
        } else {
            char *args[MAX_INPUT_LENGTH];
            char *token;
            int i = 0;
            token = strtok(command1, " ");
            while (token != NULL) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    close(pipe_fd[1]); // Close write end of the pipe in the parent process
    //waitpid(pid1, NULL, 0);

    // Check the exit status of the first command
     int status;
    waitpid(pid1, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        pid_t pid2 = fork();

        if (pid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid2 == 0) {
            // Second child process
            dup2(pipe_fd[0], STDIN_FILENO); // Redirect stdin to the pipe read end
            close(pipe_fd[0]);

            // Tokenize and execute the second command
            char *args[MAX_INPUT_LENGTH]; // Array to hold command and its arguments
            char *token; // Tokenize command string

            int i = 0;
            token = strtok(command2, " "); // Tokenize command string by space
            while (token != NULL) {
                args[i++] = token; // Store tokens in args array
                token = strtok(NULL, " ");
            }
            args[i] = NULL; // NULL-terminate the args array

            execvp(args[0], args);

            // If execvp returns, there was an error
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[0]); // Close read end of the pipe in the parent process

        // Wait for the second child process to complete
        waitpid(pid2, NULL, 0);
    } 
}


int main() {
    char input[MAX_INPUT_LENGTH];

    while (1) {
        printf("MTL458 > ");
        fgets(input, sizeof(input), stdin);

        // Remove the newline character from input
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            break; // Exit the shell if user enters "exit"
        } else if (strncmp(input, "cd", 2) == 0) {
            char *cd_cmd = input + 3; // Skip "cd " to get the directory
            execute_cd_command(cd_cmd);
            add_to_history(input);
        } else {
            char full_input[MAX_INPUT_LENGTH];
            strcpy(full_input, input);
            char *grep_cmd = strtok(input, "|");
            char *wc_cmd = strtok(NULL, "|");

            if (grep_cmd && wc_cmd) {
                execute_piped_commands(grep_cmd, wc_cmd);
                
            } else {
                execute_command(input);
            }
            add_to_history(full_input);
        }
    }

    return 0;
}


