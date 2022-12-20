#include "fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void* thread1() {
    int f1 = tfs_open("/f1", TFS_O_TRUNC);
    if (f1 == -1)
        printf("f1 não abriu\n");
    tfs_write(f1, "yoyo", 5);
    tfs_close(f1);

    int f2 = tfs_open("/f2", TFS_O_TRUNC);
    if (f2 == -1)
        printf("f2 não abriu\n");
    tfs_write(f2, "yoyo", 5);
    tfs_close(f2);

    int f3 = tfs_open("/f3", TFS_O_TRUNC);
    if (f3 == -1)
        printf("f3 não abriu\n");
    tfs_write(f3, "yoyo", 5);
    tfs_close(f3);

    if (tfs_link("/f4", "/f5") != 0)
        printf("hard link f5 para f4 falhou\n");
    if (tfs_link("/f2", "/f3") != 0)
        printf("hard link f3 para f2 falhou\n");
    if (tfs_unlink("/f4") != 0)
        printf("unlink f4 falhou\n");
    printf("--------------------------------------\n");
    return 0;
}

void* thread2() {
    int f2 = tfs_open("/f2", TFS_O_TRUNC);
    if (f2 == -1)
        printf("f2 não abriu\n");
    tfs_write(f2, "ola!", 5);
    tfs_close(f2);

    int f4 = tfs_open("/f4", TFS_O_TRUNC);
    if (f4 == -1)
        printf("f4 não abriu\n");
    tfs_write(f4, "ola!", 5);
    tfs_close(f4);

    int f5 = tfs_open("/f5", TFS_O_TRUNC);
    if (f5 == -1)
        printf("f5 não abriu\n");
    tfs_write(f5, "ola!", 5);
    tfs_close(f5);
    
    if (tfs_sym_link("/f3", "/f2") != 0)
        printf("hard link f5 para f2 falhou\n");
    if (tfs_link("/f4", "/f1") != 0)
        printf("hard link f1 para f4 falhou\n");
    if (tfs_unlink("/f3") != 0)
        printf("unlink f3 falhou\n");
    printf("--------------------------------------\n");
    return 0;
}
void* thread3() {
    int f1 = tfs_open("/f1", TFS_O_TRUNC);
    if (f1 == -1)
        printf("f1 não abriu\n");
    tfs_write(f1, "bebe", 5);
    tfs_close(f1);

    int f4 = tfs_open("/f4", TFS_O_TRUNC);
    if (f4 == -1)
        printf("f4 não abriu\n");
    tfs_write(f4, "bebe", 5);
    tfs_close(f4);

    int f5 = tfs_open("/f5", TFS_O_TRUNC);
    if (f5 == -1)
        printf("f5 não abriu\n");
    tfs_write(f5, "bebe", 5);
    tfs_close(f5);

    if (tfs_link("/f4", "/f5") != 0)
        printf("hard link f5 para f4 falhou\n");
    if (tfs_link("/f1", "/f2") != 0)
        printf("hard link f2 para f1 falhou\n");
    if (tfs_unlink("/f1") != 0)
        printf("unlink f1 falhou\n");
    printf("--------------------------------------\n");
    return 0;
}
void* thread4() {
    int f1 = tfs_open("/f1", TFS_O_TRUNC);
    if (f1 == -1)
        printf("f1 não abriu\n");
    tfs_write(f1, "pois", 5);
    tfs_close(f1);

    int f2 = tfs_open("/f2", TFS_O_TRUNC);
    if (f2 == -1)
        printf("f2 não abriu\n");
    tfs_write(f2, "pois", 5);
    tfs_close(f2);

    int f5 = tfs_open("/f5", TFS_O_TRUNC);
    if (f5 == -1)
        printf("f5 não abriu\n");
    tfs_write(f5, "pois", 5);
    tfs_close(f5);

    if (tfs_link("/f1", "/f2") != 0)
        printf("hard link f2 para f1 falhou\n");
    if (tfs_link("/f5", "/f3") != 0)
        printf("hard link f4 para f5 falhou\n");
    if (tfs_unlink("/f5") != 0)
        printf("unlink f5 falhou\n");
    printf("--------------------------------------\n");
    return 0;
}
void* thread5() {
    int f3 = tfs_open("/f3", TFS_O_TRUNC);
    if (f3 == -1)
        printf("f3 não abriu\n");
    tfs_write(f3, "lupa", 5);
    tfs_close(f3);

    int f4 = tfs_open("/f4", TFS_O_TRUNC);
    if (f4 == -1)
        printf("f4 não abriu\n");
    tfs_write(f4, "lupa", 5);
    tfs_close(f4);

    int f5 = tfs_open("/f5", TFS_O_TRUNC);
    if (f5 == -1)
        printf("f5 não abriu\n");
    tfs_write(f5, "lupa", 5);
    tfs_close(f5);

    if (tfs_link("/f4", "/f1") != 0)
        printf("hard link f1 para f4 falhou\n");
    if (tfs_link("/f1", "/f2") != 0)
        printf("hard link f2 para f1 falhou\n");
    if (tfs_sym_link("/f2", "/f5") != 0)
        printf("hard link f5 para f2 falhou\n");
    printf("--------------------------------------\n");
    return 0;
}

void *functions[5] = {thread1, thread2, thread3, thread4, thread5};

int main() {
    assert(tfs_init(NULL) != -1);

    tfs_close(tfs_open("/f1", TFS_O_CREAT));
    tfs_close(tfs_open("/f2", TFS_O_CREAT));
    tfs_close(tfs_open("/f3", TFS_O_CREAT));
    tfs_close(tfs_open("/f4", TFS_O_CREAT));
    tfs_close(tfs_open("/f5", TFS_O_CREAT));

    pthread_t tid[20];
    for (int i = 0; i < 20; i++){
        pthread_create(&tid[i], NULL, functions[rand()%5], NULL);
    }
    for (int i = 0; i < 20; i++){
        pthread_join(tid[i], NULL);
    }
    char buffer[100];
    int f1 = tfs_open("/f1", TFS_O_CREAT);
    tfs_read(f1, buffer, 100);
    printf("f1: %s\n", buffer);
    tfs_close(f1);
    int f2 = tfs_open("/f2", TFS_O_CREAT);
    tfs_read(f2, buffer, 100);
    printf("f2: %s\n", buffer);
    tfs_close(f2);
    int f3 = tfs_open("/f3", TFS_O_CREAT);
    tfs_read(f3, buffer, 100);
    printf("f3: %s\n", buffer);
    tfs_close(f3);
    int f4 = tfs_open("/f4", TFS_O_CREAT);
    tfs_read(f4, buffer, 100);
    printf("f4: %s\n", buffer);
    tfs_close(f4);
    int f5 = tfs_open("/f5", TFS_O_CREAT);
    tfs_read(f5, buffer, 100);
    printf("f5: %s\n", buffer);
    tfs_close(f5);
    return 0;
}
