#include "vector.h"
#include <string.h>
#include <stdlib.h>

Vector vector_new()
{
    Vector vec;
    vec.ptr = (void **)malloc(sizeof(void *));
    vec.capacity = 1;
    vec.size = 0;
    return vec;
}
Vector vector_push(Vector vec, void *obj)
{
    if (vec.size == vec.capacity)
    {
        void **np = (void **)malloc(vec.capacity * 2 * sizeof(void *));
        memcpy(np, vec.ptr, vec.capacity * sizeof(void *));
        vec.capacity <<= 1;
        free(vec.ptr);
        vec.ptr = np;
    }
    vec.ptr[vec.size++] = obj;
    return vec;
}
