#include "stdio.h"
#include "fs.h"
#include "vfs.h"
#include "time.h"
<<<<<<< HEAD
=======
#include "string.h"
>>>>>>> lxc_dev

const char* file_path[4] = {"orange/test.txt", 
                            "orange/test2.txt", 
                            "orange/test3.txt", 
                            "orange/test4.txt"};


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
<<<<<<< HEAD
    printf("**********************************************\n");
=======
>>>>>>> lxc_dev
    printf("begin to read and write using buffer\n");
    printf("[");
    int begin, end;
    begin = get_ticks();
    for (int j = 0; j < 78; j ++ )
    {
<<<<<<< HEAD
        for (int i = 0; i < 1000; i++)
=======
        for (int i = 0; i < 50; i++)
>>>>>>> lxc_dev
        {
            if (test(i, j) == -1)
            {
                printf("Test failed!\n");
                break;
            }
        }
        printf("*");
    }
    end = get_ticks();
    printf("]");
    printf("The test has been finished!\n");
    printf("The time cost is %d \n", end - begin);
<<<<<<< HEAD
    printf("**********************************************\n\n");
    // close(fd);
    bh_refresh();
    reset_flag();
    printf("**********************************************\n");
    printf("not use buffer to read and write\n");
    printf("[");
    begin = get_ticks();
    for (int j = 0; j < 78; j ++ )
    {
        for (int i = 0; i < 1000; i++)
        {
            int tar = i % 4;
            char read_buf[128];
            char write_buf[128] = "Never gonna give you up!";

            int fd = open(file_path[tar], O_RDWR);
            read(fd, read_buf, 128);
            // printf("%s\n", read_buf);
            close(fd);

            fd = open(file_path[tar], O_RDWR);
            write(fd, write_buf, 128);
            close(fd);

            fd = open(file_path[tar], O_RDWR);
            read(fd, read_buf, 128);
            close(fd);
            // printf("%s\n", read_buf); 
=======
    // printf("**********************************************\n\n");
    // close(fd);
    bh_refresh();
    close_buf();
    // printf("**********************************************\n");
    printf("not use buffer to read and write\n");
      printf("[");
    begin = get_ticks();
    for (int j = 0; j < 78; j ++ )
    {
        for (int i = 0; i < 50; i++)
        {
            if (test(i, j) == -1)
            {
                printf("Test failed!\n");
                break;
            }
>>>>>>> lxc_dev
        }
        printf("*");
    }
    end = get_ticks();
    printf("]");
    printf("The test has been finished!\n");
    printf("The time cost is %d \n", end - begin);
<<<<<<< HEAD
    printf("**********************************************\n\n");
=======
    // printf("**********************************************\n\n");
>>>>>>> lxc_dev
    return 0;
}