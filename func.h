#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct heads                           /* archive entry header format */
{   char *name;                        /* file name */
    size_t size;                       /* size of file, in bytes */
    int crc;                           /* cyclic redundancy check */
    long length;                       /* true file length */
}   ;

int add_files(char **, FILE **, int size);
uint32_t Crc32(unsigned char *buf, size_t len);