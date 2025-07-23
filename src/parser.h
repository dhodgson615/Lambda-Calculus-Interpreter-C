#ifndef PARSER_H
#define PARSER_H

#include "expr.h"
#include "macros.h"

#include <stdbool.h>

/**
 * @brief              Parser structure.
 */
typedef struct Parser {
    const char    *src;
    size_t         i;
    size_t         n;
} Parser;

/**
 * @brief              Peek the next character in the input without consuming it.
 * @param  p           the parser
 * @return             the next character in the input
 */
HOT PURE INLINE char peek(const Parser *p);

/**
 * @brief              Consume the next character in the input.
 * @param  p           the parser
 * @return             the consumed character
 */
HOT INLINE char consume(Parser *p);

/**
 * @brief              Skip whitespace characters.
 * @param  p           the parser
 */
HOT INLINE void skip_whitespace(Parser *p);

/**
 * @brief              Check if the next characters form a lambda.
 * @param  p           the parser
 * @return             true if the next characters form a lambda, false otherwise
 */
HOT PURE INLINE bool is_lambda(const Parser *p);

/**
 * @brief              Check if a character is a valid variable name character.
 * @param  p           the parser
 * @param  c           the character to check
 * @return             true if the character is invalid, false otherwise
 */
HOT PURE INLINE bool is_invalid_char(const Parser *p, char c);

/**
 * @brief              Parse a lambda calculus expression.
 * @param  p           the parser
 * @return             the parsed expression
 */
expr *parse(Parser *p);

/**
 * @brief              Parse an expression from the input.
 * @param  p           the parser
 * @return             the parsed expression
 */
expr *parse_expr(Parser *p);

/**
 * @brief              Parse an abstraction from the input.
 * @param  p           the parser
 * @return             the parsed abstraction
 */
expr *parse_abs(Parser *p);

/**
 * @brief              Parse an application from the input.
 * @param  p           the parser
 * @return             the parsed application
 */
expr *parse_app(Parser *p);

/**
 * @brief              Parse an atom from the input.
 * @param  p           the parser
 * @return             the parsed atom
 */
expr *parse_atom(Parser *p);

/**
 * @brief              Parse a number from the input.
 * @param  p           the parser
 * @return             the parsed number
 */
int parse_number(Parser *p);

/**
 * @brief              Parse a variable name from the input.
 * @param  p           the parser
 * @return             the parsed variable name (must be freed by caller)
 */
char *parse_varname(Parser *p);

#endif /* PARSER_H */
