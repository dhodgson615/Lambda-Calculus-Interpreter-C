# Lambda Calculus Interpreter

This project is a C implementation of a lambda calculus interpreter. It can
parse lambda expressions, perform beta and delta reductions, and normalize
expressions.

## Features

* Parses lambda expressions, including variables, abstractions (λx.M), and
* applications (M N).

* Supports Church numerals: input numbers are automatically converted to their
* Church numeral representation (e.g., `2` becomes `λf.λx.f (f x)`).

* Performs beta reduction: `(λx.M) N → M[x:=N]`

* Performs delta reduction: replaces predefined constants (like `⊤`, `⊥`, `+`,
  `*`, `is_zero`) with their lambda calculus definitions.

* Normalizes expressions by repeatedly applying reduction rules until no more
  reductions are possible.

* Outputs each step of the reduction process.

* Can abstract Church numerals back to their integer representation in the
  final output (e.g., `λf.λx.f (f x)` becomes `2`).

## Building

A `Makefile` is provided for easy compilation.

```bash
make
```

This will produce an executable named `lambda`.

## Usage

You can run the interpreter in two ways:

1.  **Interactive mode:**
    Run the executable without arguments:
    ```bash
    ./lambda
    ```
    You will be prompted to enter a lambda expression:
    ```
    λ-expr> (λx.x) y
    ```
    The interpreter will then show the reduction steps:
    ```
    Step 0: (λx.x) y
    Step 1 (β): y
    → normal form reached.

    δ-abstracted: y
    ```

2.  **Command-line argument:**
    Pass the lambda expression as a command-line argument:
    ```bash
    ./lambda "(λx.x) y"
    ```
    Or for more complex expressions, you might need to quote parts:
    ```bash
    ./lambda "((λm.λn.m (λf.λx.f (n f x)) n) 2) 1"
    ```

### Configuration

The interpreter has a couple of compile-time (actually, runtime, but set at the
top of `lambda.c`) configurations that can be modified in `lambda.c`:

* `CONFIG_SHOW_STEP_TYPE`: (Default: `true`) If `true`, shows the type of
  reduction (β or δ) for each step. If `false`, only shows "Step X: ...".

* `CONFIG_DELTA_ABSTRACT`: (Default: `true`) If `true`, attempts to convert
  Church numerals in the final normal form back to their integer
  representation.

## Predefined Constants (δ-reduction)

The interpreter predefines several common constants:
 
* `⊤` (True): `λx.λy.x`

* `⊥` (False): `λx.λy.y`

* `∧` (And): `λp.λq.p q p`

* `∨` (Or): `λp.λq.p p q`

* `↓` (Successor for Church numerals, used in subtraction): `λn.λf.λx.n
  (λg.λh.h (g f)) (λu.x) (λu.u)` (This is actually the predecessor function)

* `↑` (Successor for Church numerals, used in addition): `λn.λf.λx.f (n f x)`

* `+` (Addition): `λm.λn.m ↑ n`

* `*` (Multiplication): `λm.λn.m (+ n) 0`

* `is_zero`: `λn.n (λx.⊥) ⊤`

* `-` (Subtraction): `λm.λn.n ↓ m`

* `≤` (Less than or equal to): `λm.λn.is_zero (- m n)`

* `pair`: `λx.λy.λf.f x y`

These can be used directly in your lambda expressions.

## Example

Input:
```
λ-expr> (+ 2) 1
```

Output:
```
Step 0: (+ 2) 1
Step 1 (δ): (λm.λn.m ↑ n 2) 1
Step 2 (β): λn.2 ↑ n 1
Step 3 (β): 2 ↑ 1
Step 4 (δ): (λf.λx.f (f x)) ↑ 1
Step 5 (δ): (λf.λx.f (f x)) (λn.λf.λx.f (n f x)) 1
Step 6 (β): (λx.(λn.λf.λx.f (n f x)) ((λn.λf.λx.f (n f x)) x)) 1
Step 7 (β): (λn.λf.λx.f (n f x)) ((λn.λf.λx.f (n f x)) 1)
Step 8 (β): λf.λx.f (((λn.λf.λx.f (n f x)) 1) f x)
Step 9 (β): λf.λx.f ((λf'.λx'.f' (1 f' x')) f x)
Step 10 (δ): λf.λx.f ((λf'.λx'.f' ((λf''.λx''.f'' x'') f' x')) f x)
Step 11 (β): λf.λx.f ((λx'.f ((λf''.λx''.f'' x'') f x')) f x)
Step 12 (β): λf.λx.f (f ((λf''.λx''.f'' x'') f (f x)))
Step 13 (β): λf.λx.f (f ((λx''.f (f x'')) (f x)))
Step 14 (β): λf.λx.f (f (f (f (f x))))
→ normal form reached.

δ-abstracted: 3
```

## Code Structure

* `lambda.h`: Header file defining structures (`Expr`, `Parser`, `VarSet`,
  `strbuf`), enums (`ExprType`), and function prototypes.

* `lambda.c`: Main implementation file containing:

    * Configuration flags.

    * String buffer utilities (`sb_*`).

    * Expression creation and manipulation functions (`make_*`, `free_expr`,
      `copy_expr`, `substitute`, etc.).

    * Church numeral functions (`church`, `is_church_numeral`,
      `count_applications`, `abstract_numerals`).

    * Expression to string conversion (`expr_to_buffer*`).

    * Variable set utilities (`vs_*`, `free_vars`, `fresh_var`).

    * Reduction logic (`delta_reduce`, `beta_reduce`, `reduce_once`,
      `normalize`).

    * Parser logic (`parse*`, `peek`, `consume`, `skip_whitespace`).

    * `main` function: handles input, calls parser and normalizer, and prints
      results.

* `Makefile`: For building the project.

## Cleaning

To remove the compiled executable:
```bash
make clean
```
