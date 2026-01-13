/*
 * mini_bash.c - A minimal shell implementation using system calls
 * 
 * This program demonstrates process management and system call usage
 * by implementing a simple command interpreter.
     */

#include <unistd.h>     // For write(), read(), fork(), exec(), chdir(), access()
#include <stdlib.h>     // For getenv(), exit()
#include <string.h>     // For string manipulation functions
#include <stdio.h>      // For perror()
#include <sys/types.h>  // For pid_t type
#include <sys/wait.h>   // For wait() system call

// Constants
#define PROMPT "mini-bash$ "
#define PROMPT_LEN 11       // Length of "mini-bash$ " (10 chars + space)
#define BUFFER_SIZE 1024    // Size of input buffer
#define MAX_ARGS 64         // Maximum number of arguments (command + args)
#define MAX_PATH 512        // Maximum path length

/*
 * Function: parse_input
 * ---------------------
 * Parses the input buffer and splits it into tokens (words)
 * 
 * input: The input string to parse
 * argv: Array to store pointers to each token
 * 
 * Returns: Number of tokens found, or -1 on error
 * 
 * How it works:
 * - Uses in-place tokenization (modifies input string)
 * - Replaces spaces and tabs with '\0' to separate tokens
 * - Stores pointer to each token in argv array
 * - Argv array ends with NULL pointer (required by execv)
 */
int parse_input(char *input, char *argv[MAX_ARGS]) {
    int argc = 0;  // Argument count
    int in_token = 0;  // Flag: are we currently inside a token?
    
    // Iterate through each character in the input
    for (int i = 0; input[i] != '\0'; i++) {
        // Check if current character is a separator (space or tab)
        if (input[i] == ' ' || input[i] == '\t') {
            // Replace separator with null terminator
            input[i] = '\0';
            in_token = 0;  // We're no longer in a token
        } else {
            // We found a non-separator character
            if (!in_token) {
                // This is the start of a new token
                if (argc >= MAX_ARGS - 1) {
                    // Too many arguments - leave room for NULL terminator
                    return -1;
                }
                argv[argc] = &input[i];  // Store pointer to start of token
                argc++;
                in_token = 1;  // We're now inside a token
            }
            // If already in_token, just continue to next character
        }
    }
    
    // Null-terminate the argv array (required by execv)
    argv[argc] = NULL;
    
    return argc;
}

/*
 * Function: find_command
 * ----------------------
 * Searches for an executable command in HOME and /bin directories
 * 
 * command: The command name to search for
 * full_path: Buffer to store the full path if found
 * 
 * Returns: 1 if found, 0 if not found
 * 
 * Search order:
 * 1. $HOME/command_name
 * 2. /bin/command_name
 */
int find_command(const char *command, char *full_path) {
    // First, try searching in HOME directory
    char *home = getenv("HOME");
    if (home != NULL) {
        // Build path: HOME + "/" + command
        // Example: "/home/user" + "/" + "ls" = "/home/user/ls"
        
        // Copy HOME path
        int i = 0;
        while (home[i] != '\0' && i < MAX_PATH - 2) {
            full_path[i] = home[i];
            i++;
        }
        
        // Add slash
        if (i < MAX_PATH - 1) {
            full_path[i++] = '/';
        }
        
        // Add command name
        int j = 0;
        while (command[j] != '\0' && i < MAX_PATH - 1) {
            full_path[i++] = command[j++];
        }
        
        // Null-terminate
        full_path[i] = '\0';
        
        // Check if file exists and is executable
        // access(path, X_OK) returns 0 if file exists and is executable
        if (access(full_path, X_OK) == 0) {
            return 1;  // Found in HOME
        }
    }
    
    // Not found in HOME, try /bin directory
    // Build path: "/bin/" + command
    const char *bin_dir = "/bin/";
    int i = 0;
    
    // Copy "/bin/"
    while (bin_dir[i] != '\0' && i < MAX_PATH - 1) {
        full_path[i] = bin_dir[i];
        i++;
    }
    
    // Add command name
    int j = 0;
    while (command[j] != '\0' && i < MAX_PATH - 1) {
        full_path[i++] = command[j++];
    }
    
    // Null-terminate
    full_path[i] = '\0';
    
    // Check if file exists and is executable
    if (access(full_path, X_OK) == 0) {
        return 1;  // Found in /bin
    }
    
    // Not found in either location
    return 0;
}

/*
 * Function: int_to_string
 * -----------------------
 * Converts an integer to a string (helper for printing return codes)
 * 
 * num: The integer to convert
 * buffer: Buffer to store the resulting string
 *  
 * Returns: Pointer to the start of the string in buffer
 */
char* int_to_string(int num, char *buffer) {
    int i = 0;
    int is_negative = 0;
    
    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // Handle zero specially
    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return buffer;
    }
    
    // Convert digits (in reverse order)
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[i++] = '-';
    }
    
    // Null-terminate
    buffer[i] = '\0';
    
    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = temp;
    }
    
    return buffer;
}

/*
 * Main function - Entry point of the shell
 * 
 * Implements an infinite loop that:
 * 1. Displays a prompt
 * 2. Reads user input
 * 3. Parses the input
 * 4. Executes commands
 */
int main(void) {
    // Buffer to store user input - reused across iterations for efficiency
    char input_buffer[BUFFER_SIZE];
    
    // Array to store command arguments (pointers to tokens)
    // Format: ["command", "arg1", "arg2", ..., NULL]
    char *argv[MAX_ARGS];
    
    // Buffer to store full path to executable
    char full_path[MAX_PATH];

    // Main shell loop - runs indefinitely until user types "exit"
    while (1) {
        // STEP 1: Display the prompt using write() system call
        // write(fd, buffer, count) - writes 'count' bytes from 'buffer' to file descriptor 'fd'
        // STDOUT_FILENO (1) is the standard output (screen)
        // Returns: number of bytes written, or -1 on error
        ssize_t bytes_written = write(STDOUT_FILENO, PROMPT, PROMPT_LEN);
        
        // Check if write() failed
        if (bytes_written == -1) {
            // perror() prints the system error message
            perror("write");
            exit(1);
        }
        
        // STEP 2: Read user input using read() system call
        // read(fd, buffer, count) - reads up to 'count' bytes into 'buffer' from file descriptor 'fd'
        // STDIN_FILENO (0) is the standard input (keyboard)
        // Returns: number of bytes read, 0 on EOF, or -1 on error
        ssize_t bytes_read = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);
        
        // Check if read() failed
        if (bytes_read == -1) {
            perror("read");
            exit(1);
        }
        
        // Check if we got EOF (Ctrl+D) - exit gracefully
        if (bytes_read == 0) {
            write(STDOUT_FILENO, "\n", 1);  // Print newline for clean exit
            break;
        }
        
        // Remove the trailing newline character if present
        // When user presses Enter, read() includes the '\n' character
        if (bytes_read > 0 && input_buffer[bytes_read - 1] == '\n') {
            bytes_read--;  // Reduce the count to exclude newline
        }
        
        // Null-terminate the input string
        // This converts the raw byte array into a proper C string
        input_buffer[bytes_read] = '\0';
        
        // Handle empty input (user just pressed Enter)
        if (input_buffer[0] == '\0') {
            continue;  // Skip to next iteration - show prompt again
        }

        // STEP 3: Parse input into tokens
        int argc = parse_input(input_buffer, argv);
        
        // Check if parsing failed (too many arguments)
        if (argc == -1) {
            write(STDOUT_FILENO, "Error: Too many arguments\n", 26);
            continue;
        }
        
        // Check if parsing resulted in no tokens (shouldn't happen after empty check)
        if (argc == 0) {
            continue;
        }

        // STEP 4: Check for internal (built-in) commands
        
        // Internal command: "exit"
        // Exits the shell and terminates the program
        if (strcmp(argv[0], "exit") == 0) {
            break;  // Exit the while loop, which ends the program
        }
        
        // Internal command: "cd"
        // Changes the current working directory using chdir() system call
        if (strcmp(argv[0], "cd") == 0) {
            // Check if directory argument was provided
            if (argc < 2) {
                write(STDOUT_FILENO, "cd: missing argument\n", 21);
                continue;
            }
            
            // chdir() system call - changes current working directory
            // Returns: 0 on success, -1 on error
            if (chdir(argv[1]) == -1) {
                // Failed to change directory - print error
                perror("cd");
            }
            // If successful, chdir() silently changes directory
            continue;  // Go to next iteration - don't try to execute externally
        }
        
        // STEP 5: Search for external command
        // If we reach here, it's not an internal command
        
        if (find_command(argv[0], full_path)) {
            // STEP 6: Fork-Exec-Wait pattern
            // Command found! Now execute it in a child process
            
            // fork() creates a child process
            // Returns: PID of child in parent, 0 in child, -1 on error
            pid_t pid = fork();
            
            if (pid == -1) {
                // Fork failed - print error and continue shell
                perror("fork");
                continue;
            } else if (pid == 0) {
                // ===== CHILD PROCESS =====
                // This code runs ONLY in the child process
                
                // execv() replaces the child process with the new program
                // If successful, this function NEVER returns
                // Parameters:
                //   - full_path: path to executable
                //   - argv: array of arguments (NULL-terminated)
                execv(full_path, argv);
                
                // If we reach here, execv() failed
                perror("execv");
                exit(1);  // Child must exit (don't continue shell loop in child!)
                
            } else {
                // ===== PARENT PROCESS =====
                // This code runs ONLY in the parent process
                // pid contains the child's process ID
                
                // wait() blocks parent until child process terminates
                // Returns: PID of terminated child, or -1 on error
                // Parameter: pointer to int where exit status is stored
                int status;
                pid_t waited_pid = wait(&status);
                
                if (waited_pid == -1) {
                    perror("wait");
                } else {
                    // Child finished successfully
                    // Extract exit code using WIFEXITED and WEXITSTATUS macros
                    
                    if (WIFEXITED(status)) {
                        // Child exited normally
                        int exit_code = WEXITSTATUS(status);
                        
                        // Print "Command completed with return code: X"
                        write(STDOUT_FILENO, "Command completed with return code: ", 36);
                        
                        // Convert exit code to string and print it
                        char code_str[12];  // Enough for 32-bit int
                        int_to_string(exit_code, code_str);
                        write(STDOUT_FILENO, code_str, strlen(code_str));
                        write(STDOUT_FILENO, "\n", 1);
                    } else {
                        // Child terminated abnormally (signal, etc.)
                        write(STDOUT_FILENO, "Command terminated abnormally\n", 30);
                    }
                }
            }
            
        } else {
            // Command not found in HOME or /bin
            // Print error message: "[command]: Unknown Command"
            write(STDOUT_FILENO, "[", 1);
            write(STDOUT_FILENO, argv[0], strlen(argv[0]));
            write(STDOUT_FILENO, "]: Unknown Command\n", 19);
        }
    }
    
    return 0;
}
