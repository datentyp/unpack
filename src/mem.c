#include <stdlib.h>
#include <errno.h>

#include "mem.h"

void _free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    int save_errno = errno;
    free(*(void **) ptr);
    *(void **) ptr = NULL;
    errno = save_errno;
}
