#include "func.h"

int add_files(char **argv, FILE **f_list, int argc) {
    int i, size = 0;
    char *buf;
    struct stat *f_stat;
    FILE *archive;
    archive = fopen("archive.govno", "wb");
    
    f_stat = (struct stat*)malloc((argc-2)*sizeof(struct stat));
    
    for (i = 0; i < argc-2; i++)
    {
        if (stat(argv[i+2], &f_stat[i]))
        {
            printf("Не удалось получить информацию о файле %d\n", i);
            exit(1);
        }
        printf("file = %s\n", argv[i+2]);
        printf("size = %lld\n", f_stat[i].st_size);
        
        size += f_stat[i].st_size;
        
        if (!(buf = (char*)malloc(f_stat[i].st_size*sizeof(char))))
        {
            printf("Рот трахал память кончилась\n");
            exit(666);
        }
        
        if (fread(buf, sizeof(char), f_stat[i].st_size, f_list[i]) != f_stat[i].st_size)
        {
            printf("Нихрена не прочиталось что за говно!!");
            exit(777);
        }
        
        fprintf(archive, "%s||%lld", argv[i+2], f_stat[i].st_size);
        fwrite(buf, sizeof(char), f_stat[i].st_size, archive);
//        fwrite(argv[i+2], sizeof(char), strlen(argv[i+2]), archive);
//        fprintf(archive, "||");
//        fwrite(f_stat[i].st_size, sizeof(char), strlen(argv[i+2]), archive);
        
    }
    
    
    
    return 0;
}