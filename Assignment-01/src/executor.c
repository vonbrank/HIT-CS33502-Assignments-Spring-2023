#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

int stringEndsWith(char *self, char *suffix)
{
    return (strlen(self) > strlen(suffix)) && !strcmp(self + strlen(self) - strlen(suffix), suffix);
}

int main()
{
    int MAX_STRING_BUFFER_SIZE = 128;
    DIR *d;
    struct dirent *dir;

    char *TEST_PATH = "../test/";
    char *RESULT_PATH = "../result/";
    char *PARSER_FILE_PATH = "./parser";

    d = opendir(TEST_PATH);

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (stringEndsWith(dir->d_name, ".cmm"))
            {
                char commandStr[MAX_STRING_BUFFER_SIZE];
                char fileNameWithoutExtension[MAX_STRING_BUFFER_SIZE];

                strcpy(fileNameWithoutExtension, dir->d_name);
                fileNameWithoutExtension[strlen(fileNameWithoutExtension) - strlen(".cmm")] = '\0';

                strcpy(commandStr, PARSER_FILE_PATH);
                strcat(commandStr, " < ");
                strcat(commandStr, TEST_PATH);
                strcat(commandStr, dir->d_name);
                strcat(commandStr, " > ");
                strcat(commandStr, RESULT_PATH);
                strcat(commandStr, fileNameWithoutExtension);
                strcat(commandStr, ".output");

                printf("%s\n", commandStr);
                system(commandStr);
            }
        }
        closedir(d);
    }

    return 0;
}