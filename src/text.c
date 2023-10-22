#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "text.h"

/**
 * Returns a NULL terminated string that is a substring of string. The substring begins at the specified
 * start_idx (inclusive) and extends to the character at index end_idx - 1 (exclusive).
 *
 * Thus the length of the substring is end_idx-start_idx+1.
 * 
 * Return NULL if:
 *  - start_idx or end_idx is negative,
 *  - start_idx or end_idx are larger than the length of the source string,
 *  - start_idx is larger than end_idx
 *
 * Memory occupied by the returned substring has to be freed by the caller.
 *
 * @param source - the source for the substring.
 * @param start_idx - the beginning index, inclusive (char at this position will be included).
 * @param end_idx - the ending index, exclusive (char at this position will not be included)
 *
 * @return the specified substring.
 */
char *substr(
        const char *source,
        size_t start_idx,
        size_t end_idx
) {
    const size_t source_len = strlen(source);

    if ((start_idx < 0 || end_idx < 0)
        || (start_idx > source_len) || (end_idx > source_len)
        || (start_idx > end_idx)) {
        fprintf(stderr, "Illegal arguments for substr(): source=[%s], start_idx=[%zu], end_idx=[%zu]\n",
                source, start_idx, end_idx);
        return NULL;
    }

    const size_t substr_len = end_idx - start_idx;
    char *substring = (char *) malloc(sizeof(char) * (substr_len + 1));
    if (substring == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    for (size_t idx = start_idx; idx < end_idx && (*(source + idx) != '\0'); idx++) {
        *substring = *(source + idx);
        substring++;
    }
    *substring = '\0';

    return substring - substr_len;
}

/**
 * Returns index of first occurrence of substring in string or -1 if not available.
 *
 * @param string
 * @param substring
 * @param begin_from - start searching for substring in string beginning (inclusive) this position
 * @return
 */
size_t find_index_of_substring_in_string_beginning_from(
        const char *string,
        const char *substring,
        size_t begin_from
) {
    const size_t string_len = strlen(string);

    // check if there are enough characters
    if (string_len < begin_from) {
        return -1;
    }

    // check if string even fits substring (beginning from)
    const char *remaining_string = &string[begin_from];
    if (strlen(remaining_string) < strlen(substring)) {
        return -1;
    }

    size_t idx_of_substring_in_string = -1;
    for (size_t string_idx = begin_from; string[string_idx] != '\0'; string_idx++) {

        idx_of_substring_in_string = -1;
        for (int substring_idx = 0; substring[substring_idx] != '\0'; substring_idx++) {
            // if it doesn't match whatever substring is looking for
            if (string[string_idx + substring_idx] != substring[substring_idx]) {
                idx_of_substring_in_string = -1;
                break;
            }
            idx_of_substring_in_string = string_idx;
        }

        // ok, we found what we were looking for
        if (idx_of_substring_in_string != -1) {
            return idx_of_substring_in_string;
        }
    }

    return idx_of_substring_in_string;
}

/**
 * Returns a copy the data between two delimiters from a given string. Delimiters are not included.
 *
 * Allocates memory for parameter text that has to be freed by the caller after use.
 *
 * Neither the chars of left_delim nor those of right_delim will be included.
 *
 * Example:
 *
 * string: A,FOO,Z\0
 * left_delim: ,
 * right_delim: ,
 *
 * => text: FOO\0
 *
 * @return 0 on success, 1 if left_delim was not found, 2 if right_delim was not found
 */
int copy_text_between(
        const char *string,
        const char *left_delim,
        const char *right_delim,
        char **text
) {
    // find the left delimiter and use it as the start of the text to be copied
    // E.g. For an input like "A,FOO,Z\0" with start_delim "," => start will now contain the ",FOO,Z\0"
    const char *start = strstr(string, left_delim);

    if (start == NULL) {
        fprintf(stderr, "Failed to find left delimiter: %s\n", left_delim);
        return 1; // left delimiter not found
    }

    // increase offset of start by the length of the left delimiter => start will no longer include the left delimiter
    // E.g if start was ",FOO,Z\0" before, it will now be "FOO,Z\0"
    start += strlen(left_delim);

    // find the right delimiter
    // E g. if start is "FOO,Z\0" before, and right_delim is "," => then end will now be ",Z\0"
    const char *end = strstr(start + 1, right_delim);
    if (end == NULL) {
        fprintf(stderr, "Failed to find right delimiter: %s\n", right_delim);
        return 2; // right delimiter not found
    }

    // length of text to be copied
    // E.g. if start is "FOO,Z\0" and end ",Z\0" => then end-start will now be 3
    const ptrdiff_t text_len = end - start;

    // allocate memory and write copy of content to text
    *text = (char *) malloc(sizeof(char) * text_len + 1);
    if (*text == NULL) {
        fprintf(stderr, "Failed to allocate memory for text: start=[%s]\n", start);
        exit(EXIT_FAILURE);
    }

    strncpy(*text, start, text_len);
    // strncpy does no auto string termination for us (see man 7 strncpy)
    (*text)[text_len] = '\0';
    return 0;
}


/**
 * Extracts the data starting at provided offset up to the provided len.
 *
 * Allocates memory for parameter text that has to be freed by the caller after use.
 *
 * @return 0 on success
 */
char *copy_text_from(
        const char *source,
        int offset,
        size_t len) {

    char *text = NULL;
    size_t i = -1;

    // we cannot extract more than we have to begin with
    const size_t source_len = strlen(source);
    if (source_len < len) {
        fprintf(stderr, "Cannot extract a text of len %zu from a source with len %zu\n", len, source_len);
        exit(EXIT_FAILURE);
    }

    text = (char *) malloc(sizeof(char) * len + 1);
    if (text == NULL) {
        fprintf(stderr, "Failed to allocate memory for text\n");
        exit(EXIT_FAILURE);
    }

    // copy up to len chars from position offset of source into text
    for (i = 0; i < len; i++) {
        *(text + i) = *(source + offset - 1 + i);
        if (*(text + i) == '\0') {
            break;
        }
    }
    *(text + i) = '\0';

    return text;
}