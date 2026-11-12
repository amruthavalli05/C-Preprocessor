# C Preprocessor

## Description
This project implements a simple C Preprocessor using C programming.

It accepts a C source file as a command-line argument and generates a preprocessed `.i` output file.

## Features
- Removal of single-line comments (`//`)
- Removal of multi-line comments (`/* */`)
- File inclusion using `#include`
- Object-like macro substitution
- Function-like macro substitution
- Generates `.i` output file

## Files
- `main.c` – Main program
- `comment_removal.c` – Removes comments
- `file_inclusion.c` – Handles file inclusion
- `macro_handler.c` – Handles macro substitution
- Header files – Function declarations
- `makefile` – Builds the project

## Compilation

```bash
make
```

## Execution

```bash
./mypreprocessor test.c
```

## Output

The preprocessed output will be generated as:

```bash
test.i
```

## Language
C Programming
