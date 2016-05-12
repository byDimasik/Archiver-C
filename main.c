#include "func.h"

int main(int argc, char *argv[])
{
//    argc = 4;
//    argv[1] = "a";
//    argv[2] = "test";
//    argv[3] = "10M_zero_bytes";
//    argv[3] = "Git.pptx";
//    argv[3] = "Аттестат.pdf";
    
    if (argc < 2)
    {
        printf("Не переданы аргменты.\n");
        return ERR_NOARG;
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
