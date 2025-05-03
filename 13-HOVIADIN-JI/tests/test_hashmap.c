#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/hashmap.h"

int main() {
    HashMap *map = hashmap_create();
    if (!map) {
        printf("echec creation de hashmap\n");
        return 1;
    }

    int val1 = 42;
    int val2 = 1337;
    hashmap_insert(map, "cle1", &val1);
    hashmap_insert(map, "cle2", &val2);

    int *v1 = hashmap_get(map, "cle1");
    int *v2 = hashmap_get(map, "cle2");

    printf("cle1 : %d \n", v1 ? *v1 : -1);
    printf("cle2 : %d \n", v2 ? *v2 : -1);

    hashmap_remove(map, "cle1");
    printf("cle1 apres suppression : %p \n", hashmap_get(map, "cle1"));

    hashmap_destroy(map);
    return 0;
}
