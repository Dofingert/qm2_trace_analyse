#include "decoder.h"
// simple riscv32 decoder and divider
uint8_t categorize_rv32(uint32_t inst) {
    switch (inst & 0x7f)
    {
    case raw_branch:
    return branch;
    case raw_load:
    return load;
    case raw_store:
    return store;
    case raw_reg_imm:
    return reg_imm;
    case raw_reg_reg:
    return reg_reg;
    case raw_misc_mem:
    return misc_mem;
    case raw_sys:
    return sys;
    case raw_lui:
    return lui;
    case raw_auipc:
    return auipc;
    case raw_jal:
    return jal;
    case raw_jalr:
    return jalr;
    case raw_atomic:
    return atomic;
    default:
    return invalid;
    }
}
