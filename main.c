#include "lambda.h"

// External references to global variables defined in lambda.c
expr *def_vals[N_DEFS];
extern strbuf sb;

int main(const int argc, char *argv[]) {
    // load δ-definitions
    for (int i = 0; i < N_DEFS; i++) {
        Parser dp = {def_src[i], 0, strlen(def_src[i])};
        def_vals[i] = parse(&dp);
        if (!def_vals[i]) {
            fprintf(stderr, "Failed to parse definition: %s\n", def_src[i]);
            for (int k = 0; k < i; k++) free_expr(def_vals[k]);
            sb_destroy(&sb);
            exit(1);
        }
    }
    sb_init(&sb, MAX_PRINT_LEN);
    char *input;
    if (argc > 1) {
        size_t L = 0;
        for (int i = 1; i < argc; i++) L += strlen(argv[i]) + 1;
        input = malloc(L + 1);
        if (!input) {
            perror("malloc for input");
            // Cleanup def_vals
            for (int i = 0; i < N_DEFS; i++) free_expr(def_vals[i]);
            sb_destroy(&sb);
            exit(1);
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

        if (chars_read == -1) { // Error
            // Cleanup
            free(buf);
            for (int i = 0; i < N_DEFS; i++) free_expr(def_vals[i]);
            sb_destroy(&sb);
            exit(1);
        }

        // Remove trailing newline
        if ((chars_read > 0) && (buf[chars_read - 1] == '\n')) buf[chars_read - 1] = '\0';

        input = buf;
        if (!input) {
            perror("strdup for input");

            // Cleanup def_vals
            for (int i = 0; i < N_DEFS; i++) free_expr(def_vals[i]);
            sb_destroy(&sb);
            exit(1);
        }
    }

    Parser p = {input, 0, strlen(input)};
    expr *e = parse(&p);
    if (!e) {
        free(input);
        for (int i = 0; i < N_DEFS; i++) free_expr(def_vals[i]);
        sb_destroy(&sb);
        exit(1);
    }
    normalize(e);

    // cleanup
    free(input);
    for (int i = 0; i < N_DEFS; i++) free_expr(def_vals[i]);
    sb_destroy(&sb);

    return 0;
}
