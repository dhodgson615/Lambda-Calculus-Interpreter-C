# Lambda-Calculus-Interpreter-C

This project is a C implementation of a lambda calculus interpreter that I previously made in
[Python](https://github.com/dhodgson615/Lambda-Calculus-Interpreter). It has most of the same features and
works largely the same way, including prefix math. Sorry.

## Features

* Parses lambda expressions, including variables, abstractions (λx.M), and applications (M N).

* Supports Church numerals: input numbers are automatically converted to their Church numeral
  representation (e.g., `2` becomes `λf.λx.f (f x)`).

* Performs beta reduction: `(λx.M) N → M[x:=N]`

* Performs delta reduction: replaces predefined constants (like `⊤`, `⊥`, `+`, `*`, `is_zero`) with their
  lambda calculus definitions.

* Normalizes expressions by repeatedly applying reduction rules until no more reductions are possible.

* Outputs each step of the reduction process.

* Can abstract Church numerals back to their integer representation in the final output
  (e.g., `λf.λx.f (f x)` becomes `2`).

## Install

Clone and compile:

```bash
git clone https://github.com/dhodgson615/Lambda-Calculus-Interpreter-C.git
cd Lambda-Calculus-Interpreter-C
make
./lambda "+ 1 1"
```

You should see similar output to this if you paste the above into a Unix terminal:

```
Cloning into 'Lambda-Calculus-Interpreter-C'...
remote: Enumerating objects: 148, done.
remote: Counting objects: 100% (148/148), done.
remote: Compressing objects: 100% (128/128), done.
remote: Total 148 (delta 83), reused 38 (delta 18), pack-reused 0 (from 0)
Receiving objects: 100% (148/148), 52.10 KiB | 2.17 MiB/s, done.
Resolving deltas: 100% (83/83), done.
gcc -std=c17 -Wall -Wextra -Werror -pedantic -O3 -march=native -flto -mtune=native -funroll-loops lambda.c -o lambda
Step 0: + (λf.(λx.f x)) (λf.(λx.f x))
Step 1 (δ): (λm.(λn.m ↑ n)) (λf.(λx.f x)) (λf.(λx.f x))
Step 2 (β): (λn.(λf.(λx.f x)) ↑ n) (λf.(λx.f x))
Step 3 (β): (λf.(λx.f x)) ↑ (λf.(λx.f x))
Step 4 (β): (λx.↑ x) (λf.(λx.f x))
Step 5 (β): ↑ (λf.(λx.f x))
Step 6 (δ): (λn.(λf.(λx.f (n f x)))) (λf.(λx.f x))
Step 7 (β): λf.(λx.f ((λf.(λx.f x)) f x))
Step 8 (β): λf.(λx.f ((λx.f x) x))
Step 9 (β): λf.(λx.f (f x))

→ normal form reached.

δ-abstracted: 2
```

## Usage

You can run the interpreter in two ways:

1.  **Without Arguments**

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

2.  **With Arguments**

    Pass the lambda expression as a command-line argument:
    ```bash
    ./lambda "(λx.x) y"
    ```

    ```bash
    ./lambda "((λm.λn.m (λf.λx.f (n f x)) n) 2) 1"
    ```

### Configuration

The interpreter has a couple of compile-time (actually, runtime, but set at the top of `lambda.c`)
configurations that can be modified in `lambda.c`:

* `CONFIG_SHOW_STEP_TYPE`: (Default: `true`) If `true`, shows the type of reduction (β or δ) for each
  step. If `false`, only shows "Step X: ...".

* `CONFIG_DELTA_ABSTRACT`: (Default: `true`) If `true`, attempts to convert Church numerals in the final
  normal form back to their integer representation.

## Predefined Constants (δ-reduction)

The interpreter predefines several common constants:

* `⊤` (True): `λx.λy.x`

* `⊥` (False): `λx.λy.y`

* `∧` (And): `λp.λq.p q p`

* `∨` (Or): `λp.λq.p p q`

* `↓` (Predecessor for Church numerals, used in subtraction): `λn.λf.λx.n (λg.λh.h (g f)) (λu.x) (λu.u)`

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
Step 0: + (λf.(λx.f (f x))) (λf.(λx.f x))
Step 1 (δ): (λm.(λn.m ↑ n)) (λf.(λx.f (f x))) (λf.(λx.f x))
Step 2 (β): (λn.(λf.(λx.f (f x))) ↑ n) (λf.(λx.f x))
Step 3 (β): (λf.(λx.f (f x))) ↑ (λf.(λx.f x))
Step 4 (β): (λx.↑ (↑ x)) (λf.(λx.f x))
Step 5 (β): ↑ (↑ (λf.(λx.f x)))
Step 6 (δ): (λn.(λf.(λx.f (n f x)))) (↑ (λf.(λx.f x)))
Step 7 (β): λf.(λx.f (↑ (λf.(λx.f x)) f x))
Step 8 (δ): λf.(λx.f ((λn.(λf.(λx.f (n f x)))) (λf.(λx.f x)) f x))
Step 9 (β): λf.(λx.f ((λf.(λx.f ((λf.(λx.f x)) f x))) f x))
Step 10 (β): λf.(λx.f ((λx.f ((λf.(λx.f x)) f x)) x))
Step 11 (β): λf.(λx.f (f ((λf.(λx.f x)) f x)))
Step 12 (β): λf.(λx.f (f ((λx.f x) x)))
Step 13 (β): λf.(λx.f (f (f x)))
→ normal form reached.

δ-abstracted: 3
```

## Code Structure

* `lambda.h`: Header file defining structures (`Expr`, `Parser`, `VarSet`, `strbuf`), enums (`ExprType`),
  and function prototypes.

* `lambda.c`: Main implementation file containing:

    * Configuration flags.

    * String buffer utilities (`sb_*`).

    * Expression creation and manipulation functions (`make_*`, `free_expr`, `copy_expr`, `substitute`,
      etc.).

    * Church numeral functions (`church`, `is_church_numeral`, `count_applications`, `abstract_numerals`).

    * Expression to string conversion (`expr_to_buffer*`).

    * Variable set utilities (`vs_*`, `free_vars`, `fresh_var`).

    * Reduction logic (`delta_reduce`, `beta_reduce`, `reduce_once`, `normalize`).

    * Parser logic (`parse*`, `peek`, `consume`, `skip_whitespace`).

    * `main` function: handles input, calls parser and normalizer, and prints results.

* `Makefile`: For building the project.

## Cleaning

To remove the compiled executable:
```bash
make clean
```

## Metrics for Both Interpreters

| Metric                       | C Version      | Python Version  | Ratio (Python / C) | Increase over C |
|:-----------------------------|---------------:|----------------:|-------------------:|----------------:|
| Real time (wall-clock)       |          3.60s |          45.81s |             12.72x |      +1,172.50% |
| User CPU time                |          2.09s |          34.76s |             16.63x |      +1,563.16% |
| System CPU time              |          0.30s |           6.74s |             22.47x |      +2,146.67% |
| Max RSS (peak resident set)  |     2,490,368B |    137,674,752B |             55.28x |      +5,428.29% |
| Peak memory footprint        |     1,655,616B |    115,574,528B |             69.81x |      +6,880.76% |
| Page reclaims                |            354 |       1,809,444 |          5,111.42x |    +511,042.37% |
| Page faults                  |              1 |               8 |              8.00x |        +700.00% |
| Voluntary context switches   |         91,206 |       1,059,633 |             11.62x |      +1,061.80% |
| Involuntary context switches |             20 |           4,172 |            208.60x |     +20,760.00% |
| Instructions retired         | 39,850,552,397 | 733,017,604,485 |             18.39x |      +1,739.42% |
| Cycles elapsed               |  8,801,869,644 | 151,602,293,038 |             17.22x |      +1,622.39% |

All of these benchmarks are from my M3 MacBook Air. They're also outdated by a couple of versions.
