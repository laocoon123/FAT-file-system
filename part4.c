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
    int data;
    
    // PART 2
    int dirstart = bsize*ntohl(sb->root_dir_start_block);
    int dircount = ntohl(sb->root_dir_block_count);
    struct dir_entry_t* de;
    
    // PART 3
    
    // PART 4
    if(argc >= 4){
        char** list = tokenize_input(argv[3]);
        printf("%s\n", list[0]);

        int fp = open(argv[2], O_RDWR);
        
        if(fp==-1){
            printf("File not found");
            exit(0);
        }

        struct stat buffer2 ;
        fstat(fp, &buffer2) ;
        int input_size = (int)buffer2.st_size;
        int input_block_count;
        if(input_size%bsize == 0){
            input_block_count = input_size/512;
        }else{
            input_block_count = input_size/512+1;
        }
        void* input = mmap(NULL, buffer2.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
        int free_index;
        int is_first = 1;
        int start_block_i;
        int last_block_i;

        for(int j=0;j<input_block_count;j++){
            free_index = -1;
            for(int i=fatstart;i<(fatstart+bsize*fatcount);i+=4){
                free_index++;
                memcpy(&data, address+i, 4);
                data = htonl(data);
                if(data==0){
                    if(is_first){
                        is_first = 0;
                        data = 65535;
                        data = ntohl(data);
                        memcpy(address+i, &data, 4);
                        start_block_i = free_index;
                        last_block_i = free_index;
                    }
                    else{
                        data = 65535;
                        data = ntohl(data);
                        memcpy(address+i, &data, 4);
                        data = free_index;
                        data = ntohl(data);
                        memcpy(address+fatstart+(last_block_i)*4, &data, 4);
                        last_block_i = free_index;
                    }
                    break;
                }
            }
            
            if(j+1<input_block_count){
                memcpy(address+free_index*bsize, input+j*bsize, bsize);
            }else{
                memcpy(address+free_index*bsize, input+j*bsize, bsize-input_block_count*bsize+input_size);
            }
        }
        
        struct dir_entry_t* new_dir;
        struct dir_entry_timedate_t new_time;
        uint8_t unused[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        time_t rawtime;
        struct tm* tminfo;
        time(&rawtime);
        tminfo = localtime(&rawtime);
        
        new_dir->status = 3;
        new_dir->starting_block = start_block_i;
        new_dir->starting_block = ntohl(new_dir->starting_block);
        new_dir->block_count = input_block_count;
        new_dir->block_count = ntohl(new_dir->block_count);
        new_dir->size = (int)buffer2.st_size;
        new_dir->size = ntohl(new_dir->size);
        strcpy(new_dir->filename, argv[2]);
        strcpy(new_dir->unused, unused);
        
        new_time.year = tminfo->tm_year+1900;
        new_time.year = ntohs(new_time.year);
        new_time.month = tminfo->tm_mon+1;
        new_time.day = tminfo->tm_mday;
        new_time.hour = tminfo->tm_hour;
        new_time.minute = tminfo->tm_min;
        new_time.second = tminfo->tm_sec;
        new_dir->create_time = new_time;
        new_dir->modify_time = new_time;
        
        for(int i=dirstart;i<(dirstart+dircount*bsize);i+=64){
            de = (struct dir_entry_t*) (address+i);
            if(de->status == 0){
                memcpy(address+i, new_dir, 64);
                break;
            }
        }
        
        munmap(input, buffer2.st_size);
    }
    else{
        printf("Invalid input\n");
        exit(0);
    }

    munmap(address, buffer.st_size);
    close(fd);
}