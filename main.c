#include "func.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Не переданы аргменты.\n");
        return ERR_NOARG;
    }
    
    if (!strcmp(argv[1], "help"))
    {
        printf("Archiver - архиватор со сжатием реализованным по методу Хаффмана. Ключи:\n"
               "a - создать/добавить файлы в архив\n"
               "x - извлечь файл/файлы из архива\n"
               "d - удалить файл/файлы из архива\n"
               "l - вывести содержимое архива\n"
               "t - проверить архив на целостность\n");
        return 0;
    }
    
    if (!strcmp(argv[1], "l"))
        show_flist(argv, argc);
    
    else if (!strcmp(argv[1], "t"))
        check_integrity(argv, argc);
        
    else if (!strcmp(argv[1], "a"))
        add_files(argv, argc);

    else if (!strcmp(argv[1], "x"))
        extract_files(argv, argc);

    else if (!strcmp(argv[1], "d"))
        delete_files(argv, argc);

    else
    {
        printf("%s - неизвестная опция.\n", argv[1]);
        return ERR_UNKNOWN_OPT;
    }
    
    return 0;
}
