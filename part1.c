#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>

struct __attribute__((__packed__)) superblock_t {
    uint8_t   fs_id [8];
    uint16_t block_size;
    uint32_t file_system_block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

struct __attribute__((__packed__)) dir_entry_timedate_t{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct __attribute__((__packed__))dir_entry_t {
    uint8_t status;
    uint32_t starting_block;
    uint32_t block_count;
    uint32_t size;
    struct dir_entry_timedate_t create_time;
    struct dir_entry_timedate_t modify_time;
    uint8_t filename[31];
    uint8_t unused[6];
};

void main(int argc, char** argv) {
    int fd = open(argv[1], O_RDWR);
    if(fd==-1){
        printf("File error\n");
        exit(0);
    }
    struct stat buffer;
    int status = fstat(fd, &buffer);

    //tamplate:   pa=mmap(addr, len, prot, flags, file, offset);
    //c will implicitly cast void* to char*, while c++ does NOT
    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    struct superblock_t* sb = (struct superblock_t*)address;
    
    // PART 1
    printf("Super block informaion:\n");
    printf("Block size: %d\n", ntohs(sb->block_size));
    printf("Block count: %d\n", ntohl(sb->file_system_block_count));
    printf("FAT starts: %d\n", ntohl(sb->fat_start_block));
    printf("FAT blocks: %d\n", ntohl(sb->fat_block_count));
    printf("Root directory start: %d\n", ntohl(sb->root_dir_start_block));
    printf("Root directory blocks: %d\n\n", ntohl(sb->root_dir_block_count));
    
    int bsize = ntohs(sb->block_size);
    int fatstart = bsize * ntohl(sb->fat_start_block);
    int fatcount = ntohl(sb->fat_block_count);
    int data, bfree=0, bkeep=0, buse=0;
    
    for(int i=fatstart;i<(fatstart+bsize*fatcount);i+=4){
        memcpy(&data, address+i, 4);
        data = htonl(data);
        if(data==0){
            bfree++;
        }else if(data==1){
            bkeep++;
        }else{
            buse++;
        }
    }
    
    printf("FAT information:\n");
    printf("Free blocks: %d\n", bfree);
    printf("Rserved Blocks: %d\n", bkeep);
    printf("Allocated Blocks: %d\n", buse);

    munmap(address, buffer.st_size);
    close(fd);
}