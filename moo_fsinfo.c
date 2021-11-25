//
// Created by mooki on 10/18/21.
//

#include <stdio.h>

#include "utils/moo_fs.h"

void showSB(const SuperBlk *superBlk){
    printf("********** SuperBlk **********\n");
    printf("Block num: %d\n", superBlk->blkNum);
    printf("Block num free: %d\n", superBlk->blkNumFree);
    printf("Block size: %d\n", superBlk->blkSize);
    printf("Inode num: %d\n", superBlk->indNum);
    printf("Inode num free: %d\n", superBlk->indNumFree);
    printf("Inode size: %d\n", superBlk->indSize);
    printf("********** SuperBlk **********\n\n");
}

void showFSInfo(const FSInfo *fsInfo){
    printf("********** FSInfo **********\n");
    printf("Label: %s\n", fsInfo->label);
    printf("Inode begin: %d\n", fsInfo->indBeg);
    printf("Inode end: %d\n", fsInfo->indEnd);
    printf("Blk begin: %d\n", fsInfo->blkBeg);
    printf("Blk end: %d\n", fsInfo->blkEnd);
    printf("********** FSInfo **********\n\n");
}


int main(int argc, char* argv[]){
    FILE *vdisk;
    if((vdisk = fopen(VDISKFILE, "r")) == NULL){
        printf("Open virtual disk file %s failed. ", VDISKFILE);
        return 0;
    }

    // 输出超级块
    SuperBlk superBlk;
    if(fread(&superBlk, sizeof(SuperBlk), 1, vdisk) != 1) {
        printf("Read SuperBlk failed. ");
        return 0;
    }
    showSB(&superBlk);

    // 输出文件系统描述
    FSInfo fsInfo;
    if(fread(&fsInfo, sizeof(FSInfo), 1, vdisk) != 1){
        printf("Read FSInfo failed. ");
        return 0;
    }
    showFSInfo(&fsInfo);

    if(fclose(vdisk) != 0){
        printf("Close virtual disk file %s failed. ", VDISKFILE);

        return 0;
    }

    return 0;
}