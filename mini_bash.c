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
#define PROMPT_LEN 11  // Length of "mini-bash$ " (10 chars + space)
#define BUFFER_SIZE 1024    // Size of input buffer

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
        
        // Temporary: Print what we read to verify input reading works
        write(STDOUT_FILENO, "You entered: ", 13);
        write(STDOUT_FILENO, input_buffer, strlen(input_buffer));
        write(STDOUT_FILENO, "\n", 1);
        
        
        // TODO: STEP 3 - Parse input will go here
        
        // TODO: STEP 4 - Execute command will go here
        
        // Temporary: For testing purposes, break after one iteration
        // Remove this break once we implement input reading
        break;
    }
    
    return 0;
}
