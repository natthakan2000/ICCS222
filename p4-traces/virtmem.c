#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define FRAME_FREE 0
#define FRAME_NOT_FREE 1
#define NO_FREE_FRAME -1
#define RANDOM 0
#define FIFO 1
#define LRU 2
unsigned long arrival_clock = 0;
volatile unsigned long counter = 0;
typedef struct _Page_Frame{
    int is_free;               
    long page;                 
    unsigned long time_arrive;
    unsigned int used_time;
} pageFrame;
pageFrame *createFrame(int nframes);
void freeFrame(pageFrame *ft);
int getFreeFrame(pageFrame *ft, int nframes, int mode);
int getPageIndex(pageFrame *ft, int nframes, long vpt, int mode);
void update(pageFrame *ft, long page, int ft_idx);
int evict(pageFrame *ft, int nframes, int mode);
void memoryAccess(pageFrame *ft, long vpt, long obt, int opsize, int nframes, int pagesize, int mode);
int readline(char *buff, int size, FILE *fp);
unsigned long bitsExtractor(unsigned long bits, unsigned int from, unsigned int length);
void Div(unsigned n);
pageFrame *createFrame(int nframes){
    pageFrame *frame_table = (pageFrame *)malloc(sizeof(pageFrame) * nframes);
    if (frame_table == NULL){
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < nframes; i++){
        frame_table[i].page = 0;
        frame_table[i].time_arrive = 0;
        frame_table[i].used_time = 0;
    }
    return frame_table;
}
void freeFrame(pageFrame *ft){
    free(ft);
}
int evict(pageFrame *ft, int nframes, int mode){
    int victim = NO_FREE_FRAME;
    unsigned long min = 0;
    if (mode == RANDOM){
        victim = rand() % nframes;
        ft[victim].is_free = FRAME_FREE;
    }
    if (mode == FIFO){
        min = ft[0].time_arrive;
        for (int i = 0; i < nframes; i++){
            if (ft[i].time_arrive <= min){
                min = ft[i].time_arrive;
                victim = i;
            }
        }
        ft[victim].is_free = FRAME_FREE;
    }
    if (mode == LRU){
        min = ft[0].time_arrive;
        for (int i = 0; i < nframes; i++){
            if (ft[i].time_arrive <= min){
                min = ft[i].time_arrive;
                victim = i;
            }
        }
        ft[victim].is_free = FRAME_FREE;
    }
    return victim;
}
int getFreeFrame(pageFrame *ft, int nframes, int mode){
    for (size_t i = 0; i < nframes; i++){
        if (ft[i].is_free == FRAME_FREE){
            return i;
        }
    }

    return evict(ft, nframes, mode);
}
int getPageIndex(pageFrame *ft, int nframes, long vpt, int mode){
    for (size_t i = 0; i < nframes; i++){
        if (ft[i].page == vpt){
            if (mode == LRU)
                ft[i].time_arrive = arrival_clock;
            return i;
        }
    }
    return NO_FREE_FRAME;
}
void update(pageFrame *ft, long page, int ft_idx){
    ft[ft_idx].page = page;
    ft[ft_idx].is_free = FRAME_NOT_FREE;
    ft[ft_idx].used_time = 1;
    ft[ft_idx].time_arrive = arrival_clock;
    arrival_clock += 1;
}
void memoryAccess(pageFrame *ft, long vpt, long obt, int opsize, int nframes, int pagesize, int mode){
    int free_idx = NO_FREE_FRAME;
    int found_idx = NO_FREE_FRAME;
    if (opsize + obt > pagesize){
        counter++;
        found_idx = getPageIndex(ft, nframes, vpt, mode);
        if (found_idx != NO_FREE_FRAME){
            ft[found_idx].used_time++;
        }
        else{
            free_idx = getFreeFrame(ft, nframes, mode);
            if (free_idx != NO_FREE_FRAME)
                update(ft, vpt, free_idx);
        }
        found_idx = getPageIndex(ft, nframes, vpt + 1, mode);
        if (found_idx != NO_FREE_FRAME){
            ft[found_idx].used_time++;
        }
        else{
            free_idx = getFreeFrame(ft, nframes, mode);
            if (free_idx != NO_FREE_FRAME)
                update(ft, vpt + 1, free_idx);
        }
        return;
    }
    found_idx = getPageIndex(ft, nframes, vpt, mode);
    if (found_idx != NO_FREE_FRAME){
        ft[found_idx].used_time++;
        return;
    }
    counter++;
    free_idx = getFreeFrame(ft, nframes, mode);
    if (free_idx != NO_FREE_FRAME){
        update(ft, vpt, free_idx);
        return;
    }
}
int readline(char *buff, int size, FILE *fp){
    buff[0] = '\0';
    buff[size - 1] = '\0';
    char *tmp;
    if (fgets(buff, size, fp) == NULL){
        *buff = '\0';
        return 0;
    }
    else{
        if ((tmp = strrchr(buff, '\n')) != NULL){
            *tmp = '\0';
        }
    }
    return 1;
}
unsigned long bitsExtractor(unsigned long bits, unsigned int from, unsigned int length){
    unsigned long temp = bits;
    char mask_raw[length];
    for (size_t i = 0; i < length; i++){
        mask_raw[i] = '1';
    }
    mask_raw[length] = '\0';
    unsigned long mask = strtol(mask_raw, NULL, 2);
    for (size_t i = 0; i < from; i++){
        mask <<= 1;
    }
    return temp & mask;
}
void Div(unsigned n){
    if (n > 1)
        Div(n / 2);
}
int main(int argc, char **argv){
    if (argc != 5){
        printf("use: virtmem <trace> <page-size> <nframes> <rand|fifo|lru>\n");
        return 1;
    }
    char *trace_name = argv[1];
    int page_size = atoi(argv[2]);
    int nframes = atoi(argv[3]);
    char *algorithm = argv[4];
    FILE *trace_file;
    int mode = RANDOM;
    char str1[] = "lru", str2[] = "fifo", str3[] = "rand";
    if (strcmp(algorithm, str1) == 0){
        mode = LRU;
    }
    else if (strcmp(algorithm, str2) == 0){
        mode = FIFO;
    }
    else if (strcmp(algorithm, str3) == 0){
        mode = RANDOM;
    }
    else{
        printf("use: virtmem <trace> <page-size> <nframes> <rand|fifo|lru>\n");
        return 1;
    }
    char buffer[50];
    int idx = 3;
    char ops[2] = "a\0";
    int ops_size = 0;
    char address[17] = "0000000000000000\0";
    int adr_idx = strlen(address);
    int offset_size = page_size;
    int count = 0;
    while (offset_size > 1){
        offset_size = offset_size / 2;
        count++;
    }
    offset_size = count;
    int virtual_size = 15 - offset_size;
    long vp_bits_translated = 0;
    long off_bits_translated = 0;
    pageFrame *frame_table = createFrame(nframes);
    if ((trace_file = fopen(trace_name, "r")) == NULL){
        exit(1);
    }
    while (readline(buffer, sizeof(buffer), trace_file)){
        if (buffer[0] == 'I'){
            ops[0] = buffer[0];
        }
        else{
            ops[0] = buffer[1];
        }
        idx = 3;
        while (buffer[idx] != ','){
            idx++;
        }
        ops_size = atoi(&buffer[idx + 1]);
        adr_idx = strlen(address);
        while (idx != 3){
            address[adr_idx - 1] = buffer[--idx];
            adr_idx--;
        }
        unsigned long address_binary = strtol(address, NULL, 16);
        off_bits_translated = bitsExtractor(address_binary, 0, offset_size);
        vp_bits_translated = bitsExtractor(address_binary, offset_size, 64 - offset_size);
        Div(address_binary);
        Div(off_bits_translated);
        Div(vp_bits_translated);
        memoryAccess(
            frame_table,
            vp_bits_translated,
            off_bits_translated,
            ops_size,
            nframes,
            page_size,
            mode);
    }
    freeFrame(frame_table);
    fclose(trace_file);
    printf("%ld\n", counter);
}
