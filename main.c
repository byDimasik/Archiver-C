#include "func.h"

int main(int argc, char *argv[])
{
    argc = 3;
    argv[1] = "x";
    argv[2] = "jopa.govno";
//    argv[3] = "6843-11.jpg";
//    argv[4] = "Git.pptx";
//    argv[5] = "Аттестат.pdf";
    
    if (argc < 2)
    {
        printf("Не переданы аргменты.\n");
        return ERR_NOARG;
    }
    
    if (!strcmp(argv[1], "l"))
        printf("Информация о файлах\n");
    
    else if (!strcmp(argv[1], "t"))
        printf("Проверка целостности архива\n");
        
    else if (!strcmp(argv[1], "a"))
        add_files(argv, argc);

    else if (!strcmp(argv[1], "x"))
    {
        extract_files(argv, argc);
        printf("Извлечение файлов\n");
    }

    else if (!strcmp(argv[1], "d"))
        printf("Удаление файла из архива\n");

    else
    {
        printf("%s - неизвестная опция.\n", argv[1]);
        return ERR_UNKNOWN_OPT;
    }
    
    return 0;
}
