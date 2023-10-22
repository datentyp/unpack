//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "unpack.h"

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc <= 1) {
        // like classic UNIX tools we proceed to read from standard input if no file was provided as argument
        fp = stdin;
        unpack_file(fp);

        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
    } else {
        for (int i = 1; i < argc; i++) {
            // again, like classic UNIX tools we do not print any output except if something goes wrong
            if ((fp = fopen(argv[i], "r")) == NULL) {
                fprintf(stderr, "Cannot open file: %s", argv[1]);
                exit(EXIT_FAILURE);
            }

            unpack_file(fp);

            if (fp != NULL) {
                fclose(fp);
                fp = NULL;
            }
        }
    }

    return EXIT_SUCCESS;
}
