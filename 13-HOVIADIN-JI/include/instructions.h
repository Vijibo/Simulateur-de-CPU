#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "cpu.h"
#include "parser.h"

void handle_MOV(CPU *cpu, void *src, void *dest);
int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest);
int execute_instruction(CPU *cpu, Instruction *instr);
Instruction *fetch_next_instruction(CPU *cpu);

#endif
