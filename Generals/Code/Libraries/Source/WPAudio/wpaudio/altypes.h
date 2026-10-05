#ifndef ALTYPES_H
#define ALTYPES_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define ALIGN_UPBY(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define ALLOC_STRUCT(s, t) ((t *)calloc(1, sizeof(t)))
#define MEM_ALLOC_STRUCT(s, t, f) ((t *)calloc(1, sizeof(t)))

#endif
