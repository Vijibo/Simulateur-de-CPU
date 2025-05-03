#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> // Inclusion pour mesurer le temps d'exécution
#include "../include/cpu.h"
#include "../include/instructions.h"

// Fonction pour tester la vitesse de l'initialisation et de la destruction du CPU
void test_cpu_performance(int memory_size) {
    clock_t start_time, end_time;
    double elapsed_time;

    // Test d'initialisation
    start_time = clock();
    CPU* cpu = cpu_init(memory_size);
    if (cpu == NULL) {
        printf("Erreur d'initialisation du CPU\n");
        return;
    }
    end_time = clock();
    elapsed_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Temps d'initialisation du CPU: %f secondes\n", elapsed_time);

    // Test de destruction
    start_time = clock();
    cpu_destroy(cpu);
    end_time = clock();
    elapsed_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Temps de destruction du CPU: %f secondes\n", elapsed_time);
}

int main() {
    int memory_size = 1024 * 1024;  // Par exemple, 1 Mo de mémoire
    test_cpu_performance(memory_size);

    return 0;
}

/*
int main() {
    int memory_size = 1024 * 1024;  // 1 Mo
    int iterations[] = {100, 500, 1000, 5000, 10000, 20000};
    int count = sizeof(iterations) / sizeof(iterations[0]);

    FILE *f = fopen("iterations_perf.csv", "w");
    if (!f) {
        perror("fopen");
        return 1;
    }

    fprintf(f, "Iterations,TotalTime\n");

    for (int i = 0; i < count; i++) {
        int n = iterations[i];
        clock_t start = clock();

        for (int j = 0; j < n; j++) {
            CPU* cpu = cpu_init(memory_size);
            cpu_destroy(cpu);
        }

        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

        printf("Itérations : %d, Temps total : %f sec\n", n, elapsed);
        fprintf(f, "%d,%f\n", n, elapsed);
    }

    fclose(f);
    return 0;
}


import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("iterations_perf.csv")

plt.plot(data["Iterations"], data["TotalTime"], marker='o', linestyle='-')
plt.title("Temps d'exécution en fonction du nombre d'itérations")
plt.xlabel("Nombre d'itérations")
plt.ylabel("Temps total (secondes)")
plt.grid(True)
plt.show()

*/