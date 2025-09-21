# Enhanced Output Module

This module provides **colored parentheses** and **compact mode** functionality for the Lambda Calculus Interpreter. It can be enabled via compiler directives without modifying existing source files.

## Features

### 1. Colored Parentheses
- Each nesting level uses a different color
- 12 different colors cycle through: Red, Green, Yellow, Blue, Magenta, Cyan, and their bright variants
- Uses ANSI color codes for terminal display

### 2. Compact Mode
- Removes unnecessary parentheses for cleaner output
- Maintains mathematical correctness while improving readability
- Especially useful for deeply nested expressions

### 3. Drop-in Replacement
- Can replace existing `expr_to_buffer` calls without code modification
- Activated entirely through compiler directives
- Zero impact when disabled

## Usage

### Basic Compilation

To enable the enhanced output module, add these compiler flags:

```bash
# Enable colored parentheses only
gcc -DENABLE_ENHANCED_OUTPUT -DENHANCED_OUTPUT_MODE=1 ...

# Enable compact mode only  
gcc -DENABLE_ENHANCED_OUTPUT -DENHANCED_OUTPUT_MODE=2 ...

# Enable both colored parentheses and compact mode
gcc -DENABLE_ENHANCED_OUTPUT -DENHANCED_OUTPUT_MODE=3 ...
```

### Mode Options

| Mode | Value | Description |
|------|-------|-------------|
| Normal | 0 | Standard parentheses, full format (default) |
| Colored | 1 | Colored parentheses only |
| Compact | 2 | Compact formatting only |
| Both | 3 | Colored parentheses + compact formatting |

### Examples

#### Standard Output (without enhancement):
```
(λx.x) y
(λf.λx.f (f x))
((λx.λy.x) a) b
```

#### Colored Mode:
```
([31m(λx.x)[0m) y                    # Red parentheses
([31mλf.[32mλx.f ([33mf x[33m)[32m[31m) # Nested colors
```

#### Compact Mode:
```
(λx.x) y
λf.λx.f (f x)
(λx.λy.x) a b
```

#### Both Modes:
```
([31mλx.x[31m) y                     # Colored + compact
[31mλf.λx.f ([32mf x[32m)[31m        # Clean and colorized
```

## Integration

### With Existing Code

The module automatically replaces `expr_to_buffer` calls when enabled:

```c
#include "expr.h"

// This call will use enhanced output if enabled at compile time
expr_to_buffer(expression, buffer, buffer_size);
```

### Direct Usage

You can also call the enhanced functions directly:

```c
#include "enhanced_output.h"

enhanced_expr_to_buffer(expr, buf, cap, OUTPUT_MODE_COLORED);
enhanced_expr_to_buffer(expr, buf, cap, OUTPUT_MODE_COMPACT); 
enhanced_expr_to_buffer(expr, buf, cap, OUTPUT_MODE_BOTH);
```

## Files Added

- `include/enhanced_output.h` - Core enhanced output interface
- `src/enhanced_output.c` - Implementation of colored and compact formatting
- `include/enhanced_wrapper.h` - Drop-in replacement wrapper
- `test/test_enhanced_simple.c` - Test suite for enhanced functionality

## Building Examples

```bash
# Build with colored parentheses
make CFLAGS="-DENABLE_ENHANCED_OUTPUT -DENHANCED_OUTPUT_MODE=1" 

# Build with compact mode
make CFLAGS="-DENABLE_ENHANCED_OUTPUT -DENHANCED_OUTPUT_MODE=2"

# Build with both features
make CFLAGS="-DENABLE_ENHANCED_OUTPUT -DENHANCED_OUTPUT_MODE=3"

# Build without enhancement (default)
make
```

## Testing

Run the test suite to see all modes in action:

```bash
# Compile and run tests
gcc -std=c11 -Iinclude -D_GNU_SOURCE test/test_enhanced_simple.c src/enhanced_output.c src/expr.c src/strbuf.c -o test_enhanced
./test_enhanced
```

## Terminal Compatibility

The colored output works best with:
- Modern terminal emulators (xterm, gnome-terminal, iTerm2, etc.)
- Terminals that support ANSI color codes
- Can be safely used in pipes (colors are filtered out automatically)

## Performance

- Minimal overhead when disabled (compile-time conditional)
- Colored mode adds small overhead for color code insertion
- Compact mode may actually improve performance by reducing output size