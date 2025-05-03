#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/cpu.h"
#include "../include/instructions.h"

// Initialise le CPU, les registres et les structures de gestion mémoire
CPU* cpu_init(int memory_size){
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    if (!cpu){
        printf(" erreur malloc");
        return NULL;
    }

    cpu->memory_handler = memory_init(memory_size);
    if (cpu->memory_handler == NULL){
        printf("erreur memory_init");
        free(cpu);
        return NULL;
    }

    cpu->context = hashmap_create();
    if (cpu->context == NULL){
        printf("erreur hashmap_create");
        memory_destroy(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    cpu->constant_pool = hashmap_create();
    if (cpu->constant_pool == NULL){
        printf("echec hashmap_create pour constant_pool");
        hashmap_destroy(cpu->context);
        memory_destroy(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    // Initialisation des registres
    int* ax = calloc(1, sizeof(int));
    int* bx = calloc(1, sizeof(int));
    int* cx = calloc(1, sizeof(int));
    int* dx = calloc(1, sizeof(int));
    int* ip = calloc(1, sizeof(int));
    int* zf = calloc(1, sizeof(int));
    int* sf = calloc(1, sizeof(int));
    int* sp = calloc(1, sizeof(int));
    int* bp = calloc(1, sizeof(int));
    int* es = calloc(1, sizeof(int)); (*es) = -1;

    if (!ax || !bx || !cx || !dx || !ip || !zf || !sf || !sp || !bp || !es){
        printf("echec calloc");
        hashmap_destroy(cpu->context);
        memory_destroy(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    hashmap_insert(cpu->context, "AX", ax);
    hashmap_insert(cpu->context, "BX", bx);
    hashmap_insert(cpu->context, "CX", cx);
    hashmap_insert(cpu->context, "DX", dx);
    hashmap_insert(cpu->context, "IP", ip);
    hashmap_insert(cpu->context, "ZF", zf);
    hashmap_insert(cpu->context, "SF", sf);
    hashmap_insert(cpu->context, "SP", sp);
    hashmap_insert(cpu->context, "BP", bp);
    hashmap_insert(cpu->context, "ES", es);

    // Création du segment mémoire
    if (create_segment(cpu->memory_handler, "SS", memory_size - TABLE_SIZE, TABLE_SIZE) < 0){
        printf("erreur create_segment");
        hashmap_destroy(cpu->context);
        memory_destroy(cpu->memory_handler);
        free(cpu);
        return NULL;
    }

    *sp = 127;
    *bp = 127;

    return cpu;
}


// Libère toutes les ressources utilisées par le CPU
void cpu_destroy(CPU* cpu){
    if (cpu == NULL){
        return;
    }

    free(hashmap_get(cpu->context, "AX"));
    free(hashmap_get(cpu->context, "BX"));
    free(hashmap_get(cpu->context, "CX"));
    free(hashmap_get(cpu->context, "DX"));
    free(hashmap_get(cpu->context, "IP"));
    free(hashmap_get(cpu->context, "ZF"));
    free(hashmap_get(cpu->context, "SF"));

    // Libération de la pile et des registres SP et BP
    int* bp = hashmap_get(cpu->context, "BP");
    int* sp = hashmap_get(cpu->context, "SP");
    if (sp && bp){
        while (*sp < *bp){
            int* value = load(cpu->memory_handler, "SS", *sp);
            if (value != NULL){
                free(value);
            }
            (*sp)++;
        }
    }

    if (hashmap_get(cpu->memory_handler->allocated, "ES")) {
        free_es_segment(cpu);
    }
    int* es = hashmap_get(cpu->context, "ES");
    if (es) {
        free(es);
    }
    free(hashmap_get(cpu->context, "BP"));
    free(hashmap_get(cpu->context, "SP"));

    // Libération des variables dans le segment "DS"
    Segment* DS = hashmap_get(cpu->memory_handler->allocated, "DS");
    if (DS != NULL){
        for (int i = 0; i < DS->size; i++){
            void* var = load(cpu->memory_handler, "DS", i);
            if (var != NULL){
                free(var);
            }
        }
    }

    remove_segment(cpu->memory_handler, "SS");
    remove_segment(cpu->memory_handler, "CS");
    remove_segment(cpu->memory_handler, "DS");

    for (int i = 0; i < cpu->constant_pool->size; i++){
        if (cpu->constant_pool->table[i].key != NULL && cpu->constant_pool->table[i].key != TOMBSTONE){
            free(cpu->constant_pool->table[i].value);
        }
    }

    hashmap_destroy(cpu->constant_pool);
    hashmap_destroy(cpu->context);

    memory_destroy(cpu->memory_handler);

    free(cpu);
}

// Stocke une valeur dans un segment s'il est alloue et à l'interieur des bornes
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data){
    Segment *seg = hashmap_get(handler->allocated, segment_name);
    if (!seg || pos >= seg->size){
        fprintf(stderr, "Erreur store: segment %s non trouve ou position %d hors limites\n", segment_name, pos);
        return NULL;
    }

    uintptr_t addr = seg->start + pos;
    handler->memory[addr] = data;
    return handler->memory[addr];
}

// Charge une valeur a partir d'un segment s'il est alloue
void* load(MemoryHandler *handler, const char *segment_name, int pos){
    Segment *seg = hashmap_get(handler->allocated, segment_name);
    if (!seg || pos >= seg->size){
        fprintf(stderr, "Erreur load: segment %s non trouve ou position %d hors limites\n", segment_name, pos);
        return NULL;
    }
    uintptr_t addr = seg->start + pos;
    return handler->memory[addr];
}

// Alloue le segment DS et stocke les valeurs des instructions .DATA
void allocate_variables(CPU *cpu, Instruction **data_instructions, int data_count){
    if (!cpu || !data_instructions || data_count <= 0){
        fprintf(stderr, "Erreur allocate_variables: parametres invalides\n");
        return;
    }
    
    int size = 0;
    for (int i = 0; i < data_count; i++){
        int count = 1;
        for (char *p = data_instructions[i]->operand2; p && *p; p++){
            if (*p == ',') count++;
        }
        size += count;
    }

    printf("Allocation segment DS de taille %d\n", size);
    if (create_segment(cpu->memory_handler, "DS", 0, size) != 0){
        fprintf(stderr, "Erreur allocate_variables: impossible de creer le segment DS\n");
        return;
    }

    int addr = 0;
    for (int i = 0; i < data_count; i++){
        // Dupliquer la chaine pour ne pas la modifier
        char *values = strdup(data_instructions[i]->operand2);

        if (!values) continue;
        
        char *token = strtok(values, ",");
        while (token){
            // Trim token
            while (*token && isspace(*token)) token++;

            char *end = token + strlen(token) - 1;

            while (end > token && isspace(*end)){
                *end = '\0';
                end--;
            }
            
            int *val = (int*)malloc(sizeof(int));
            if (!val){
                free(values);
                continue;
            }

            *val = atoi(token);
            printf("Variable a DS[%d] = %d\n", addr, *val);
            store(cpu->memory_handler, "DS", addr++, val);
            token = strtok(NULL, ",");
        }
        free(values);
    }
}

// Affiche les valeurs stockees dans le segment de donnees
void print_data_segment(CPU *cpu){
    Segment *seg = hashmap_get(cpu->memory_handler->allocated, "DS");

    if (!seg){
        printf("Segment DS non alloue\n");
        return;
    }

    printf("Contenu du segment DS: \n");

    for (int i = 0; i < seg->size; i++){
        int *val = load(cpu->memory_handler, "DS", i);
        if (val) printf("[%d] = %d\n", i, *val);
        else printf("[%d] = NULL\n", i);
    }
}

// Alloue et stocke les instructions dans le segment de code CS
void allocate_code_segment(CPU* cpu, Instruction** code_instructions, int code_count){
    if(!cpu || !code_instructions || code_count <= 0){
        fprintf(stderr, "Erreur allocate_code_segment:parametres invalides\n");
        return;
    }
    
    // On place le segment CS apres DS et avant SS
    int position = cpu->memory_handler->total_size / 2;
    printf("Allocation segment CS de taille %d a la position %d\n", code_count, position);
    
    if(create_segment(cpu->memory_handler, "CS", position, code_count) != 0){
        fprintf(stderr, "Erreur allocate_code_segment: impossible de creer le segment CS\n");
        return;
    }
    
    for(int i = 0; i < code_count; i++){
        if(!code_instructions[i]){
            fprintf(stderr, "Instruction %d est NULL\n", i);
            continue;
        }
        
        Instruction* instr_copy = (Instruction*)malloc(sizeof(Instruction));
        if(!instr_copy){
            fprintf(stderr, "Erreur d'allocation pour l'instruction %d\n", i);
            continue;
        }
        
        instr_copy->mnemonic = code_instructions[i]->mnemonic ? strdup(code_instructions[i]->mnemonic) : NULL;
        instr_copy->operand1 = code_instructions[i]->operand1 ? strdup(code_instructions[i]->operand1) : NULL;
        instr_copy->operand2 = code_instructions[i]->operand2 ? strdup(code_instructions[i]->operand2) : NULL;
        
        printf("Instruction %d: %s %s %s\n", i, instr_copy->mnemonic ? instr_copy->mnemonic : "NULL", instr_copy->operand1 ? instr_copy->operand1 : "", instr_copy->operand2 ? instr_copy->operand2 : "");
        
        store(cpu->memory_handler, "CS", i, instr_copy);
    }
    
    int* ip = (int*)hashmap_get(cpu->context, "IP");
    if(ip){
        *ip = 0;
    }
}

// Execute le programme pas a pas
int run_program(CPU *cpu){
    if(!cpu){
        fprintf(stderr, "Erreur run_program: CPU NULL\n");
        return -1;
    }
    
    printf("=== Etat initial du CPU ===\n");
    print_data_segment(cpu);
    printf("\n Registres:");

    for(int i = 0; i < cpu->context->size; i++){
        if(cpu->context->table[i].key && cpu->context->table[i].key != (void*)-1){
            int *val = cpu->context->table[i].value;
            printf("%s=%d ", cpu->context->table[i].key, *val);
        }
    }

    printf("\n\n");
    printf("Appuyez sur [Entree] pour executer l'instruction suivante, ou 'q' pour quitter\n");
    Instruction *instr;

    while((instr = fetch_next_instruction(cpu))){
        char c = getchar();
        if(c == 'q' || c == 'Q'){
            printf("Execution interrompue par l'utilisateur.\n");
            break;
        }

        printf("\nInstruction: %s %s %s\n", instr->mnemonic ? instr->mnemonic : "", instr->operand1 ? instr->operand1 : "", instr->operand2 ? instr->operand2 : "");
        int result = execute_instruction(cpu, instr);

        if(result == 2){
            printf("Programme termine par instruction HALT\n");
            break;
        }
        
        printf("Registres: ");
        for(int i = 0; i < cpu->context->size; i++){
            if(cpu->context->table[i].key && cpu->context->table[i].key != (void*)-1){
                int *val = cpu->context->table[i].value;
                printf("%s=%d ", cpu->context->table[i].key, *val);
            }
        }

        printf("\n");
    }
    
    printf("\n=== Etat final du CPU ===\n");
    print_data_segment(cpu);
    return 0;
}


// Pousse une valeur sur la pile SS
int push_value(CPU *cpu, int value){
    int *sp = hashmap_get(cpu->context, "SP");
    Segment *ss = hashmap_get(cpu->memory_handler->allocated, "SS");

    if (!sp || !ss || *sp < 0){
        fprintf(stderr, "Erreur push_value: pile pleine ou non initialisee\n");
        return -1;
    }

    int *val = (int*)malloc(sizeof(int));
    if (!val) return -1;
    *val = value;
    store(cpu->memory_handler, "SS", *sp, val);
    (*sp)--;
    return 0;
}

// Depile une valeur de SS
int pop_value(CPU *cpu, int *dest){
    int *sp = hashmap_get(cpu->context, "SP");
    Segment *ss = hashmap_get(cpu->memory_handler->allocated, "SS");

    if (!sp || !ss || *sp >= ss->size - 1){
        fprintf(stderr, "Erreur pop_value: pile vide ou non initialisee\n");
        return -1;
    }

    (*sp)++ ;
    int *val = load(cpu->memory_handler, "SS", *sp);

    if (!val){
        fprintf(stderr, "Erreur pop_value: valeur NULL dans la pile\n");
        return -1;
    }

    *dest = *val;
    return 0;
}

// Alloue dynamiquement le segment ES selon la strategie dans BX
int alloc_es_segment(CPU *cpu){
    int *ax = hashmap_get(cpu->context, "AX");
    int *bx = hashmap_get(cpu->context, "BX");
    int *es = hashmap_get(cpu->context, "ES");
    int *zf = hashmap_get(cpu->context, "ZF");

    if (!ax || !bx || !es || !zf){
        fprintf(stderr, "Erreur alloc_es_segment: registres non trouves\n");
        return -1;
    }

    int addr = find_free_address_strategy(cpu->memory_handler, *ax, *bx);
    if (addr == -1){
        *zf = 0; 
        fprintf(stderr, "Erreur alloc_es_segment: aucun espace libre suffisant\n");
        return -1;
    }

    printf("Allocation segment ES de taille %d a l'adresse %d avec strategie %d\n", *ax, addr, *bx);
    
    if (create_segment(cpu->memory_handler, "ES", addr, *ax) != 0){
        *zf = 0; // 
        fprintf(stderr, "Erreur alloc_es_segment: echec de creation du segment\n");
        return -1;
    }
    
    *es = addr;
    *zf = 1; 

    // Initialiser a zero
    for (int i = 0; i < *ax; i++){
        int *zero = (int*)malloc(sizeof(int));
        if (!zero) continue;
        *zero = 0;
        store(cpu->memory_handler, "ES", i, zero);
    }

    return 0;
}

// Libere le segment dynamique ES
int free_es_segment(CPU *cpu){
    Segment *seg = hashmap_get(cpu->memory_handler->allocated, "ES");
    int *es = hashmap_get(cpu->context, "ES");

    if (!seg || !es){
        fprintf(stderr, "Erreur free_es_segment: segment ES non trouve\n");
        return -1;
    }

    printf("Liberation du segment ES (%d bytes)\n", seg->size);
    
    for (int i = 0; i < seg->size; i++){
        void *val = load(cpu->memory_handler, "ES", i);
        if (val) free(val);
    }

    remove_segment(cpu->memory_handler, "ES");
    *es = -1; // Reinitialiser
    return 0;
}