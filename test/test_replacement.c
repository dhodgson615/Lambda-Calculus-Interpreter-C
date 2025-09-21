#include "../include/expr.h"
#include "../include/enhanced_output.h"

/* Test if the macro override is working */
#ifdef ENABLE_ENHANCED_OUTPUT
#include "../include/enhanced_wrapper.h"
#endif

#include <stdio.h>
#include <stdlib.h>

/* Simple test program to demonstrate drop-in replacement */
int main() {
    printf("=== Drop-in Replacement Test ===\n\n");
    
    /* Create a test expression manually */
    expr *var_x = make_variable("x");
    expr *var_y = make_variable("y");
    expr *identity = make_abstraction("x", var_x);
    expr *app = make_application(identity, var_y);
    
    char buf[256];
    
    /* Test original function */
    printf("Using original expr_to_buffer:\n");
    expr_to_buffer(app, buf, sizeof(buf));
    printf("  Result: %s\n\n", buf);
    
    /* Test enhanced functions */
    printf("Using enhanced functions directly:\n");
    
    enhanced_expr_to_buffer(app, buf, sizeof(buf), OUTPUT_MODE_NORMAL);
    printf("  Normal:   %s\n", buf);
    
    enhanced_expr_to_buffer(app, buf, sizeof(buf), OUTPUT_MODE_COLORED);
    printf("  Colored:  %s\n", buf);
    
    enhanced_expr_to_buffer(app, buf, sizeof(buf), OUTPUT_MODE_COMPACT);
    printf("  Compact:  %s\n", buf);
    
    enhanced_expr_to_buffer(app, buf, sizeof(buf), OUTPUT_MODE_BOTH);
    printf("  Both:     %s\n", buf);
    
    free_expr(app);
    
    return 0;
}