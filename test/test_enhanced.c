#include "../include/enhanced_output.h"
#include "../include/expr.h"
#include "../include/parser.h"
#include "../include/strbuf.h"
#include "../include/types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test expressions */
static const char* test_expressions[] = {
    "x",
    "λx.x",
    "(λx.x) y",
    "λf.λx.f (f x)",
    "(λx.λy.x) a b",
    "((λx.x) y) z",
    "λf.λg.λx.f (g x)",
    NULL
};

void test_enhanced_output() {
    printf("=== Enhanced Output Module Test ===\n\n");
    
    for (int i = 0; test_expressions[i]; i++) {
        const char* input = test_expressions[i];
        printf("Input: %s\n", input);
        
        Parser p = {input, 0, strlen(input)};
        expr *e = parse(&p);
        
        if (!e) {
            printf("  Parse error!\n\n");
            continue;
        }
        
        char buf[512];
        
        /* Normal mode */
        enhanced_expr_to_buffer(e, buf, sizeof(buf), OUTPUT_MODE_NORMAL);
        printf("  Normal:   %s\n", buf);
        
        /* Colored mode */
        enhanced_expr_to_buffer(e, buf, sizeof(buf), OUTPUT_MODE_COLORED);
        printf("  Colored:  %s\n", buf);
        
        /* Compact mode */
        enhanced_expr_to_buffer(e, buf, sizeof(buf), OUTPUT_MODE_COMPACT);
        printf("  Compact:  %s\n", buf);
        
        /* Both modes */
        enhanced_expr_to_buffer(e, buf, sizeof(buf), OUTPUT_MODE_BOTH);
        printf("  Both:     %s\n", buf);
        
        free_expr(e);
        printf("\n");
    }
}

int main() {
    test_enhanced_output();
    return 0;
}