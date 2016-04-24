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
int add_files(char **argv, int argc) {
    int i, size = 0;
    unsigned char *buf;     //массив данных файла
    struct stat *f_stat;    //массив структур stat для файла
    FILE *archive;          //архив
    char *filename;         //имя архива
    FILE **f_list, *f;
    
    f_list = (FILE**)malloc((argc-3)*sizeof(FILE));
    for (i = 0; i < argc-3; i++)
        f_list[i] = NULL;
    for (i = 3; i < argc; i++)
    {
        if (!(f = fopen(argv[i], "rb")))
        {
            printf("Файл с именем %s не найден!\n", argv[i]);
            return ERR_NOFILE;
        }
        f_list[i-3] = f;
    }
    
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
            printf("Не удалось получить информацию о файле %s\n", argv[i+3]);
            exit(ERR_GETINFO);
        }
        
        size += f_stat[i].st_size; //считаем размер файла
        
        if (!(buf = (unsigned char*)malloc(f_stat[i].st_size*sizeof(char))))
            exit(ERR_OUTMEM);
        
        if (fread(buf, sizeof(char), f_stat[i].st_size, f_list[i]) != f_stat[i].st_size) //если колчество считанных байт не равно размеру файла
            exit(ERR_READ);
        
        //записываем данные о файле через ||    длина имени файла, имя файла,       размер,      контрольная сумма
        fprintf(archive, "%lu||%s||%lld||%x||", strlen(argv[i+3]), argv[i+3], f_stat[i].st_size, Crc32(buf, f_stat[i].st_size));
        //записываем сам файл
        if (fwrite(buf, sizeof(char), f_stat[i].st_size, archive) != f_stat[i].st_size)
            exit(ERR_WRITE);
        
        fclose(f_list[i]);
    }
    
    free(f_list);
    free(f_stat);
    
    return 0;
}

int extract_files(char **argv, int argc)
{
    int fsize = 0, crc = 0, len_name = 0;
    char *buf, *filename;
    FILE *archive, *f;
    struct stat f_stat;
    
    stat(argv[2], &f_stat);
    
    archive = fopen(argv[2], "rb");
    
    while (ftell(archive) != f_stat.st_size)
    {
        fscanf(archive, "%d%*c%*c", &len_name);
        filename = (char*)malloc((len_name+1)*sizeof(char));
        fread(filename, sizeof(char), len_name, archive);
        filename[len_name] = '\0';
        fscanf(archive, "%*c%*C%d%*c%*c%x%*c%*c", &fsize, &crc);
        
        printf("len_name = %d\nfilename = %s\nfsize = %d\ncrc = %x\n", len_name, filename, fsize, crc);
        
        buf = (char*)malloc(fsize*sizeof(char));
        if (fread(buf, sizeof(char), fsize, archive) != fsize)
            exit(ERR_WRITE);
        
        f = fopen(filename, "wb");
        fwrite(buf, sizeof(char), fsize, f);
    }
    return 0;
}












