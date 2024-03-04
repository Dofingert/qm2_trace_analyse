#include "phmap.h"

struct pred_key_t {
    uint32_t r_reg[2];
    uint32_t w_reg;
    uint32_t pc;
    uint32_t inst;
};

template<>
struct std::hash<pred_key_t> {
    size_t operator()(const pred_key_t &k) const {
        return (k.w_reg ^ k.r_reg[0]) | ((uint64_t)(k.pc ^ k.r_reg[1]) << 32);
    }
};

bool operator==(const pred_key_t &a, const pred_key_t &b) {
    return a.pc == b.pc && a.w_reg == b.w_reg && a.r_reg[0] == b.r_reg[0] && a.r_reg[1] == b.r_reg[1];
}

typedef phmap::node_hash_map<pred_key_t, uint64_t> inst_to_freq_t;

inst_to_freq_t freq;
uint64_t global_cnt;

void parse_one_inst_gathered(pred_key_t* inst_info)
{
    auto& cnt = freq[*inst_info];
    global_cnt += 1;
    cnt += 1;
}

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <iostream>
#include <queue>

uint32_t reverse(uint32_t i) {
    uint32_t n = (i << 24) | ((i & 0xff00) << 8) | ((i & 0xff0000) >> 8) | (i >> 24);
    return n;
}

struct freq_p_type
{
    uint64_t freq;
    uint8_t type;
};
bool operator<(const freq_p_type &a, const freq_p_type &b) {
    return a.freq < b.freq;
}
extern uint8_t categorize_rv32(uint32_t inst);
#include <stdio.h>
void print_analyse(std::ostream &output) {
    // 每 10% 一个目标 10 - 20 - 30 - 40 - 50 - 60 - 70 - 80 - 90 - 100
    uint64_t now_iter = 0;
    uint64_t cnt_d_10 = global_cnt / 10;
    uint64_t tgt = cnt_d_10;
    uint64_t bev_cnt = 0;
    uint64_t exc_cnt = 0;

    // 构建堆
    std::priority_queue<freq_p_type> heap;
    for(auto iter : freq) {
        heap.push(freq_p_type{iter.second,categorize_rv32(iter.first.inst)});
    }

    // 结果
    uint64_t results_for_types[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
    // 打印结果
    output << global_cnt << ",";

    // 遍历堆
    while (now_iter != 9)
    {
        auto v = heap.top();
        exc_cnt += v.freq;
        results_for_types[v.type] += 1;
        heap.pop();
        bev_cnt += 1;
        if(exc_cnt > tgt) {
            tgt += cnt_d_10;
            now_iter ++;
            // 打印结果
            output << bev_cnt << ",";
            for(int i = 0 ; i < 13 ; i ++) {
                output << results_for_types[i] << ",";
            }
        }
    }

    output << freq.size() << ",";
    for(int i = 0 ; i < 12 ; i ++) {
        output << results_for_types[i] << ",";
    }
    output << results_for_types[12] << std::endl;
    // 所有资源均被自动回收
}

#pragma pack(1)
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
#pragma pack ()

void parse_one_input_gather(void* buffer, uint64_t elem_cnt, uint64_t interval)
{
    struct pred_key_t pred;
    for(uint64_t i = 0; i < elem_cnt; i++) {
        if ((global_cnt % interval) == 0 && global_cnt) print_analyse(std::cout);
        struct inst_info_t *raw_info = (struct inst_info_t *) ((char*)buffer + i * 39);
        pred.r_reg[0] = raw_info->r_reg0_value;
        pred.r_reg[1] = raw_info->r_reg1_value;
        pred.w_reg = raw_info->w_reg_value;
        pred.pc = raw_info->inst_num;
        pred.inst = raw_info->inst;
        parse_one_inst_gathered(&pred);
    }
}

void parse_one_file_gather(char* filename, uint64_t interval) {
    // Open file.
    int fd = open(filename, O_RDONLY);
    if(fd < 0) {
        std::cout << "Not a valid filename: " << filename << std::endl;
        return;
    }
    struct stat s;

    // Calculate file size.
    fstat(fd, &s);
    size_t filesize = s.st_size;

    // MMAP to buffer.
    void* buffer = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);

    // Calculate infomation.
    parse_one_input_gather(buffer, filesize / 39, interval);

    // Cleanup
    munmap(buffer, filesize);
    close(fd);
}

#include <fstream>

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cout << "Usage: ./piority_analyse [input_file list]" << std::endl;;
        return 0;
    }
    for(int i = 0 ; i < argc - 1 ; i++) {
        parse_one_file_gather(argv[1 + i], 1024 * 1024 * 2); // 78M per print info.
        std::cerr << "finish file:" << argv[1 + i] << std::endl;
    }
    print_analyse(std::cout);
}
