/*--------------------------------------------------------------------------------
    Definitions for the text processing module.
--------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "helper_definitions.h"
#include "text_processing.h"

//--------------------------------------------------------------------------------
// Line-eating functions. For line-based "lexical analysis".
//--------------------------------------------------------------------------------
/*
 * Attempt to read an exactly matched line from the file. Return whether this line existed.
 * Advance in the line-eating only if the line was matched.
 *
 * Useful for line-level non-pattern case-by-case matching, for example ASCII file format metadata.
 */
static bool _line_buf_ready = false;
char _line_buf[_LINE_BUF_SIZE];
bool try_read_line(FILE *file, char *line)
{
    if (!_line_buf_ready) {
        if (fgets(_line_buf, _LINE_BUF_SIZE, file) == NULL) {
            return false;
        }
        /* printf("got: %s\n", _line_buf); */
        _line_buf_ready = true;
        // remove newline
        int len = strlen(_line_buf);
        _line_buf[len - 1] = '\0';
    }

    if (strncmp(line, _line_buf, _LINE_BUF_SIZE) == 0) {
        _line_buf_ready = false;
        return true;
    } else {
        return false;
    }
}

bool try_read_line_startswith(FILE *file, char *startswith)
{
    if (!_line_buf_ready) {
        if (fgets(_line_buf, _LINE_BUF_SIZE, file) == NULL)
            return false;
        _line_buf_ready = true;
        // remove newline
        int len = strlen(_line_buf);
        _line_buf[len - 1] = '\0';
    }
    int len = strlen(startswith);
    if (len > _LINE_BUF_SIZE) {
        fprintf(stderr, ERROR_ALERT "String larger than line buffer when attempting to check whether the next line in a file starts with a string.\n");
        exit(EXIT_FAILURE);
    }
    if (strncmp(startswith, _line_buf, len) == 0) {
        /* printf(" match on %s\n", startswith); */
        _line_buf_ready = false;
        return true;
    } else {
        return false;
    }
}
