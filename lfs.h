#ifndef LFS_H
#define LFS_H

#define BLOCK_SIZE 4096
#define INODE_SIZE (BLOCK_SIZE / sizeof(struct Inode_block))
#include <stdbool.h>
struct data_block {
    char data[BLOCK_SIZE];
};

struct Inode_block {
    int Inode_number;
    char filename[20];  
    int data_block_pointer[14];
    int data_block_number;
};

struct imap_table_row {
    int Inode_number;
    int Inode_offset;
};
struct Imap_block {
    struct imap_table_row rows [16];
    int offset;
    _Bool full;
    //struct Imap_block * previous_block;
};
struct folder_block_row {
    int Inode_number;
    char filename[20];
};
struct folder_block{
    struct folder_block_row files [200];
    int folder_offset;
    char foldername[20] ;
};
struct block {
    _Bool isfolder;
    struct data_block data_block;
    struct Inode_block Inode_block;
    struct Imap_block Imap_block;
    struct folder_block folder_block;
};

struct checkpoint_region {
    int imap_pointer [10];
    int disk_offset ;
    int available_inode_num;
    int current_folder_offset;
};

struct fileSystem {
    char * filename ;
    struct block block_region [100];
    struct checkpoint_region CR;
};
void close (struct fileSystem * fileSystem );
void initializeFileSystem(char *filename,struct fileSystem *fileSystem);
//open(), read(), write(), lseek(), close(), fsync()
void open (char *name,struct fileSystem* fileSystem);
char * read_file (struct fileSystem * fileSystem , char *filename);
void __cd (char *foldername, struct fileSystem *MyfileSystem);
void __ls (struct fileSystem *fileSystem);
void __mkdir (char *foldername, struct fileSystem *fileSystem);
void fsync(struct fileSystem *fileSystem);
#endif