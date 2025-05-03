#include <stdio.h>
#include <string.h>
#include "../include/instructions.h"
#include "../include/addressing.h"

// Simuler le comportement de l’instruction MOV en pseudo-assembleur
void handle_MOV(CPU *cpu, void *src, void *dest){ 
    if(!cpu || !src || !dest){
        fprintf(stderr, "Erreur dans handle_MOV: parametres invalides\n");
        return;
    }
    
    *((int *)dest) = *((int *)src);
    printf("MOV: %d -> [adresse dest]\n", *((int *)src));
}

// Generalise la fonction handle MOV en permettant d’executer une instruction dans le CPU en fonction de son mnemonique
int handle_instruction(CPU* cpu, Instruction* instr, void* src, void* dest){
    if(!cpu || !instr){
        fprintf(stderr, "Erreur:CPU ou instruction NULL\n");
        return -1;
    }
    
    printf("Execution de %s", instr->mnemonic);
    if (src) printf(" src=%d", *(int*)src);
    if (dest) printf(" dest=%d", *(int*)dest);
    printf("\n");
    
    int* ip = (int*)hashmap_get(cpu->context, "IP");
    int* zf = (int*)hashmap_get(cpu->context, "ZF");
    int* sf = (int*)hashmap_get(cpu->context, "SF");
    
    if(!ip || !zf || !sf){
        fprintf(stderr, "Erreur: registres systme non trouves\n");
        return -1;
    }
    
    Segment* cs = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
    if(!cs){
        fprintf(stderr, "Erreur: Segment CS non trouve\n");
        return -1;
    }
    
    if(strcmp(instr->mnemonic, "MOV") == 0){
        if(!src || !dest){
            fprintf(stderr, "Erreur MOV: operandes invalides\n");
            return -1;
        }

        *(int*)dest = *(int*)src;
    }

    else if(strcmp(instr->mnemonic, "ADD") == 0){
        if(!src || !dest){
            fprintf(stderr, "Erreur ADD:operandes invalides\n");
            return -1;
        }
    
        *(int*)dest += *(int*)src;
        int result = *(int*)dest;
        *zf = (result == 0); 
        *sf = (result < 0);
    }

    else if(strcmp(instr->mnemonic, "CMP") == 0){
        if(!src || !dest){
            fprintf(stderr, "Erreur CMP : operandes invalides\n");
            return -1;
        }

        int result = *(int*)dest - *(int*)src;
        *zf = (result == 0);
        *sf = (result < 0);
    }

    else if(strcmp(instr->mnemonic, "JMP") == 0){
        if(!src){
            fprintf(stderr, "Erreur JMP: adresse de saut invalide\n");
            return -1;
        }
        
        *ip = *(int*)src;
        printf("Saut a l'adresse %d\n", *ip);
        return 1;  
    }
    else if(strcmp(instr->mnemonic, "JZ") == 0){

        if(!src){
            fprintf(stderr, "Erreur JZ: adresse de saut invalide\n");
            return -1;
        }

        if(*zf == 1){
            *ip = *(int*)src;
            printf("Saut conditionnel (ZF=1) a l'adresse %d\n", *ip);
            return 1; 
        }
    }
    else if(strcmp(instr->mnemonic, "JNZ") == 0){

        if(!src) {
            fprintf(stderr, "Erreur JNZ :adresse de saut invalide\n");
            return -1;
        }

        if(*zf == 0) {
            *ip = *(int*)src;
            printf("Saut conditionnel (ZF=0) à l'adresse %d\n", *ip);
            return 1; 
        }
    }
    else if(strcmp(instr->mnemonic, "HALT") == 0){
        *ip = cs->size;
        printf("Fin du programme (HALT)\n");
        return 2; 
    }
    else if(strcmp(instr->mnemonic, "PUSH") == 0){
        if(!src){
            fprintf(stderr, "Erreur PUSH: operande invalide\n");
            return -1;
        }

        push_value(cpu, *(int*)src);
    }
    else if(strcmp(instr->mnemonic, "POP") == 0){
        if(!dest){
            fprintf(stderr, "Erreur POP: operande invalide\n");
            return -1;
        }

        pop_value(cpu, (int*)dest);
    }
    else if(strcmp(instr->mnemonic, "ALLOC") == 0){
        alloc_es_segment(cpu);
    }
    else if(strcmp(instr->mnemonic, "FREE") == 0){
        free_es_segment(cpu);
    }
    else {
        fprintf(stderr, "Instruction inconnue: %s\n", instr->mnemonic);
        return -1;
    }
    
    return 0;
}

// Resout les adresses des opérandes et execute l'instruction
int execute_instruction(CPU* cpu, Instruction* instr){
    if(!cpu || !instr){
        fprintf(stderr, "Erreur: CPU ou instruction NULL\n");
        return -1;
    }
    
    printf("Execution: %s %s %s\n", instr->mnemonic ? instr->mnemonic : "NULL", instr->operand1 ? instr->operand1 : "", instr->operand2 ? instr->operand2 : "");
    void* src = NULL;
    void* dest = NULL;

    if(strcmp(instr->mnemonic, "HALT") == 0){
        return handle_instruction(cpu, instr, NULL, NULL);
    }
    else if(strcmp(instr->mnemonic, "ALLOC") == 0){
        return handle_instruction(cpu, instr, NULL, NULL);
    }
    else if(strcmp(instr->mnemonic, "FREE") == 0){
        return handle_instruction(cpu, instr, NULL, NULL);
    }
    
    // Instructions de saut (JMP, JZ, JNZ)
    if(strcmp(instr->mnemonic, "JMP") == 0 || strcmp(instr->mnemonic, "JZ") == 0 || strcmp(instr->mnemonic, "JNZ") == 0){
        
        src = resolve_addressing(cpu, instr->operand1);
        if(!src){
            fprintf(stderr, "Erreur: Impossible de resoudre l'adresse de saut %s\n", instr->operand1 ? instr->operand1 : "NULL");
            return -1;
        }
        
        return handle_instruction(cpu, instr, src, NULL);
    }
    
    // Instructions a une operande (PUSH, POP)
    if(strcmp(instr->mnemonic, "PUSH") == 0){
        src = resolve_addressing(cpu, instr->operand1);

        if(!src){
            fprintf(stderr, "Erreur: Impossible de resoudre l'operande %s pour PUSH\n", instr->operand1 ? instr->operand1 : "NULL");
            return -1;
        }

        return handle_instruction(cpu, instr, src, NULL);
    }
    
    if(strcmp(instr->mnemonic, "POP") == 0){
        dest = resolve_addressing(cpu, instr->operand1);

        if(!dest){
            fprintf(stderr, "Erreur: Impossible de resoudre l'operande %s pour POP\n", instr->operand1 ? instr->operand1 : "NULL");
            return -1;
        }

        return handle_instruction(cpu, instr, NULL, dest);
    }
    
    // Instructions a deux operandes (MOV, ADD, CMP, etc.)
    if(instr->operand1){
        dest = resolve_addressing(cpu, instr->operand1);

        if(!dest){
            fprintf(stderr, "Erreur: Impossible de resoudre l'operande destination %s\n", instr->operand1);
            return -1;
        }

    }
    
    if(instr->operand2){
        src = resolve_addressing(cpu, instr->operand2);

        if(!src){
            fprintf(stderr, "Erreur: Impossible de resoudre l'operande source %s\n", instr->operand2);
            return -1;
        }

    }
    
    return handle_instruction(cpu, instr, src, dest);
}

//  Recupere l’instruction suivante dans le segment de codes et incremente le pointeur d’instruction(IP)
Instruction* fetch_next_instruction(CPU* cpu){
    if(!cpu){
        fprintf(stderr, "Erreur: CPU NULL\n");
        return NULL;
    }
    
    int* ip = (int*)hashmap_get(cpu->context, "IP");
    if(!ip){
        fprintf(stderr, "Erreur: Registre IP non trouve\n");
        return NULL;
    }
    
    Segment* cs = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");

    if(!cs){
        fprintf(stderr, "Erreur: Segment CS non trouve\n");
        return NULL;
    }
    
    if(*ip < 0 || *ip >= cs->size){
        printf("Programme termine: IP=%d n'est pas dans segment CS(taille: %d)\n", *ip, cs->size);
        return NULL;
    }

    Instruction* instr = (Instruction*)load(cpu->memory_handler, "CS", *ip);

    if (!instr){
        fprintf(stderr, "Erreur: Instruction non trouvee a l'adresse %d\n", *ip);
        return NULL;
    }
    
    (*ip)++;
    
    return instr;
}