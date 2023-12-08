#include "lfs.h"
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <string.h>  
  
void initializeFileSystem(char *filename ,struct fileSystem *fileSystem) {
    fileSystem->CR.disk_offset = 0;
    fileSystem->filename = (char *)malloc(strlen(filename) + 1);  // allocate Memory
}
void open(char *filename, struct fileSystem *fileSystem) {
    const char   *directoryPath = "/Users/karimfeki/Desktop/IMT/systémes d'exploitation/filesystems-distributed/lfs2.0/";
    _Bool exist = 0;
    DIR *dir;
    struct dirent *entry;
    initializeFileSystem(filename,fileSystem);  // Initialize the file system
    if ((dir = opendir(directoryPath)) == NULL) {
        perror("Error opening\n");
    }
    printf("dir exists\n");

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, filename) == 0) {
            FILE *file = fopen(filename, "rb");
            if (file == NULL) {
                perror("Error opening file for reading\n");
                return;
            }
            fread(fileSystem, sizeof(struct fileSystem), 1, file);
            fileSystem->filename = (char *)malloc(strlen(filename) + 1);
            strcpy(fileSystem->filename, filename);
            fileSystem->CR.current_folder_offset = 1;
            exist = 1;
            printf("file exists and the file system is loaded\n");
            fclose(file);
        }
    }
    closedir(dir);
    //creating a new FS
    if (!exist) {
        FILE *file = fopen(filename, "wb");
        if (file == NULL) {
            perror("Error opening file for writing\n");
            return;
        }
        if (fileSystem->filename == NULL) {
            perror("Error allocating memory for filename\n");
            fclose(file);
            return;
        }
        strcpy(fileSystem->filename, filename);
        
        fileSystem->CR.disk_offset = 2;
        fileSystem->CR.available_inode_num = 2;
        fileSystem->CR.current_folder_offset = 1;
        // building imap
        fileSystem->block_region[0].Imap_block.offset = 1;
        fileSystem->block_region[0].Imap_block.rows[0].Inode_number = 1;
        fileSystem->block_region[0].Imap_block.rows[0].Inode_offset = 1;
        fileSystem->CR.imap_pointer[0] = 0;
        
        //root folder initialize
        
        fileSystem->block_region[1].folder_block.folder_offset = 1;
        strcpy(fileSystem->block_region[1].folder_block.foldername,"/");
        strcpy(fileSystem->block_region[1].folder_block.files[0].filename , ".");
        fileSystem->block_region[1].folder_block.files[0].Inode_number = 1;
        fileSystem->block_region[1].isfolder = 1;
        //fileSystem->CR.imap_pointer[0].offset= 0;
        fwrite(fileSystem, sizeof(struct fileSystem), 1, file);
        fclose(file);
        printf("file does not exist, and data has been written to the file.\n");
    }
}
void write (struct fileSystem*fileSystem , char*name ,char*content){
    int folder_offset_buffer = fileSystem->CR.current_folder_offset;  
    for (int i = 0 ; i < fileSystem->block_region[folder_offset_buffer].folder_block.folder_offset; i++){
        if (strcmp(fileSystem->block_region[folder_offset_buffer].folder_block.files[i].filename, name)==0){
            printf("file already exists");
            exit(1);
        }  
    }
    //creating a file
    int writing_offset = fileSystem->CR.disk_offset;

    size_t length = strlen(content);
    size_t blockSize = 4096;
    size_t writtenBytes = 0;
    //writing the data blocks
    while (writtenBytes < length) {
        size_t remainingBytes = length - writtenBytes;
        size_t bytesToWrite = (remainingBytes < blockSize) ? remainingBytes : blockSize;
        struct block writing_block;
        strncpy(fileSystem->block_region[writing_offset].data_block.data, content + writtenBytes, bytesToWrite);
        fileSystem->block_region[writing_offset].isfolder = 0;
        writtenBytes += bytesToWrite;
        writing_offset ++;
    }

    //writing inode block
    fileSystem->block_region[writing_offset].isfolder = 0;
    strcpy(fileSystem->block_region[writing_offset].Inode_block.filename, name);
    fileSystem->block_region[writing_offset].Inode_block.Inode_number = fileSystem -> CR.available_inode_num;
    fileSystem->block_region[writing_offset].Inode_block.data_block_number = writing_offset - fileSystem->CR.disk_offset ;
    for (int i = 0; i < fileSystem->block_region[writing_offset].Inode_block.data_block_number; i ++){
        fileSystem->block_region[writing_offset].Inode_block.data_block_pointer[i] = fileSystem -> CR.disk_offset + i;
    }
    
    
    //write to folder

    strcpy(fileSystem->block_region[folder_offset_buffer].folder_block.files[fileSystem->block_region[folder_offset_buffer].folder_block.folder_offset].filename,name);
    fileSystem->block_region[folder_offset_buffer].folder_block.files[fileSystem->block_region[folder_offset_buffer].folder_block.folder_offset].Inode_number =fileSystem-> CR.available_inode_num;

    fileSystem->block_region[folder_offset_buffer].folder_block.folder_offset ++;
    //writing to the imap
    // for now we assume we have one block
            //for (imap_counter = 0 ; imap_counter < sizeof(fileSystem->CR.imap_pointer) ; imap_counter ++){

            //}
    
    int imap_table_offset = fileSystem->block_region[fileSystem->CR.imap_pointer[0]].Imap_block.offset;
    fileSystem->block_region[fileSystem->CR.imap_pointer[0]].Imap_block.rows[imap_table_offset].Inode_number = fileSystem ->CR.available_inode_num ;
    fileSystem->block_region[fileSystem->CR.imap_pointer[0]].Imap_block.rows[imap_table_offset].Inode_offset = writing_offset;
    //increment all counters
    fileSystem->block_region[fileSystem->CR.imap_pointer[0]].Imap_block.offset ++;
    fileSystem ->CR.available_inode_num ++;
    fileSystem->CR.disk_offset = writing_offset +1;

}
void fsync(struct fileSystem * fileSystem){
   
    FILE *file = fopen(fileSystem->filename, "wb");
    if (file == NULL) {
        perror("Error opening file\n");    
    }
    fwrite(fileSystem, sizeof(struct fileSystem), 1, file);
    fclose(file);
}
void __mkdir (char *foldername, struct fileSystem *fileSystem){
    int current_folder_offset = fileSystem->CR.current_folder_offset;
    for (int i = 0 ; i < fileSystem->block_region[current_folder_offset].folder_block.folder_offset; i++){
        if (strcmp(fileSystem->block_region[current_folder_offset].folder_block.files[i].filename, foldername)==0 ){
            printf("folder can not be created");
            exit(1);
        }  
    }
    int DO = fileSystem->CR.disk_offset; // disk_offset
    //writing folder block
    strcpy(fileSystem->block_region[DO].folder_block.foldername , foldername);
    fileSystem->block_region[DO].isfolder = 1;
    fileSystem->block_region[DO].folder_block.folder_offset = 2;
    //creating . and ..
    strcpy(fileSystem->block_region[DO].folder_block.files[0].filename , ".");
    strcpy(fileSystem->block_region[DO].folder_block.files[1].filename , "..");
    fileSystem->block_region[DO].folder_block.files[0].Inode_number = fileSystem-> CR.available_inode_num;
    fileSystem->block_region[DO].folder_block.files[1].Inode_number = fileSystem->block_region[current_folder_offset].folder_block.files[0].Inode_number;
    //updating the the father folder
    strcpy(fileSystem->block_region[current_folder_offset].folder_block.files[fileSystem->block_region[current_folder_offset].folder_block.folder_offset].filename,foldername);
    fileSystem->block_region[current_folder_offset].folder_block.files[fileSystem->block_region[current_folder_offset].folder_block.folder_offset].Inode_number =fileSystem-> CR.available_inode_num;
    fileSystem->block_region[current_folder_offset].folder_block.folder_offset ++;
    //writing to the imap 
    int imap_table_offset = fileSystem->block_region[fileSystem->CR.imap_pointer[0]].Imap_block.offset;
    fileSystem->block_region[fileSystem->CR.imap_pointer[0]].Imap_block.rows[imap_table_offset].Inode_number = fileSystem ->CR.available_inode_num ;
    fileSystem->block_region[fileSystem->CR.imap_pointer[0]].Imap_block.rows[imap_table_offset].Inode_offset = DO;
    //increment all counters
    fileSystem->block_region[fileSystem->CR.imap_pointer[0]].Imap_block.offset ++;
    fileSystem ->CR.available_inode_num ++;
    fileSystem->CR.disk_offset ++;
}
void __cd (char *foldername, struct fileSystem *fileSystem){
    int folder_offset_buffer = fileSystem->CR.current_folder_offset;
    int current_inode_offset ;
    for (int i = 0 ; i < fileSystem->block_region[folder_offset_buffer].folder_block.folder_offset; i++){
        int current_inode_number = fileSystem->block_region[folder_offset_buffer].folder_block.files[i].Inode_number;
        for (int j = 0 ; j <fileSystem -> block_region[0].Imap_block.offset ; j++){
            if (fileSystem -> block_region[0].Imap_block.rows[j].Inode_number== current_inode_number){   
                current_inode_offset = fileSystem -> block_region[0].Imap_block.rows[j].Inode_offset;   
            }
        }
        if (strcmp(fileSystem->block_region[folder_offset_buffer].folder_block.files[i].filename, foldername)==0 && fileSystem->block_region[current_inode_offset].isfolder == 1){
            fileSystem->CR.current_folder_offset =current_inode_offset;

            return;
        }
    }
    printf("folder couldn't be found");
    exit (1);
}
void __ls (struct fileSystem *fileSystem){
    int folder_offset_buffer = fileSystem->CR.current_folder_offset;
    
    for (int i = 0 ; i < fileSystem->block_region[folder_offset_buffer].folder_block.folder_offset; i++){
        printf("%s\n",fileSystem->block_region[folder_offset_buffer].folder_block.files[i].filename);
    }
}
char * read_file (struct fileSystem * fileSystem , char *filename){
    int folder_offset_buffer = fileSystem->CR.current_folder_offset;
    int file_inode ;
    int file_offset ;
    for (int i = 0 ; i < fileSystem->block_region[folder_offset_buffer].folder_block.folder_offset; i++){
        if (strcmp(fileSystem->block_region[folder_offset_buffer].folder_block.files[i].filename, filename)==0){
            file_inode = fileSystem->block_region[folder_offset_buffer].folder_block.files[i].Inode_number;
            break;
        }
    }
    // always assuming we have one imap block
        //fetch inode offset
        for (int i =0 ; i <fileSystem->block_region[0].Imap_block.offset; i++ ) {
            if (fileSystem->block_region[0].Imap_block.rows[i].Inode_number == file_inode){
                file_offset = fileSystem->block_region[0].Imap_block.rows[i].Inode_offset;
                printf("%d",file_offset);
                break;
            }
        }
        //fetch Inode content
        char * file_content = (char*)malloc(4096*fileSystem->block_region[file_offset].Inode_block.data_block_number);
        for (int i =0 ; i < fileSystem->block_region[file_offset].Inode_block.data_block_number; i ++ ){
            int data_offset = fileSystem->block_region[file_offset].Inode_block.data_block_pointer[i];
            strcat(file_content, fileSystem->block_region[data_offset].data_block.data);
        }
        printf ("%s",file_content);

    return file_content;
    
}
 void close (struct fileSystem * fileSystem ){

    fsync(fileSystem);
    free(fileSystem->filename);
    free (fileSystem);
 }

int main() {
char *filename = "FileSystem";
struct fileSystem *lfs = (struct fileSystem *)malloc(sizeof(struct fileSystem));
if (lfs == NULL) {
    perror("Error allocating memory for lfs\n");
    return 1;
}
open(filename, lfs);
write (lfs, "newfile","hi there i am karim");
//printf("%d", lfs->CR.disk_offset);
//printf("%d\n", lfs->CR.disk_offset);
//printf("%d\n", lfs->block_region[0].Imap_block.offset);
//char * read_buffer = read_file(lfs,"newfile");
//printf("%s",read_buffer);
__mkdir("folder",lfs);
__cd("folder",lfs);
write (lfs, "newfile2","hi there i am karim again");
printf("///////////////am inside the folder//////////\n");
__ls(lfs);
__cd("..",lfs);
printf("///////////////am now outside the folder back at rooooooot //////////\n");
__ls(lfs);
close(lfs);
return 0;
}



////TO DO LIST
// 1 add robustness ******
// 1.5 CORRECT the data_block_assignement problem ****
// 2 l_seek
// 3 create interactable executable
