#include "lambda.h"

expr *def_vals[N_DEFS];
extern strbuf sb;

int main(const int argc, char *argv[]) {
    char *input = NULL;
    expr *e = NULL;
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
        char *buf = NULL;
        size_t bufsize = 0;

        printf("λ-expr> ");
        const ssize_t chars_read = getline(&buf, &bufsize, stdin);

        if (chars_read == -1) {
            // Error
            free(buf);
            goto cleanup;
        }

        // Remove trailing newline
        if ((chars_read > 0) && (buf[chars_read - 1] == '\n')) buf[chars_read - 1] = '\0';

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
    e = NULL;

    status = 0;

    cleanup:

    if (input) free(input);
    for (int i = 0; i < N_DEFS; i++) if (def_vals[i]) free_expr(def_vals[i]);
    sb_destroy(&sb);

    return status;
}
