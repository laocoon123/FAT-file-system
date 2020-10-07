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

char** tokenize_input(char* line){
	char** token_list = malloc(sizeof(char*)*30);
	char* token = strtok(line, "/");
	int i = 0;
	while(token != NULL){
		token_list[i] = token;
		i++;
		token = strtok(NULL, "/");
	}
	token_list[i] = NULL;
	return token_list;
}

void main(int argc, char** argv) {
    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);

    //tamplate:   pa=mmap(addr, len, prot, flags, file, offset);
    //c will implicitly cast void* to char*, while c++ does NOT
    void* address = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    struct superblock_t* sb = (struct superblock_t*)address;
    
    // PART 1
    int bsize = ntohs(sb->block_size);
    int fatstart = bsize * ntohl(sb->fat_start_block);
    int fatcount = ntohl(sb->fat_block_count);
    
    // PART 2
    int dirstart = bsize*ntohl(sb->root_dir_start_block);
    int dircount = ntohl(sb->root_dir_block_count);
    struct dir_entry_t* de;
    
    // PART 3
    if(argc >=3){
        char** list = tokenize_input(argv[2]);
        char* fname = list[0];
        int is_found = 0;

        for(int i=dirstart;i<(dirstart+dircount*bsize);i+=64){
            de = (struct dir_entry_t*) (address+i);
            if(strcmp(de->filename, fname)==0){
                is_found = 1;
                break;
            }
        }
        if(!is_found){
            printf("File not found\n");
            exit(0);
        }

        FILE* fp = fopen(fname, "w");
        int file_block_index = ntohl(de->starting_block);
        void* file_block_addr = address+bsize*file_block_index;
        
        for(int i=0;;i++){
            if(i==ntohl(de->block_count)-1){
                int last_block_size = ntohl(de->size)-i*bsize;
                fwrite(file_block_addr, 1, last_block_size, fp);
                break;
            }
            fwrite(file_block_addr, 1, bsize, fp);
            memcpy(&file_block_index, address+fatstart+(4*file_block_index), 4);
            file_block_index = ntohl(file_block_index);
            file_block_addr = address+bsize*file_block_index;
        }

        fclose(fp);
    }
    else{
        printf("Invalid input\n");
        exit(0);
    }

    munmap(address, buffer.st_size);
    close(fd);
}