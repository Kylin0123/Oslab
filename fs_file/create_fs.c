/*************************************************************************
	> File Name: create_fs.c
	> Author: 
	> Mail: 
	> Created Time: Sat 10 Jun 2017 10:56:04 PM CST
 ************************************************************************/

#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>

#define POINTER_NUM     10
#define NAME_LENGTH     10
#define SUPER_BLOCK_SIZE    1024
#define GROUP_DESC_SIZE     1024
#define INODE_SIZE      1024 
#define DIRENTRY_SIZE   (4+NAME_LENGTH)
union SuperBlock {
      uint8_t byte[SUPER_BLOCK_SIZE];
      struct {
             int32_t sectorNum;                   // 文件系统中扇区总数
             int32_t inodeNum;                    // 文件系统中inode总数
             int32_t blockNum;                    // 文件系统中data block总数
             int32_t availInodeNum;               // 文件系统中可用inode总数
             int32_t availBlockNum;               // 文件系统中可用data block总数
             int32_t blockSize;                   // 每个block所含字节数
             int32_t inodesPerGroup;              // 每个group所含inode数
             int32_t blocksPerGroup;              // 每个group所含data block数
      };
};

union GroupDesc {                                 // Group Descriptor Table的表项
      uint8_t byte[GROUP_DESC_SIZE];
      struct {
             int32_t inodeBitmap;                 // 该group中inodeBitmap的偏移量
             int32_t blockBitmap;                 // 该group中blockBitmap的偏移量
             int32_t inodeTable;                  // 该group中inodeTable的偏移量
             int32_t availInodeNum;               // 该group中可用inode总数
             int32_t availBlockNum;               // 该group中可用data block总数
      };
};

union Inode {                                     // Inode Table的表项
      uint8_t byte[INODE_SIZE];
      struct {
             int16_t type;                        // 该文件的类型、访存控制等
             int16_t linkCount;                   // 该文件的链接数
             int32_t blockCount;                  // 该文件的data block总数
             int32_t size;                        // 该文件所含字节数
             int32_t pointer[POINTER_NUM];        // data block偏移量
             int32_t singlyPointer;               // 一级data block偏移量索引
             int32_t doublyPointer;               // 二级data block偏移量索引
             int32_t triplyPointer;               // 三级data block偏移量索引
      };
};

union DirEntry {                                  // 目录文件的表项
      uint8_t byte[DIRENTRY_SIZE];
      struct {
             int32_t inode;                       // 该目录项对应的inode的偏移量
             char name[NAME_LENGTH];              // 该目录项对应的文件名
      };
};

union DirEntryTable {
    uint8_t byte[1024];
    struct{
        uint32_t size;
        union DirEntry dirs[3];
    };
};

int main(){
    union SuperBlock sb;
    sb.sectorNum = 2000;
    sb.inodeNum = 10;
    sb.blockNum = 250;
    sb.availInodeNum = 7;
    sb.availBlockNum = 12;
    sb.blockSize = 1024;
    sb.inodesPerGroup = 10;
    sb.blocksPerGroup = 250;

    union GroupDesc gd;
    gd.inodeBitmap = 2;
    gd.blockBitmap = 3;
    gd.inodeTable = 4;
    gd.availInodeNum = sb.availInodeNum;
    gd.availBlockNum = sb.availBlockNum;
    
    uint8_t InodeBitmap[sb.blockSize]; 
    
    uint8_t BlockBitmap[sb.blockSize];
    
    union Inode ids[sb.inodeNum];

    union DirEntryTable det;
    det.size = 3;
    det.dirs[0].inode = 1;
    strcpy(det.dirs[0].name, "boot");
    det.dirs[1].inode = 2;
    strcpy(det.dirs[1].name, "dev");
    det.dirs[2].inode = 3;
    strcpy(det.dirs[2].name, "usr");

    ids[0].type = 2; //file type 2
    ids[0].linkCount = 0;
    ids[0].blockCount = sizeof(det)/sb.blockSize + 1;
    ids[0].size = sizeof(det);
    ids[0].pointer[0] = 0;

    union DirEntryTable det_boot;
    det_boot.size = 1;
    det_boot.dirs[0].inode = 4;
    strcpy(det_boot.dirs[0].name, "initrd");

    ids[1].type = 2;
    ids[1].linkCount = 0;
    ids[1].blockCount = sizeof(det_boot)/sb.blockSize + 1;
    ids[1].size = sizeof(det_boot);
    ids[1].pointer[0] = 1;

    union DirEntryTable det_dev;
    det_dev.size = 2;
    det_dev.dirs[0].inode = 5;
    strcpy(det_dev.dirs[0].name, "stdin");
    det_dev.dirs[1].inode = 6;
    strcpy(det_dev.dirs[1].name, "stdout");

    ids[2].type = 2;
    ids[2].linkCount = 0;
    ids[2].blockCount = sizeof(det_dev)/sb.blockSize + 1;
    ids[2].size = sizeof(det_dev);
    ids[2].pointer[0] = 2;

    union DirEntryTable det_usr;
    det_usr.size = 0;
    
    ids[3].type = 2;
    ids[3].linkCount = 0;
    ids[3].blockCount = sizeof(det_usr)/sb.blockSize + 1;
    ids[3].size = sizeof(det_usr);
    ids[3].pointer[0] = 3;

    FILE* f = fopen("../app/uMain.elf", "rb");
    char app_buf[10000];
    int app_size = fread(app_buf,1,10000,f);
    
    ids[4].type = 1;  //common file
    ids[4].linkCount = 0;
    ids[4].blockCount = app_size / sb.blockSize + 1;
    printf("%d\n",ids[4].blockCount);
    ids[4].size = app_size;
    ids[4].pointer[0] = 4;

    ids[5].type = 3;
    ids[5].linkCount = 0;
    ids[5].blockCount = 0;
    ids[5].size = 0;
    ids[5].pointer[0] = ids[4].pointer[0]+ids[4].blockCount;

    ids[6].type = 3;
    ids[6].linkCount = 0;
    ids[6].blockCount = 0;
    ids[6].size = 0;
    ids[6].pointer[0] = ids[5].pointer[0]+ids[5].blockCount;
    printf("wtf:%d\n",ids[6].pointer[0]);

    remove("out.txt");
    FILE * out = fopen("out.txt", "wb+");
    #define PRINT printf("%d\n",ret);
    int ret = 0;
    ret += sizeof(sb) * fwrite(&sb, sizeof(sb), 1, out);PRINT
    ret += sizeof(gd) * fwrite(&gd, sizeof(gd), 1, out);PRINT
    ret += sb.blockSize * fwrite(&InodeBitmap, sb.blockSize, 1, out);PRINT
    ret += sb.blockSize * fwrite(&BlockBitmap, sb.blockSize, 1, out);PRINT
    ret += sizeof(union Inode) * fwrite(&ids, sizeof(union Inode), sb.inodeNum, out);PRINT
    //assert(sb.inodeNum * sizeof(union Inode) == fwrite(&ids, sizeof(union Inode), sb.inodeNum, out));
    ret += sizeof(det) * fwrite(&det, sizeof(det), 1, out);PRINT
    ret += sizeof(det_boot) * fwrite(&det_boot, sizeof(det_boot), 1, out);PRINT
    ret += sizeof(det_dev) * fwrite(&det_dev, sizeof(det_dev), 1, out);PRINT
    ret += sizeof(det_usr) * fwrite(&det_usr, sizeof(det_usr), 1, out);PRINT
    ret += fwrite(app_buf, 1, app_size, out);PRINT
    char zero = 0;
    ret += fwrite(&zero, 1, 1024, out);PRINT
    return 0;
}
