# Call Tree

A small C tool that scans C source files and prints a function's source code
together with all project-local functions it calls recursively.

## What it does

Given:
- an entry C file
- a function name

`call tree`:
- discovers project-local files from `#include "..."` directives
- lexes and indexes function definitions
- finds the requested function
- recursively follows calls to other known functions
- prints the source of each discovered function

## Current scope

This project is intentionally lightweight and uses a custom lexer instead of a
full C parser.

It works best on regular C code and is currently aimed at project-local
analysis.

## Limitations

This is not a full C parser, so some cases may be missed or approximated:

- macro-generated calls
- function pointers
- conditional compilation branches
- unusual declarations
- compiler-specific extensions

## Planned features

- recursive discovery of local includes
- indexing across multiple `.c` and `.h` files
- recursive function call tracing
- duplicate/cycle protection
- optional call tree output

## Example

```bash
call_tree src/main.c my_function
