#include "func.h"

int main(int argc, char *argv[])
{
    int i;
    FILE *f, **f_list = NULL;

    argc = 5;
    argv[1] = "a";
    argv[2] = "6843-11.jpg";
    argv[3] = "Git.pptx";
    argv[4] = "Аттестат.pdf";
    
    if (argc < 2)
    {
        printf("Не переданы аргменты.\n");
        return 1;
    }
    else
    {
        f_list = (FILE**)malloc((argc-2)*sizeof(FILE));
        for (i = 0; i < argc-2; i++)
            f_list[i] = NULL;
        for (i = 2; i < argc; i++) {
            if (!(f = fopen(argv[i], "rb")))
            {
                printf("Файл с именем %s не найден!\n", argv[i]);
                return 1;
            }
            f_list[i-2] = f;
        }
    }
    
    if (!strcmp(argv[1], "l"))
        printf("Информация о файлах\n");
    
    else if (!strcmp(argv[1], "t"))
        printf("Проверка целостности архива\n");
    
    else if (argc < 3)
        {
            printf("Не передан файл(ы) или неизвестная опция!\n");
            return 1;
        }
        
    else if (!strcmp(argv[1], "a"))
    {
        add_files(argv, f_list, argc);
        printf("Добавление файлов\n");
    }

    else if (!strcmp(argv[1], "x"))
        printf("Извлечение файлов\n");

    else if (!strcmp(argv[1], "d"))
            printf("Удаление файла из архива\n");

    else
        printf("%s - неизвестная опция.\n", argv[1]);
        return 1;
    
    return 0;
}
