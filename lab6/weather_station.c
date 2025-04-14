#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>

#define MAX_READINGS 10
#define SHM_KEY 0x1234
#define MSG_KEY 0x1234

struct reading {
    float temperature;
    float humidity;
    float pressure;
    char timestamp[20];
};

struct weather_data {
    int count;
    struct reading readings[MAX_READINGS];
};

struct msg_buffer {
    long mtype; // 1 = data, 2 = done
    int reading_index;
};

void generate_reading(struct reading *r) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    r->temperature = 15.0 + (rand() % 200) / 10.0;
    r->humidity    = 30.0 + (rand() % 600) / 10.0;
    r->pressure    = 980.0 + (rand() % 500) / 10.0;

    strftime(r->timestamp, 20, "%H:%M:%S", t);
}

int main() {
    srand(time(NULL));

    int shmid = shmget(SHM_KEY, sizeof(struct weather_data), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    struct weather_data *data = (struct weather_data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    data->count = 0;

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    for (int i = 0; i < MAX_READINGS; ++i) {
        sleep(1 + rand() % 2); // Sleep 1-2 sec
        struct reading r;
        generate_reading(&r);
        data->readings[i] = r;
        data->count++;

        struct msg_buffer msg;
        msg.mtype = 1;
        msg.reading_index = i;

        if (msgsnd(msgid, &msg, sizeof(msg.reading_index), 0) == -1) {
            perror("msgsnd failed");
        }

        printf("[Weather Station] Sent Reading %d: Temp=%.1f Hum=%.1f Press=%.1f Time=%s\n",
               i + 1, r.temperature, r.humidity, r.pressure, r.timestamp);
    }

    struct msg_buffer done_msg = {2, -1};
    msgsnd(msgid, &done_msg, sizeof(done_msg.reading_index), 0);

    shmdt(data);
    return 0;
}
