#include "string.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

String string_new(const char *str)
{ // 构造一个 String
    int len = strlen(str);
    String s;
    s.length = len;
    s.str = (char *)malloc(len + 2);
    memcpy(s.str, str, len + 1);
    return s;
}

String string_with_length(int n)
{
    static char buf[30];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", n);
    return string_new(buf);
}

String string_from_rand()
{
    static char buf[30];
    for (int i = 0; i < 5; i++)
        buf[i] = rand() % 26 + 'a';
    buf[5] = 0;
    return string_new(buf);
}

String string_clone(String str)
{
    return string_new(str.str);
}

String string_push_string(String a, String b)
{
    String c;
    c.length = a.length + b.length;
    c.str = (char *)malloc(a.length + b.length + 2);
    memcpy(c.str, a.str, a.length + 1);
    memcpy(c.str + a.length, b.str, b.length + 1);
    return c;
}

String string_push_char(String a, const char *b)
{
    int lb = strlen(b);
    String c;
    c.length = a.length + lb;
    c.str = (char *)malloc(a.length + lb + 2);
    memcpy(c.str, a.str, a.length + 1);
    memcpy(c.str + a.length, b, lb + 1);
    return c;
}

int string_equals(String a, String b)
{ // 判断两个 String 是否相等
    if (a.length != b.length)
        return 0;
    return strcmp(a.str, b.str) == 0;
}
