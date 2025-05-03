#ifndef ADDRESSING_H
#define ADDRESSING_H

#include "cpu.h"
#include <regex.h>

void* immediate_addressing(CPU* cpu, const char *operand);
void* register_addressing(CPU* cpu, const char *operand);
void* memory_direct_addressing(CPU* cpu, const char *operand);
void* register_indirect_addressing(CPU* cpu, const char *operand);
void handle_MOV(CPU* cpu, void* src, void* dest);
void* resolve_addressing(CPU* cpu, const char *operand);
int is_number(const char *s);
int matches(const char *pattern, const char *string);



#endif