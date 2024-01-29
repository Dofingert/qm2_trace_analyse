#ifndef _HEADER_DECODER
#define _HEADER_DECODER
#include <stdint.h>

enum rawopcode {
  raw_branch   = 0b1100011,
  raw_load     = 0b0000011,
  raw_store    = 0b0100011,
  raw_reg_imm  = 0b0010011,
  raw_reg_reg  = 0b0110011,
  raw_misc_mem = 0b0001111,
  raw_sys      = 0b1110011,
  raw_lui      = 0b0110111,
  raw_auipc    = 0b0010111,
  raw_jal      = 0b1101111,
  raw_jalr     = 0b1100111,
};

enum myopcode {
  branch,
  load,
  store,
  reg_imm,
  reg_reg,
  misc_mem,
  sys,
  lui,
  auipc,
  jal,
  jalr,
  invalid
};

#endif
