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

#endif //TEST_H
