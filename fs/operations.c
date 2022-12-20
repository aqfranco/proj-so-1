#include "operations.h"
#include "config.h"
#include "state.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "betterassert.h"

pthread_rwlock_t rw_lock;

tfs_params tfs_default_params() {
    tfs_params params = {
        .max_inode_count = 64,
        .max_block_count = 1024,
        .max_open_files_count = 16,
        .block_size = 1024,
    };
    return params;
}

int tfs_init(tfs_params const *params_ptr) {
    tfs_params params;
    if (params_ptr != NULL) {
        params = *params_ptr;
    } else {
        params = tfs_default_params();
    }

    if (state_init(params) != 0) {
        return -1;
    }

    // create root inode
    int root = inode_create(T_DIRECTORY);
    if (root != ROOT_DIR_INUM) {
        return -1;
    }
    pthread_rwlock_init(&rw_lock, NULL);
    return 0;
}

int tfs_destroy() {
    if (state_destroy() != 0) {
        return -1;
    }
    return 0;
}

static bool valid_pathname(char const *name) {
    return name != NULL && strlen(name) > 1 && name[0] == '/';
}

/**
 * Looks for a file.
 *
 * Note: as a simplification, only a plain directory space (root directory only)
 * is supported.
 *
 * Input:
 *   - name: absolute path name
 *   - root_inode: the root directory inode
 * Returns the inumber of the file, -1 if unsuccessful.
 */
static int tfs_lookup(char const *name, inode_t *root_inode) {
    if (!valid_pathname(name) || root_inode->i_node_type != T_DIRECTORY) {
        return -1;
    }

    // skip the initial '/' character
    name++;

    return find_in_dir(root_inode, name);
}

int tfs_open(char const *name, tfs_file_mode_t mode) {
    // Checks if the path name is valid
    if (!valid_pathname(name)) {
        return -1;
    }

    inode_t *root_dir_inode = inode_get(ROOT_DIR_INUM);
    ALWAYS_ASSERT(root_dir_inode != NULL,
                  "tfs_open: root dir inode must exist");
    int inum = tfs_lookup(name, root_dir_inode);
    size_t offset = 0;
    if (inum >= 0) {
        // The file already exists
        inode_t *inode = inode_get(inum);
        if (pthread_mutex_lock(&inode->mutex) != 0)
            return -1;
        ALWAYS_ASSERT(inode != NULL,
                      "tfs_open: directory files must have an inode");
        
        if (inode->i_node_type == T_SOFTLINK) { // if the given file is a soft (symbolic) link
            int soft_link_file = add_to_open_file_table(inum, offset);
            char file_name[MAX_FILE_NAME];
            tfs_read(soft_link_file, file_name, FILENAME_MAX);
            tfs_close(soft_link_file);
            pthread_mutex_unlock(&inode->mutex);
            return tfs_open(file_name, mode);
        }

        // Truncate (if requested)
        if (mode & TFS_O_TRUNC) {
            if (inode->i_size > 0) {
                data_block_free(inode->i_data_block);
                inode->i_size = 0;
            }
        }
        // Determine initial offset
        if (mode & TFS_O_APPEND) {
            offset = inode->i_size;
        }
        pthread_mutex_unlock(&inode->mutex);
    } else if (mode & TFS_O_CREAT) {
        // The file does not exist; the mode specified that it should be created
        // Create inode
        inum = inode_create(T_FILE);
        if (inum == -1) {
            return -1; // no space in inode table
        }

        // Add entry in the root directory
        if (add_dir_entry(root_dir_inode, name + 1, inum) == -1) {
            inode_delete(inum);
            return -1; // no space in directory
        }
        offset = 0;
    } else {
        return -1;
    }

    // Finally, add entry to the open file table and return the corresponding
    // handle
    return add_to_open_file_table(inum, offset);

    // Note: for simplification, if file was created with TFS_O_CREAT and there
    // is an error adding an entry to the open file table, the file is not
    // opened but it remains created
}

int tfs_sym_link(char const *target, char const *link_name) {
    inode_t *root_inode = inode_get(ROOT_DIR_INUM);
    pthread_rwlock_wrlock(&rw_lock);
    if (tfs_lookup(link_name, root_inode) != -1 ) { // link_name is already in the directory
        pthread_rwlock_unlock(&rw_lock); 
        return -1;
    }
    int inumber = inode_create(T_SOFTLINK);
    if (inumber == -1) {// error creating new inode
        pthread_rwlock_unlock(&rw_lock); 
        return -1;
    }
    if (add_dir_entry(root_inode, link_name+1, inumber) == -1) { // error adding link to directory
        inode_delete(inumber);
        pthread_rwlock_unlock(&rw_lock);
        return -1;
    }
    int link_file = add_to_open_file_table(inumber, 0);
    if (tfs_write(link_file, target, strlen(target)+1) < 0) { // writes target's path in the link's data
        pthread_rwlock_unlock(&rw_lock);
        return -1;
    }
    tfs_close(link_file);
    pthread_rwlock_unlock(&rw_lock);
    return 0;
}

int tfs_link(char const *target, char const *link_name) {
    inode_t *root_inode = inode_get(ROOT_DIR_INUM);
    if (tfs_lookup(link_name, root_inode) != -1) // link_name is already in the directory
        return -1;
    int inumber = tfs_lookup(target, root_inode);
    if (inumber == -1) // target file doesn't exist
        return -1;
    pthread_rwlock_rdlock(&rw_lock);
    inode_t *target_inode = inode_get(inumber);
    if (pthread_rwlock_wrlock(&target_inode->rw) != 0){
        pthread_rwlock_unlock(&rw_lock);
        return -1;
    }
    if (target_inode->i_node_type == T_SOFTLINK || add_dir_entry(root_inode, link_name + 1, inumber) == -1) { // can't create hard links to soft links
        pthread_rwlock_unlock(&target_inode->rw);
        pthread_rwlock_unlock(&rw_lock);
        return -1; // no space in root directory
    }
    target_inode->link_number += 1; // inode has one more link
    pthread_rwlock_unlock(&target_inode->rw);
    pthread_rwlock_unlock(&rw_lock);
    return 0;
}

int tfs_close(int fhandle) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1; // invalid fd
    }
    remove_from_open_file_table(fhandle);
    return 0;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t to_write) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) 
        return -1;
    inode_t *inode = inode_get(file->of_inumber);
    if (pthread_rwlock_wrlock(&inode->rw) != 0) {
        return -1;
    }

    ALWAYS_ASSERT(inode != NULL, "tfs_write: inode of open file deleted");

    // Determine how many bytes to write
    size_t block_size = state_block_size();
    if (to_write + file->of_offset > block_size) {
        to_write = block_size - file->of_offset;
    }

    if (to_write > 0) {
        if (inode->i_size == 0) {
            // If empty file, allocate new block
            int bnum = data_block_alloc();
            if (bnum == -1) {
                pthread_rwlock_unlock(&inode->rw);
                return -1; // no space
            }

            inode->i_data_block = bnum;
        }

        void *block = data_block_get(inode->i_data_block);
        ALWAYS_ASSERT(block != NULL, "tfs_write: data block deleted mid-write");

        // Perform the actual write
        memcpy(block + file->of_offset, buffer, to_write);

        // The offset associated with the file handle is incremented accordingly
        file->of_offset += to_write;
        if (file->of_offset > inode->i_size) {
            inode->i_size = file->of_offset;
        }
    }
    pthread_rwlock_unlock(&inode->rw);
    return (ssize_t)to_write;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }
    pthread_rwlock_rdlock(&rw_lock);
    inode_t *inode = inode_get(file->of_inumber);
    if (pthread_rwlock_rdlock(&inode->rw) != 0) {
        pthread_rwlock_unlock(&rw_lock);
        return -1;
    }
    ALWAYS_ASSERT(inode != NULL, "tfs_read: inode of open file deleted");

    // Determine how many bytes to read
    size_t to_read = inode->i_size - file->of_offset;
    if (to_read > len) {
        to_read = len;
    }

    if (to_read > 0) {
        void *block = data_block_get(inode->i_data_block);
        ALWAYS_ASSERT(block != NULL, "tfs_read: data block deleted mid-read");

        // Perform the actual read
        memcpy(buffer, block + file->of_offset, to_read);
        // The offset associated with the file handle is incremented accordingly
        file->of_offset += to_read;
    }
    pthread_rwlock_unlock(&inode->rw);
    pthread_rwlock_unlock(&rw_lock);
    return (ssize_t)to_read;
}

int tfs_unlink(char const *target) {
    inode_t *root_inode = inode_get(ROOT_DIR_INUM);
    int inumber = tfs_lookup(target, root_inode);
    if (inumber == -1) // file doesn't exist in root directory
        return -1;
    pthread_rwlock_rdlock(&rw_lock);
    inode_t *target_inode = inode_get(inumber);
    if (pthread_rwlock_wrlock(&target_inode->rw) != 0){
        pthread_rwlock_unlock(&rw_lock);
        return -1;
    }
    if (target_inode->link_number == 1) // the inode has no more links
        inode_delete(inumber);
    else 
        target_inode->link_number -= 1;
    clear_dir_entry(root_inode, target+1);
    pthread_rwlock_unlock(&target_inode->rw);
    pthread_rwlock_unlock(&rw_lock);
    return 0;
}

int tfs_copy_from_external_fs(char const *source_path, char const *dest_path) {
    FILE *source = fopen(source_path, "r");
    int tfs_file;
    char buffer[1024]; // 1024 = 1Kb, maximum file size in TécnicoFS
    memset(buffer, 0, 1024);
    if (!source) // source file doesn't exist / isn't correctly written in call
        return -1;
    if (tfs_lookup(dest_path, inode_get(ROOT_DIR_INUM)) == -1) // file doesn't exist in TécnicoFS
        tfs_file = tfs_open(dest_path, TFS_O_CREAT);
    else 
        tfs_file = tfs_open(dest_path, TFS_O_TRUNC);
    size_t read = fread(buffer, sizeof(*buffer), 1024, source); 
    ssize_t written = tfs_write(tfs_file, buffer, read);
    int source_close = fclose(source);
    int tfs_file_close = tfs_close(tfs_file);
    if (written < 0 || source_close == -1 || tfs_file_close == -1){// error writing or closing the files
        return -1;
    } 
    return 0;
}
