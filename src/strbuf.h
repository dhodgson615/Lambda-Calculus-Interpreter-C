#ifndef STRBUF_H
#define STRBUF_H

#include <stdlib.h>

/**
 * @brief              String buffer structure.
 */
typedef struct strbuf {
    char          *data;
    size_t         len;
    size_t         cap;
} strbuf;

/**
 * @brief              Initialize a string buffer.
 * @param  sb          the string buffer to initialize
 * @param  init_cap    the initial capacity of the string buffer
 */
void sb_init(strbuf *sb, size_t init_cap);

/**
 * @brief              Ensure the string buffer has enough capacity.
 * @param  sb          the string buffer to ensure
 * @param  need        the amount of space needed
 */
void sb_ensure(strbuf *sb, size_t need);

/**
 * @brief              Reset the string buffer.
 * @param  sb          the string buffer to reset
 */
void sb_reset(strbuf *sb);

/**
 * @brief              Destroy the string buffer.
 * @param  sb          the string buffer to destroy
 */
void sb_destroy(strbuf *sb);

/* Global strbuf instance used across the application */
extern strbuf sb;

#endif /* STRBUF_H */
