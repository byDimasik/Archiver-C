#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sym //структуры или записи
{
    unsigned char ch;
    float freq;
    char code[255];
    struct sym *left;
    struct sym *right;
}; typedef struct sym sym;

union code
{
    unsigned char chhh;//переменная содержащая код для записи в сжатый файл
    
    struct byte
    {
        unsigned b1:1;
        unsigned b2:1;
        unsigned b3:1;
        unsigned b4:1;
        unsigned b5:1;
        unsigned b6:1;
        unsigned b7:1;
        unsigned b8:1;
    }byte;
};

sym *makeTree(sym *psym[], int k)//рeкурсивная функция создания дерева Хаффмана
{
    sym *temp;
    temp = (sym*)malloc(sizeof(sym));
    temp->freq = psym[k-1]->freq + psym[k-2]->freq;
    temp->code[0] = 0;
    temp->left = psym[k-1];
    temp->right = psym[k-2];
    
    if (k == 2)
        return temp;
    else //внесение в массив в нужное место элемента дерева Хаффмана
    {
        for (int i = 0; i < k; i++)
            if (temp->freq > psym[i]->freq)
            {
                for (int j = k-1; j > i; j--)
                    psym[j] = psym[j-1];
                
                psym[i] = temp;
                break;
            }
    }
    return makeTree(psym,k-1);
}

void makeCodes(sym *root)//Рекурсивная функция кодирования
{
    if (root->left)
    {
        strcpy(root->left->code, root->code);
        strcat(root->left->code, "0");
        makeCodes(root->left);
    }
    if (root->right)
    {
        strcpy(root->right->code, root->code);
        strcat(root->right->code, "1");
        makeCodes(root->right);
    }
}


int code(FILE *fp, FILE *fp3)
{
    FILE *fp2;                        //указатели на файлы
    int chh;                          //в эту переменную читается информация из файла
    int k = 0;                        //счётчик уникальных символов
    int kk = 0;                       //счётчик всех знаков в файле
    int fsize2 = 0;                   //счётчик символов из 0 и 1 в промежуточном файле teemp
    int ts;                           //размер хвоста файла (то, что не кратно 8 в промежуточном файле)
    int kolvo[256] = {0};             //инициализируем массив количества уникальных символов
    sym simbols[256] = {0};           //инициализируем массив записей
    sym *psym[256];                   //инициализируем массив указателей на записи
    int mes[8];                       //массив 0 и 1
    int i, j = 0;                     //вспомогательные переменные
    int count = 0;
    unsigned char *buf_end = NULL;
    
    for (i = 0; i < 256; i++)
        simbols[i].ch = 0;
    
    //Начинаем побайтно читать файл и составлять таблицу встречаемости
    while ((chh = fgetc(fp)) != EOF)
    {
        for (i = 0; i < 256; i++)
        {
            if ((unsigned char)chh == simbols[i].ch)
            {
                kolvo[i]++;
                kk++;
                break;
            }
            if (simbols[i].ch == 0)
            {
                simbols[i].ch = (unsigned char)chh;
                kolvo[i] = 1;
                k++;
                kk++;
                break;
            }
        }
    }
    
    // Рассчёт частоты встречаемости
    for (i = 0; i <= k; i++)
        simbols[i].freq = (float)kolvo[i]/kk;
    
    for (i = 0; i <= k; i++) //в массив указателей заносим адреса записей
        psym[i] = &simbols[i];
    
    //Сортировка по убыванию
    sym tempp;
    for (i = 0; i < k; i++)
        for (j = 0; j < k-1; j++)
            if (simbols[j].freq < simbols[j+1].freq)
            {
                tempp = simbols[j];
                simbols[j] = simbols[j+1];
                simbols[j+1] = tempp;
            }
    
    sym *root = makeTree(psym, k+1); //создание дерева Хаффмана
    
    makeCodes(root); //вызов функции получения кода
    
    rewind(fp); //возвращаем указатель в файле в начало файла

    fp2 = fopen("teemp.txt","wb");    //открываем файл для записи бинарного кода
    
    //в цикле читаем исходный файл, и записываем полученные в функциях коды в промежуточный файл
    while ((chh = fgetc(fp)) != EOF)
    {
        for (i = 0; i <= k; i++)
            if ((unsigned char)chh == simbols[i].ch)
                fputs(simbols[i].code, fp2);
    }
    fclose(fp2);
    
    //Заново открываем файл с бинарным кодом, но теперь для чтения
    fp2 = fopen("teemp.txt", "rb");
    //Считаем размер бинарного файла(количество символов в нём)
    while ((chh = fgetc(fp2)) != EOF)
        fsize2++;
    
    ts = fsize2 % 8;//находим остаток, количество символов не кратных 8 (хвост)
    
    //формируем заголовок сжатого файла через поля байтов
    
    fwrite(&k, sizeof(int), 1, fp3);                 //количество уникальных символов
    fwrite(&ts, sizeof(int), 1, fp3);                //величина хвоста
    
    //Записываем в сжатый файл таблицу встречаемости
    for (i = 0; i <= k; i++)
    {
        fwrite(&simbols[i].ch, sizeof(unsigned char), 1, fp3);
        fwrite(&simbols[i].freq, sizeof(float), 1, fp3);
    }
    
    rewind(fp2);//возвращаем указатель в промежуточном файле в начало файла
    
    union code code1; //инициализируем переменную code1
    
    //Читаем бинарный файл, занося последовательно каждые 8 элементов в массив для последующей побитовой обработки в объединении union
    j = 0;
    for (i = 0; i < fsize2-ts; i++)
    {
        mes[j] = fgetc(fp2);
        if (j == 7)
        {
            code1.byte.b1 = mes[0]-'0';
            code1.byte.b2 = mes[1]-'0';
            code1.byte.b3 = mes[2]-'0';
            code1.byte.b4 = mes[3]-'0';
            code1.byte.b5 = mes[4]-'0';
            code1.byte.b6 = mes[5]-'0';
            code1.byte.b7 = mes[6]-'0';
            code1.byte.b8 = mes[7]-'0';
            
//            fwrite(&code1.chhh, sizeof(unsigned char), 1, fp3);
            count++;
            buf_end = (unsigned char*)realloc(buf_end, count*sizeof(unsigned char));
            buf_end[count-1] = code1.chhh;
            
            j = 0;
            continue;
        }
        j++;
    }
    
    for (i = 0; i < 8; i++)
        mes[i] = 0;
    
    //Записываем хвост
    j = 0;
    for (i = 0; i <= ts; i++)
    {
        mes[j] = fgetc(fp2);
        if (j == ts)
        {
            code1.byte.b1 = mes[0]-'0';
            code1.byte.b2 = mes[1]-'0';
            code1.byte.b3 = mes[2]-'0';
            code1.byte.b4 = mes[3]-'0';
            code1.byte.b5 = mes[4]-'0';
            code1.byte.b6 = mes[5]-'0';
            code1.byte.b7 = mes[6]-'0';
            code1.byte.b8 = mes[7]-'0';
            
//            fwrite(&code1.chhh, sizeof(unsigned char), 1, fp3);
            count++;
            buf_end = (unsigned char*)realloc(buf_end, count*sizeof(unsigned char));
            buf_end[count-1] = code1.chhh;
            
            j = 0;
            continue;
        }
        j++;
    }
    fclose(fp2);
    remove("teemp.txt");
    
    fwrite(&count, sizeof(int), 1, fp3);
    printf("count = %d\n", count);
    fwrite(buf_end, sizeof(unsigned char), count, fp3);
    
    return count;
}

int decode(FILE *f, FILE *f_end) {
    FILE *f_temp;                     //временный файл
    char buf_code[256], chh, end, mes[8];
    int i, j, ch, ts, k, count = 0;
    unsigned char c;
    sym simbols[256] = {0};           //массив структур символов
    sym *psym[256];                   //массив указателей на структуры символов
    union code code1;                 //объединение для побитовой записи символов
    
    for (i = 0; i < 256; i++) //зануляем символы и частоту встречаемости
    {
        simbols[i].ch = 0;
        simbols[i].freq = 0;
    }
    
    fread(&k, sizeof(int), 1, f);                 //количество уникальных символов
    fread(&ts, sizeof(int), 1, f);                //величина хвоста
    
    for (i = 0; i <= k; i++) //считываем таблицу встречаемости
    {
        fread(&simbols[i].ch, sizeof(unsigned char), 1, f);
        fread(&simbols[i].freq, sizeof(float), 1, f);
    }
    fread(&count, sizeof(int), 1, f);
    
    for (i = 0; i <= k; i++) //в массив указателей заносим адреса записей
        psym[i] = &simbols[i];
    
    sym *root = makeTree(psym, k+1); //создание дерева Хаффмана
    
    makeCodes(root);   //получение кода для символов
    
    f_temp = fopen("teemp.txt", "wb");
    
    j = 0;
    //---- записываем во временный файл двоичную запись раскодированного файла
    while (j != count)
    {
        ch = fgetc(f);
        c = (unsigned char)ch;

        code1.chhh = c;
        
        mes[0] = code1.byte.b1 + '0';
        mes[1] = code1.byte.b2 + '0';
        mes[2] = code1.byte.b3 + '0';
        mes[3] = code1.byte.b4 + '0';
        mes[4] = code1.byte.b5 + '0';
        mes[5] = code1.byte.b6 + '0';
        mes[6] = code1.byte.b7 + '0';
        mes[7] = code1.byte.b8 + '0';
        
        fwrite(mes, sizeof(char), 8, f_temp);
        for (i = 0; i < 8; i++)
            mes[i] = 0;
        j++;
    }
    //----
    
    //---- обрезаем остаток
    if ((ts % 2) || (ts == 0))
        fseek(f_temp, -ts-1, SEEK_CUR);
    else
        fseek(f_temp, -ts, SEEK_CUR);
    int a = -1;
    fwrite(&a, sizeof(EOF), 1, f_temp);
    //----
    fclose(f_temp);
    
    for (i = 0; i < 256; i++)
        buf_code[i] = 0;
    
    f_temp = fopen("teemp.txt", "rb");
    //---- считываем из временного файла битовые последовательности и преобразуем в исходные символы
    while ((ch = fgetc(f_temp)) != EOF)
    {
        chh = ch;
        strncat(buf_code, &chh, 1);
 
        for (i = 0; i <= k; i++)
            if (!strcmp(buf_code, simbols[i].code))
            {
                end = (char)simbols[i].ch;
                fwrite(&end, sizeof(char), 1, f_end);
                memset(buf_code, 0, strlen(buf_code));
            }
    }
    //----
    
    fclose(f_temp);
    fclose(f_end);
    remove("teemp.txt");
    
    return 0;
}