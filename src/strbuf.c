#include "strbuf.h"
#include <stdio.h>
#include <stdlib.h>

void sb_init(strbuf *sb, const size_t init_cap) {
    sb->data = malloc(init_cap);
    if (!sb->data) {
        perror("malloc for strbuf");
        exit(1);
    }
    sb->cap = init_cap;
    sb->len = 0;
    sb->data[0] = '\0';
}

void sb_ensure(strbuf *sb, const size_t need) {
    if (sb->len + need + 1 > sb->cap) {
        size_t new = sb->cap * 2;
        while (new < sb->len + need + 1) new *= 2;

        sb->data = realloc(sb->data, new);
        if (!sb->data) {
            perror("realloc for strbuf");
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