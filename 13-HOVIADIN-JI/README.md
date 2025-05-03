# SDC - Simulateur de CPU

## Description Generale

SDC est un simulateur de CPU ecrit en C, capable d'executer un langage assembleur personnalise. Il est concu pour simuler les aspects cles d'une architecture de processeur, notamment :

1. Table de hachage generique
2. Gestionnaire de memoire dynamique
3. Analyse syntaxique d’un pseudo-langage assembleur
4. Segmentation memoire (CS, DS, SS, ES)
5. Modes d’adressage varies (immediat, direct, indirect, segmente)
6. Simulation d’un jeu d’instructions (MOV, ADD, JMP, etc.)
7. Gestion de pile (avec SP et BP)
8. Gestion dynamique avec segment ES (ALLOC, FREE)

## Structure du Projet

```
cpu_simulator/
├── include/                # Fichiers d'en-tete
│   ├── addressing.h        # Modes d'adressage et acces memoire
│   ├── cpu.h               # Definition de la structure CPU et fonctions associees
│   ├── hashmap.h           # Table de hachage generique
│   ├── instructions.h      # Enumerations des instructions et registres
│   ├── memory_handler.h    # Segments memoire (DS, SS, ES)
│   └── parser.h            # Fonctions d'analyse de fichiers .asm
│
├── src/                    # Implementations C
│   ├── addressing.c        # Resolution des adresses memoire
│   ├── cpu.c               # Fonctionnement de l’unite de traitement
│   ├── hashmap.c           # Table de hachage (labels, variables, etc.)
│   ├── instructions.c      # Traitement des instructions
│   ├── memory_handler.c    # Allocation/desallocation segmentee
│   ├── parser.c            # Analyse syntaxique des programmes ASM
├   ├───main.c    # Point d’entree du programme utilisateur
│
├── tests/                  # Fichiers de test
│   ├── test_addressing.c
│   ├── test_cpu.c
│   ├── test_hashmap.c
│   ├── test_memory.c
│   └── test_parser.c
│
├── example.asm             # Programme assembleur d’exemple
├── Makefile                # Compilation de tout le projet
├── plan_detaillee.txt                # Fichier de planification du projet
```

## Compilation

### Prerequis

- Systeme UNIX/Linux ou WSL
- gcc (compilateur C)
- make

### Commande

```bash
make 
make test
```

Les fichiers objets et executables sont generes dans le dossier racine.

## Utilisation

```bash
./cpu_simulator <FICHIER_ASM> 
```

- FICHIER_ASM : chemin vers un fichier assembleur `.asm`

Une instruction ou une variable simple = 1 case memoire.

## Concepts Importants

### Architecture CPU simulee

| Registre | Role |
|----------|------|
| AX, BX, CX, DX | Registres generaux |
| IP | Instruction Pointer (position courante) |
| ZF | Zero Flag (resultat = 0) |
| SF | Sign Flag (resultat negatif) |
| SP | Stack Pointer (pile) |
| BP | Base Pointer (base pile) |
| ES | Extra Segment (allocation dynamique) |

### Segments Memoire

| Segment | Utilisation |
|---------|-------------|
| CS | Code (instructions) |
| DS | Donnees (variables globales) |
| SS | Stack (pile) |
| ES | Heap (allocation dynamique) |

### Instructions Disponibles

| Mnémonique | Description |
|------------|-------------|
| MOV dst, src | Copie la valeur |
| ADD dst, src | Addition |
| CMP dst, src | Compare et met a jour ZF, SF |
| JMP addr | Saut inconditionnel |
| JZ addr / JNZ addr | Saut conditionnel selon ZF |
| PUSH val / POP reg | Operations sur la pile |
| ALLOC / FREE | Allocation/liberation dynamique |
| HALT | Arrete l’execution |

ALLOC utilise AX (taille) et BX (strategie) :
- 0 : First Fit
- 1 : Best Fit
- 2 : Worst Fit

### Modes d’adressage

| Exemple | Signification |
|---------|----------------|
| 42 | Valeur immediate |
| AX | Registre |
| [5] | Adresse memoire dans DS |
| [AX] | Indexe par registre |
| [DS:AX] | Segment explicite |

## Tests Inclus

Des tests unitaires sont disponibles pour valider chaque module :
- test_cpu.c : Instructions de base et registres
- test_parser.c : Lecture et parsing de fichier ASM
- test_hashmap.c : Table de hachage
- test_memory.c : Segments memoire
- test_addressing.c : Resolution des adresses

Lance les avec :

```bash
make tests
./test_cpu
./test_parser
# etc.
```


## Auteur et Contexte

Projet universitaire de Licence 2 - LU2IN006 - 2024-2025
Kyrylo Hoviadin et Victor Ji - Groupe 10 n°13
le 02/05/2025