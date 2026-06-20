# Call Tree

A small C tool that scans an entry source file, recursively follows local
`#include "..."` directives, finds a target function, discovers project-local
functions it calls, and prints the relevant source code.

## What it does

Given:
- an entry C file
- a function name

`call_tree`:

- resolves the entry file to an absolute normalized path
- recursively discovers local includes from `#include "..."` directives
- lexes all discovered files with a lightweight custom lexer
- indexes:
  - function definitions
  - function calls
  - `#define` macro definitions
- finds the requested entry function by name
- recursively follows calls to other known functions
- avoids re-processing functions that were already visited
- prints:
  - macro definitions used by the selected functions
  - all reachable local function definitions

## Usage

```bash
call_tree [-o output_file] entry_file.c function_name
```

## Arguments

- `entry_file.c` — the starting file
- `function_name` — the function to trace
- `-o output_file`, `--output output_file` — write extracted output to a file

If `-o` / `--output` is omitted, extracted output is written to stdout.

If the output path is `-`, output also goes to stdout.

## Examples

```bash
call_tree src/main.c my_function
```

```bash
call_tree -o extracted.c src/main.c my_function
```

```bash
call_tree --output extracted.c src/main.c my_function
```

## What gets printed

The extracted output contains:

1. macro definitions referenced by the discovered functions
2. the discovered function definitions

Each printed item is separated by a blank line.

Functions are printed only once, even if they are reached from multiple call
paths or recursive cycles.

The current implementation prints function definitions in reverse discovery
order, which usually means callees appear before the entry function.

## Include handling

The tool currently follows only project-local quoted includes:

```c
#include "my_header.h"
```

It does not resolve system includes such as:

```c
#include <stdio.h>
```

Include paths are resolved relative to the file containing the directive and
then normalized.

## How function discovery works

This project uses a custom lexer, not a full C parser.

A function definition is recognized approximately as:

- identifier
- `(`
- matching `)`
- `{`
- matching `}`

A function call is recognized approximately as:

- identifier
- `(`
- matching `)`

A call is treated as project-local only if its name matches a known function
definition found in the scanned files.

## Macro handling

The tool indexes `#define` preprocessor directives and prints only macros whose
names appear inside the selected function bodies.

This helps when extracted functions depend on local macros.

## Current scope

This project is intentionally lightweight and works best on regular C code where:

- source files are reachable through recursive local includes
- function definitions use conventional syntax
- calls are explicit identifier-based calls

## Limitations

This is not a full C parser, so some cases may be missed or approximated:

- macro-generated function definitions or calls
- function pointers
- indirect calls
- conditional compilation branches
- unusual declarators
- compiler-specific extensions
- system include resolution
- code not reachable through recursive local `#include "..."` discovery
- multiple functions with the same name in different files
- identifier-plus-parentheses patterns that are not actually function calls in
  all edge cases

## Implemented features

- recursive discovery of local includes
- indexing across multiple discovered files
- recursive function call tracing
- duplicate / cycle protection for functions
- extraction of referenced macro definitions
- optional file output with `-o` / `--output`

## Notes

- The tool operates on tokens, not an AST.
- The entry function is matched by name only.
- The program also prints diagnostic/info messages during execution.
  If you want clean extracted code, prefer using `-o output_file`.