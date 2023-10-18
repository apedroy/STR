#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>  

typedef struct {
    char name[10];
    int execution_time;
    int period;
    int deadline;
} Task;

typedef struct {
    int num_tasks;
    Task tasks[100]; // Assumindo um máximo de 100 tarefas por matriz
} Matrix;

int check_utilization(Matrix matrix) {
    double utilization = 0.0;

    for (int i = 0; i < matrix.num_tasks; i++) {
        double task_utilization = (double)matrix.tasks[i].execution_time / matrix.tasks[i].period;
        utilization += task_utilization;
    }

    return (utilization <= 1.0);
}


 //Função para calcular o máximo divisor comum (GCD)
int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

// Função para calcular o mínimo múltiplo comum (LCM)
int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

// Função para calcular o MMC dos períodos
int max_time(Matrix matrix) {
    int result = matrix.tasks[0].period;
    for (int i = 1; i < matrix.num_tasks; i++) {
        result = lcm(result, matrix.tasks[i].period);
    }
    return result;
}


int *min_time(Matrix matrix, int max_period) {
    int menor_periodo = matrix.tasks[0].period;
    int maior_execucao = matrix.tasks[0].execution_time;
    int *candidatos = NULL;
    int n = 0;
    int m = 0;
    int h = 0;

    for (int i = 0; i < matrix.num_tasks; i++) {
        if (menor_periodo > matrix.tasks[i].period) {
            menor_periodo = matrix.tasks[i].period;
        }
        if (maior_execucao < matrix.tasks[i].execution_time) {
            maior_execucao = matrix.tasks[i].execution_time;
        }
    }

    // Lógica para encontrar candidatos aqui...

    for (int i = maior_execucao; i <= menor_periodo; i++) {
        if (max_period % i == 0) {
            // Aloca memória para mais um candidato
            int *temp = (int *)realloc(candidatos, (n + 1) * sizeof(int));
            if (temp == NULL) {
                // Lida com falha na alocação de memória
                free(candidatos);
                return NULL;
            }
            candidatos = temp;
            candidatos[n] = i;
            n++;
        }
    }

    int *resultado = (int *)malloc((n + 1) * sizeof(int)); // Alocando memória para os candidatos e um elemento extra para zero

    for (int i = 0; i < n; i++) {
        for (int x = 0; x < matrix.num_tasks; x++) {
            // Lógica para verificar se o ciclo candidato é adequado com base nos prazos
            if (2 * candidatos[i] - gcd(candidatos[i], matrix.tasks[x].period) <= matrix.tasks[x].deadline) {
                m++;
            }
        }
        if (m == matrix.num_tasks) {
            resultado[h] = candidatos[i];
            h++;
        }
        m=0;
    }

    resultado[h] = 0; // Marca o final do array com zero

    free(candidatos); // Libera a memória alocada dinamicamente para candidatos

    return resultado;
}

// Função para determinar se a matriz é escalonável pelo algoritmo RM (Rate Monotonic)
int is_rm_schedulable(Matrix matrix) {
 float total_utilization = 0;
    
    for (int i = 0; i < matrix.num_tasks; i++) {
        total_utilization += (float)matrix.tasks[i].execution_time / matrix.tasks[i].period;
    }
    float upper_bound = matrix.num_tasks * (pow(2, 1.0/matrix.num_tasks) - 1);
    return (total_utilization <= upper_bound);
}

// Função para determinar se a matriz é escalonável pelo algoritmo EDF (Earliest Deadline First)
int is_edf_schedulable(Matrix matrix) {
    float current_time = 0;
    int verifica = 0;
    
    
    for (int i = 0; i < matrix.num_tasks; i++) {
        if(matrix.tasks[i].deadline == matrix.tasks[i].period){
        	verifica++;
        	//printf("   verifica: %d\n", verifica);
		}
	}
    if(verifica == matrix.num_tasks){
       	for (int i = 0; i < matrix.num_tasks; i++) {
       		current_time += (float)matrix.tasks[i].execution_time/matrix.tasks[i].period;
       		//printf("    current_time: %f\n", current_time); // Preencha com o valor apropriado
       	}
       	if (current_time > 1 ) {
	    	return 0; // não é escalonavel.
       	}
	}
    else{
    	for (int i = 0; i < matrix.num_tasks; i++) {
     		current_time += (float) matrix.tasks[i].execution_time/matrix.tasks[i].deadline;
      	}
       	if (current_time > 1) {
	    	return -1; // inconclusivo.
    	}
	}
            	
    
    
    return 1; // Escalonável, todas as tarefas atendem aos prazos

}

int main() {
    FILE *file = fopen("arquivo.txt", "r");

    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    int num_matrices = 0;
    int current_matrix = 0;
    Matrix matrices[100]; // Assumindo um máximo de 100 matrizes

    while (!feof(file)) {
        char line[100];
        if (fgets(line, sizeof(line), file) != NULL) {
            if (isdigit(line[0])) {
                // Começou uma nova matriz
                current_matrix++;
                matrices[current_matrix].num_tasks = 0;
            } else {
                // Continuar lendo tarefas na matriz atual
                sscanf(line, "%s %d %d %d", matrices[current_matrix].tasks[matrices[current_matrix].num_tasks].name,
                       &matrices[current_matrix].tasks[matrices[current_matrix].num_tasks].execution_time,
                       &matrices[current_matrix].tasks[matrices[current_matrix].num_tasks].period,
                       &matrices[current_matrix].tasks[matrices[current_matrix].num_tasks].deadline);
                matrices[current_matrix].num_tasks++;
            }
        }
    }

    fclose(file);

    // Processar as matrizes e determinar escalonabilidade para os algoritmos
    for (int i = 1; i <= current_matrix; i++) {
        printf("Carga %d\n", i);

        // Executivo Cíclico (Assumindo que todas as tarefas podem ser executadas)
        // Verificar taxa de utilização
        int utilization_check = check_utilization(matrices[i]);
        printf("  Executivo: %s\n", utilization_check ? "Sim" : "Nao");
        
 	 if (utilization_check) {
          int max_period = max_time(matrices[i]);
          int *min_period = min_time(matrices[i], max_period);
        printf("    Ciclo maior: %d\n", max_period);
        printf("    Ciclo menor(possiveis valores): ");
		for (int y = 0; min_period[y] != 0; y++) {
        printf("%d ",min_period[y]);
    	}
    	printf("\n");
		 free(min_period);
		
		
		
        } 
		else {
            // Definir Ciclo Maior e Ciclo Menor como 0
            printf("    Ciclo maior: 0\n");
            printf("    Ciclo menor: 0\n");
        }

        // EDF (Earliest Deadline First)
        int edf_schedulable = is_edf_schedulable(matrices[i]);
        if (edf_schedulable == -1){
        	printf("  EDF: inconclusivo\n");
		}
        else{
        	printf("  EDF: %s\n", edf_schedulable ? "Sim" : "Nao");
    	}
    	// RM (Rate Monotonic)
		if(utilization_check == 0){
    		printf("  RM: Nao\n");	
		}
    	else{
        	int rm_schedulable = is_rm_schedulable(matrices[i]);
			printf("  RM: %s\n", rm_schedulable ? "Sim" : "inconclusivo");	
		}
    	
	}
	
    return 0;
}
