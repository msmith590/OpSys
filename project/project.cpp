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

void generateProcesses(vector<Process>& p, long int s, int numP, int tail, double lam, double alph) {
    srand48(s); // Re-seeds the pseudo-random number generator
    p.clear(); // Ensures that vector is empty before adding new processes
    vector<char> pid{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}; // vector containing usable process IDs
    int arrival = 0;
    int numBursts = 0;
    
    for (int i = 0; i < numP; i++) {
        arrival = floor(next_exp(tail, lam));
        p.push_back(Process(pid[i], arrival, lam));
        numBursts = ceil(drand48() * 100);
        for (int j = 0; j < numBursts - 1; j++) {
            p[i].addCPUBurst(ceil(next_exp(tail, lam)));
            p[i].addIOBurst(ceil(next_exp(tail, lam)) * 10);
        }
        p[i].addCPUBurst(ceil(next_exp(tail, lam))); // Adds the last CPU Burst time without a following IO Burst
        p[i].calculateTau(alph);
        p[i].printBursts();
    #ifdef DEBUG_MODE
        p[i].printTau();
    #endif
    }

}

// ---------------------------ALGORITHMS----------------------------------




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

    vector<Process> processes;
    generateProcesses(processes, seed, numProc, upperBound, lambda, alpha);
    sort(processes.begin(), processes.end(), Arrival_Compare::earlyArrival);

    return EXIT_SUCCESS;
}