#include <stdio.h>
#include <stdlib.h>
#include "../include/addressing.h"
#include "../include/cpu.h"

CPU* setup_test_environment() {
    // Initialiser le CPU
    CPU *cpu = cpu_init(1024);
    if (!cpu) {
        printf("Error: CPU initialization failed\n");
        return NULL;
    }

    // Initialiser les registres avec des valeurs spécifiques
    int *ax = (int *)hashmap_get(cpu->context, "AX");
    int *bx = (int *)hashmap_get(cpu->context, "BX");
    int *cx = (int *)hashmap_get(cpu->context, "CX");
    int *dx = (int *)hashmap_get(cpu->context, "DX");

    *ax = 3;
    *bx = 6;
    *cx = 100;
    *dx = 0;

    // Créer et initialiser le segment de données
    if (!hashmap_get(cpu->memory_handler->allocated, "DS")) {
        create_segment(cpu->memory_handler, "DS", 0, 20);
    }

    // Initialiser le segment de données avec des valeurs de test
    for (int i = 0; i < 10; i++) {
        int *value = (int *)malloc(sizeof(int));
        *value = i * 10 + 5; // Valeurs 5, 15, 25, 35...
        store(cpu->memory_handler, "DS", i, value);
    }
    printf("Test environment initialized.\n");
    return cpu;
}

int main() {
    CPU *cpu = setup_test_environment();
    if (!cpu) return 1;

    void *res1 = immediate_addressing(cpu, "42");
    void *res2 = register_addressing(cpu, "AX");
    void *res3 = memory_direct_addressing(cpu, "[5]");
    void *res4 = register_indirect_addressing(cpu, "[AX]");

    printf("Résultats des tests d'adressage :\n");
    printf("Immediat (42) => %d\n", *((int *)res1));
    printf("Registre (AX) => %d\n", *((int *)res2));
    printf("Direct ([5]) => %d\n", *((int *)res3));
    printf("Indirect ([AX]) => %d\n", *((int *)res4));

    cpu_destroy(cpu);
    return 0;
}