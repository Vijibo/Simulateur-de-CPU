#ifndef CPU_H
#define CPU_H

#include "memory_handler.h"
#include "hashmap.h"
#include "parser.h"
#include <stdint.h>

typedef struct {
    MemoryHandler* memory_handler;
    HashMap* context;       // Registres (AX, BX, CX, DX)
    HashMap* constant_pool; // Nouveau : stocke les valeurs immédiates
} CPU;

CPU *cpu_init(int memory_size);
void cpu_destroy(CPU *cpu);
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data);
void* load(MemoryHandler *handler, const char *segment_name, int pos);
void allocate_variables(CPU *cpu, Instruction **data_instructions, int data_count);
void print_data_segment(CPU *cpu);
void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count);
int run_program(CPU *cpu);
int push_value(CPU *cpu, int value);
int pop_value(CPU *cpu, int *dest);
int alloc_es_segment(CPU *cpu);
int free_es_segment(CPU *cpu);

#endif