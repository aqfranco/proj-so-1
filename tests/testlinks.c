#include "fs/operations.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char const target_path1[] = "/file";
char const link_path1[] = "/slink";
char const link_path2[] = "/hlink1";
char const link_path3[] = "/hlink2";
char const link_path4[] = "/slink2";

int main() {

    char buffer[16];
    // init TécnicoFS
    assert(tfs_init(NULL) != -1);

    // create file with content
    {
        int f = tfs_open(target_path1, TFS_O_CREAT);
        assert(tfs_close(f) != -1);
    }

    // create soft link on a file
    assert(tfs_sym_link(target_path1, link_path1) != -1);
    // try to create hard link on a soft link
    assert(tfs_link(link_path1, link_path2) == -1);
    // try to create a hard link with the same pathname as a soft link already created
    assert(tfs_link(target_path1, link_path1) == -1);
    // create a hard link on a file
    assert(tfs_link(target_path1, link_path2) != -1);
    // create a soft link to a soft link
    assert(tfs_sym_link(link_path1, link_path4) != -1);

    int sfile = tfs_open(link_path4, TFS_O_TRUNC);
    int tfile = tfs_open(target_path1, TFS_O_TRUNC);
    assert(sfile != -1);
    assert(tfile != -1);
    assert(tfs_write(sfile, "ola, ola", 9) != -1);
    assert(tfs_read(tfile, buffer, 9) != -1);
    assert(tfs_close(sfile) != -1);
    assert(tfs_close(tfile) != -1);
    // checks if when opening a soft link, it's correctly pointed to the file
    assert(!strcmp("ola, ola", buffer));


    // unlink /file
    assert(tfs_unlink(target_path1) != -1);
    // try to create a hard link to a file (which is deleted)
    assert(tfs_link(target_path1, link_path3) == -1);
    // creates a hard link to a hard link
    assert(tfs_link(link_path2, link_path3) != -1);


    // destroy TécnicoFS
    assert(tfs_destroy() != -1);

    printf("Successful test.\n");

    return 0;
}
