#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/stat.h>
#include <string.h>

#include "util.h"
#include "text.h"
#include "mem.h"
#include "debug.h"
#include "unpack.h"

struct stat st = {0};
const unsigned char SINGLE_QUOTE = '\''; // \x27

#define MKDIR_MODE  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

/**
 * Read a topic reader export file and try to unpack all it's records, line by line.
 */
void unpack_file(FILE *fp) {
    char *line = NULL;
    size_t len = 0;
    size_t line_number = 0;
    size_t minimum_length_of_valid_csv_lines = 8;

    char *environment = NULL;
    char *topic = NULL;
    char *search_value = NULL;
    char *time_from = NULL;
    char *time_to = NULL;

    // man 3 getline: The buffer is null-terminated and includes the newline character, if one was found.
    while ((getline(&line, &len, fp)) != -1) {
        line_number = line_number + 1;

        // unpack export metadata
        if (line_number == 1) {
            exit_on_failure(
                    copy_text_between(
                            line,
                            "environment: ",
                            "\n",
                            &environment),
                    "Failed to extract environment from line 1.\n"
            );

        } else if (line_number == 2) {
            exit_on_failure(
                    copy_text_between(
                            line,
                            "topic      : ",
                            "\n",
                            &topic
                    ),
                    "Failed to extract topic from line 2.\n"
            );
        } else if (line_number == 3) {
            exit_on_failure(
                    copy_text_between(
                            line,
                            "searchValue: ",
                            "\n",
                            &search_value
                    ),
                    "Failed to extract search_value from line 3.\n"
            );
        } else if (line_number == 4) {
            exit_on_failure(
                    copy_text_between(
                            line,
                            "timeFrom   : ",
                            "\n",
                            &time_from
                    ), "Failed to extract time_from from line 4.\n"
            );
        } else if (line_number == 5) {
            exit_on_failure(
                    copy_text_between(
                            line,
                            "timeTo     : ",
                            "\n",
                            &time_to
                    ), "Failed to extract time_to from line 5.\n"
            );
        } else if (line_number > 5) {
            // require all valid "csv" lines to have at least 8 chars (1,2,3,,\n)
            if (strlen(line) >= minimum_length_of_valid_csv_lines) {
                unpack_record(line, environment, topic);
            }
        }
    }

    FREE(line);
    FREE(environment);
    FREE(topic);
    FREE(search_value);
    FREE(time_from);
    FREE(time_to);
}

void unpack_record(char *line, const char *environment, const char *topic) {
    char *partition = NULL;
    char *offset = NULL;
    char *timestamp = NULL;
    char *key = NULL;
    char *value = NULL;

    size_t start_idx = 0; /* inclusive - should point to first char of content to be included */
    size_t end_idx = 0; /* exclusive - should point to the first char being excluded (after content) */

    end_idx = find_index_of_substring_in_string_beginning_from(line, ",", start_idx);
    partition = substr(line, start_idx, end_idx);
    warn_on_empty_field(partition, "partition", line);

    start_idx = end_idx + 1;
    end_idx = find_index_of_substring_in_string_beginning_from(line, ",", start_idx);
    offset = substr(line, start_idx, end_idx);
    warn_on_empty_field(offset, "offset", line);

    start_idx = end_idx + 1;
    end_idx = find_index_of_substring_in_string_beginning_from(line, ",", start_idx);
    timestamp = substr(line, start_idx, end_idx);
    warn_on_empty_field(timestamp, "timestamp", line);

    start_idx = end_idx + 1;

    // Both, key and value fields, might have been wrapped within '' that don't actually belong to the content
    // We want the data within those '' but not those '' themselves.
    unsigned char field_start_char = line[start_idx];

    /* extract key */
    if (field_start_char == SINGLE_QUOTE) {
        end_idx = find_index_of_substring_in_string_beginning_from(line, "',", start_idx);
        // +1 to skip leading ', end doesn't require this due to subString already including ' before ,
        key = substr(line, start_idx + 1, end_idx);
        start_idx = end_idx + 2; // + 2 because end points now to the beginning of "'," and not ","
    } else {
        end_idx = find_index_of_substring_in_string_beginning_from(line, ",", start_idx);
        key = substr(line, start_idx, end_idx);
        start_idx = end_idx + 1; // start of next field
    }

    field_start_char = line[start_idx];

    /* extract value */
    const size_t line_len = strlen_without_trailing_carriage_return_and_line_feed(line);

    end_idx = line_len;

    if (field_start_char == SINGLE_QUOTE) {
        /* Ok, let's assume there is some closing ' as well - reverse search line for it */
        while (line[end_idx] != SINGLE_QUOTE && end_idx > start_idx) {
            --end_idx;
        }
        if (start_idx == end_idx) {
            // it seems value contains just a single ' but no ending '. Do not skip it, take it as it is.
            value = substr(line, start_idx, start_idx + 1);
        } else {
            // value seems to be enclosed within '' => include everything after the first ' up to (but excluding) the last '

            // start_idx points now to the leading '. start_idx is inclusive, thus start + 1 skips the leading '
            // end_idx points now to the trailing '. end_idx is exclusive, thus no + 1.
            value = substr(line, start_idx + 1, end_idx);

            // KNOWN BUG:
            // For certain inputs it is know that we actually might cut off too much - leading to a wrong value.
            // I do not really care about this except printing a warning in obvious cases. Maybe someone else cares?
            // Example: '{"name":"pam's blog"}
            if (line_len - end_idx > 2) {
                fprintf(stdout,
                        "Warning: Encountered unexpected position of token ' while parsing field value of record: "
                        "environment=[%s], topic=[%s], partition=[%s], offset=[%s], timestamp=[%s], key=[%s], value=[%s], line=[%s]\n",
                        environment, topic, partition, offset, timestamp, key, value, line
                );
            }
        }
    } else { /* value field is not enclosed within '' */
        value = substr(line, start_idx, end_idx);
    }

    DF("environment=[%s], topic=[%s], partition=[%s], offset=[%s], timestamp=[%s], key=[%s], value=[%s], line=[%s], start_idx=[%zu], end_idx=[%zu]",
       environment, topic, partition, offset, timestamp, key, value, line, start_idx, end_idx);

    if (environment != NULL && topic != NULL && partition != NULL && offset != NULL) {
        write_record_file(environment, topic, partition, offset, timestamp, key, value);
    } else {
        fprintf(stderr,
                "Warning: Encountered incomplete data while parsing line. Cannot unpack record into file."
                "environment=[%s], topic=[%s], partition=[%s], offset=[%s], timestamp=[%s], key=[%s], value=[%s], line=[%s]\n",
                environment, topic, partition, offset, timestamp, key, value, line
        );
    }
    FREE(partition);
    FREE(offset);
    FREE(timestamp);
    FREE(key);
    FREE(value);
}


void write_record_file(const char *environment, const char *topic, const char *partition, const char *offset,
                       const char *timestamp, const char *key, const char *value) {

    // create directories and value file
    if (stat((environment), &st) == -1) {
        if (mkdir((environment), MKDIR_MODE) != 0) {
            fprintf(stderr, "Failed to create directory %s\n", environment);
            exit(EXIT_FAILURE);
        }
    }
    size_t file_path_len = strlen((environment)) + strlen("/") + strlen((topic)) + 1;
    char environment_topic_directory[file_path_len];
    sprintf(environment_topic_directory, "%s/%s", environment, topic);

    if (stat(environment_topic_directory, &st) == -1) {
        if (mkdir(environment_topic_directory, MKDIR_MODE) != 0) {
            fprintf(stderr, "Failed to create directory %s\n", environment_topic_directory);
            exit(EXIT_FAILURE);
        };
    }

    file_path_len = file_path_len + strlen("/") + strlen(partition) + 1;
    char environment_topic_partition_directory[file_path_len];
    sprintf(environment_topic_partition_directory, "%s/%s", environment_topic_directory, partition);

    if (stat(environment_topic_partition_directory, &st) == -1) {
        if (mkdir(environment_topic_partition_directory, MKDIR_MODE) != 0) {
            fprintf(stderr, "Failed to create directory %s\n", environment_topic_partition_directory);
            exit(EXIT_FAILURE);
        }
    }
    file_path_len = file_path_len + strlen("/") + strlen(offset) + strlen(".json5") + 1;

    char environment_topic_partition_offset_file[file_path_len];
    sprintf(environment_topic_partition_offset_file, "%s/%s.json5", environment_topic_partition_directory, offset);

    char metadata_fmt[] = "// environment=[%s], topic=[%s], partition=[%s], offset=[%s], timestamp=[%s], key=[%s]\n";
    size_t metadata_line_length =
            strlen(metadata_fmt) + strlen(environment) + strlen(topic) + strlen(partition) +
            strlen(offset) + strlen(timestamp) + strlen(key) + 1;

    char heading_metadata_line[metadata_line_length];
    snprintf(heading_metadata_line, metadata_line_length, metadata_fmt,
             environment, topic, partition, offset, timestamp, key
    );

    // create file
    FILE *fptr;
    fptr = fopen(environment_topic_partition_offset_file, "w");
    if (fptr != NULL) {
        fprintf(fptr, "%s", heading_metadata_line);
        fprintf(fptr, "%s", value);
        fclose(fptr);
        fptr = NULL;
    }
}

void warn_on_empty_field(const char *field_value, const char *field_name, const char *line) {
    if (strlen(field_value) < 1) {
        fprintf(stdout,
                "Warning: Encountered unexpected empty field_name '%s' in line '%s'\n",
                field_name, line
        );
    }
}

/**
 * Topic reader export files carry \r\n (CARRIAGE_RETURN \x0D, LINE_FEED \x0A) line endings.
 *
 * Returns the length of str similiar to strlen() but not counting trailing \r\n if present.
 *
 * Examples:
 *   "123\r\n" => 3
 *   "123\n" => 3
 *   "123\r" => 3
 *   "123" => 3
 *   "123\r\n\r\n" => 5
 */
size_t strlen_without_trailing_carriage_return_and_line_feed(const char *str) {
    if (str == NULL) {
        return -1;
    }
    size_t len = strlen(str);

    if (len < 1) {
        return len;
    }

    if (str[len - 1] == '\n') {
        len--;
    }

    if (str[len - 1] == '\r') {
        len--;
    }

    return len;
}