#include <stdio.h>
#include <assert.h>
#include "test.h"
#include "../src/expr.h"
#include "../src/lambda.h"
#include "../src/strbuf.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Test framework
#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running %-32s", #name); \
    test_##name(); \
    printf("[ PASS ]\n"); \
} while(0)

// Global variables needed by the main program
expr *def_vals[N_DEFS];
strbuf sb;

// Helper to compare expressions for equality
static bool expr_equal(const expr *e1, const expr *e2) {
    if (!e1 && !e2) return true;
    if (!e1 || !e2) return false;
    if (e1->type != e2->type) return false;

    switch (e1->type) {
        case VAR_expr: return strcmp(e1->var_name, e2->var_name) == 0;
        case ABS_expr: return (strcmp(e1->abs_param, e2->abs_param) == 0) && (expr_equal(e1->abs_body, e2->abs_body));
        case APP_expr: return (expr_equal(e1->app_fn, e2->app_fn)) && (expr_equal(e1->app_arg, e2->app_arg));
    }
    return false;
}

// Initialize delta definitions
static void setup_delta_defs(void) {
    for (int i = 0; i < N_DEFS; i++) {
        Parser dp = {def_src[i], 0, strlen(def_src[i])};
        def_vals[i] = parse(&dp);
        assert(def_vals[i] != NULL);
    }
}

// Cleanup delta definitions
static void cleanup_delta_defs(void) {
    for (int i = 0; i < N_DEFS; i++) {
        if (def_vals[i]) {
            free_expr(def_vals[i]);
            def_vals[i] = NULL;
        }
    }
}

// Test expression creation and manipulation
TEST(expr_creation) {
    // Test variable
    expr *var = make_variable("x");
    assert(var != NULL);
    assert(var->type == VAR_expr);
    assert(strcmp(var->var_name, "x") == 0);

    // Test abstraction
    expr *body = make_variable("y");
    expr *abs = make_abstraction("x", body);
    assert(abs != NULL);
    assert(abs->type == ABS_expr);
    assert(strcmp(abs->abs_param, "x") == 0);
    assert(expr_equal(abs->abs_body, body));

    // Test application
    expr *fn = make_variable("f");
    expr *arg = make_variable("a");
    expr *app = make_application(fn, arg);
    assert(app != NULL);
    assert(app->type == APP_expr);
    assert(expr_equal(app->app_fn, fn));
    assert(expr_equal(app->app_arg, arg));

    free_expr(var);
    free_expr(abs); // This frees body too
    free_expr(app); // This frees fn and arg too
}

// Test copying expressions
TEST(expr_copy) {
    expr *var = make_variable("x");
    expr *copy = copy_expr(var);
    assert(expr_equal(var, copy));
    assert(var != copy);

    free_expr(var);
    free_expr(copy);
}

// Test parsing
TEST(parsing) {
    // Test variable
    const char *input1 = "x";
    Parser p1 = {input1, 0, strlen(input1)};
    expr *e1 = parse(&p1);
    assert(e1->type == VAR_expr);
    assert(strcmp(e1->var_name, "x") == 0);

    // Test abstraction (lambda x.x)
    const char *input2 = "λx.x";
    Parser p2 = {input2, 0, strlen(input2)};
    expr *e2 = parse(&p2);
    assert(e2->type == ABS_expr);
    assert(strcmp(e2->abs_param, "x") == 0);

    // Test application (f x)
    const char *input3 = "f x";
    Parser p3 = {input3, 0, strlen(input3)};
    expr *e3 = parse(&p3);
    assert(e3->type == APP_expr);

    free_expr(e1);
    free_expr(e2);
    free_expr(e3);
}

// Test Church numerals
TEST(church_numerals) {
    for (int i = 0; i < 5; i++) {
        expr *c = church(i);
        assert(is_church_numeral(c));
        assert(count_applications(c) == i);
        free_expr(c);
    }
}

// Test variable sets
TEST(var_sets) {
    VarSet s;
    vs_init(&s);

    vs_add(&s, "x");
    vs_add(&s, "y");
    vs_add(&s, "z");

    assert(vs_has(&s, "x"));
    assert(vs_has(&s, "y"));
    assert(!vs_has(&s, "w"));

    vs_rm(&s, "y");
    assert(!vs_has(&s, "y"));

    vs_free(&s);
}

// Test free variables calculation
TEST(free_vars) {
    // λx.y x has free variable y
    expr *var_y = make_variable("y");
    expr *var_x = make_variable("x");
    expr *app = make_application(var_y, var_x);
    expr *abs = make_abstraction("x", app);

    VarSet fv = free_vars(abs);
    assert(fv.c == 1);
    assert(vs_has(&fv, "y"));
    assert(!vs_has(&fv, "x"));

    vs_free(&fv);
    free_expr(abs);
}

// Test substitution
TEST(substitution) {
    // Test (λx.x)[y := z] = λx.x (no substitution)
    expr *id = make_abstraction("x", make_variable("x"));
    expr *z = make_variable("z");
    expr *result = substitute(id, "y", z);

    assert(result->type == ABS_expr);
    assert(strcmp(result->abs_param, "x") == 0);
    assert(result->abs_body->type == VAR_expr);
    assert(strcmp(result->abs_body->var_name, "x") == 0);

    free_expr(id);
    free_expr(z);
    free_expr(result);
}

// Test beta reduction
TEST(beta_reduction) {
    // (λx.x) y -> y
    expr *id = make_abstraction("x", make_variable("x"));
    expr *y = make_variable("y");
    expr *app = make_application(id, y);

    expr *result;
    bool reduced = beta_reduce(app, &result);

    assert(reduced);
    assert(result->type == VAR_expr);
    assert(strcmp(result->var_name, "y") == 0);

    free_expr(app);
    free_expr(result);
}

// Test delta reduction
TEST(delta_reduction) {
    setup_delta_defs();

    expr *true_var = make_variable("true");
    expr *result;
    const bool reduced = delta_reduce(true_var, &result);

    assert(reduced);
    assert(result->type == ABS_expr);

    free_expr(true_var);
    free_expr(result);

    cleanup_delta_defs();
}

// Test string buffer functionality
TEST(string_buffer) {
    strbuf buffer;
    sb_init(&buffer, 10);

    sb_ensure(&buffer, 100);
    assert(buffer.cap >= 110);

    sb_reset(&buffer);
    assert(buffer.len == 0);

    sb_destroy(&buffer);
}

// Test expression to string conversion
TEST(expr_to_buffer) {
    expr *var = make_variable("x");
    char buf[100];
    expr_to_buffer(var, buf, sizeof(buf));
    assert(strcmp(buf, "x") == 0);
    free_expr(var);
}

// Integration test for normalization
TEST(normalization) {
    sb_init(&sb, 1024);
    setup_delta_defs();

    // Test (λx.x) y -> y
    const char *input = "(λx.x) y";
    Parser p = {input, 0, strlen(input)};
    expr *e = parse(&p);

    // Capture and redirect stdout
    FILE *original_stdout = stdout;
    FILE *temp = tmpfile();
    stdout = temp;

    normalize(e); // note: this frees e

    // Restore stdout
    fflush(stdout);
    stdout = original_stdout;

    // Check results
    rewind(temp);
    char line[1024];
    bool found_normal_form = false;
    while (fgets(line, sizeof(line), temp)) {
        if (strstr(line, "normal form reached")) {
            found_normal_form = true;
            break;
        }
    }
    fclose(temp);

    assert(found_normal_form);

    cleanup_delta_defs();
    sb_destroy(&sb);
}

int main(void) {
    printf("\n==== Lambda Calculus Test Suite ====\n\n");

    RUN_TEST(expr_creation);
    RUN_TEST(expr_copy);
    RUN_TEST(parsing);
    RUN_TEST(church_numerals);
    RUN_TEST(var_sets);
    RUN_TEST(free_vars);
    RUN_TEST(substitution);
    RUN_TEST(beta_reduction);
    RUN_TEST(delta_reduction);
    RUN_TEST(string_buffer);
    RUN_TEST(expr_to_buffer);
    RUN_TEST(normalization);

    printf("\n==== All tests passed successfully. ====\n\n");
    return 0;
}
