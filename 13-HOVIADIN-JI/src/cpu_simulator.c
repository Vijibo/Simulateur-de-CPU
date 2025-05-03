#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../include/parser.h"
#include "../include/cpu.h"
#include "../include/instructions.h"

// Programme principal : lit un fichier .asm, parse, initialise CPU, exécute le programme
int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Usage : %s fichier.asm\n", argv[0]);
        return 1;
    }

    printf("Parsing du fichier %s...\n", argv[1]);
    ParserResult *result = parse(argv[1]);
    if (!result){
        printf("Erreur: Impossible de parser le fichier\n");
        return 1;
    }

    printf("Initialisation du CPU...\n");
    CPU *cpu = cpu_init(1024);
    if (!cpu){
        printf("Erreur: Initialisation du CPU echouee\n");
        free_parser_result(result);
        return 1;
    }

    // Suppression de l'affichage des informations de parsing

    printf("\nResolution des constantes...\n");
    resolve_constants(result);

    // Suppression de l'affichage des instructions après résolution

    printf("\nAllocation des segments...\n");
    allocate_variables(cpu, result->data_instructions, result->data_count);
    allocate_code_segment(cpu, result->code_instructions, result->code_count);

    printf("\nExecution du programme...\n");
    run_program(cpu);

    printf("\nNettoyage...\n");
    free_parser_result(result);
    cpu_destroy(cpu);
    printf("Termine.\n");
    return 0;
}