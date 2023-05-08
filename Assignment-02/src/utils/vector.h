#ifndef VECTOR_H
#define VECTOR_H

typedef struct
{
    void **ptr;
    int size;
    int capacity;
} Vector;

Vector vector_new();
Vector vector_push(Vector vec, void *obj);

#endif