#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/memory_handler.h"

// Initialise un gestionnaire de mémoire avec un segment libre total
MemoryHandler *memory_init(int size){
    MemoryHandler *handler = (MemoryHandler*)malloc(sizeof(MemoryHandler));
    if (!handler) return NULL;

    handler->memory = (void**)malloc(sizeof(void*) * size);

    if (!handler->memory){
        free(handler);
        return NULL;
    }

    for (int i = 0; i < size; i++) handler->memory[i] = NULL;
    handler->total_size = size; 
    handler->allocated = hashmap_create();

    if (!handler->allocated){
        free(handler->memory);
        free(handler);
        return NULL;
    }

    handler->free_list = (Segment*)malloc(sizeof(Segment));

    if (!handler->free_list){
        hashmap_destroy(handler->allocated);
        free(handler->memory);
        free(handler);
        return NULL;
    }

    handler->free_list->start = 0;
    handler->free_list->size = size;
    handler->free_list->next = NULL;
    return handler;
}

// Cherche un segment libre qui peut contenir [start, start+size)
Segment *find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev){
    Segment *curr = handler->free_list;
    *prev = NULL;

    while (curr){
        int end = curr->start + curr->size;
        if (curr->start <= start && end >= start + size){
            return curr;
        }
        *prev = curr;
        curr = curr->next;
    }

    return NULL;
}

// Cree un segment nomme a l'adresse 'start' et taille 'size'
int create_segment(MemoryHandler *handler, const char *name, int start, int size){
    Segment *prev = NULL;
    Segment *seg = find_free_segment(handler, start, size, &prev);
    if (!seg) return -1;

    Segment *new_seg = malloc(sizeof(Segment));
    if (!new_seg) return -1;
    new_seg->start = start;
    new_seg->size = size;
    new_seg->next = NULL;

    hashmap_insert(handler->allocated, name, new_seg);

    int end = seg->start + seg->size;
    Segment *before = NULL, *after = NULL;

    if (seg->start < start){
        before = malloc(sizeof(Segment));
        if (!before) return -1;
        before->start = seg->start;
        before->size = start - seg->start;
        before->next = NULL;
    }

    if (end > start + size){
        after = malloc(sizeof(Segment));
        if (!after) return -1;
        after->start = start + size;
        after->size = end - (start + size);
        after->next = NULL;
    }

    // Mise à jour de la free_list
    if (prev){
        if (before){
            prev->next = before;
            before->next = after ? after : seg->next;
        } else{
            prev->next = after ? after : seg->next;
        }
    } else{
        if (before){
            handler->free_list = before;
            before->next = after ? after : seg->next;
        } else{
            handler->free_list = after ? after : seg->next;
        }
    }

    if (after){
        after->next = seg->next;
    }

    free(seg);
    return 0;
}

// Supprime un segment alloue et le reinsere dans la liste libre en fusionnant
int remove_segment(MemoryHandler *handler, const char *name){
    Segment *seg = hashmap_get(handler->allocated, name);
    if (!seg) return -1;

    hashmap_remove(handler->allocated, name);

    Segment *curr = handler->free_list;
    Segment *prev = NULL;

    // Trouver la bonne position dans la free_list pour insérer seg
    while (curr && curr->start < seg->start){
        prev = curr;
        curr = curr->next;
    }

    // Fusion avec le précédent
    if (prev && (prev->start + prev->size == seg->start)){
        prev->size += seg->size;
        free(seg);
        seg = prev;
    } else{
        seg->next = curr;
        if (prev) prev->next = seg;
        else handler->free_list = seg;
    }

    // Fusion avec le suivant
    if (curr && (seg->start + seg->size == curr->start)){
        seg->size += curr->size;
        seg->next = curr->next;
        free(curr);
    }

    return 0;
}

// Choisit une adresse libre selon une stratégie (0: First Fit, 1: Best Fit, 2: Worst Fit)
int find_free_address_strategy(MemoryHandler* handler, int size, int strategy){
    Segment* best = NULL;
    Segment* current = handler->free_list;

    if (current == NULL) return -1;

    while (current){
        if (current->size >= size){
            if (strategy == 0) return current->start; // First Fit

            if (best == NULL){
                best = current;
            } else{
                int diff = current->size - size;
                int best_diff = best->size - size;

                if ((strategy == 1 && diff < best_diff) ||  // Best Fit
                    (strategy == 2 && diff > best_diff))    // Worst Fit
               {
                    best = current;
                }
            }
        }
        current = current->next;
    }

    return (best != NULL) ? best->start : -1;
}

/* Implémentation de départ qu'on a ensuite optimisé et condensé en un seul if car la logique reste la même 
else if (strategy == 1){ // Best Fit
    while (curr){
        if (curr->size >= size && (!best || curr->size < best->size)) best = curr;
        curr = curr->next;
    }
}
else if (strategy == 2){ // Worst Fit
    while (curr){
        if (curr->size >= size && (!best || curr->size > best->size)) best = curr;
        curr = curr->next;
    }
} */



void memory_destroy(MemoryHandler *handler){
    if (handler == NULL){
        return;
    }
    Segment *current = handler->free_list;
    while (current != NULL){
        Segment *next = current->next;
        free(current);  
        current = next;
    }

    if (handler->allocated != NULL){
        for (int i = 0; i < TABLE_SIZE; i++){
            HashEntry *entry = &handler->allocated->table[i];
            if (entry->key != NULL && entry->value != TOMBSTONE){
                free(entry->key); 
                free(entry->value);  
            }
        }
        free(handler->allocated->table);  
        free(handler->allocated);  
    }

    if (handler->memory != NULL){
        for (int i = 0; i < handler->total_size; i++){
            free(handler->memory[i]);  
        }
        free(handler->memory);  
    }
    free(handler);
}
