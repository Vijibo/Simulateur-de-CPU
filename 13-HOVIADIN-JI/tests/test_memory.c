#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/memory_handler.h"

// Affiche les segments alloués connus
void print_known_allocations(MemoryHandler* handler, const char* names[], int count) {
    printf("  Segments alloués : ");
    for (int i = 0; i < count; i++) {
        Segment* seg = hashmap_get(handler->allocated, names[i]);
        if (seg) {
            printf("[%s:%d-%d] ", names[i], seg->start, seg->start + seg->size - 1);
        }
    }
    printf("\n");
}

// Affiche la liste des segments libres
void print_free_segments(MemoryHandler* handler) {
    printf("  Segments libres  : ");
    Segment* curr = handler->free_list;
    while (curr) {
        printf("[%d-%d] ", curr->start, curr->start + curr->size - 1);
        curr = curr->next;
    }
    printf("\n");
}

// Affiche tout l’état de la mémoire
void print_memory_state(MemoryHandler* handler, const char* known_allocations[], int count) {
    printf("\nÉtat de la mémoire :\n");
    print_known_allocations(handler, known_allocations, count);
    print_free_segments(handler);
}

// Test complet
void test_allocation_strategies() {
    const char* known_segments[] = {"A", "B", "C"};

    MemoryHandler* mem = memory_init(100);
    printf("Memoire initialisée (100 octets)\n");
    print_memory_state(mem, known_segments, 3);

    // Fragmentation : création manuelle de segments à des endroits non contigus
    create_segment(mem, "A", 0, 10);   
    create_segment(mem, "B", 20, 15);   
    create_segment(mem, "C", 60, 10);   
    printf("\nAprès allocation des segments A, B, C :");
    print_memory_state(mem, known_segments, 3);

    // Libération partielle
    remove_segment(mem, "B");
    remove_segment(mem, "C");
    printf("\nAprès suppression de B et C :");
    print_memory_state(mem, known_segments, 3);

    // Recherche d'une place pour un segment de taille 10 avec différentes stratégies
    int size = 10;
    printf("\nRecherche d’un emplacement pour un segment de %d unités :\n", size);

    int addr_first = find_free_address_strategy(mem, size, 0);
    int addr_best  = find_free_address_strategy(mem, size, 1);
    int addr_worst = find_free_address_strategy(mem, size, 2);

    printf("- First Fit : %d\n", addr_first);
    printf("- Best Fit  : %d\n", addr_best);
    printf("- Worst Fit : %d\n", addr_worst);

}

int main() {
    test_allocation_strategies();
    return 0;
}
