# Mini-Bash Design Document

**Project:** Mini-Bash - Command Interpreter Using System Calls
**Author:** Peleg Bendor
**Course:** Operating Systems - Exercise 3

---

## 1. Overview

This document describes the design and implementation of a minimal shell that demonstrates core operating system concepts: process management, system calls, and inter-process communication.

### Goals
- Implement shell using **only system calls** (no `system()` wrapper)
- Support internal commands (`exit`, `cd`)
- Execute external commands using fork-exec-wait pattern
- Demonstrate efficient memory management

---

## 2. System Calls Analysis

### System Calls Used

| System Call | Purpose | When Used | Return Value |
|------------|---------|-----------|--------------|
| `write()` | Display prompt and messages | Every iteration | Bytes written or -1 |
| `read()` | Read user input | Every iteration | Bytes read, 0 (EOF), or -1 |
| `chdir()` | Change directory | `cd` command | 0 success, -1 error |
| `strcmp()` | Compare strings | Command identification | 0 if equal |
| `strlen()` | Get string length | Output operations | String length |
| `getenv()` | Get HOME path | Command search | Pointer or NULL |
| `access()` | Check file executable | Command search (2x) | 0 if exists, -1 if not |
| `fork()` | Create child process | External commands | PID in parent, 0 in child, -1 error |
| `execv()` | Execute program | Child process | Never returns (or -1) |
| `wait()` | Wait for child | Parent process | Child PID or -1 |
| `perror()` | Print errors | Error handling | void |

### Key System Call Details

**`write(fd, buffer, count)`**
- Used for all output (prompt, messages)
- Direct system call - no buffering
- Returns bytes written

**`read(fd, buffer, count)`**
- Reads user input from stdin
- Returns 0 on EOF (Ctrl+D)
- Handles newlines manually

**`chdir(path)`**
- Changes current directory
- **Must** run in parent process (not child)
- Returns 0 on success

**`fork()`**
- Creates child process (copy of parent)
- Returns twice: child PID in parent, 0 in child
- Child inherits file descriptors

**`execv(path, argv)`**
- Replaces process with new program
- Never returns on success
- If returns, exec failed

**`wait(&status)`**
- Blocks parent until child exits
- Retrieves child exit status
- Prevents zombie processes

---

## 3. Program Flow

### Main Loop Flow

```
START
  │
  ├─→ LOOP:
  │     │
  │     ├─→ write() prompt
  │     ├─→ read() input
  │     ├─→ Check EOF → Exit
  │     ├─→ Strip newline
  │     ├─→ Parse into tokens
  │     │
  │     ├─→ Command == "exit"? → Exit
  │     ├─→ Command == "cd"?
  │     │     └─→ chdir() → Loop
  │     │
  │     ├─→ Search for executable:
  │     │     ├─→ Check $HOME/command (access())
  │     │     └─→ Check /bin/command (access())
  │     │
  │     ├─→ Found?
  │     │     ├─ NO → "Unknown Command" → Loop
  │     │     │
  │     │     └─ YES → fork()
  │     │              ├─ Child: execv()
  │     │              └─ Parent: wait() → Report → Loop
  │     │
  └─────┘
EXIT
```

### Command Execution Steps

**Internal Command (e.g., `cd`):**
1. Parse input
2. Identify as `cd`
3. Call `chdir(path)`
4. Check error
5. Continue loop

**External Command (e.g., `ls`):**
1. Parse input
2. Search for executable
3. `fork()` child process
4. Child: `execv()` to run program
5. Parent: `wait()` for child
6. Report exit code
7. Continue loop

---

## 4. Memory Management & Efficiency

### Buffer Strategy

**Input Buffer (1024 bytes)**
```c
char input_buffer[BUFFER_SIZE];  // Stack allocation
```
- Allocated once on stack
- Reused every iteration
- No malloc/free needed

**Argument Array (64 pointers)**
```c
char *argv[MAX_ARGS];  // Stack allocation
```
- Points into input_buffer
- No string copying

### In-Place Tokenization

**Before parsing:** `"ls -la /home\n"`
```
['l','s',' ','-','l','a',' ','/',h','o','m','e','\n']
```

**After parsing:** `"ls\0-la\0/home\0"`
```
['l','s','\0','-','l','a','\0','/','h','o','m','e','\0']
  ^          ^              ^
  argv[0]    argv[1]        argv[2]
```

- Replaces spaces/tabs with `\0`
- argv[] points to each token
- Zero-copy parsing

### Efficiency Metrics

- **Stack usage:** ~1088 bytes
- **Heap allocations:** 0
- **System calls per command:**
  - Internal: 2 (write + read)
  - External: 7-9 (write + read + search + fork + exec + wait + output)
- **Time complexity:** O(n) where n = input length
- **Space complexity:** O(1) per iteration

---

## 5. Error Handling Strategy

### Error Classification

**Fatal Errors (exit shell):**
- `write()` fails → Can't display prompt
- `read()` fails → Can't get input

**Recoverable Errors (continue shell):**
- Command not found
- `cd` fails
- `fork()` fails
- `exec()` fails

### Error Handling Pattern

```c
if (system_call() == -1) {
    perror("context");
    // Either exit(1) or continue
}
```

### Error Examples

**`cd` error:**
```c
if (chdir(argv[1]) == -1) {
    perror("cd");  // Prints: "cd: No such file or directory"
    continue;      // Stay in shell
}
```

**`fork()` error:**
```c
pid_t pid = fork();
if (pid == -1) {
    perror("fork");  // Prints: "fork: Cannot allocate memory"
    continue;        // Try again later
}
```

**`exec()` error (in child):**
```c
execv(path, argv);
perror("execv");  // Only reached if exec fails
exit(1);          // Child must exit
```

---

## 6. Implementation Decisions

### Key Design Choices

**1. Why `write()` instead of `printf()`?**
- Direct system call (no buffering)
- Demonstrates low-level I/O
- Simpler error handling

**2. Why in-place tokenization?**
- Zero-copy (no malloc)
- O(1) space complexity
- Fast single-pass parsing

**3. Why search HOME before /bin?**
- Allows user overrides
- Common Unix pattern
- User customization

**4. Why `access()` before `fork()`?**
- Avoid expensive fork if command doesn't exist
- Better error messages
- More efficient

**5. Why fixed-size buffers?**
- Stack allocation (fast)
- No dynamic memory management
- Sufficient for typical commands
- Limits: 1023 char input, 63 arguments

---

## 7. Testing

### Test Categories

**1. Basic Functionality**
```bash
mini-bash$ ls
mini-bash$ pwd
mini-bash$ echo hello
```

**2. Internal Commands**
```bash
mini-bash$ cd /tmp
mini-bash$ cd ..
mini-bash$ exit
```

**3. Command Arguments**
```bash
mini-bash$ ls -la
mini-bash$ echo hello world
```

**4. Error Handling**
```bash
mini-bash$ invalidcmd          # Unknown command
mini-bash$ cd /nonexistent     # cd error
mini-bash$ cd                  # Missing argument
```

**5. Edge Cases**
```bash
mini-bash$ [Enter]             # Empty input
mini-bash$ ls    -l            # Multiple spaces
mini-bash$ [Ctrl+D]            # EOF exit
```

---

## 8. Limitations & Future Work

### Current Limitations

- No pipes (`|`)
- No redirection (`>`, `<`)
- No background jobs (`&`)
- No environment variable expansion (`$VAR`)
- No command history
- No signal handling (Ctrl+C)
- Fixed buffer sizes

### Why These Limitations?

This is a **minimal** shell for educational purposes, focusing on:
- System call usage
- Process management (fork-exec-wait)
- Basic shell functionality

Full shell features would obscure core concepts.

---

## Conclusion

This mini-bash implementation demonstrates:

1. **System call mastery** - Direct use of low-level calls
2. **Efficient design** - Minimal memory, optimal algorithms
3. **Robust error handling** - Graceful failure recovery
4. **Clear architecture** - Simple, maintainable code

The design proves that fundamental OS concepts can be implemented with minimal overhead while maintaining clarity and correctness.

---

**End of Document**
