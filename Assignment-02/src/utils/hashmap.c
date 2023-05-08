#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "hashmap.h"

HashMap hashmap_new()
{
    HashMap hm = (HashMap)malloc(sizeof(HashNode *));
    hm[0] = NULL;
    return hm;
}
void *hashmap_find(HashMap map, String str)
{
    for (HashNode *ptr = map[0]; ptr != NULL; ptr = ptr->next)
        if (string_equals(ptr->str, str))
            return ptr->value;
    return NULL;
}
void hashmap_insert(HashMap map, String key, void *value)
{

    HashNode *nd = (HashNode *)malloc(sizeof(HashNode));
    nd->next = map[0];
    nd->str = string_clone(key);
    nd->value = value;
    map[0] = nd;
}