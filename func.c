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
    int i, update = 0, this_f = 0;
    unsigned char *buf;               //массив данных файла
    struct stat *f_stat, arc_stat;    //массив структур stat для файла и струтура для архива
    FILE *archive = NULL, *old_arc;   //архив
    FILE **f_list, *f;
    unsigned long len_name;
    uint32_t crc;
    char *name_old_arc = NULL, *filename;
    long long fsize;

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
    
    if (!(f_stat = (struct stat*)malloc((argc-3)*sizeof(struct stat)))) //тут и далее argc-3 количество файлов
        exit(ERR_OUTMEM);
    
    if (stat(argv[2], &arc_stat) != -1) //если архив с переданным именем уже существует
    {
        update = 1;
        name_old_arc = (char*)malloc((strlen(argv[2])+strlen(".old"))*sizeof(char));
        strcpy(name_old_arc, argv[2]);
        strcat(name_old_arc, ".old");
        if (rename(argv[2], name_old_arc)) //переименовывем старый
        {
            printf("Не удалось переименовать архив!\n");
            exit(ERR_WRITE);
        }
    }
    
    //Создаем архив из переданных файлов
    for (i = 0; i < argc-3; i++)
    {
        if (stat(argv[i+3], &f_stat[i])) //получаем информацию о файле
            exit(ERR_GETINFO);
        
        if (!f_stat[i].st_size)
        {
            printf("Файл %s пустой и не будет добавлен в архив!\n", argv[i+3]);
            fclose(f_list[i]);
            continue;
        }
        
        if (!i)
            if (!(archive = fopen(argv[2], "wb")))
                exit(ERR_CREATEFILE);
        
        if (!(buf = (unsigned char*)malloc(f_stat[i].st_size*sizeof(char))))
            exit(ERR_OUTMEM);
        
        if (fread(buf, sizeof(char), f_stat[i].st_size, f_list[i]) != f_stat[i].st_size) //если колчество считанных байт не равно размеру файла
            exit(ERR_READ);
        
        //----- Записываем заголовок
        len_name = strlen(argv[i+3]);
        crc = Crc32(buf, f_stat[i].st_size);
        fwrite(&len_name, sizeof(unsigned long), 1, archive);
        fwrite(argv[i+3], sizeof(char), len_name, archive);
        fwrite(&f_stat[i].st_size, sizeof(long long), 1, archive);
        fwrite(&crc, sizeof(uint32_t), 1, archive);
        //------
        
        fseek(f_list[i], 0, SEEK_SET);

        code(f_list[i], archive);
        
        fclose(f_list[i]); //закрываем записанный файл
        free(buf);         //освбождаем буфер
    }
    free(filename);
    
    if (update) //Если есть старый архив, копируем из него файлы, которые не были переданы
    {
        old_arc = fopen(name_old_arc, "rb");
        
        while (ftell(old_arc) != arc_stat.st_size)
        {
            int k, ts, count;
            unsigned char *ch;
            float *freq;
            
            //-------- Считывание заголовка файла
            fread(&len_name, sizeof(unsigned long), 1, old_arc);
            if (!(filename = (char*)malloc((len_name+1)*sizeof(char))))
                exit(ERR_OUTMEM);
            if (fread(filename, sizeof(char), len_name, old_arc) != len_name)
                exit(ERR_READ);
            filename[len_name] = '\0';
            fread(&fsize, sizeof(long long), 1, old_arc);
            fread(&crc, sizeof(uint32_t), 1, old_arc);
            //--------
        
            for (i = 3; i < argc; i++)
                if (!strcmp(argv[i], filename))
                    this_f = 1; //Если найден файл, который был обновлен, устанавливаем флаг

            
            fread(&k, sizeof(int), 1, old_arc);                 //количество уникальных символов
            fread(&ts, sizeof(int), 1, old_arc);                //величина хвоста
            
            ch = (unsigned char*)malloc((k+1)*sizeof(unsigned char));
            freq = (float*)malloc((k+1)*sizeof(float));
            
            for (i = 0; i <= k; i++) //считываем таблицу встречаемости
            {
                fread(&ch[i], sizeof(unsigned char), 1, old_arc);
                fread(&freq[i], sizeof(float), 1, old_arc);
            }
            fread(&count, sizeof(int), 1, old_arc);
            
            if (this_f) //пропускаем файл, который есть уже обновленный в новом архиве
            {
                if (fseek(old_arc, count, SEEK_CUR))
                    exit(ERR_READ);
                
                free(filename);
                free(ch);
                free(freq);

                continue;
            }
    
            buf = (unsigned char*)malloc(count*sizeof(unsigned char));
            
            fread(buf, sizeof(unsigned char), count, old_arc);
            
            //----- Записываем заголовок
            fwrite(&len_name, sizeof(unsigned long), 1, archive);
            fwrite(filename, sizeof(char), len_name, archive);
            fwrite(&fsize, sizeof(long long), 1, archive);
            fwrite(&crc, sizeof(uint32_t), 1, archive);
            //------
            
            fwrite(&k, sizeof(int), 1, archive);                 //количество уникальных символов
            fwrite(&ts, sizeof(int), 1, archive);                //величина хвоста
            
            //Записываем в сжатый файл таблицу встречаемости
            fwrite(ch, sizeof(unsigned char), k+1, archive);
            fwrite(freq, sizeof(float), k+1, archive);
            
            fwrite(&count, sizeof(int), 1, archive);
            
            fwrite(buf, sizeof(unsigned char), count, archive);
            
            free(filename);
            free(buf);
            free(ch);
            free(freq);
        }
        fclose(old_arc);
        remove(name_old_arc);
        free(name_old_arc);
    }
    
    fclose(archive); //закрываем архив

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
    long long fsize = 0;          //размер файла
    int this_f = 0;               //флаг для извлечения отдельного файла
    unsigned long len_name = 0;   //длина имени
    uint32_t crc = 0;             //контрольная сумма
    
    char *filename;         //имя файла
    
    FILE *archive;          //переданный архив
    FILE *f = NULL;         //обрабатываемый файл
    struct stat arc_stat;   //структура с информацией об архиве
    
    stat(argv[2], &arc_stat);
    
    if (!(archive = fopen(argv[2], "rb"))) //открываем архив на чтение
        exit(ERR_CREATEFILE);
    
    while (ftell(archive) != arc_stat.st_size)
    {
        //-------- Считывание заголовка файла
        fread(&len_name, sizeof(unsigned long), 1, archive);
        if (!(filename = (char*)malloc((len_name+1)*sizeof(char))))
            exit(ERR_OUTMEM);
        if (fread(filename, sizeof(char), len_name, archive) != len_name)
            exit(ERR_READ);
        filename[len_name] = '\0';
        fread(&fsize, sizeof(long long), 1, archive);
        fread(&crc, sizeof(uint32_t), 1, archive);
        //--------

        if (argc > 3)
            for (i = 3; i < argc; i++)
                if (!strcmp(argv[i], filename))
                    this_f = 1; //если переданы файлы, которые надо извлечь, если эти файлы найдены в архиве, устанавливаем флаг this_f в 1
        
        if (this_f || (argc == 3)) //если найден файл для извлечения, либо если файлов не передано вообще
        {
            if (!(f = fopen(filename, "wb"))) //создаем пустой файл с именем извлекаемого
                exit(ERR_CREATEFILE);
            
            printf("Start decoding %s...\n", filename);
            decode(archive, f, fsize);
            
            fclose(f); //закрываем созданный файл
            
            this_f = 0;
        }
        else
        {
            int k, ts, count;
            unsigned char ch;
            float freq;
            fread(&k, sizeof(int), 1, archive);                 //количество уникальных символов
            fread(&ts, sizeof(int), 1, archive);                //величина хвоста
            
            for (i = 0; i <= k; i++) //считываем таблицу встречаемости
            {
                fread(&ch, sizeof(unsigned char), 1, archive);
                fread(&freq, sizeof(float), 1, archive);
            }
            fread(&count, sizeof(int), 1, archive);

            if (fseek(archive, count, SEEK_CUR)) //если передан файл для извлечения
                exit(ERR_READ);
        }
    
        free(filename); //освобождаем имя файла
        
    }
    
    fclose(archive); //закрываем архив
    
    return 0;
}


/* Отображение файлов в архиве
   Принимает: переданные программе параметры
   Возвращает: код ошибки, либо 0 при успешном завершении
*/
int show_flist(char **argv, int argc) {
    FILE *arc;
    
    int k, count;
    long long fsize = 0;          //размер файла
    unsigned long len_name = 0;   //длина имени
    uint32_t crc = 0;             //контрольная сумма
    
    char *filename;         //имя файла
    
    struct stat arc_stat;   //структура с информацией об архиве
    
    stat(argv[2], &arc_stat);
    
    if (argc != 3)
        exit(ERR_NOARG);
    arc = fopen(argv[2], "rb");
    
    while (ftell(arc) != arc_stat.st_size)
    {
        //-------- Считывание заголовка файла
        fread(&len_name, sizeof(unsigned long), 1, arc);
        if (!(filename = (char*)malloc((len_name+1)*sizeof(char))))
            exit(ERR_OUTMEM);
        if (fread(filename, sizeof(char), len_name, arc) != len_name)
            exit(ERR_READ);
        filename[len_name] = '\0';
        fread(&fsize, sizeof(long long), 1, arc);
        fread(&crc, sizeof(uint32_t), 1, arc);
        //--------
        
        printf("File: %s\nSize = %lld bytes\t\tCrc = %x\n\n", filename, fsize, crc);

        fread(&k, sizeof(int), 1, arc);                 //количество уникальных символов
        fseek(arc, sizeof(int)+(k+1)*sizeof(unsigned char)+(k+1)*sizeof(float), SEEK_CUR);
        fread(&count, sizeof(int), 1, arc);
        
        if (fseek(arc, count, SEEK_CUR))
            exit(ERR_READ);
    
    }
    return 0;
}












