#ifndef _PROCESS_
#define _PROCESS_

#include <list>
#include <cmath>
#include <stdio.h>

using namespace std;

class Process {

friend class Arrival_Compare; // friendly comparator class for process objects
// friend class Anticipated_Compare;

private:    
    list<int> cpuBursts;
    list<int> ioBursts;
    list<int> tauEstimates; // Estimated CPU Burst times
    char processID;
    int arrivalTime; // Gets set once by constructor and never modified afterwards

public:
    Process() {
        processID = 'A';
        arrivalTime = 0;
        tauEstimates.push_back(0);
    }

    /* arr is the arrival time and lam is used to calculate the initial tau */
    Process(char p, int arr, double lam) {
        processID = p;
        arrivalTime = arr;
        tauEstimates.push_back(ceil(1.0 / lam)); // the return value of ceil is a double which gets downcasted to an integer
    }

// ---------------GETTERS---------------------------------------

    char getProcessID() {
        return processID;
    }

    int getArrival() {
        return arrivalTime;
    }

    int getCurrentCPUBurstTime() {
        return *(cpuBursts.begin());
    }

    int getCurrentIOBurstTime() {
        return *(ioBursts.begin());
    }


// ---------------MODIFIERS---------------------------------

    void addCPUBurst(int x) {
        cpuBursts.push_back(x);
    }

    void addIOBurst(int x) {
        ioBursts.push_back(x);
    }

    /* This function gets called to calculate all estimated CPU burst times 
        once simulation has generated random CPU Bursts */
    void calculateTau(double alph) {
        double tau;
        list<int>::iterator itC = cpuBursts.begin();
        list<int>::iterator itT = tauEstimates.begin();
        for (int i = 1; i < (int) cpuBursts.size(); i++, itC++, itT++) {
            tau = ceil((alph * *(itC)) + ((1.0 - alph) * *(itT)));
            tauEstimates.push_back(tau);
        }
    }

    void cpuElapsed(int e) {
        list<int>::iterator it = cpuBursts.begin();
        *it = *it - e;
        if (*it < 0) {
            fprintf(stderr, "ERROR: Missed an earlier event!\n");
            abort();
        } else if (*it == 0) {
            cpuBursts.pop_front();
        }
    }

    void ioElapsed(int e) {
        list<int>::iterator it = ioBursts.begin();
        *it = *it - e;
        if (*it < 0) {
            fprintf(stderr, "ERROR: Missed an earlier event!\n");
            abort();
        } else if (*it == 0) {
            ioBursts.pop_front();
        }
    }

// ---------------------OUTPUT---------------------------------------

    void printBursts() {
        if ((int) cpuBursts.size() == 1) {
            printf("Process %c: arrival time %dms; tau %dms; %ld CPU burst:\n", processID, arrivalTime, *(tauEstimates.begin()), cpuBursts.size());
        } else {
            printf("Process %c: arrival time %dms; tau %dms; %ld CPU bursts:\n", processID, arrivalTime, *(tauEstimates.begin()), cpuBursts.size());
        }
        list<int>::iterator itC = cpuBursts.begin();
        list<int>::iterator itI = ioBursts.begin();
        for (int i = 0; i < (int) ioBursts.size(); i++, itC++, itI++) {
            printf("--> CPU burst %dms --> I/O burst %dms\n", *(itC), *(itI));
        }
        printf("--> CPU burst %dms\n", *(itC));
    }

    void printTau() {
        printf("Process %c: Tau Estimates\n", processID);
        list<int>::iterator itT = tauEstimates.begin();
        for (int i = 0; i < (int) tauEstimates.size(); i++, itT++) {
            printf("Estimate for CPU Burst %d: %d\n", i + 1, *(itT));
        }
    }

};

class Arrival_Compare {
public:
    static bool earlyArrival(Process a, Process b) {
        if (a.arrivalTime == b.arrivalTime) {
            return a.processID < b.processID;
        } else {
            return a.arrivalTime < b.arrivalTime;
        }
    }
};


#endif
