#include <stdio.h>
#include "../include/parser.h"

int main(){
    ParserResult *res = parse("exemple.asm");

    if (!res){
        printf("Erreur parsing\n");
        return 1;
    }
    printf("Instructions .DATA : %d\n", res->data_count);
    printf("Instructions .CODE : %d\n", res->code_count);
    printf("Labels : \n");

    for (int i = 0; i < res->labels->size; i++){
        if (res->labels->table[i].key && res->labels->table[i].key != (void*)-1){
            printf("%s -> %ld\n", res->labels->table[i].key, (long)res->labels->table[i].value);
        }
    }

    free_parser_result(res);
    return 0;
}
