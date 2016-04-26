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
   Возвращает код ошибки в случае ошибки и 0 при успешном выполнении
*/
int add_files(char **argv, int argc) {
    int i, size = 0;
    unsigned char *buf;     //массив данных файла
    struct stat *f_stat;    //массив структур stat для файла
    FILE *archive;          //архив
    char *arcname;          //имя архива
    FILE **f_list, *f;
    
    //FIXME добавить проверку для файлов, защищенных от записей
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
    if (!(arcname = (char*)malloc((sizeof(argv[2])+strlen(".govno"))*sizeof(char)))) //длина переданного программе имени + длина расширения
        exit(ERR_OUTMEM);
    strncat(arcname, argv[2], strlen(argv[2]));    //добавляем переданное имя
    strncat(arcname, ".govno", 6);                 //добавляем расширение
    //-----
    
    if (!(archive = fopen(arcname, "wb")))
        exit(ERR_CREATEFILE);
    
    if (!(f_stat = (struct stat*)malloc((argc-3)*sizeof(struct stat)))) //тут и далее argc-3 количество файлов
        exit(ERR_OUTMEM);
    
    //цикл генерации архива
    for (i = 0; i < argc-3; i++)
    {
        if (stat(argv[i+3], &f_stat[i])) //получаем информацию о файле
            exit(ERR_GETINFO);
        
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
        
        fclose(f_list[i]); //закрываем записанный файл
        free(buf);         //освбождаем буфер
    }
    
    fclose(archive); //закрываем архив
    
    free(arcname);   //освобождаем имя архива
    free(f_list);    //освобождаем массив файлов
    free(f_stat);    //освобождаем массив структур
    
    return 0;
}

/* Функция извлечения файла(ов) из архива. Если в argv переданы файлы для извлечения, извлекутся только они, иначе извлекутся все файлы в архиве
   Принимает:
    argv - аргументы, переданные программе
    argc - количество этих аргументов
   Возвращает код ошибки в случае ошибки и 0 при успешном выполнении
*/
int extract_files(char **argv, int argc)
{
    int i;
    int fsize = 0;          //размер файла
    int crc = 0;            //контрольная сумма
    int len_name = 0;       //длина имени
    int this_f = 0;         //флаг для извлечения отдельного файла
    
    char *buf;              //буфер для считывания файла
    char *filename;         //имя файла
    
    FILE *archive;          //переданный архив
    FILE *f;                //обрабатываемый файл
    struct stat arc_stat;   //структура с информацией об архиве
    
    stat(argv[2], &arc_stat);
    
    if (!(archive = fopen(argv[2], "rb"))) //открываем архив на чтение
        exit(ERR_CREATEFILE);
    
    while (ftell(archive) != arc_stat.st_size)
    {
        //-------- Считывание заголовка файла
        fscanf(archive, "%d%*c%*c", &len_name);
        if (!(filename = (char*)malloc((len_name+1)*sizeof(char))))
            exit(ERR_OUTMEM);
        if (fread(filename, sizeof(char), len_name, archive) != len_name)
            exit(ERR_READ);
        filename[len_name] = '\0';
        fscanf(archive, "%*c%*c%d%*c%*c%x%*c%*c", &fsize, &crc);
        //--------
        
        if (argc > 3)
            for (i = 3; i < argc; i++)
                if (!strcmp(argv[i], filename))
                    this_f = 1; //если переданы файлы, которые надо извлечь, если эти файлы найдены в архиве, устанавливаем флаг this_f в 1
        
        if (this_f || (argc == 3)) //если найден файл для извлечения, либо если файлов не передано вообще
        {
            if (!(buf = (char*)malloc(fsize*sizeof(char))))
                exit(ERR_OUTMEM);
            if (fread(buf, sizeof(char), fsize, archive) != fsize) //считываем файл из архива
                exit(ERR_WRITE);
        
            if (!(f = fopen(filename, "wb"))) //создаем пустой файл с именем извлекаемого
                exit(ERR_CREATEFILE);
            
            if (fwrite(buf, sizeof(char), fsize, f) != fsize) //записываем извлеченный файл отдельно
                exit(ERR_WRITE);
            
            fclose(f); //закрываем созданный файл
            free(buf); //освобождаем буфер
            
            this_f = 0;
        }
        else
        {
            if (fseek(archive, fsize, SEEK_CUR)) //если передан файл для извлечения
                exit(ERR_READ);
        }
    
        free(filename); //освобождаем имя файла
        
    }
    
    fclose(archive); //закрываем архив
    
    return 0;
}












