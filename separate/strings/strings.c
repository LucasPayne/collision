#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int string_in(char *string, char *csv)
{
    /* Returns -1 if the string is not in the comma-separated list.
     * Otherwise, returns the least 0-starting index position into the list
     * that the string appears. Commas cannot be escaped. */

    char *entry = csv;
    char *end;
    int i = 0;
    do {
        end = strchr(entry, ',');
        if ((end == NULL && strcmp(string, entry) == 0) || (end != NULL && strncmp(string, entry, end - entry) == 0)) {
            return i;
        }
        i++;
        if (end != NULL) entry = end + 1;
    } while (end != NULL);
    return -1;
}



void test_string_in(char *string, char *csv)
{
    printf("string_in(\"%s\", \"%s\") = %d\n", string, csv, string_in(string, csv));
}

int main(void)
{
    test_string_in("in", "stuff,thing,like");
    test_string_in("in", "stuff,in,like");
    test_string_in("cool", "stuff,just,cool");
    test_string_in("cool", "stuff,just,cool,nice");
    test_string_in("nice", "stuff,just,cool,nice,as,ice");
    test_string_in("cool", "stuff,just,bool");
}
