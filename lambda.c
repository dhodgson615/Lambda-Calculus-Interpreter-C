#include "lambda.h"

static strbuf sb;

static bool CONFIG_SHOW_STEP_TYPE = true;
static bool CONFIG_DELTA_ABSTRACT = true;
//static bool COLOR_PARENS        = true;  /* TODO: implement based on the */
                                           /* functionality in the Python  */
                                           /* version                      */

/* Python functions needed:

def rgb(r: int, g: int, b: int) -> str:
    """Return ANSI SGR sequence for RGB color."""
    return f"{ESC}38;2;{r};{g};{b}m"

def apply_color(depth: int, max_depth: int, ch: str) -> str:
    """Apply color to a character based on the current depth and max depth."""
    if max_depth > 1:
        ratio: float = (depth - 1) / (max_depth - 1)
    else:
        ratio = 0

    r: int = int(0 * (1 - ratio) + 0 * ratio)
    g: int = int(128 * (1 - ratio) + 255 * ratio)
    b: int = int(128 * (1 - ratio) + 255 * ratio)

    return rgb(r, g, b) + ch + RESET


def color_parens(string: str) -> str:
    """Color parentheses by nesting level."""
    if not COLOR_PARENS:
        return string

    # first pass: find max nesting depth
    depth: int = 0
    max_depth: int = 0
    for char in string:
        if char == "(":
            depth += 1
            max_depth = max(max_depth, depth)
        elif char == ")":
            depth -= 1

    # second pass: insert ANSI colors
    result: str = ""
    depth = 0
    for char in string:
        if char == "(":
            depth += 1
            result += apply_color(depth, max_depth, char)
        elif char == ")":
            result += apply_color(depth, max_depth, char)
            depth -= 1
        else:
            result += char

    return result

 */

inline bool get_config_show_step_type(void) { return CONFIG_SHOW_STEP_TYPE; }

inline void set_config_show_step_type(bool value) { CONFIG_SHOW_STEP_TYPE = value; }

inline bool get_config_delta_abstract(void) { return CONFIG_DELTA_ABSTRACT; }

inline void set_config_delta_abstract(bool value) { CONFIG_DELTA_ABSTRACT = value; }

//char *rgb(uint8 r, uint8 g, uint8 b) { //FIXME: buffer handling
//    char buf[32];
//    snprintf(buf, 32, "\033[38;2;%d;%d;%dm", r, g, b);
//    return buf;
//}

void sb_init(strbuf *sb, size_t init_cap) {
    sb->data = malloc(init_cap);
    if (!sb->data) {
        perror("malloc");
        exit(1);
    }
    sb->cap = init_cap;
    sb->len = 0;
    sb->data[0] = '\0';
}

void sb_ensure(strbuf *sb, size_t need) {
    if (sb->len + need + 1 > sb->cap) {
        size_t new = sb->cap * 2;
        while (new < sb->len + need + 1) new *= 2;

        sb->data = realloc(sb->data, new);
        if (!sb->data) {
            perror("realloc");
            exit(1);
        }
        sb->cap = new;
    }
}

void sb_reset(strbuf *sb) {
    sb->len = 0;
    sb->data[0] = '\0';
}

void sb_destroy(strbuf *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->cap = sb->len = 0;
}

expr *make_variable(const char *n) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = VAR_expr;
    e->var_name = strdup(n);

    return e;
}

expr *make_abstraction(const char *p, expr *b) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = ABS_expr;
    e->abs_param = strdup(p);
    e->abs_body = b;

    return e;
}

expr *make_application(expr *f, expr *a) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = APP_expr;
    e->app_fn = f;
    e->app_arg = a;

    return e;
}

void free_expr(expr *e) {
    if (!e) return;

    switch (e->type) {
        case VAR_expr:
            free(e->var_name);
            break;
        case ABS_expr:
            free(e->abs_param);
            free_expr(e->abs_body);
            break;
        case APP_expr:
            free_expr(e->app_fn);
            free_expr(e->app_arg);
            break;
    }
    free(e);
}

expr *copy_expr(expr *e) {
    if (!e) return NULL;

    switch (e->type) {
        case VAR_expr:
            return make_variable(e->var_name);
        case ABS_expr:
            return make_abstraction(e->abs_param, copy_expr(e->abs_body));
        case APP_expr:
            return make_application(copy_expr(e->app_fn), copy_expr(e->app_arg));
    }

    return NULL; // unreachable
}

expr *church(int n) {
    expr *body = make_variable("x");
    for (int i = 0; i < n; i++) {
        expr *fv = make_variable("f");
        body = make_application(fv, body);
    }
    expr *abs_x = make_abstraction("x", body);

    return make_abstraction("f", abs_x);
}

void expr_to_buffer_rec(expr *e, char *buf, size_t *pos, size_t cap) {
    if (*pos >= cap - 1) return;

    switch (e->type) {
        case VAR_expr: {
            size_t L = strlen(e->var_name);
            if (*pos + L > cap - 1) L = cap - 1 - *pos;
            memcpy(buf + *pos, e->var_name, L);
            *pos += L;
            break;
        }

        case ABS_expr: {
            // UTF-8 'λ' = 0xCE 0xBB
            if (*pos + 2 < cap - 1) {
                buf[(*pos)++] = (char) 0xCE;
                buf[(*pos)++] = (char) 0xBB;
            }
            size_t L = strlen(e->abs_param);
            if (*pos + L > cap - 1) L = cap - 1 - *pos;
            memcpy(buf + *pos, e->abs_param, L);
            *pos += L;
            if (*pos < cap - 1) buf[(*pos)++] = '.';
            if (e->abs_body->type == ABS_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->abs_body, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->abs_body, buf, pos, cap);
            break;
        }

        case APP_expr: {
            if (e->app_fn->type == ABS_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->app_fn, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->app_fn, buf, pos, cap);
            if (*pos < cap - 1) buf[(*pos)++] = ' ';
            if (e->app_arg->type != VAR_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->app_arg, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->app_arg, buf, pos, cap);
            break;
        }
    }
}

void expr_to_buffer(expr *e, char *buf, size_t cap) {
    size_t pos = 0;
    expr_to_buffer_rec(e, buf, &pos, cap);
    buf[pos < cap ? pos : cap - 1] = '\0';
}

void vs_init(VarSet *s) {
    s->v = NULL;
    s->c = 0;
}

bool vs_has(VarSet *s, const char *x) {
    for (int i = 0; i < s->c; i++) if (!strcmp(s->v[i], x)) return true;

    return false;
}

void vs_add(VarSet *s, const char *x) {
    if (vs_has(s, x)) return;
    s->v = realloc(s->v, sizeof(char *) * (s->c + 1));
    s->v[s->c++] = strdup(x);
}

void vs_rm(VarSet *s, const char *x) {
    for (int i = 0; i < s->c; i++) {
        if (!strcmp(s->v[i], x)) {
            free(s->v[i]);
            memmove(&s->v[i], &s->v[i + 1], sizeof(char *) * (s->c - i - 1));
            s->c--;
            return;
        }
    }
}

void vs_free(VarSet *s) {
    for (int i = 0; i < s->c; i++) free(s->v[i]);
    free(s->v);
}

void free_vars_rec(expr *e, VarSet *s) {
    if (e->type == VAR_expr) vs_add(s, e->var_name);
    else if (e->type == ABS_expr) {
        free_vars_rec(e->abs_body, s);
        vs_rm(s, e->abs_param);
    } else {
        free_vars_rec(e->app_fn, s);
        free_vars_rec(e->app_arg, s);
    }
}

VarSet free_vars(expr *e) {
    VarSet s;
    vs_init(&s);
    free_vars_rec(e, &s);

    return s;
}

char *fresh_var(VarSet *s) {
    for (char c = 'a'; c <= 'z'; c++) {
        const char buf[2] = {c, '\0'};
        if (!vs_has(s, buf)) return strdup(buf);
    }
    int idx = 1;
    while (true) {
        for (char c = 'a'; c <= 'z'; c++) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%c%d", c, idx);
            if (!vs_has(s, buf)) return strdup(buf);
        }
        idx++;
    }
}

expr *substitute(expr *e, const char *v, expr *val) {
    if (e->type == VAR_expr) {
        if (strcmp(e->var_name, v) == 0) return copy_expr(val);
        else return copy_expr(e);
    }
    if (e->type == ABS_expr) {
        if (strcmp(e->abs_param, v) == 0) return copy_expr(e);
        VarSet fv_val = free_vars(val);
        if (vs_has(&fv_val, e->abs_param)) {
            VarSet forbidden_vars = free_vars(e);
            vs_add(&forbidden_vars, e->abs_param);
            for (int i = 0; i < fv_val.c; i++) vs_add(&forbidden_vars, fv_val.v[i]);

            char *nv_name = fresh_var(&forbidden_vars);
            expr *nv_expr = make_variable(nv_name);
            expr *renamed_body = substitute(e->abs_body, e->abs_param, nv_expr);
            expr *substituted_renamed_body = substitute(renamed_body, v, val);
            expr *result_expr = make_abstraction(nv_name, substituted_renamed_body);

            free_expr(nv_expr);
            free(nv_name);
            free_expr(renamed_body);
            vs_free(&forbidden_vars);
            vs_free(&fv_val);

            return result_expr;
        }
        expr *new_body = substitute(e->abs_body, v, val);
        expr *result_expr = make_abstraction(e->abs_param, new_body);
        vs_free(&fv_val);

        return result_expr;
    }
    expr *substituted_fn = substitute(e->app_fn, v, val);
    expr *substituted_arg = substitute(e->app_arg, v, val);

    return make_application(substituted_fn, substituted_arg);
}

int find_def(const char *s) {
    for (int i = 0; i < N_DEFS; i++) if (!strcmp(def_names[i], s)) return i;

    return -1;
}

bool delta_reduce(expr *e, expr **out) {
    if (e->type == VAR_expr) {
        int i = find_def(e->var_name);
        if (i >= 0) {
            *out = copy_expr(def_vals[i]);
            return true;
        }
    }

    return false;
}

bool beta_reduce(expr *e, expr **out) {
    if ((e->type == APP_expr) && (e->app_fn->type == ABS_expr)) {
        expr *argcp = copy_expr(e->app_arg);
        *out = substitute(e->app_fn->abs_body, e->app_fn->abs_param, argcp);
        free_expr(argcp);
        return true;
    }

    return false;
}

bool reduce_once(expr *e, expr **ne, const char **rtype) {
    expr *tmp;
    if (delta_reduce(e, &tmp)) {
        *ne = tmp;
        *rtype = "δ";
        return true;
    }
    if (beta_reduce(e, &tmp)) {
        *ne = tmp;
        *rtype = "β";
        return true;
    }
    if (e->type == APP_expr) {
        if (reduce_once(e->app_fn, &tmp, rtype)) {
            *ne = make_application(tmp, copy_expr(e->app_arg));
            return true;
        }
        if (reduce_once(e->app_arg, &tmp, rtype)) {
            *ne = make_application(copy_expr(e->app_fn), tmp);
            return true;
        }
    }
    if (e->type == ABS_expr && reduce_once(e->abs_body, &tmp, rtype)) {
        *ne = make_abstraction(e->abs_param, tmp);
        return true;
    }

    return false;
}

bool is_church_numeral(expr *e) {
    if (e->type != ABS_expr) return false;
    expr *e1 = e->abs_body;
    if (e1->type != ABS_expr) return false;
    const char *f = e->abs_param;
    const char *x = e1->abs_param;
    expr *cur = e1->abs_body;
    while ((cur->type == APP_expr) && (cur->app_fn->type == VAR_expr) &&
           (!strcmp(cur->app_fn->var_name, f))) {
        cur = cur->app_arg;
    }

    return (cur->type == VAR_expr) && (!strcmp(cur->var_name, x));
}

int count_applications(expr *e) {
    expr *cur = e->abs_body->abs_body;
    const char *f = e->abs_param;
    int cnt = 0;
    while ((cur->type == APP_expr) && (cur->app_fn->type == VAR_expr) &&
           (!strcmp(cur->app_fn->var_name, f))) {
        cnt++;
        cur = cur->app_arg;
    }

    return cnt;
}

expr *abstract_numerals(expr *e) {
    if (is_church_numeral(e)) {
        const int n = count_applications(e);
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", n);
        return make_variable(buf);
    }
    if (e->type == ABS_expr)
        return make_abstraction(e->abs_param, abstract_numerals(e->abs_body));
    if (e->type == APP_expr)
        return make_application(abstract_numerals(e->app_fn), abstract_numerals(e->app_arg));

    return make_variable(e->var_name);
}

void normalize(expr *e) {
    sb_reset(&sb);
    expr_to_buffer(e, sb.data, sb.cap);
    printf("Step 0: %s\n", sb.data);
    int step = 1;
    while (true) {
        expr *next;
        const char *rtype;
        if (!reduce_once(e, &next, &rtype)) {
            printf("\n→ normal form reached.\n");
            break;
        }
        free_expr(e);
        e = next;
        sb_reset(&sb);
        expr_to_buffer(e, sb.data, sb.cap);

        if (CONFIG_SHOW_STEP_TYPE) printf("Step %d (%s): %s\n", step++, rtype, sb.data);
        else printf("Step %d: %s\n", step++, sb.data);
    }
    if (CONFIG_DELTA_ABSTRACT) {
        expr *abs = abstract_numerals(e);
        sb_reset(&sb);
        expr_to_buffer(abs, sb.data, sb.cap);
        printf("\nδ-abstracted: %s\n", sb.data);
        free_expr(abs);
    }
    free_expr(e);
}

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
            //for (int i = 0; i < N_DEFS; i++) free_expr(def_vals[i]);
            //sb_destroy(&sb);
            exit(1);
        }
        input[0] = '\0';
        for (int i = 1; i < argc; i++) {
            strcat(input, argv[i]);
            if (i < argc - 1) strcat(input, " ");
        }
    } else {
        char buf[2048];
        printf("λ-expr> ");
        if (!fgets(buf, sizeof(buf), stdin)) {
            //for (int i = 0; i < N_DEFS; i++) free_expr(def_vals[i]);
            //sb_destroy(&sb);
            exit(1);
        }
        buf[strcspn(buf, "\n")] = '\0';
        input = strdup(buf);
        if (!input) {
            perror("strdup for input");
 
            // Cleanup def_vals
            //for (int i = 0; i < N_DEFS; i++) free_expr(def_vals[i]);
            //sb_destroy(&sb);
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

char peek(const Parser *p) { return p->i < p->n ? p->src[p->i] : '\0'; }

char consume(Parser *p) {
    if (!peek(p)) return '\0';

    // UTF-8 λ = 0xCE 0xBB
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE) && ((uchar) p->src[p->i + 1] == 0xBB)) {
        p->i += 2;
        return '\0';
    }

    return p->src[p->i++];
}

void skip_whitespace(Parser *p) { while (isspace((uchar) peek(p))) p->i++; }

expr *parse(Parser *p) {
    skip_whitespace(p);
    expr *e = parse_expr(p);
    skip_whitespace(p);
    if (peek(p)) {
        fprintf(stderr, "Unexpected '%c' at %zu\n", peek(p), p->i);
        exit(1);
    }

    return e;
}

expr *parse_expr(Parser *p) {
    skip_whitespace(p);
    // detect λ
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE) && ((uchar) p->src[p->i + 1] == 0xBB)) {
        return parse_abs(p);
    }

    return parse_app(p);
}

expr *parse_abs(Parser *p) {
    p->i += 2; // consume λ
    char *v = parse_varname(p);
    skip_whitespace(p);
    if (consume(p) != '.') {
        fprintf(stderr, "Expected '.' after λ\n");
        exit(1);
    }
    expr *body = parse_expr(p);
    expr *ret = make_abstraction(v, body);
    free(v);

    return ret;
}

expr *parse_app(Parser *p) {
    skip_whitespace(p);
    expr *e = parse_atom(p);
    skip_whitespace(p);
    char c = peek(p);
    while (c && c != ')' && c != '.') {
        expr *a = parse_atom(p);
        e = make_application(e, a);
        skip_whitespace(p);
        c = peek(p);
    }

    return e;
}

expr *parse_atom(Parser *p) {
    skip_whitespace(p);
    // λ as atom
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE) && ((uchar) p->src[p->i + 1] == 0xBB)) {
        return parse_abs(p);
    }
    char c = peek(p);
    if (c == '(') {
        consume(p);
        expr *e = parse_expr(p);
        skip_whitespace(p);
        if (consume(p) != ')') {
            fprintf(stderr, "Expected ')'\n");
            exit(1);
        }
        return e;
    }
    if (isdigit((uchar) c)) {
        const int v = parse_number(p);
        return church(v);
    }
    char *name = parse_varname(p);
    expr *v = make_variable(name);
    free(name);

    return v;
}

int parse_number(Parser *p) {
    int v = 0;
    if (!isdigit((uchar) peek(p))) {
        fprintf(stderr, "Expected digit at %zu\n", p->i);
        exit(1);
    }
    while (isdigit((uchar) peek(p))) v = v * 10 + (consume(p) - '0');

    return v;
}

inline bool is_invalid_char(Parser *p, char c) {
    return (!c) || (c == '(') || (c == ')') || (c == '.') ||
            (isspace((uchar) c)) || ((p->i + 1 < p->n) &&
             ((uchar) p->src[p->i] == 0xCE) &&
              ((uchar) p->src[p->i + 1] == 0xBB));
}

char *parse_varname(Parser *p) {
    skip_whitespace(p);
    char c = peek(p);
    if (is_invalid_char(p, c)) {
        fprintf(stderr, "Invalid var start at %zu\n", p->i);
        exit(1);
    }
    const size_t start = p->i;
    while (p->i < p->n) {
        c = peek(p);
        if (is_invalid_char(p, c)) break;
        p->i++;
    }
    size_t len = p->i - start;
    char *out = malloc(len + 1);
    if (!out) {
        perror("malloc");
        exit(1);
    }
    memcpy(out, p->src + start, len);
    out[len] = '\0';

    return out;
}
