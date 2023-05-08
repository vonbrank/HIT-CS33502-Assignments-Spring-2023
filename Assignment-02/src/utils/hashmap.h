#ifndef HASHMAP_H
#define HASHMAP_H
#include "string.h"

typedef struct _hn
{
    String str;
    void *value;
    struct _hn *next;

} HashNode;
typedef HashNode **HashMap;

HashMap hashmap_new();
void *hashmap_find(HashMap map, String str);
void hashmap_insert(HashMap map, String key, void *value);

#endif