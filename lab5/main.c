#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

int wordCounter(const char *word, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file)
        return -1;
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), file))
        if (strstr(line, word))
            count++;
    fclose(file);
    return count;
}

// Keywords and signals for children (0: WARNING, 1: ERROR, 2: ALERT)
const char *keywords[3] = {"WARNING", "ERROR", "ALERT"};
int signals_arr[3] = {SIGUSR1, SIGUSR2, SIGURG};

int p2c[3][2], c2p[3][2];
int child_index = -1;    
int child_p2c_fd, child_c2p_fd;

void signal_handler(int sig) {
    char fname[256];
    read(child_p2c_fd, fname, sizeof(fname));
    int cnt = wordCounter(keywords[child_index], fname);
    write(child_c2p_fd, &cnt, sizeof(cnt));
}

void termination_handler(int sig) {
    close(child_p2c_fd);
    close(child_c2p_fd);
    exit(0);
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s security_log.txt\n", argv[0]);
        exit(1);
    }
    char *filename = argv[1];
    // Check if the file exists
    if(access(filename, F_OK) == -1) {
        printf("File %s does not exist. Exiting.\n", filename);
        exit(1);
    }
    
    int i;
    for(i = 0; i < 3; i++) {
        pipe(p2c[i]);
        pipe(c2p[i]);
    }
    
    pid_t pids[3];
    // Fork three child processes
    for(i = 0; i < 3; i++) {
        pid_t pid = fork();
        if(pid == 0) { // Child process
            child_index = i;
            close(p2c[i][1]);   
            close(c2p[i][0]);   
            // Close unused pipes in child
            for(int j = 0; j < 3; j++) {
                if(j != i) {
                    close(p2c[j][0]); close(p2c[j][1]);
                    close(c2p[j][0]); close(c2p[j][1]);
                }
            }
            child_p2c_fd = p2c[i][0];
            child_c2p_fd = c2p[i][1];
            signal(signals_arr[i], signal_handler);
            signal(SIGTERM, termination_handler);
            while(1)
                pause();
            exit(0);
        } else { // Parent process
            pids[i] = pid;
            close(p2c[i][0]);   
            close(c2p[i][1]);   
        }
    }
    
    int choice;
    while(1) {
        printf("\nCybersecurity Log Analysis Dashboard\n");
        printf("1. How many WARNING entries?\n");
        printf("2. How many ERROR entries?\n");
        printf("3. How many ALERT entries?\n");
        printf("4. Exit\n");
        printf("Enter 1-4: ");
        if(scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            while(getchar() != '\n');
            continue;
        }
        if(choice == 4) {
            printf("Exiting the parent and its three child processes.\n");
            for(i = 0; i < 3; i++) {
                kill(pids[i], SIGTERM);
                wait(NULL);
            }
            break;
        }
        if(choice < 1 || choice > 4)
            continue;
        
        int index = choice - 1;
        char *signal_name = (index == 0) ? "SIGUSR1" : (index == 1) ? "SIGUSR2" : "SIGURG";
        
        printf("Sending %s to child with PID=%d...\n", signal_name, pids[index]);
        write(p2c[index][1], filename, strlen(filename) + 1);
        kill(pids[index], signals_arr[index]);
        int count;
        read(c2p[index][0], &count, sizeof(count));
        if(count == -1)
            printf("Error counting %s entries in the log file.\n", keywords[index]);
        else
            printf("There are %d %s entries in the log file.\n", count, keywords[index]);
    }
    
    return 0;
}
