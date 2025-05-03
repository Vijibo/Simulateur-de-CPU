#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <regex.h>
#include "../include/addressing.h"

// Fonction pour verifier si une chaine correspond a un motif regex
int matches(const char *pattern, const char *string){
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result){
        fprintf(stderr, "Regex compilation failed for pattern: %s\n", pattern);
        return 0;
    }
    result = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

// Verifie si la chaine est un nombre
int is_number(const char *s){
    if (!s || !*s) return 0;
    return matches("^[0-9]+$", s);
}

// Adressage immediat : l'operande est une valeur numerique directement contenue dans l'instruction
void *immediate_addressing(CPU *cpu, const char *operand){
    if (!cpu || !operand) return NULL;
    
    char buf[64];
    strncpy(buf, operand, 63);
    buf[63] = '\0';
    
    char *p = buf;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
    if (!*p) return NULL;
    
    char *end = p + strlen(p) - 1;
    while (end > p && (*end == ' ' || *end == '\t' || *end == '\n')){
        *end = '\0';
        end--;
    }
    
    if (!is_number(p)) return NULL;
    
    void *val = hashmap_get(cpu->constant_pool, p);
    if (val) return val;
    
    int *newval = (int*)malloc(sizeof(int));
    if (!newval) return NULL;
    *newval = atoi(p);
    hashmap_insert(cpu->constant_pool, p, newval);
    return newval;
}

// Adressage par registre : l'operande est le nom d'un registre du processeur
void *register_addressing(CPU *cpu, const char *operand){
    if (!cpu || !operand) return NULL;
    
    char buf[64];
    strncpy(buf, operand, 63);
    buf[63] = '\0';
    
    char *p = buf;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
    if (!*p) return NULL;
    
    char *end = p + strlen(p) - 1;
    while (end > p && (*end == ' ' || *end == '\t' || *end == '\n')){
        *end = '\0';
        end--;
    }
    
    if (!matches("^[A-Z][A-Z]$", p)) return NULL;
    
    return hashmap_get(cpu->context, p);
}

// Adressage direct : l'operande specifie directement l'adresse en memoire ou se trouve la valeur
void *memory_direct_addressing(CPU *cpu, const char *operand){
    if (!cpu || !operand) return NULL;
    
    char buf[64];
    strncpy(buf, operand, 63);
    buf[63] = '\0';
    
    char *p = buf;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
    if (!*p) return NULL;
    
    char *end = p + strlen(p) - 1;
    while (end > p && (*end == ' ' || *end == '\t' || *end == '\n')){
        *end = '\0';
        end--;
    }
    
    if (!matches("^\\[[0-9]+\\]$", p)) return NULL;
    
    int addr;
    sscanf(p, "[%d]", &addr);
    return load(cpu->memory_handler, "DS", addr);
}

// Adressage indirect par registre : l'operande specifie un registre qui contient l'adresse memoire
void *register_indirect_addressing(CPU *cpu, const char *operand){
    if (!cpu || !operand) return NULL;
    
    char buf[64];
    strncpy(buf, operand, 63);
    buf[63] = '\0';
    
    char *p = buf;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
    if (!*p) return NULL;
    
    char *end = p + strlen(p) - 1;
    while (end > p && (*end == ' ' || *end == '\t' || *end == '\n')){
        *end = '\0';
        end--;
    }
    
    if (!matches("^\\[[A-Z][A-Z]\\]$", p)) return NULL;
    
    char regname[3] ={p[1], p[2], '\0'};
    
    int *reg = hashmap_get(cpu->context, regname);
    if (!reg) return NULL;
    
    return load(cpu->memory_handler, "DS", *reg);
}

// Adressage avec segment : permet d'acceder a des elements situes hors du segment de donnees
void *segment_override_addressing(CPU *cpu, const char *operand){
    if (!cpu || !operand) return NULL;
    
    char buf[64];
    strncpy(buf, operand, 63);
    buf[63] = '\0';
    
    char *p = buf;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n')) p++;
    if (!*p) return NULL;
    
    char *end = p + strlen(p) - 1;
    while (end > p && (*end == ' ' || *end == '\t' || *end == '\n')){
        *end = '\0';
        end--;
    }
    
    if (!matches("^\\[ES:[A-Z][A-Z]\\]$", p)) return NULL;
    
    char regname[3] ={p[4], p[5], '\0'};
    
    int *reg = hashmap_get(cpu->context, regname);
    if (!reg) return NULL;
    
    return load(cpu->memory_handler, "ES", *reg);
}

// Essaie tous les types d'adressage et retourne le premier qui fonctionne
void *resolve_addressing(CPU *cpu, const char *operand){
    if (!cpu || !operand) return NULL;
    printf("Resolution de l'adressage pour: '%s'\n", operand);
    
    void *res = NULL;
    
    if ((res = immediate_addressing(cpu, operand))){
        printf("  -> Adressage immediat: valeur = %d\n", *(int*)res);
        return res;
    }
    if ((res = register_addressing(cpu, operand))){
        printf("  -> Adressage par registre: %s = %d\n", operand, *(int*)res);
        return res;
    }
    if ((res = memory_direct_addressing(cpu, operand))){
        printf("  -> Adressage direct par memoire: %s = %d\n", operand, *(int*)res);
        return res;
    }
    if ((res = register_indirect_addressing(cpu, operand))){
        printf("  -> Adressage indirect par registre: %s = %d\n", operand, *(int*)res);
        return res;
    }
    if ((res = segment_override_addressing(cpu, operand))){
        printf("  -> Adressage avec segment: %s = %d\n", operand, *(int*)res);
        return res;
    }
    
    printf("  -> ERREUR: Aucun adressage trouve pour '%s'\n", operand);
    return NULL;
}