/*
 * TODO: Split lambda.h / lambda.c into logical modules: definitions, variable
 *       set, substitution, reduction, normalisation.
 *
 *       Eliminate global def_vals and sb. Pass them as parameters or make them
 *       local.
 *
 *       Fix sb_ensure realloc bug (temporary pointer) and add
 *       overflow/zero-size guards.
 *
 *       Protect count_applications from invalid input.
 *
 *       Decouple I/O from reduction. Normalisation should write to a provided
 *       stream or buffer.
 *
 *       Improve memory management. Consider arena allocation for expressions
 *       to reduce overhead and avoid recursive free.
 *
 *       Strengthen const‑correctness. Decide on true immutability and stick to
 *       it.
 *
 *       Address the multiple‑definition issue immediately to avoid linker
 *       errors on some platforms.
 */

#include "../include/expr.h"
#include "../include/lambda.h"
#include "../include/strbuf.h"
#include "../include/types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

expr *def_vals[N_DEFS];
strbuf sb;

int main(cint argc, char *argv[]) {
    char *input = nullptr;
    expr *e = nullptr;
    int status = 1; // Default to error

    // load δ-definitions
    for (int i = 0; i < N_DEFS; i++) {
        Parser dp = {def_src[i], 0, strlen(def_src[i])};
        def_vals[i] = parse(&dp);

        if (!def_vals[i]) {
            fprintf(stderr, "Failed to parse definition: %s\n", def_src[i]);
            goto cleanup;
        }
    }

    sb_init(&sb, MAX_PRINT_LEN);

    if (argc > 1) {
        size_t L = 0;
        for (int i = 1; i < argc; i++) L += strlen(argv[i]) + 1;
        input = malloc(L + 1);

        if (!input) {
            perror("malloc for input");
            goto cleanup;
        }

        input[0] = '\0';

        for (int i = 1; i < argc; i++) {
            strcat(input, argv[i]);
            if (i < argc - 1) strcat(input, " ");
        }
    } else {
        char *buf = nullptr;
        size_t bufsize = 0;

        printf("λ-expr> ");
        const ssize_t chars_read = getline(&buf, &bufsize, stdin);

        if (chars_read == -1) {
            // Error
            free(buf);
            goto cleanup;
        }

        // Remove trailing newline
        if ((chars_read > 0) && (buf[chars_read - 1] == '\n'))
            buf[chars_read - 1] = '\0';

        input = buf;

        if (!input) {
            perror("strdup for input");
            goto cleanup;
        }
    }

    Parser p = {input, 0, strlen(input)};
    e = parse(&p);
    if (!e) goto cleanup;
    normalize(e);
    e = nullptr;  // TODO: Does this actually need to be set to nullptr?

    status = 0;

    cleanup:

    if (input) free(input);
    for (int i = 0; i < N_DEFS; i++) if (def_vals[i]) free_expr(def_vals[i]);
    sb_destroy(&sb);

    return status;
}
