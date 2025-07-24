#ifndef TEST_H
#define TEST_H

#include "../include/expr.h"
#include "../include/types.h"

#include <stdbool.h>

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do {     \
printf("Running %-32s", #name); \
test_##name();                  \
printf("[ PASS ]\n");           \
} while(0)

// Function declarations for tests
void vs_init(VarSet *s);
bool vs_has(const VarSet *s, const char *x);
void vs_add(VarSet *s, const char *x);
void vs_rm(VarSet *s, const char *x);
void vs_free(const VarSet *s);

VarSet free_vars(const expr *e);
char *fresh_var(const VarSet *s);

expr *substitute(expr *e, const char *v, expr *val);
bool beta_reduce(const expr *e, expr **out);
bool delta_reduce(const expr *e, expr **out);
bool reduce_once(const expr *e, expr **ne, const char **rtype);

#endif //TEST_H
