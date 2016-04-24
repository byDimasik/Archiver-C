#include "func.h"

/* Рассчет контрольной суммы crc32
   Принимает:
    *buf - указатель на массив данных, для которого нужно посчитать контрольную сумму
    len - длина buf
   Возвращает:
    crc - контрольная сумма
*/
uint32_t Crc32(unsigned char *buf, size_t len)
{
    uint32_t crc_table[256];
    uint32_t crc;
    int i, j;
    
    for (i = 0; i < 256; i++)
    {
        crc = i;
        for (j = 0; j < 8; j++)
            crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
        
        crc_table[i] = crc;
    };
    
    crc = 0xFFFFFFFFUL;
    
    while (len--)
        crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    
    return crc ^ 0xFFFFFFFFUL;
}


/* Создание архива и добавление в него файлов
   Принимает:
    **argv - указатель на массив аргументов, переданных программе
    **f_list - указатель на список файлов для архивации
    argc - длина argv
*/
int add_files(char **argv, FILE **f_list, int argc) {
    int i, size = 0;
    unsigned char *buf;     //массив данных файла
    struct stat *f_stat;    //массив структур stat для файла
    FILE *archive;          //архив
    char *filename;         //имя архива
    
    //----- Создание имени архива
    filename = (char*)malloc((strlen(argv[2])+strlen(".govno"))*sizeof(char)); //длина переданного программе имени + длина расширения
    strncat(filename, argv[2], strlen(argv[2]));    //добавляем переданное имя
    strncat(filename, ".govno", 6);                 //добавляем расширение
    //-----
    
    archive = fopen(filename, "wb");
    
    f_stat = (struct stat*)malloc((argc-3)*sizeof(struct stat)); //тут и далее argc-3 количество файлов
    
    //цикл генерации архива
    for (i = 0; i < argc-3; i++)
    {
        if (stat(argv[i+3], &f_stat[i])) //получаем информацию о файле
        {
            printf("Не удалось получить информацию о файле %d\n", i);
            exit(555);
        }
        printf("file = %s\n", argv[i+3]);
        printf("size = %lld\n", f_stat[i].st_size);
        
        size += f_stat[i].st_size; //считаем размер файла
        
        if (!(buf = (unsigned char*)malloc(f_stat[i].st_size*sizeof(char))))
        {
            printf("Рот трахал память кончилась\n");
            exit(666);
        }
        
        if (fread(buf, sizeof(char), f_stat[i].st_size, f_list[i]) != f_stat[i].st_size) //если колчество считанных байт не равно размеру
        {
            printf("Нихрена не прочиталось что за говно!!");
            exit(777);
        }
        
        //записываем данные о файле через ||  имя файла,        размер,      контрольная сумма
        fprintf(archive, "%s||%lld||%x||", argv[i+3], f_stat[i].st_size, Crc32(buf, f_stat[i].st_size));
        //записываем сам файл
        fwrite(buf, sizeof(char), f_stat[i].st_size, archive);
    }
    
    return 0;
}