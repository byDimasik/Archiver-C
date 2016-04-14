#include "func.h"

int main(int argc, char *argv[]) {
    int i;
    FILE *f, **f_list = NULL;
    
    if (argc < 2)
    {
        printf("Не переданы аргменты.\n");
        return 1;
    }
    else
        for (i = 2; i < argc; i++) {
            if (!(f = fopen(argv[i], "r")))
            {
                printf("Файл с именем %s не найден!\n", argv[i]);
                return 1;
            }
            f_list = (FILE**)realloc(f_list, (i-1)*sizeof(FILE));
            f_list[i-2] = f;
        }
    
    if (!strcmp(argv[1], "l"))
        printf("Информация о файлах\n");
    
    else if (!strcmp(argv[1], "t"))
        printf("Проверка целостности архива\n");
    
    else
    {
        if (!strcmp(argv[1], "a"))
        {
            if (argc < 3)
            {
                printf("Не передан файл(ы)!\n");
                return 1;
            }
            printf("Добавление файлов\n");
        }

        else if (!strcmp(argv[1], "x"))
        {
            if (argc < 3)
            {
                printf("Не переданы файл(ы)!\n");
                return 1;
            }
            printf("Извлечение файлов\n");
        }

        else if (!strcmp(argv[1], "d"))
        {
            if (argc < 3)
            {
                printf("Не передан файл(ы)!\n");
                return 1;
            }
            printf("Удаление файла из архива\n");
        }

        else
        {
            printf("%s - неизвестная опция.\n", argv[1]);
            return 1;
        }
    }
    
    return 0;
}
