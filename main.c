#include <stdio>
#include "lfs.c"

int main(int argc, char *argv[]) {
    char * lfsname = argv  [1];
    
    struct * fileSystem lfs;
    open(lfsname,lfs);
    printf ("///////////WELCOME TO THE LOG STRUCTURED FILE SYSTEM///////////\n");
    char *pwd = lfs->block_region[lfs->CR.current_folder_offset].folder_block.foldername;
    while (lfs != NULL){
    printf ("\n %s %c ",pwd,'%');
    

    }   
    return 0;
}