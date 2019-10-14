//
//  selection_tree.c
//  K_way_mergesort
//
//  Created by 林重翰 on 2019/10/9.
//  Copyright © 2019 林重翰. All rights reserved.
//

#include "selection_tree.h"

unsigned long *heap_list;
single_data *single_data_list;
file_offset *list;

void send_mode_to_heap(boolean begin, boolean size , boolean key, boolean time, boolean ignore, boolean reverse, boolean unique){
    BEGIN = begin;
    SIZE = size;
    KEY = key;
    TIME = time;
    REVERSE = reverse;
    UNIQUE = unique;
}

void fill_heap(unsigned long block_num){
    // 記錄 file_index
    heap_list = (unsigned long *)malloc(sizeof(unsigned long) * block_num * 2);
    memset(heap_list, 0, block_num);
    
    // store struct of "single_data"
    single_data_list = (single_data *)malloc(sizeof(single_data) * block_num);
    
    // 記錄讀取位置 -> ftell() 回傳型態為 long
    list = (file_offset *)malloc(sizeof(file_offset) * block_num);
    // 可能沒辦法正常歸零
    // memset(list->offset, 0, block_num);

    char path[64];
    
    // 開每個 [block_1] 的檔案
    unsigned long block_index;
    FILE *fp;
    for(block_index = 0; block_index < block_num; block_index++){
        // 預先開啟每個 block 的第一個檔案
        sprintf(path, "tmp/%lu_0", block_index);
        // 理論上到這已經把該存的存好了
        fp = fopen(path, "r");
        read_file_to_block(fp, path, block_index);
        strcpy(list[block_index].filename, path);
        fclose(fp);
    }

    // 實際填寫 heap 值 -> 從 block_num 的 index 開始填
    unsigned long heap_index;
    block_index = 0;
    for(heap_index = block_num; heap_index < 2 * block_num; heap_index++){
        heap_list[heap_index] = block_index;
        block_index++;
    }
    
    // 開始進行 winner tree
    select_winner_tree(block_num);
    
    free(heap_list);
    free(single_data_list);
    free(list);
}

void read_file_to_block(FILE *fp, char *path, unsigned long block_index){
    char buffer[MAX_BUFFER_SIZE];
    char next_file_path[64];
    char rm_path[64];
    boolean is_not_file_end = false;
    boolean is_content = false;
    boolean is_pattern = false;
    boolean open_next_file = false;
    
    unsigned long write_buffer_size = 8192, write_buffer_cnt = 0;
    char *write_buffer = (char *)malloc(write_buffer_size);
    if(!write_buffer){
        fprintf(stderr, "ERROR: Malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    
    unsigned long pattern_buffer_size = 8192, pattern_buffer_cnt = 0;
    char *pattern_buffer = (char *)malloc(pattern_buffer_size);
    if(!pattern_buffer){
        fprintf(stderr, "ERROR: Malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    
    // 直到讀取到正確檔案時才結束
    while(1){
        is_not_file_end = false;
        is_content = false;
        is_pattern = false;
        write_buffer[0] = '\0';
        pattern_buffer[0] = '\0';
        
        while(fgets(buffer, MAX_BUFFER_SIZE, fp)){
            rm_newline(buffer);
            if(BEGIN == false){
                if(buffer[0] != '\0'){
                    single_data_list[block_index].size = strlen(buffer);
                    single_data_list[block_index].content = (char *)malloc(strlen(buffer) + 1);
                    if(!single_data_list[block_index].content){
                        fprintf(stderr, "ERROR: Malloc failed.\n");
                        exit(EXIT_FAILURE);
                    }
                    strcpy(single_data_list[block_index].content, buffer);
                    is_not_file_end = true;
                }
                else{
                    break;
                }
            }
            else{
                if(strstr(buffer, ">Size:")){
                    single_data_list[block_index].size = toNumber(get_pattern(buffer, ">Size:"));
                    is_not_file_end = true;
                }
                else if(strstr(buffer, ">Content:")){
                    is_content = true;
                    is_not_file_end = true;
                    continue;
                }
                else if(strstr(buffer, ">Pattern:")){
                    is_content = false;
                    is_pattern = true;
                    is_not_file_end = true;
                    continue;
                }
                else if(buffer[0] == '\0'){
                    if(KEY == true || TIME == true){
                        single_data_list[block_index].pattern = (char *)malloc(strlen(pattern_buffer) + 1);
                        if(!single_data_list[block_index].pattern){
                            fprintf(stderr, "ERROR: Malloc failed.\n");
                            exit(EXIT_FAILURE);
                        }
                        strcpy(single_data_list[block_index].pattern, pattern_buffer);
                    }
                    //printf("%s\n", write_buffer);
                    single_data_list[block_index].content = (char *)malloc(strlen(write_buffer) + 1);
                    if(!single_data_list[block_index].content){
                        fprintf(stderr, "ERROR: Malloc failed.\n");
                        exit(EXIT_FAILURE);
                    }
                    strcpy(single_data_list[block_index].content, write_buffer);
                    // 表示該 content 讀取完畢
                    is_not_file_end = true;
                    break;
                }
                
                if(is_content == true){
                    write_buffer_cnt += (strlen(buffer) + 1);
                    while(write_buffer_size * 0.8 < write_buffer_cnt){
                        write_buffer_size *= 2;
                        write_buffer = (char *)realloc(write_buffer, write_buffer_size);
                        if(!write_buffer){
                            fprintf(stderr, "ERROR: Realloc failed.\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if(write_buffer[0] == '\0'){
                        sprintf(write_buffer, "%s", buffer);
                    }
                    else{
                        sprintf(write_buffer, "%s\n%s", write_buffer, buffer);
                    }
                }
                else if(is_pattern == true){
                    pattern_buffer_cnt += (strlen(buffer) + 1);
                    while(pattern_buffer_size * 0.8 < pattern_buffer_cnt){
                        pattern_buffer_size *= 2;
                        pattern_buffer = (char *)realloc(pattern_buffer, pattern_buffer_size);
                        if(!pattern_buffer){
                            fprintf(stderr, "ERROR: Realloc failed.\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if(pattern_buffer[0] == '\0'){
                        sprintf(pattern_buffer, "%s\n", buffer);
                    }
                    else{
                        sprintf(pattern_buffer, "%s\n%s", pattern_buffer, buffer);
                    }
                }
            }
        }
        // 尚未至該檔案結尾
        if(is_not_file_end == true){
            break;
        }
        // 開啟下個檔案
        else{
            // 該 block 資料到底
            if(!get_next_file(path, block_index)){
                single_data_list[block_index].size = 0;
                sprintf(rm_path, "rm %s", path);
                system(rm_path);
                break;
            }
            // 該 block 開啟其他檔案
            else{
                sprintf(rm_path, "rm %s", path);
                system(rm_path);
                strcpy(next_file_path, get_next_file(path, block_index));
                fclose(fp);
                fp = fopen(next_file_path, "r");
                open_next_file = true;
            }
        }
    }
    // 運用 ftell 回傳 offset 讀取位置，並記錄以便接續讀寫
    list[block_index].offset = ftell(fp);
    
    if(open_next_file == true){
        strcpy(list[block_index].filename, next_file_path);
    }
    fclose(fp);
    free(write_buffer);
    free(pattern_buffer);
}

unsigned long toNumber(char *pattern){
    unsigned long num = 0, index;
    unsigned long pattern_len = strlen(pattern);
    
    for(index = 0; index < pattern_len; index++){
        if(pattern[index] >= '0' && pattern[index] <= '9'){
            num = (num * 10) + (pattern[index] - '0');
        }
    }
    
    return num;
}

void select_winner_tree(unsigned long block_num){
    unsigned long start_index = block_num;
    unsigned long end_index = (block_num * 2) - 1;
    unsigned long index;
    
    // 寫入新檔案名稱
    char path[64] = "sort_ettoday.txt";
    // a+ 可以接續寫入，並不會覆蓋原文檔
    if(access(path, 0) >= 0){
        system("rm sort_ettoday.txt");
    }
    FILE *fw = fopen(path, "a+");
    
    // 預設 由小到大 排序
    while(1){
        start_index = block_num;
        end_index = (block_num * 2) - 1;
        while(start_index != end_index){
            for(index = start_index; index < end_index; index += 2){
                if(!single_data_list[heap_list[index]].content){
                    heap_list[index / 2] = heap_list[index + 1];
                }
                else if(!single_data_list[heap_list[index + 1]].content){
                    heap_list[index / 2] = heap_list[index];
                }
                else{
                    if(SIZE == true){
                        if(REVERSE == false){
                            if(single_data_list[heap_list[index]].size > single_data_list[heap_list[index + 1]].size){
                                heap_list[index / 2] = heap_list[index + 1];
                            }
                            else{
                                heap_list[index / 2] = heap_list[index];
                            }
                        }
                        else{
                            if(single_data_list[heap_list[index]].size > single_data_list[heap_list[index + 1]].size){
                                heap_list[index / 2] = heap_list[index];
                            }
                            else{
                                heap_list[index / 2] = heap_list[index + 1];
                            }
                        }
                    }
                    else if(TIME == true || KEY == true){
                        if(REVERSE == false){
                            if(strcmp(single_data_list[heap_list[index]].pattern, single_data_list[heap_list[index + 1]].pattern) > 0){
                                heap_list[index / 2] = heap_list[index + 1];
                            }
                            else{
                                heap_list[index / 2] = heap_list[index];
                            }
                        }
                        else{
                            if(strcmp(single_data_list[heap_list[index]].pattern, single_data_list[heap_list[index + 1]].pattern) > 0){
                                heap_list[index / 2] = heap_list[index];
                            }
                            else{
                                heap_list[index / 2] = heap_list[index + 1];
                            }
                        }
                    }
                    else{
                        if(REVERSE == false){
                            if(strcmp(single_data_list[heap_list[index]].content, single_data_list[heap_list[index + 1]].content) > 0){
                                heap_list[index / 2] = heap_list[index + 1];
                            }
                            else{
                                heap_list[index / 2] = heap_list[index];
                            }
                        }
                        else{
                            if(strcmp(single_data_list[heap_list[index]].content, single_data_list[heap_list[index + 1]].content) > 0){
                                heap_list[index / 2] = heap_list[index];
                            }
                            else{
                                heap_list[index / 2] = heap_list[index + 1];
                            }
                        }
                    }
                }
            }
            start_index = start_index / 2;
            end_index = end_index / 2;
        }
        
        // move offset
        if(single_data_list[heap_list[1]].content != NULL){
            // 確認 heap_list[1] 中的 index 為 winner，將資料寫入->新檔案中
            fprintf(fw, "%s\n", single_data_list[heap_list[1]].content);
            free(single_data_list[heap_list[1]].content);
            single_data_list[heap_list[1]].content = NULL;
            if(TIME_MODE == true || KEY_MODE == true){
                free(single_data_list[heap_list[1]].pattern);
                single_data_list[heap_list[1]].pattern = NULL;
            }
            
            push_new_block();
        }
        else{
            break;
        }
     }
}

void push_new_block(){
    if(access(list[heap_list[1]].filename, 0) >= 0){
        //fprintf(stderr, "\t>%s\n", list[heap_list[1]].filename);
        // 根據上次讀取到 offset 進行讀取
        FILE *fp = fopen(list[heap_list[1]].filename, "r");
        //printf("%s\n", list[heap_list[1]].filename);
        fseek(fp, list[heap_list[1]].offset, SEEK_SET);
        
        read_file_to_block(fp, list[heap_list[1]].filename, heap_list[1]);
        
        fclose(fp);
    }
}

char *get_next_file(char *path, unsigned long block_index){
    unsigned long file_num;
    char current_file[64], next_file[64];
    
    sprintf(current_file, "tmp/%lu_", block_index);
    file_num = toNumber(get_pattern(path, current_file)) + 1;
    sprintf(next_file, "tmp/%lu_%lu", block_index, file_num);
    
    if(access(next_file, 0) < 0){
        // 該檔案不存在，表示該 block 已無資料
        return NULL;
    }
    else{
        return next_file;
    }
}
