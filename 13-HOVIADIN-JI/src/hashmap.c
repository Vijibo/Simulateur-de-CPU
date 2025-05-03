#include "../include/hashmap.h"

// Exercice 1 (1.1 à 1.6)

//	Transforme une string en un nombre entre 0 et 127. 
unsigned long simple_hash(const char *str){
    unsigned long hash=0;

    for (int i=0; str[i]!='\0'; i++){
        hash=hash*19+str[i];
    }

    return hash%TABLE_SIZE;
}


// Cree et initialise une hashmap
HashMap *hashmap_create(){
    HashMap* map=(HashMap*)malloc(sizeof(HashMap));
    if(!map)return NULL;
    map->table=(HashEntry*)calloc(TABLE_SIZE, sizeof(HashEntry));
    map->size=TABLE_SIZE;
    return map;
}

//insere un element dans la table de hachage.
int hashmap_insert(HashMap *map, const char *key, void *value){
    if(!map || !key) return -1;

    unsigned long hash=simple_hash(key);
    int index=hash;

    while (map->table[hash].key!=NULL && map->table[hash].value!=TOMBSTONE){
        if(strcmp(map->table[hash].key, key)==0){
            map->table[hash].value=value;
            return 0;
        }

        hash=(hash+1)%map->size;
        if(index==(int)hash)return -1;
    }

    if (map->table[hash].value==TOMBSTONE && map->table[hash].key){
        free(map->table[hash].key);  
    }

    map->table[hash].key = strdup(key);
    map->table[hash].value = value;
    return 0;
}

//recupere un element a partir de sa cle.
void *hashmap_get(HashMap *map, const char *key){
    if(!map || !key) return NULL;
    unsigned long hash=simple_hash(key);
    int index=hash;

    while (map->table[hash].key && map->table[hash].value!=TOMBSTONE){
        if(strcmp(map->table[hash].key, key) == 0){
            return map->table[hash].value;
        }

        hash=(hash+1)%map->size;
        if(index==(int)hash)break;
    }

    return NULL;
}

//supprime un element de la table de hachage tout en assurant la continuite du sondage lineaire.
int hashmap_remove(HashMap *map, const char *key){
    if(!map || !key) return -1;
    unsigned long hash=simple_hash(key);
    int index=hash;

    while(map->table[hash].key && map->table[hash].value!=TOMBSTONE){
        if(strcmp(map->table[hash].key,key)==0){
            free(map->table[hash].key);
            map->table[hash].value=TOMBSTONE;
            map->table[hash].key=NULL;
            return 0;
        }

        hash=(hash+1)%map->size;
        if(index==(int)hash)break;
    }

    return -1;
}

// detruit une hashmap
void hashmap_destroy(HashMap *map){
    if(!map)return;

    for(int i=0;i<map->size;i++){
        if(map->table[i].key && map->table[i].value!=TOMBSTONE){
            free(map->table[i].key);
        }
    }
    
    free(map->table);
    free(map);
}
