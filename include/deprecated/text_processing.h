/*================================================================================
   Text processing module.
================================================================================*/
#ifndef HEADER_DEFINED_TEXT_PROCESSING
#define HEADER_DEFINED_TEXT_PROCESSING

#define _LINE_BUF_SIZE 4096
char _line_buf[_LINE_BUF_SIZE];

bool try_read_line(FILE *fd, char *line);
bool try_read_line_startswith(FILE *file, char *startswith);

#endif // HEADER_DEFINED_TEXT_PROCESSING
