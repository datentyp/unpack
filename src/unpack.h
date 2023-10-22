// Created by Wolfgang Kaufmann on 05.08.23.

#ifndef UNPACK_UNPACK_H
#define UNPACK_UNPACK_H

#include <stdio.h>

void unpack_record(char *line,
                   const char *environment,
                   const char *topic
);

void unpack_file(FILE *fp);

void warn_on_empty_field(const char *field_value,
                         const char *field_name,
                         const char *line
);

size_t strlen_without_trailing_carriage_return_and_line_feed(const char *str);

void write_record_file(const char *environment,
                       const char *topic,
                       const char *partition,
                       const char *offset,
                       const char *timestamp,
                       const char *key,
                       const char *value
);

#endif // UNPACK_UNPACK_H
