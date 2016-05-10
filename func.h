#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define ERR_UNKNOWN_OPT 222 //неизвестный режим работы
#define ERR_NOFILE 333      //не найден файл для чтения
#define ERR_CREATEFILE 334  //не удалось создать файл для записи
#define ERR_NOARG 444       //не передано необходимое количество аргументов
#define ERR_GETINFO 555     //не удалось получить информацию о файле
#define ERR_OUTMEM 666      //нехватка памяти
#define ERR_READ 777        //ошибка чтения файла
#define ERR_WRITE 778       //ошибка записи файла

struct heads                           /* archive entry header format */
{   char *name;                        /* file name */
    size_t size;                       /* size of file, in bytes */
    int crc;                           /* cyclic redundancy check */
    long length;                       /* true file length */
}   ;

int add_files(char **, int size);
int extract_files(char **argv, int argc);
int show_flist(char **argv, int argc);
uint32_t Crc32(unsigned char *buf, size_t len);


int code(FILE *fp, FILE *fp3);
int decode(FILE *f, FILE *f_end);