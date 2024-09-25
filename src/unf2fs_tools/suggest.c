// https://github.com/python/cpython/blob/4a5e4aa/Python/suggestions.c

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#define MOVE_COST             2
#define CASE_COST             1

#define LEAST_FIVE_BITS(n)    ((n) & 31)

#ifndef MIN
# define MIN(a, b)            ((a < b) ? a : b)
#endif

static inline int
substitution_cost(char a, char b)
{
    if (LEAST_FIVE_BITS(a) != LEAST_FIVE_BITS(b)) {
        // Not the same, not a case flip.
        return MOVE_COST;
    }

    if (a == b) {
        return 0;
    }

    if ('A' <= a && a <= 'Z') {
        a += ('a' - 'A');
    }
    if ('A' <= b && b <= 'Z') {
        b += ('a' - 'A');
    }
    if (a == b) {
        return CASE_COST;
    }

    return MOVE_COST;
}

/* Calculate the Levenshtein distance between string1 and string2 */
static ssize_t
levenshtein_distance(const char *a, size_t a_size,
                     const char *b, size_t b_size,
                     size_t max_cost, size_t *buffer)
{
    // Both strings are the same (by identity)
    if (a == b) {
        return 0;
    }

    // Trim away common affixes.
    while (a_size && b_size && a[0] == b[0]) {
        a++; a_size--;
        b++; b_size--;
    }
    while (a_size && b_size && a[a_size-1] == b[b_size-1]) {
        a_size--;
        b_size--;
    }
    if (a_size == 0 || b_size == 0) {
        return (a_size + b_size) * MOVE_COST;
    }

    // Prefer shorter buffer
    if (b_size < a_size) {
        const char *t = a; a = b; b = t;
        size_t t_size = a_size; a_size = b_size; b_size = t_size;
    }

    // quick fail when a match is impossible.
    if ((b_size - a_size) * MOVE_COST > max_cost) {
        return max_cost + 1;
    }

    // Instead of producing the whole traditional len(a)-by-len(b)
    // matrix, we can update just one row in place.
    // Initialize the buffer row
    size_t tmp = MOVE_COST;
    for (size_t i = 0; i < a_size; i++) {
        // cost from b[:0] to a[:i+1]
        buffer[i] = tmp;
        tmp += MOVE_COST;
    }

    size_t result = 0;
    for (size_t b_index = 0; b_index < b_size; b_index++) {
        char code = b[b_index];
        // cost(b[:b_index], a[:0]) == b_index * MOVE_COST
        size_t distance = result = b_index * MOVE_COST;
        size_t minimum = SIZE_MAX;
        for (size_t index = 0; index < a_size; index++) {

            // cost(b[:b_index+1], a[:index+1]) = min(
            //     // 1) substitute
            //     cost(b[:b_index], a[:index])
            //         + substitution_cost(b[b_index], a[index]),
            //     // 2) delete from b
            //     cost(b[:b_index], a[:index+1]) + MOVE_COST,
            //     // 3) delete from a
            //     cost(b[:b_index+1], a[index]) + MOVE_COST
            // )

            // 1) Previous distance in this row is cost(b[:b_index], a[:index])
            size_t substitute = distance + substitution_cost(code, a[index]);
            // 2) cost(b[:b_index], a[:index+1]) from previous row
            distance = buffer[index];
            // 3) existing result is cost(b[:b_index+1], a[index])

            size_t insert_delete = MIN(result, distance) + MOVE_COST;
            result = MIN(insert_delete, substitute);

            // cost(b[:b_index+1], a[:index+1])
            buffer[index] = result;
            if (result < minimum) {
                minimum = result;
            }
        }
        if (minimum > max_cost) {
            // Everything in this row is too big, so bail early.
            return max_cost + 1;
        }
    }
    return result;
}

const char *
calc_suggestion(const char *dir[],
             size_t dir_size,
             const char *name)
{
    ssize_t suggestion_distance = SSIZE_MAX;
    const char *suggestion = NULL;
    size_t name_size = strlen(name);
    size_t max_size = name_size;

    for (int i = 0; i < dir_size; i++) {
        size_t len = strlen(dir[i]);
        if (len > max_size)
            max_size = len;
    }

    size_t buffer[max_size];
    for (size_t i = 0; i < dir_size; ++i) {
        const char *item = dir[i];
        if (!strcmp(name, item)) {
            continue;
        }

        size_t item_size = strlen(item);
        // No more than 1/3 of the involved characters should need changed.
        ssize_t max_distance = (name_size + item_size + 3) * MOVE_COST / 6;
        // Don't take matches we've already beaten.
        max_distance = MIN(max_distance, suggestion_distance - 1);
        ssize_t current_distance =
            levenshtein_distance(name, name_size, item,
                                 item_size, max_distance, buffer);
        if (current_distance > max_distance) {
            continue;
        }
        if (!suggestion || current_distance < suggestion_distance) {
            suggestion = item;
            suggestion_distance = current_distance;
        }
    }
    return suggestion;
}
