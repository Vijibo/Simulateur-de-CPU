.DATA
val1 DW 42     ; Initialiser val1 à 42
val2 DW 123    ; Initialiser val2 à 123
array DB 1, 2, 3, 4, 5  ; Tableau de 5 valeurs
result DW 0    ; Résultat initialisé à 0

.CODE
start:  MOV AX, 10      ; Charger 10 dans AX
        MOV BX, 20      ; Charger 20 dans BX
        MOV CX, [val1]  ; Charger val1 dans CX (42)
        
        ; Test des opérations de pile
        PUSH AX         ; Empiler AX (10)
        PUSH BX         ; Empiler BX (20)
        PUSH CX         ; Empiler CX (42)
        
        ; Modifier les registres pour tester le POP
        MOV AX, 0
        MOV BX, 0
        MOV CX, 0
        
        ; Dépiler dans l'ordre inverse
        POP CX          ; CX = 42
        POP BX          ; BX = 20
        POP AX          ; AX = 10
        
        ; Test de calcul avec ADD
        ADD AX, BX      ; AX = AX + BX (10 + 20 = 30)
        
        ; Test de comparaison
        CMP AX, CX      ; Comparer AX (30) avec CX (42)
        JZ equal        ; Si égal, sauter à 'equal' (ne sera pas exécuté)
        JMP continue    ; Sinon, continuer
        
equal:  MOV DX, 100     ; Cette ligne ne doit pas être exécutée
        
continue:
        ; Test de boucle
        MOV DX, 0       ; Initialiser compteur à 0
        
loop:   CMP DX, 5       ; Comparer DX avec 5
        JZ end_loop     ; Si égal, sortir de la boucle
        
        ; Accéder au tableau avec adressage indirect
        MOV AX, DX      ; AX = indice actuel
        MOV BX, [array] ; Charger la valeur à array[0]
        ADD BX, DX      ; Ajouter l'indice
        
        PUSH BX         ; Empiler le résultat
        
        ADD DX, 1       ; Incrémenter le compteur
        JMP loop        ; Répéter la boucle
        
end_loop:
        ; Test final de lecture/écriture mémoire
        MOV AX, 999
        MOV [result], AX  ; Stocker AX dans result
        MOV BX, [result]  ; Lire result dans BX
        
        HALT            ; Arrêter l'exécution