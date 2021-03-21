
# simple-calculator
A simple text-based calculator. Gets input either from the standard input or the file in argument. Prints output to standard output.

## Features
- Supports `+`, `-`, `*`, `/`, and `^` operators
- Supports `sin`, `cos`, `tan`, `log`, `exp`, and `abs` functions.
- Result of the last expression can be accessed with `%`
- Has a memory of 10 numbers. Can be accessed with `%<index>` and set with `set <index> <value>`

## Requirements
- `g++`

## Building
```
git clone https://github.com/omarr09/simple-calculator
cd https://github.com/omarr09/simple-calculator
g++ src/main.cpp -o simple-calculator -std=c++17
```

## Usage
To use standard input as the input
```
simple-calculator
```
To use a file as the input
```
simple-calculator <file-path>
```