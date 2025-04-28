#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

const int DATA_SIZE = 5;
const int NUM_READERS = 3;
const int NUM_WRITERS = 2;
const int READS_PER_READER = 4;
const int WRITES_PER_WRITER = 3;

int shared_data[5];
int reader_count = 0;
sem_t mutex, wrt, readTry;

void display_data() {
    printf("Data: [");
    for (int i = 0; i < DATA_SIZE; i++) {
        printf("%d", shared_data[i]);
        if (i < DATA_SIZE - 1) printf(", ");
    }
    printf("]\n");
}

void* reader(void* arg) {
    int reader_id = *(int*)arg;
    for (int i = 0; i < READS_PER_READER; i++) {
        printf("Reader-%d is trying to read...\n", reader_id);
        sem_wait(&readTry);
        sem_wait(&mutex);
        reader_count++;
        if (reader_count == 1) sem_wait(&wrt);
        sem_post(&mutex);
        sem_post(&readTry);

        printf("Reader-%d is reading. Active readers: %d\n", reader_id, reader_count);
        display_data();
        usleep(rand() % 500000);

        sem_wait(&mutex);
        reader_count--;
        if (reader_count == 0) sem_post(&wrt);
        sem_post(&mutex);

        printf("Reader-%d has finished reading.\n", reader_id);
        usleep(rand() % 300000);
    }
    return NULL;
}

void* writer(void* arg) {
    int writer_id = *(int*)arg;
    for (int i = 0; i < WRITES_PER_WRITER; i++) {
        printf("Writer-%d is trying to write...\n", writer_id);
        sem_wait(&readTry);
        sem_wait(&wrt);

        printf("Writer-%d is writing.\n", writer_id);
        for (int j = 0; j < DATA_SIZE; j++) {
            shared_data[j] = rand() % 100 + 1;
        }
        display_data();
        usleep(rand() % 500000);

        sem_post(&wrt);
        sem_post(&readTry);

        printf("Writer-%d has finished writing.\n", writer_id);
        usleep(rand() % 500000);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    for (int i = 0; i < DATA_SIZE; i++) {
        shared_data[i] = rand() % 100 + 1;
    }

    printf("Initial data:\n");
    display_data();

    sem_init(&mutex, 0, 1);
    sem_init(&wrt, 0, 1);
    sem_init(&readTry, 0, 1);

    pthread_t readers[3], writers[2];
    int reader_ids[3], writer_ids[2];

    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    printf("All reads and writes completed!\n");

    sem_destroy(&mutex);
    sem_destroy(&wrt);
    sem_destroy(&readTry);

    return 0;
}
