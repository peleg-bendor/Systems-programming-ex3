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
        
        // If we reach here, it's not an internal command
        // Temporary: Print that we would execute external command
        write(STDOUT_FILENO, "[External command: ", 19);
        write(STDOUT_FILENO, argv[0], strlen(argv[0]));
        write(STDOUT_FILENO, "]\n", 2);
        
        // TODO: STEP 5 - Command search will go here
        
        // TODO: STEP 6 - Fork-Exec-Wait will go here
    }
    
    return 0;
}
