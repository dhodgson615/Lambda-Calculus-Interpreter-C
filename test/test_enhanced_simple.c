#include "../include/enhanced_output.h"
#include "../include/expr.h"
#include "../include/types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_enhanced_output_simple() {
    printf("=== Enhanced Output Module Test (Simple) ===\n\n");
    
    /* Create simple expressions manually */
    
    /* Test 1: Simple variable */
    printf("Test 1: Variable 'x'\n");
    expr *var_x = make_variable("x");
    
    char buf[256];
    
    enhanced_expr_to_buffer(var_x, buf, sizeof(buf), OUTPUT_MODE_NORMAL);
    printf("  Normal:   %s\n", buf);
    
    enhanced_expr_to_buffer(var_x, buf, sizeof(buf), OUTPUT_MODE_COLORED);
    printf("  Colored:  %s\n", buf);
    
    enhanced_expr_to_buffer(var_x, buf, sizeof(buf), OUTPUT_MODE_COMPACT);
    printf("  Compact:  %s\n", buf);
    
    enhanced_expr_to_buffer(var_x, buf, sizeof(buf), OUTPUT_MODE_BOTH);
    printf("  Both:     %s\n", buf);
    
    free_expr(var_x);
    printf("\n");
    
    /* Test 2: Simple abstraction λx.x */
    printf("Test 2: Identity function λx.x\n");
    expr *var_x2 = make_variable("x");
    expr *identity = make_abstraction("x", var_x2);
    
    enhanced_expr_to_buffer(identity, buf, sizeof(buf), OUTPUT_MODE_NORMAL);
    printf("  Normal:   %s\n", buf);
    
    enhanced_expr_to_buffer(identity, buf, sizeof(buf), OUTPUT_MODE_COLORED);
    printf("  Colored:  %s\n", buf);
    
    enhanced_expr_to_buffer(identity, buf, sizeof(buf), OUTPUT_MODE_COMPACT);
    printf("  Compact:  %s\n", buf);
    
    enhanced_expr_to_buffer(identity, buf, sizeof(buf), OUTPUT_MODE_BOTH);
    printf("  Both:     %s\n", buf);
    
    free_expr(identity);
    printf("\n");
    
    /* Test 3: Application (λx.x) y */
    printf("Test 3: Application (λx.x) y\n");
    expr *var_x3 = make_variable("x");
    expr *identity2 = make_abstraction("x", var_x3);
    expr *var_y = make_variable("y");
    expr *app = make_application(identity2, var_y);
    
    enhanced_expr_to_buffer(app, buf, sizeof(buf), OUTPUT_MODE_NORMAL);
    printf("  Normal:   %s\n", buf);
    
    enhanced_expr_to_buffer(app, buf, sizeof(buf), OUTPUT_MODE_COLORED);
    printf("  Colored:  %s\n", buf);
    
    enhanced_expr_to_buffer(app, buf, sizeof(buf), OUTPUT_MODE_COMPACT);
    printf("  Compact:  %s\n", buf);
    
    enhanced_expr_to_buffer(app, buf, sizeof(buf), OUTPUT_MODE_BOTH);
    printf("  Both:     %s\n", buf);
    
    free_expr(app);
    printf("\n");
    
    /* Test 4: Nested application ((λx.x) y) z */
    printf("Test 4: Nested application ((λx.x) y) z\n");
    expr *var_x4 = make_variable("x");
    expr *identity3 = make_abstraction("x", var_x4);
    expr *var_y2 = make_variable("y");
    expr *app1 = make_application(identity3, var_y2);
    expr *var_z = make_variable("z");
    expr *nested_app = make_application(app1, var_z);
    
    enhanced_expr_to_buffer(nested_app, buf, sizeof(buf), OUTPUT_MODE_NORMAL);
    printf("  Normal:   %s\n", buf);
    
    enhanced_expr_to_buffer(nested_app, buf, sizeof(buf), OUTPUT_MODE_COLORED);
    printf("  Colored:  %s\n", buf);
    
    enhanced_expr_to_buffer(nested_app, buf, sizeof(buf), OUTPUT_MODE_COMPACT);
    printf("  Compact:  %s\n", buf);
    
    enhanced_expr_to_buffer(nested_app, buf, sizeof(buf), OUTPUT_MODE_BOTH);
    printf("  Both:     %s\n", buf);
    
    free_expr(nested_app);
    printf("\n");
}

int main() {
    test_enhanced_output_simple();
    return 0;
}