#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void exit_on_failure(int result, char *message) {
    if (result != 0) {
        fprintf(stderr, "%s", message);
        exit(EXIT_FAILURE);
    }
}
