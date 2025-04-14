#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>

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
    long mtype;
    int reading_index;
};

int main() {
    int shmid = shmget(SHM_KEY, sizeof(struct weather_data), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    struct weather_data *data = (struct weather_data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    int msgid = msgget(MSG_KEY, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    int received = 0;

    while (1) {
        struct msg_buffer msg;
        if (msgrcv(msgid, &msg, sizeof(msg.reading_index), 0, 0) == -1) {
            perror("msgrcv failed");
            break;
        }

        if (msg.mtype == 2) {
            printf("\n[Dashboard] Received completion message. Exiting...\n");
            break;
        }

        int idx = msg.reading_index;
        struct reading r = data->readings[idx];

        printf("\n[New Reading] Time: %s\n", r.timestamp);
        printf("Temperature: %.1f°C\nHumidity: %.1f%%\nPressure: %.1f hPa\n",
               r.temperature, r.humidity, r.pressure);

        received++;
        float sum_temp = 0.0, sum_hum = 0.0, sum_pres = 0.0;
        for (int i = 0; i < received; ++i) {
            sum_temp += data->readings[i].temperature;
            sum_hum  += data->readings[i].humidity;
            sum_pres += data->readings[i].pressure;
        }

        printf("\n[Statistics] Based on %d readings:\n", received);
        printf("Avg Temperature: %.1f°C\n", sum_temp / received);
        printf("Avg Humidity: %.1f%%\n", sum_hum / received);
        printf("Avg Pressure: %.1f hPa\n", sum_pres / received);
    }

    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
