#include "phmap.h"

struct pred_key_t {
    uint32_t r_reg[2];
    uint32_t w_reg;
};

template<>
struct std::hash<pred_key_t> {
    size_t operator()(const pred_key_t &k) const {
        return (k.r_reg[0] ^ k.r_reg[1]) | ((uint64_t)k.w_reg << 32);
    }
};

bool operator==(const pred_key_t &a, const pred_key_t &b) {
    return a.w_reg == b.w_reg && a.r_reg[0] == b.r_reg[0] && a.r_reg[1] == b.r_reg[1];
}

typedef phmap::node_hash_map<pred_key_t, uint32_t> inst_to_freq_t;
phmap::flat_hash_map<uint32_t, inst_to_freq_t> pc_map;

void parse_one_inst(uint32_t pc, pred_key_t* inst_info)
{
    auto &freq = pc_map[pc];
    uint32_t cnt = freq[*inst_info];
    //printf("%d: %d\n",pc,cnt);
    freq[*inst_info] = cnt + 1;
}

struct inst_info_t {
    uint32_t inst_num;
    uint32_t inst;
    uint8_t w_reg_id;
    uint32_t w_reg_value;
    uint32_t w_mem_addr;
    uint32_t w_mem_value;
    uint8_t r_reg0_id;
    uint32_t r_reg0_value;
    uint8_t r_reg1_id;
    uint32_t r_reg1_value;
    uint32_t r_mem_addr;
    uint32_t r_mem_value;
};
void parse_one_input(void* buffer, uint32_t elem_cnt)
{
    struct pred_key_t pred;
    for(uint32_t i = 0;i < elem_cnt; i++) {
        struct inst_info_t *raw_info = (struct inst_info_t*)buffer + i;
        pred.r_reg[0] = raw_info->r_reg0_value;
        pred.r_reg[1] = raw_info->r_reg1_value;
        pred.w_reg = raw_info->w_reg_value;
        parse_one_inst(raw_info->inst_num, &pred);
    }
}

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
void parse_one_file(char* filename) {
    // Open file.
    int fd = open(filename, O_RDONLY);
    struct stat s;

    // Calculate file size.
    fstat(fd, &s);
    size_t filesize = s.st_size;

    // MMAP to buffer.
    void* buffer = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);

    // Calculate infomation.
    parse_one_input(buffer, filesize / sizeof(inst_info_t));

    // Cleanup
    munmap(buffer, filesize);
    close(fd);
}

#include <iostream>
uint32_t print_information(std::ostream &output) {
    uint32_t pc_cnt;
    for(auto pc_pair : pc_map) {
        pc_cnt++;
        uint32_t pc = pc_pair.first;
        uint64_t count = 0;
        output << "pc: " << std::hex << pc << "\n";
        for(auto key_pair : pc_pair.second) {
            count += key_pair.second;
            output << "r_reg: " << key_pair.first.r_reg[0] << "  " <<key_pair.first.r_reg[1] << ", w_reg: " << key_pair.first.w_reg << std::endl;
        }
        uint32_t key_cnt = pc_pair.second.size(); // 此指标越小越好
        float avg_count = (float)count / key_cnt; // 此指标越大越好
        output << "exe times: " << std::dec << key_cnt << "\t" << avg_count  << "\n" << std::endl;
    }
    return pc_cnt;
}
#include <fstream>

int main() {
    int file_num;
    std::cin >> file_num;
    for(int i = 1; i<= file_num;i++){
        char file[100];
        sprintf(file,"trace_%d",i);
        parse_one_file(file);
    }
    std::ofstream outfile("output_file.txt");
    print_information(outfile);
}
