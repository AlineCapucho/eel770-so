// Especificações:
// código desenvolvido utilizando MAC OS Ventura com CHIP M1 e {...}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <assert.h>
#include <math.h>
#include <time.h>
#include <string.h>

// Parametrization macros
#define NUM_REINDEER 9
#define NUM_ELVES 6
#define NUM_ITERATIONS 2
// Program must end when Santa Claus delivery presents NUM_ITERATIONS times

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_cond = PTHREAD_COND_INITIALIZER;

int reindeer_ready = 0;
int elves_waiting = 0;
int iteration = 1;

void prepareSleigh(void) {
    printf("Santa Claus is preparing sleigh for the reindeers.\n");
    // Simulate preparing sleigh
    sleep(2);
} 

void getHitched(int id) {
    printf("Reindeer %d hitched to the sleigh.\n", id);
}

void helpElves(void) {
    printf("Santa Claus is helping elves.\n");
    // Simulate helping elves
    sleep(2);
}

void getHelp(int id) {
    printf("Elf %d getting help from Santa Claus.\n", id);
}

void* santa_claus(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        
        while (reindeer_ready != NUM_REINDEER && elves_waiting < 3) {
            pthread_cond_wait(&santa_cond, &mutex);
        }
        
        if (reindeer_ready == NUM_REINDEER) {
            prepareSleigh();
            printf("Santa Claus is delivering gifts.\n");
            // Simulate delivering gifts
            sleep(2);
            printf("Santa Claus is back from delivering gifts.\n");
            reindeer_ready = 0;
            pthread_cond_broadcast(&reindeer_cond);
            printf("------ Iteration %d Completed ------\n", iteration);
            ++iteration;
            // Ending program because Santa Claus deliveried presents NUM_ITERATIONS times
            if (iteration == NUM_ITERATIONS+1) {
                exit(0);
            }
        } else {
            for (int i = 0; i < 3; ++i) {
                helpElves();
                pthread_cond_broadcast(&elves_cond);
            }
            printf("Santa Claus is back from helping the elves.\n");
            elves_waiting -= 3;
        }
        
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* reindeer(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        int id = *(int*)arg;
        sleep(rand() % 5 + 1); // Simulate time to get ready
        
        pthread_mutex_lock(&mutex);
        printf("Reindeer %d returned from vacation.\n", id);
        ++reindeer_ready;
        
        if (reindeer_ready == NUM_REINDEER) {
            pthread_cond_signal(&santa_cond);
        }
        
        pthread_cond_wait(&reindeer_cond, &mutex); // Wait for Santa Claus
        
        getHitched(id);
        pthread_mutex_unlock(&mutex);
        
        // Simulate delivering gifts
        sleep(2);
    }
    
    return NULL;
}

void* elf(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        int id = *(int*)arg;
        sleep(rand() % 5 + 1); // Simulate time to need help
        
        pthread_mutex_lock(&mutex);
        printf("Elf %d needs help from Santa Claus.\n", id);
        ++elves_waiting;
        
        if (elves_waiting == 3) {
            pthread_cond_signal(&santa_cond);
        } else {
            pthread_cond_wait(&elves_cond, &mutex);
        }
        
        getHelp(id);
        pthread_mutex_unlock(&mutex);
        
        // Simulate getting help
        sleep(2);
    }
    
    return NULL;
}

int main() {
    pthread_t santa_thread;
    pthread_t reindeer_threads[NUM_REINDEER];
    pthread_t elf_threads[NUM_ELVES];
    
    srand(time(NULL));
    
    // Create Santa Claus thread
    pthread_create(&santa_thread, NULL, santa_claus, NULL);
    
    // Create reindeer threads
    int reindeer_ids[NUM_REINDEER];
    for (int i = 0; i < NUM_REINDEER; i++) {
        reindeer_ids[i] = i + 1;
        pthread_create(&reindeer_threads[i], NULL, reindeer, &reindeer_ids[i]);
    }
    
    // Create elf threads
    int elf_ids[NUM_ELVES];
    for (int i = 0; i < NUM_ELVES; i++) {
        elf_ids[i] = i + 1;
        pthread_create(&elf_threads[i], NULL, elf, &elf_ids[i]);
    }
    
    // Join threads
    pthread_join(santa_thread, NULL);
    for (int i = 0; i < NUM_REINDEER; i++) {
        pthread_join(reindeer_threads[i], NULL);
    }
    for (int i = 0; i < NUM_ELVES; i++) {
        pthread_join(elf_threads[i], NULL);
    }
    
    return 0;
}