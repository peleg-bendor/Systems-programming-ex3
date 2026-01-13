# Mini-Bash - Command Interpreter Using System Calls

A minimal shell (command interpreter) written in C that uses **only Linux system calls** to manage processes and execute commands.

---

## Description

This program implements a simplified version of the Bash shell, demonstrating how operating systems manage processes, create process copies (fork), and execute programs. The shell runs in an infinite loop, accepting user commands and executing them using low-level system calls.

**Key Features:**

- [x] Pure system calls - no `system()` or high-level wrappers
- [x] Internal commands: `exit`, `cd`
- [x] External command execution with PATH search (HOME and /bin)
- [x] Process management using fork-exec-wait pattern
- [x] Efficient memory management with minimal buffer copies
- [x] Comprehensive error handling with detailed system error messages
- [x] Return code reporting for executed commands

---

## Compilation

### Using Make (recommended):

```bash
make
```

### Manual compilation:

```bash
gcc -Wall -Wextra -Werror -std=c99 mini_bash.c -o mini_bash
```

---

## Usage

### Start the shell:

```bash
./mini_bash
```

### Shell prompt:

```
mini-bash$
```

### Exit the shell:

```
mini-bash$ exit
```

---

## Supported Commands

### Internal Commands (Built-in)

**1. `exit`**

- Exits the shell and terminates the program
- Example:

  ```
  mini-bash$ exit
  ```

**2. `cd <directory>`**

- Changes the current working directory
- Examples:

  ```
  mini-bash$ cd /home
  mini-bash$ cd ..
  mini-bash$ cd ~/Documents
  ```

### External Commands

Any executable found in:

1. **Home directory** (`$HOME`)
2. **System binaries** (`/bin`)

Examples:

```
mini-bash$ ls
mini-bash$ ls -l
mini-bash$ cat file.txt
mini-bash$ pwd
mini-bash$ echo Hello World
```

---

## How It Works

### The Shell Loop (Main Logic)

```
1. Display prompt: "mini-bash$ "
   ↓
2. Read user input (command + arguments)
   ↓
3. Parse input into tokens (split by spaces/tabs)
   ↓
4. Check if internal command (exit/cd)
   ├─ Yes → Execute internally
   └─ No → Continue to external command search
   ↓
5. Search for executable:
   - First in HOME directory
   - Then in /bin directory
   ↓
6. If found → Fork-Exec-Wait
   - Fork: Create child process
   - Exec: Replace child with command
   - Wait: Parent waits for child to finish
   ↓
7. Report return code and repeat
```

---

## Implementation Steps

### **STEP 1: Main Shell Loop**

- Infinite loop structure
- Display prompt using `write()`
- Basic input/output setup

### **STEP 2: Input Reading**

- Read user command using `read()`
- Buffer management
- Handle newlines and empty input

### **STEP 3: Parsing (Tokenization)**

- Split input by spaces (`' '`) and tabs (`'\t'`)
- Extract command name and arguments
- Build argument array for `exec()`

### **STEP 4: Internal Commands**

- **`exit`**: Break loop and terminate
- **`cd`**: Use `chdir()` system call to change directory

### **STEP 5: Command Search**

- Get HOME directory using `getenv("HOME")`
- Check `$HOME/command_name` with `access(X_OK)`
- If not found, check `/bin/command_name`
- Report "Unknown Command" if not found

### **STEP 6: Process Management**

- **Fork**: Create child process with `fork()`
- **Exec**: Execute command in child with `execv()`
- **Wait**: Parent waits for child with `wait()`
- **Report**: Display return code

### **STEP 7: Error Handling**

- Check all system call return values
- Use `perror()` for detailed error messages
- Handle edge cases gracefully

### **STEP 8: Optimization**

- Minimize memory copies
- Reuse buffers
- Efficient string manipulation

---

## System Calls Used

| System Call          | Purpose                                | Used In                    |
| -------------------- | -------------------------------------- | -------------------------- |
| `write()`            | Display prompt and messages            | Main loop                  |
| `read()`             | Read user input                        | Input reading              |
| `getenv()`           | Get HOME directory path                | Command search             |
| `access()`           | Check if file exists and is executable | Command search             |
| `chdir()`            | Change working directory               | `cd` command               |
| `fork()`             | Create child process                   | External command execution |
| `execv()`/`execve()` | Replace process with new program       | External command execution |
| `wait()`/`waitpid()` | Wait for child process to finish       | External command execution |
| `perror()`           | Print system error messages            | Error handling             |

**No high-level wrappers like `system()` are used.**

---

## Example Session

```bash
$ ./mini_bash
mini-bash$ pwd
/home/user
Command completed with return code: 0

mini-bash$ ls -l
total 24
-rw-r--r-- 1 user user  1234 Jan 12 10:30 mini_bash.c
-rwxr-xr-x 1 user user 16832 Jan 12 10:35 mini_bash
Command completed with return code: 0

mini-bash$ cd /tmp
mini-bash$ pwd
/tmp
Command completed with return code: 0

mini-bash$ invalid_command
[invalid_command]: Unknown Command

mini-bash$ exit
```

---

## Testing

### Test 1: Internal commands

```bash
./mini_bash
mini-bash$ cd /home
mini-bash$ pwd
mini-bash$ exit
```

### Test 2: External commands

```bash
./mini_bash
mini-bash$ ls
mini-bash$ cat /etc/hostname
mini-bash$ echo test
```

### Test 3: Command with arguments

```bash
./mini_bash
mini-bash$ ls -la /bin
```

### Test 4: Unknown command

```bash
./mini_bash
mini-bash$ nonexistent_command
[nonexistent_command]: Unknown Command
```

### Test 5: Error handling

```bash
./mini_bash
mini-bash$ cd /nonexistent_directory
cd: No such file or directory
```

---

## Technical Details

### Input Buffer

- **Size**: 1024 bytes
- Sufficient for typical command lines
- Handles overflow gracefully

### Token Parsing

- **Separators**: Space (`' '`) and tab (`'\t'`)
- **Max tokens**: 64 (command + arguments)
- Null-terminated array for `execv()`

### Process Management

- **Fork**: Creates exact copy of parent process
- **Exec**: Replaces child process image with new program
- **Wait**: Blocks parent until child terminates
- **Return code**: Extracted from child's exit status

### Search Order

1. `$HOME/command_name` (executables in user's home directory)
2. `/bin/command_name` (system commands)

### Memory Efficiency

- Single input buffer reused across iterations
- In-place tokenization (modifying buffer with null terminators)
- No unnecessary string duplication
- Minimal heap allocations

---

## Error Handling

Every system call is checked for errors. The program provides:

- Clear error messages using `perror()`
- Graceful degradation (shell continues after command errors)
- Proper cleanup of resources

Common errors handled:

- Failed `fork()`: Resource exhaustion
- Failed `exec()`: File not found, permission denied
- Failed `chdir()`: Directory doesn't exist
- Failed `read()`/`write()`: I/O errors

---

## Clean Up

Remove compiled files:

```bash
make clean
```

---

## Project Structure

```
ex3/
├── mini_bash.c           # Source code (heavily commented)
├── Makefile              # Build configuration
├── README.md             # This file
├── Design Document.md    # Detailed design documentation
├── ex3.md                # Assignment requirements
├── .gitignore            # Git ignore rules
└── mini_bash             # Compiled executable (created by make)
```

---

## Assignment Requirements Met

### Core Functionality

- [x] Infinite shell loop (prompt → read → parse → execute)
- [x] Display prompt: `mini-bash$ `
- [x] Read and parse user commands
- [x] Tokenization with space and tab separators
- [x] Internal command: `exit`
- [x] Internal command: `cd` using `chdir()`
- [x] External command search (HOME then /bin)
- [x] Executable verification with `access(X_OK)`
- [x] Process management (fork-exec-wait pattern)
- [x] Return code reporting
- [x] "Unknown Command" error message

### Technical Requirements

- [x] Uses only system calls (no `system()` wrapper)
- [x] No threads (process-based only)
- [x] Memory-efficient buffer management
- [x] Comprehensive error handling with `perror()`
- [x] Extensive code comments explaining system calls

### Documentation

- [x] Detailed code comments
- [x] Complete README with usage examples
- [x] Design Document with system call analysis
- [x] Flow logic description
- [x] Public GitHub repository
- [x] Working Makefile

---

## Design Document

See [Design Document.md](Design%20Document.md) for:

- Detailed system call analysis
- Flowchart of program logic
- Efficiency proof (minimal system calls)
- Error handling strategy
- Implementation decisions

---

## Repository

**GitHub**: https://github.com/peleg-bendor/Systems-programming-ex3

---

## Learning Objectives Achieved

Through this project, you will understand:

- How shells work at the system level
- Process creation and management in Unix/Linux
- The fork-exec-wait pattern
- How the kernel loads and runs programs
- Difference between internal and external commands
- System call API and error handling
- Efficient memory management in C

---

## Notes

- This is a **minimal** shell implementation for educational purposes
- Does not support: pipes, redirection, background jobs, environment variables (except HOME), aliases, scripting, etc.
- Focuses on core concepts: process management and system calls
- Runs on Linux/Unix systems (requires POSIX system calls)