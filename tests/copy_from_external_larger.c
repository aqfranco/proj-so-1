#include "fs/operations.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
 Checks if the copied content is limited to 1024 bytes (1Kb), the maximum allowed
in the TécnicoFS.
*/
int main() {

    char *str_ext_file =
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! "
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! " 
        "RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR! RRR!";
    char *path_copied_file = "/f1";
    char *path_src = "tests/file_to_copy_over1024.txt";
    char buffer[1050];

    assert(tfs_init(NULL) != -1);

    int f;
    ssize_t r;

    f = tfs_copy_from_external_fs(path_src, path_copied_file);
    assert(f != -1);

    f = tfs_open(path_copied_file, TFS_O_CREAT);
    assert(f != -1);

    r = tfs_read(f, buffer, sizeof(buffer));
    assert(r == 1024);
    assert(!memcmp(buffer, str_ext_file, 1024));

    printf("Successful test.\n");

    return 0;
}
