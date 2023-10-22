#ifndef UNPACK_TEXT_H
#define UNPACK_TEXT_H

#include <sys/types.h>
#include <stdio.h>

char *substr(
        const char *source,
        size_t start_idx,
        size_t end_idx
);

size_t find_index_of_substring_in_string_beginning_from(
        const char *string,
        const char *substring,
        size_t begin_from
);

int copy_text_between(
        const char *string,
        const char *left_delim,
        const char *right_delim,
        char **text
);

char *copy_text_from(
        const char *source,
        int offset,
        size_t len
);

#endif // UNPACK_TEXT_H
