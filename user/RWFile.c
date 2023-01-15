#include "stdio.h"
#include "fs.h"
#include "vfs.h"
#include "string.h"

const char* file_path[4] = {"dev_tty0/test.txt", 
                            "dev_tty0/test2.txt", 
                            "dev_tty0/test3.txt", 
                            "dev_tty0/test4.txt"};

void test_read(int file_id, char* buf)
{
    int fd = open(file_path[file_id], O_RDWR);
    read(fd, buf, 512);
    close(fd);
}

void test_write(int file_id, char* buf)
{
    int fd = open(file_path[file_id], O_RDWR);
    write(fd, buf, 512);
    close(fd);
}

int test(int i, int j)
{
    char buf1[520], buf2[520];
    int file1 = (i * 134245 + j * 234235) % 4;
    int file2 = (i * 324523 + j * 323423) % 4;
    test_read(file1, buf1);
    test_read(file2, buf2);
    test_write(file1, buf2);
    test_read(file1, buf1);
    // printf("%s\n", buf2);
    if (strcmp(buf1, buf2) != 0) return -1;
    return 0;
}

int main(int arg, char *argv[])  {
    int start = get_ticks();


    printf("[");
    for (int j = 0; j < 78; j ++ )
    {
        for (int i = 0; i < 200; i++)
        {
            if (test(i, j) == -1)
            {
                printf("Test failed!\n");
                break;
            }
        }
        printf("*");
    }
    printf("]");

    // if (test(0, 0) == -1) printf("Test faile!\n");
    int end = get_ticks();
    printf("The test has been finished!\nTotal time spent: %d ticks\n", end - start);
    
    // close(fd);
    return 0;
}