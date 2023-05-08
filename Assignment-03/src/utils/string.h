#ifndef STRING_H
#define STRING_H

typedef struct
{
    char *str;
    int length;
} String;

String string_new(const char *str);
String string_with_length(int n);
String string_from_rand();
String string_clone(String str);
String string_push_string(String a, String b);
String string_push_char(String a, const char *b);
int string_equals(String a, String b);

#endif