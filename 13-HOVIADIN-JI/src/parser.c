#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/parser.h"

// Fonction trim d'annonce
char *trim(char *str){
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')){
        *end = '\0';
        end--;
    }
    return str;
}

// Fonction search_and_replace d'annonce
int search_and_replace(char **str, HashMap *values){
    if (!str || !*str || !values) return 0;
    
    int replaced = 0;
    char *input = *str;
    
    // Iterate through all keys in the hashmap
    for (int i = 0; i < values->size; i++){
        if (values->table[i].key && values->table[i].key != (void*)-1){
            char *key = values->table[i].key;
            int value = (int)(long)values->table[i].value;
            
            // Find potential substring match
            char *substr = strstr(input, key);
            if (substr){
                // Construct replacement buffer
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "%d", value);
                
                // Calculate lengths
                int key_len = strlen(key);
                int repl_len = strlen(replacement);
                //int remain_len = strlen(substr + key_len);
                
                // Create new string
                char *new_str = (char*)malloc(strlen(input) - key_len + repl_len + 1);
                strncpy(new_str, input, substr - input);
                new_str[substr - input] = '\0';
                strcat(new_str, replacement);
                strcat(new_str, substr + key_len);
                
                // Free and update original string
                free(input);
                *str = new_str;
                input = new_str;
                
                replaced = 1;
            }
        }
    }
    
    // Trim the final string
    if (replaced){
        char *trimmed = trim(input);
        if (trimmed != input){
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }
    
    return replaced;
}

//Supprimer les commentaires d'une ligne
void remove_comments(char *line){
    if (!line) return;
    char *comment_start = strstr(line, "//");

    if (!comment_start){
        comment_start = strchr(line, ';');
    }
    
    if (comment_start){
        *comment_start = '\0';
    }
}

// Analyser et stocker une ligne de la section .DATA
Instruction *parse_data_instruction(const char *line, HashMap *memory_locations){
    if (!line || !memory_locations) return NULL;
    char *line_copy = strdup(line);
    if (!line_copy) return NULL;
    // Supprimer les commentaires
    remove_comments(line_copy);
    // Appliquer trim pour enlever les espaces
    char *trimmed_line = trim(line_copy);

    // Ignorer les lignes vides
    if (!*trimmed_line){
        free(line_copy);
        return NULL;
    }

    Instruction *instr = (Instruction*)malloc(sizeof(Instruction));

    if (!instr){
        free(line_copy);
        return NULL;
    }
    
    instr->mnemonic = NULL;
    instr->operand1 = NULL;
    instr->operand2 = NULL;
    char *token = strtok(trimmed_line, " \t");

    if (token){
        instr->mnemonic = strdup(token);
        token = strtok(NULL, " \t");

        if (token){
            instr->operand1 = strdup(token);
            token = strtok(NULL, "");

            if (token){
                instr->operand2 = strdup(trim(token));
            } else{
                instr->operand2 = strdup("");
            }

        } else{
            instr->operand1 = strdup("");
            instr->operand2 = strdup("");
        }
    }
    
    if (!instr->mnemonic || !instr->operand1 || !instr->operand2){
        if (instr->mnemonic) free(instr->mnemonic);
        if (instr->operand1) free(instr->operand1);
        if (instr->operand2) free(instr->operand2);
        free(instr);
        free(line_copy);
        return NULL;
    }
    
    static int current_address = 0;
    hashmap_insert(memory_locations, instr->mnemonic, (void*)(long)current_address);
    int count = 1;

    for (char *p = instr->operand2; *p; p++){
        if (*p == ',') count++;
    }

    current_address += count;
    free(line_copy);
    return instr;
}

// Analyser et stocker une ligne de la section .CODE
Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count){
    if (!line || !labels) return NULL;
    char *line_copy = strdup(line);
    if (!line_copy) return NULL;
    // Supprimer les commentaires
    remove_comments(line_copy);
    // Appliquer trim pour enlever les espaces
    char *trimmed_line = trim(line_copy);

    // Ignorer les lignes vides
    if (!*trimmed_line){
        free(line_copy);
        return NULL;
    }
    
    Instruction *instr = (Instruction*)malloc(sizeof(Instruction));

    if (!instr){
        free(line_copy);
        return NULL;
    }
    
    instr->mnemonic = NULL;
    instr->operand1 = NULL;
    instr->operand2 = NULL;
    char *label_end = strchr(trimmed_line, ':');

    if (label_end){
        *label_end = '\0';
        char *label = trim(trimmed_line);
        hashmap_insert(labels, strdup(label), (void*)(long)code_count);
        // Partie apres le label
        trimmed_line = trim(label_end + 1);
    }
    
    if (!*trimmed_line) {
        free(instr);
        free(line_copy);
        return NULL;
    }
    
    char *token = strtok(trimmed_line, " \t");
    if (token){
        instr->mnemonic = strdup(token);
        char *operands = strtok(NULL, "");

        if (operands){
            operands = trim(operands);
            char *comma = strchr(operands, ',');

            if (comma){
                *comma = '\0';
                instr->operand1 = strdup(trim(operands));
                instr->operand2 = strdup(trim(comma + 1));
            } else{
                // Seulement 1 operande
                instr->operand1 = strdup(operands);
                instr->operand2 = strdup("");
            }

        } else{
            // Pas d'operandes
            instr->operand1 = strdup("");
            instr->operand2 = strdup("");
        }

    } else{
        free(instr);
        free(line_copy);
        return NULL;
    }
    
    if (!instr->mnemonic || !instr->operand1 || !instr->operand2){
        if (instr->mnemonic) free(instr->mnemonic);
        if (instr->operand1) free(instr->operand1);
        if (instr->operand2) free(instr->operand2);
        free(instr);
        free(line_copy);
        return NULL;
    }
    
    free(line_copy);
    return instr;
}

// Analyse un fichier assembleur complet en identifiant les sections .DATA et .CODE et en traitant chaque ligne de la maniere appropriee.
ParserResult *parse(const char *filename){
    if (!filename) return NULL;
    FILE *file = fopen(filename, "r");

    if (!file){
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }
    
    ParserResult *result = (ParserResult*)malloc(sizeof(ParserResult));

    if (!result){
        fclose(file);
        return NULL;
    }
    
    result->data_instructions = NULL;
    result->code_instructions = NULL;
    result->data_count = 0;
    result->code_count = 0;
    result->labels = hashmap_create();
    result->memory_locations = hashmap_create();
    
    if (!result->labels || !result->memory_locations){
        if (result->labels) hashmap_destroy(result->labels);
        if (result->memory_locations) hashmap_destroy(result->memory_locations);
        free(result);
        fclose(file);
        return NULL;
    }
    
    char line[256];
    int in_data = 0, in_code = 0;
    
    while (fgets(line, sizeof(line), file)){
        // Supprimer les commentaires
        remove_comments(line);
        char *trimmed = trim(line);
        if (!*trimmed) continue;

        if (strncmp(trimmed, ".DATA", 5) == 0){
            in_data = 1;
            in_code = 0;
            continue;
        } else if (strncmp(trimmed, ".CODE", 5) == 0){
            in_code = 1;
            in_data = 0;
            continue;
        }
        
        if (in_data){
            Instruction *instr = parse_data_instruction(trimmed, result->memory_locations);

            if (instr){
                result->data_instructions = realloc(result->data_instructions, sizeof(Instruction*) * (result->data_count + 1));
                if (!result->data_instructions){
                    free_parser_result(result);
                    fclose(file);
                    return NULL;
                }
                result->data_instructions[result->data_count++] = instr;
            }
        } else if (in_code){
            Instruction *instr = parse_code_instruction(trimmed, result->labels, result->code_count);
            if (instr){
                result->code_instructions = realloc(result->code_instructions, sizeof(Instruction*) * (result->code_count + 1));
                if (!result->code_instructions){
                    free_parser_result(result);
                    fclose(file);
                    return NULL;
                }
                result->code_instructions[result->code_count++] = instr;
            }
        }
    }
    
    fclose(file);
    return result;
}

// Supprimer un element de type ParserResult
void free_parser_result(ParserResult *result){
    if (!result) return;

    //DATA
    if (result->data_instructions){
        for (int i = 0; i < result->data_count; i++){
            if (result->data_instructions[i]){

                if (result->data_instructions[i]->mnemonic) 
                    free(result->data_instructions[i]->mnemonic);

                if (result->data_instructions[i]->operand1) 
                    free(result->data_instructions[i]->operand1);

                if (result->data_instructions[i]->operand2) 
                    free(result->data_instructions[i]->operand2);

                free(result->data_instructions[i]);
            }
        }
        free(result->data_instructions);
    }
    
    //CODE
    if (result->code_instructions){
        for (int i = 0; i < result->code_count; i++){
            if (result->code_instructions[i]){

                if (result->code_instructions[i]->mnemonic) 
                    free(result->code_instructions[i]->mnemonic);

                if (result->code_instructions[i]->operand1) 
                    free(result->code_instructions[i]->operand1);

                if (result->code_instructions[i]->operand2) 
                    free(result->code_instructions[i]->operand2);

                free(result->code_instructions[i]);
            }
        }
        free(result->code_instructions);
    }
    
    // Hashmaps
    if (result->labels) hashmap_destroy(result->labels);
    if (result->memory_locations) hashmap_destroy(result->memory_locations);
    free(result);
}

// Remplace les variables par leur adresse dans le segment de donnees et les etiquettes par leur adresse dans le code
int resolve_constants(ParserResult *result){
    if (!result) return -1;
    
    for (int i = 0; i < result->code_count; i++){
        Instruction *instr = result->code_instructions[i];
        if (!instr) continue;

        if (instr->operand1 && *instr->operand1){
            char *op1_copy = strdup(instr->operand1);
            if (search_and_replace(&op1_copy, result->memory_locations)){
                free(instr->operand1);
                instr->operand1 = op1_copy;
            } 
            else if (search_and_replace(&op1_copy, result->labels)){
                free(instr->operand1);
                instr->operand1 = op1_copy;
            }
            else{
                free(op1_copy);
            }
        }
        
        if (instr->operand2 && *instr->operand2){
            char *op2_copy = strdup(instr->operand2);
            if (search_and_replace(&op2_copy, result->memory_locations)){
                free(instr->operand2);
                instr->operand2 = op2_copy;
            } 
            else if (search_and_replace(&op2_copy, result->labels)){
                free(instr->operand2);
                instr->operand2 = op2_copy;
            }
            else{
                free(op2_copy);
            }
        }
    }
    
    return 0;
}