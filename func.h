#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define ERR_UNKNOWN_OPT 222
#define ERR_NOFILE 333
#define ERR_NOARG 444
#define ERR_GETINFO 555
#define ERR_OUTMEM 666
#define ERR_READ 777
#define ERR_WRITE 778

struct heads                           /* archive entry header format */
{   char *name;                        /* file name */
    size_t size;                       /* size of file, in bytes */
    int crc;                           /* cyclic redundancy check */
    long length;                       /* true file length */
}   ;

int add_files(char **, int size);
int extract_files(char **argv, int argc);
uint32_t Crc32(unsigned char *buf, size_t len);