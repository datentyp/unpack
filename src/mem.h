#ifndef UNPACK_MEM_H
#define UNPACK_MEM_H

#define FREE(p) _free(1 ? (void *)&(p) : (p))

void _free(void *ptr);

#endif // UNPACK_MEM_H