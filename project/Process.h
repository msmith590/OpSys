#ifndef _PROCESS_
#define _PROCESS_

#include <list>
#include <cmath>
#include <vector>
#include <stdio.h>

using namespace std;

class Process {

friend class Arrival_Compare; // friendly comparator class for process objects
// friend class Anticipated_Compare;

private:    
    list<int> cpuBursts;
    list<int> ioBursts;
    vector<int> tauEstimates; // Estimated CPU Burst times
    vector<int> wait;
    vector<int> turnaround;
    vector<int> preemptions;
    char processID;
    int arrivalTime; // Gets set once by constructor and never modified afterwards
    int cs; // context switch time

public:
    Process() {
        processID = 'A';
        arrivalTime = 0;
        tauEstimates.push_back(0);
        cs = 0;
    }

    /* arr is the arrival time and lam is used to calculate the initial tau */
    Process(char p, int arr, double lam, int cs_time) {
        processID = p;
        arrivalTime = arr;
        tauEstimates.push_back(ceil(1.0 / lam)); // the return value of ceil is a double which gets downcasted to an integer
        cs = cs_time;
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

    int getCurrentTau() {
        if (cpuBursts.size() == 0) {
            fprintf(stderr, "ERROR: All CPU Bursts completed...cannot provide next tau estimate!\n");
            abort();
        }
        return tauEstimates[tauEstimates.size() - cpuBursts.size()];
    }

    int getTau(int x) {
        if (x < 0 || x >= (int) tauEstimates.size()) {
            fprintf(stderr, "ERROR: Index out of bounds for tau estimates!\n");
            abort();
        } else {
            return tauEstimates[x];
        }
    }

    int numCPUBursts() {
        return cpuBursts.size();
    }

    int numIOBursts() {
        return ioBursts.size();
    }

    int getTurnaround(int x) {
        return turnaround[x];
    }

    int getWait(int x) {
        return wait[x];
    }

    int getPreemptions(int x) {
        return preemptions[x];
    }

    int getCS() {
        return cs;
    }

// ---------------MODIFIERS---------------------------------

    void addCPUBurst(int x) {
        cpuBursts.push_back(x);
        turnaround.push_back(x + cs);
        wait.push_back(0);
        preemptions.push_back(0);
    }

    void addIOBurst(int x) {
        ioBursts.push_back(x);
    }

    void completedCPU() {
        if (this->getCurrentCPUBurstTime() == 0) {
            cpuBursts.pop_front();
        } else {
            fprintf(stderr, "ERROR: CPU Burst has only been partially completed -- cannot complete!\n");
        }
    }

    void completedIO() {
        if (this->getCurrentIOBurstTime() == 0) {
            ioBursts.pop_front();
        } else {
            fprintf(stderr, "ERROR: IO Burst has only been partially completed -- cannot complete!\n");
        }
    }

    /* This function gets called to calculate all estimated CPU burst times 
        once simulation has generated random CPU Bursts */
    void calculateTau(double alph) {
        double tau;
        list<int>::iterator itC = cpuBursts.begin();
        for (int i = 1; i < (int) cpuBursts.size(); i++, itC++) {
            tau = ceil((alph * *(itC)) + ((1.0 - alph) * tauEstimates[i - 1]));
            tauEstimates.push_back(tau);
        }
    }

    int cpuElapsed(int e) {
        /* Function that elapses the time for the current cpu burst and returns remaining time */
        list<int>::iterator it = cpuBursts.begin();
        *it = *it - e;
        if (*it < 0) {
            fprintf(stderr, "ERROR: Skipped over CPU burst completion!\n");
            abort();
        }
        return *it;
    }

    int ioElapsed(int e) {
        /* Function that elapses the time for the current io burst and returns remaining time */
        list<int>::iterator it = ioBursts.begin();
        *it = *it - e;
        if (*it < 0) {
            fprintf(stderr, "ERROR: Skipped over I/O burst completion!\n");
            abort();
        }
        return *it;
    }

    void addWait(int e) {
        /* Function that increments the wait time of single cpu burst in the ready queue and adjusts overall turnaround time */
        wait[wait.size() - cpuBursts.size()] += e;
        turnaround[turnaround.size() - cpuBursts.size()] += e;
    }

    void addPreemption() {
        preemptions[preemptions.size() - cpuBursts.size()] += 1;
        turnaround[turnaround.size() - cpuBursts.size()] += cs;
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
        for (int i = 0; i < (int) tauEstimates.size(); i++) {
            printf("Estimate for CPU Burst %d: %d\n", i + 1, tauEstimates[i]);
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
