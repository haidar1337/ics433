#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int n;
    
    do {
        printf("Enter the number of child processes (>= 0): ");
        scanf("%d", &n);
        if(n < 0)
            printf("Invalid input. Please enter a non-negative integer.\n");
    } while(n < 0);
    
    createChain(0, n);
    return 0;
}

void createChain(int depth, int n) {
    printf("PID: %d, Depth: %d, PPID: %d\n", getpid(), depth, getppid());
    if(depth == n) {
        // print the date if its the last process
        execl("/bin/date", "date", (char *)NULL);
        perror("execl failed");
        exit(1);
    } else {
        pid_t pid = fork();
        if(pid < 0) {
            perror("fork failed");
            exit(1);
        } else if(pid == 0) {
            createChain(depth + 1, n);
        } else {
            wait(NULL);
        }
    }
}

