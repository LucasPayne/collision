
int string_in(char *string, char *csv)
{
    /* Returns -1 if the string is not in the comma-separated list.
     * Otherwise, returns the least 0-starting index position into the list
     * that the string appears. Commas can be escaped. */

    char *entry = csv;
    char *end;
    int i = 0;
    do {
        end = strchr(entry, ',');
        if ((end == NULL && strcmp(string, entry)) || (end != NULL && strncmp(string, entry, end - entry) == 0)) {
            return i;
        }
        i++;
    } while (end != NULL);
    return -1;
}
