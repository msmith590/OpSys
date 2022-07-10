#include <iostream>
#include <cctype>
#include <cstdlib>
#include <stdlib.h>
#include <cmath>
#include <climits>
#include <algorithm>
#include <vector>
#include "Process.h"

double next_exp(int tail, double lam) {
    double r, x;
    while (1) {
        r = drand48();   /* uniform dist [0.00,1.00) */
        x = -log(r) / lam;  /* log() is natural log */
        if (x > tail) {
            continue;
        } else {
            return x;
        }
    }
}

void generateProcesses(list<Process>& processes, long int s, int numP, int tail, double lam, double alph, int t_cs) {
    srand48(s); // Re-seeds the pseudo-random number generator
    vector<char> pid{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}; // vector containing usable process IDs
    vector<Process> p;
    int arrival = 0;
    int numBursts = 0;
    
    for (int i = 0; i < numP; i++) {
        arrival = floor(next_exp(tail, lam));
        p.push_back(Process(pid[i], arrival, lam));
        numBursts = ceil(drand48() * 100);
        for (int j = 0; j < numBursts - 1; j++) {
            p[i].addCPUBurst(ceil(next_exp(tail, lam)), t_cs);
            p[i].addIOBurst(ceil(next_exp(tail, lam)) * 10);
        }
        p[i].addCPUBurst(ceil(next_exp(tail, lam)), t_cs); // Adds the last CPU Burst time without a following IO Burst
        p[i].calculateTau(alph);
        p[i].printBursts();
    #ifdef DEBUG_MODE
        p[i].printTau();
    #endif
    }
    sort(p.begin(), p.end(), Arrival_Compare::earlyArrival);

    processes.clear(); // Ensures that list is empty before adding new processes
    for (int i = 0; i < (int) p.size(); i++) {
        processes.push_back(p[i]);
    }
}

int nextEvent(list<Process>& incoming, vector<Process>& cpu, list<Process>& readyQ, list<Process>& io, int timer, bool specific) {
   /* Function for determining when the next "interesting" event is during a simulation
    * If the value of 'specific' is true, this function returns the actual time (ms) for the next event, otherwise
    * the value returned corresponds to the legend below:
    * 
    * LEGEND:
    *  -1 ==> No next event possible
    *   0 ==> next event occurs in CPU
    *   1 ==> next event occurs in readyQ
    *   2 ==> next event occurs in io
    *   3 ==> next event occurs in incoming
    * 
    * Note: Function does not handle specifics within a system -- must check separately!
    */
    int rc = -1;
    int next = INT_MAX;
    list<Process>::iterator it;
    if (!cpu.empty()) {
        next = cpu[0].getCurrentCPUBurstTime();
        rc = 0;
    } else { // if CPU is empty, check ready queue for process to add
        if (!readyQ.empty()) { 
            // empty cpu and non-empty ready queue implies that the next action is to add a process to the cpu
            it = readyQ.begin();
            next = it->getCurrentCPUBurstTime();
            rc = 1;
        }
    }
    if (!io.empty()) {
        it = io.begin();
        for (int i = 0; i < (int) io.size(); i++, it++) {
            if (next > it->getCurrentIOBurstTime()) {
                next = it->getCurrentIOBurstTime();
                rc = 2;
                break;
            }
        }
    }
    if (!incoming.empty()) {
        it = incoming.begin();
        if (next > (it->getArrival() - timer)) {
            next = it->getArrival() - timer;
            rc = 3;
        }
    }
#ifdef DEBUG_MODE
    if (rc == -1) {
        printf("No more events can occur...\n");
    } else if (rc == 0) {
        printf("Next event occurs in CPU in %dms\n", next);
    } else if (rc == 1) {
        printf("Next event occurs in READY QUEUE in %dms\n", next);
    } else if (rc == 2) {
        printf("Next event occurs in I/O in %dms\n", next);
    } else if (rc == 3) {
        printf("Next event occurs in ARRIVING in %dms\n", next);
    }
#endif
    if (specific) {
        return next;
    } else {
        return rc;
    }
}

void advanceWait(list<Process>& readyQ, list<Process>& io, int elapse) {
   /* Function that advances time in the ready queue and io subsystem
    * Note: When advancing the time for all applicable processes, ensure that 
    * the next event is not modified 
    */
    for (Process& p : readyQ) {
        p.addWait(elapse);
    }
    for (Process& p : io) {
        p.ioElapsed(elapse);
    }
}

void printQ(list<Process>& readyQ) {
    printf("[Q: ");
    if (readyQ.empty()) {
        printf("empty]\n");
    } else {
        list<Process>::iterator it = readyQ.begin();
        for (int i = 0; i < (int) readyQ.size(); i++, it++) {
            if (i == (int) readyQ.size() - 1) {
                printf("%c]\n", it->getProcessID());
            } else {
                printf("%c ", it->getProcessID());
            }
        }
    }
}

// ---------------------------ALGORITHMS----------------------------------

void FCFS(list<Process>& incoming, int t_cs) {
    vector<Process> cpu;
    list<Process> readyQ;
    list<Process> io;
    Process p;
    int timer = 0;
    int elapse;

    printf("time %dms: Simulator started for FCFS ", timer);
    printQ(readyQ);
    while (!incoming.empty() || !cpu.empty() || !readyQ.empty() || !io.empty())
    {
        int next = nextEvent(incoming, cpu, readyQ, io, timer, false);
        if (next == -1) {
            fprintf(stderr, "ERROR: while loop did not terminate correctly...\n");
            abort();
        } else if (next == 0) { // CPU event ==> process completing a cpu burst
            if (cpu.size() != 1) {
                fprintf(stderr, "ERROR: CPU has too many processes...\n");
                abort();
            }
            elapse = cpu[0].getCurrentCPUBurstTime();
            timer += elapse;
            if ((cpu[0].cpuElapsed(elapse) == 0) && cpu[0].numIOBursts()) {
                cpu[0].completedCPU();
                if (cpu[0].numIOBursts() > 1) {
                    printf("time %dms: Process %c completed a CPU burst; %d bursts to go ", timer, cpu[0].getProcessID(), cpu[0].numCPUBursts());
                    printQ(readyQ);
                } else {
                    printf("time %dms: Process %c completed a CPU burst; %d burst to go ", timer, cpu[0].getProcessID(), cpu[0].numCPUBursts());
                    printQ(readyQ);
                }
                printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", timer, cpu[0].getProcessID(), timer + (t_cs / 2) + cpu[0].getCurrentIOBurstTime());
                printQ(readyQ);
                io.push_back(cpu[0]);
            } else {
                cpu[0].completedCPU();
                printf("time %dms: Process %c terminated ", timer, cpu[0].getProcessID());
                printQ(readyQ);
            }
            cpu.clear();
            // Corner case: does adding context switch out to timer skip over another process?
            timer += t_cs / 2; // time added for switching process out of cpu
        } 
    }
    

}


int main(int argc, char* argv[]) {
    
    if (argc != 8) {
        fprintf(stderr, "ERROR: Incorrect Number of Arguments!\n");
        return EXIT_FAILURE;
    }
    
    int i = 0;
    while (argv[1][i] != '\0') {
        if (isdigit(argv[1][i]) == 0) {
            fprintf(stderr, "ERROR: Argument 1 is not a positive integer!\n");
            return EXIT_FAILURE;
        }
        i++;
    }
    int numProc = atoi(argv[1]);
    if (numProc < 1 || numProc > 26) {
        fprintf(stderr, "Number of processes needs to be in the range of 1 - 26!\n");
        return EXIT_FAILURE;
    }

    i = 0;
    while (argv[2][i] != '\0') {
        if (isdigit(argv[2][i]) == 0) {
            if (argv[2][i] != '-') {
                fprintf(stderr, "ERROR: Argument 2 is not a long integer!\n");
                return EXIT_FAILURE;
            }
        }
        i++;
    }
    long int seed = atol(argv[2]);

    i = 0;
    while (argv[3][i] != '\0') {
        if (isdigit(argv[3][i]) == 0) {
            if (argv[3][i] != '.') {
                fprintf(stderr, "ERROR: Argument 3 is not a float!\n");
                return EXIT_FAILURE;
            }
        }
        i++;
    }
    double lambda = atof(argv[3]);

    i = 0;
    while (argv[4][i] != '\0') {
        if (isdigit(argv[4][i]) == 0) {
            fprintf(stderr, "ERROR: Argument 4 is not a positive integer!\n");
            return EXIT_FAILURE;
        }
        i++;
    }
    int upperBound = atoi(argv[4]);

    i = 0;
    while (argv[5][i] != '\0') {
        if (isdigit(argv[5][i]) == 0) {
            fprintf(stderr, "ERROR: Argument 5 is not a positive integer!\n");
            return EXIT_FAILURE;
        }
        i++;
    }
    int time_cs = atoi(argv[5]);
    if (time_cs % 2 != 0) {
        fprintf(stderr, "ERROR: Argument 5 is not an even integer!\n");
        return EXIT_FAILURE;
    }

    i = 0;
    while (argv[6][i] != '\0') {
        if (isdigit(argv[6][i]) == 0) {
            if (argv[6][i] != '.') {
                fprintf(stderr, "ERROR: Argument 6 is not a float!\n");
                return EXIT_FAILURE;
            }
        }
        i++;
    }
    double alpha = atof(argv[6]);

    i = 0;
    while (argv[7][i] != '\0') {
        if (isdigit(argv[7][i]) == 0) {
            fprintf(stderr, "ERROR: Argument 7 is not a positive integer!\n");
            return EXIT_FAILURE;
        }
        i++;
    }
    int time_slice = atoi(argv[7]);

#ifdef DEBUG_MODE
    printf("# of processes: %d\n", numProc);
    printf("seed: %li\n", seed);
    printf("lamda: %f\n", lambda);
    printf("upper bound: %d\n", upperBound);
    printf("context switch time (milliseconds): %d\n", time_cs);
    printf("alpha: %f\n", alpha);
    printf("time slice for RR (milliseconds): %d\n", time_slice);
#endif

    list<Process> processes;
    generateProcesses(processes, seed, numProc, upperBound, lambda, alpha, time_cs);
    FCFS(processes, time_cs);

    return EXIT_SUCCESS;
}