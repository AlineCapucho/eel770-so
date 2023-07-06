#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <math.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

// Comando para compilar: gcc -std=c17 -pthread producer_consumer.c -o producer_consumer 
// Comando para executar: ./producer_consumer (parâmetro 1 - N) (parâmetro 2 - Np) (parâmetro 3 - Nc)

// Semáforos para coordenação e sincronização
sem_t mutex;
sem_t empty;
sem_t full;

int M = 100000; // Número de números que devem ser consumidos
// int M = 5; // Número de números que devem ser consumidos
int max = 10000000; // Maior valor possível de número aleatório

// Buffer para armazenar números aleatórios e count para indicar último número aleatório gerado
// O consumo de números aleatórios ocorre de forma LIFO
int* buffer;
int count = 0;

// Tamanho do buffer, número de produtores e número de consumidores
int N;
int Np;
int Nc;

// Variável de controle que indica se os consumidores terminaram de consumir M recursos
// Se sim, então o próximo produtor a executar deve usar o sinal do semáforo para liberar
// todas as threads consumidoras que estão aguardando continuar, para finalizarem
int sig_consumers = 0;

// Função de validação de número primo
int is_prime(int n) {
    int i;
    for (i = 2; i <= n / 2; ++i) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

void write_buffer_data(int filled, int firstcall) {
    char filename[] = "buffer_usage.txt";
    FILE* ptr;
    ptr = fopen(filename, "a");

    if (ptr == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    if (firstcall == 1) {
        fprintf(ptr, "%s", "\n");
        fprintf(ptr, "%s", "# ");
    }
    else {
        fprintf(ptr, "%d", filled);
        fprintf(ptr, "%s", " ");
    }

    fclose(ptr);
}

// Produz número aleatório
void* producer(void* ptr) {
    srand(time(NULL)); // random number generator seed
    while (1) {
        int x = 1 + rand() % max;
        sem_wait(&empty);
        sem_wait(&mutex);
        if (sig_consumers == 1) {
            for (int i = 0; i <= Nc; ++i) {
                sem_post(&full);
            }
            sig_consumers = 0;
            sem_post(&mutex);
            pthread_exit(NULL);
        }
        else if (M == 0) {
            sem_post(&mutex);
            sem_post(&full);
            pthread_exit(NULL);
        }
        buffer[count] = x;
        count++;
        write_buffer_data(count, 0);
        sem_post(&mutex);
        sem_post(&full);
    }
}

// Consome número aleatório
void* consumer(void* ptr) {
    int y = -1;
    while (1) {
        sem_wait(&full);
        sem_wait(&mutex);
        if (M == 0) {
            sem_post(&mutex);
            sem_post(&empty);
            pthread_exit(NULL);
        }
        y = buffer[count-1];
        count--;
        write_buffer_data(count, 0);
        if (is_prime(y)) {
            printf("Número consumido %d é primo.\n", y);
        }
        else {
            printf("Número consumido %d não é primo.\n", y);
        }
        M--;
        if (M == 0) {
            printf("Consumidores consumiram o número de índice 10^5. Encerrando programa.\n");
            sig_consumers = 1;
            for (int i = 0; i <= Np; ++i) {
                sem_post(&empty);
            }
            sem_post(&mutex);
            pthread_exit(NULL);
        }
        sem_post(&mutex);
        sem_post(&empty);
    }
}

// Inicia um vetor de threads produtores
void init_producers_threads(pthread_t* arr, int size) {
    for (int i = 0; i < size; ++i) {
        if (pthread_create(&arr[i], NULL, &producer, NULL) != 0) {
            printf("Creation of thread %d failed.\n", i);
            exit(1);
        }
    }
}

// Inicia um vetor de threads consumidores
void init_consumers_threads(pthread_t* arr, int size) {
    for (int i = 0; i < size; ++i) {
        if (pthread_create(&arr[i], NULL, &consumer, NULL) != 0) {
            printf("Creation of thread %d failed.\n", i);
            exit(1);
        }
    }
}

// Aguarda a finalização de um vetor de threads
void join_pthread_arr(pthread_t* arr, int size) {
    for (int i = 0; i < size; ++i) {
        if (pthread_join(arr[i], NULL) != 0) {
            printf("Execution of thread %d failed.\n", i);
            exit(1);
        }
    }
}

// Escreve os dados de tempo de execução em função dos parâmetros em um arquivo
void write_runtime_data(int N, int Np, int Nc, double runtime) {
    char filename[] = "Runtimes.txt";
    FILE* ptr;
    ptr = fopen(filename, "a");

    if (ptr == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    fprintf(ptr, "%d", N);
    fprintf(ptr, "%s", " ");
    fprintf(ptr, "%d", Np);
    fprintf(ptr, "%s", " ");
    fprintf(ptr, "%d", Nc);
    fprintf(ptr, "%s", " ");
    fprintf(ptr, "%f", runtime);
    fprintf(ptr, "%s", "\n");

    fclose(ptr);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Número incorreto de parâmetros passados. Encerrando programa.\n");
        return 1;
    }
    else {
        N = atoi(argv[1]);
        Np = atoi(argv[2]);
        Nc = atoi(argv[3]);

        buffer = malloc(N * sizeof(int));
        sem_init(&mutex, 0, 1);
        sem_init(&empty, 0, N);
        sem_init(&full, 0, 0);
        write_buffer_data(count, 1);

        struct timespec start, finish;
        double elapsed;
        clock_gettime(CLOCK_MONOTONIC, &start);

        pthread_t producers[Np];
        pthread_t consumers[Nc];
        init_producers_threads(producers, Np);
        init_consumers_threads(consumers, Nc);
        join_pthread_arr(producers, Np);
        join_pthread_arr(consumers, Nc);

        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

        printf("Execução das threads levou %f segundos.\n", elapsed);

        write_runtime_data(N, Np, Nc, elapsed);

        free(buffer);
        sem_destroy(&mutex);
        sem_destroy(&empty);
        sem_destroy(&full);

        printf("Encerrando programa.\n");
        return 0;
    }
}