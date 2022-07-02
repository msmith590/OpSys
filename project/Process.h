#ifndef _PROCESS_
#define _PROCESS_

#include <vector>
#include <cmath>

using namespace std;

class Process {

friend class Arrival_Compare; // friendly comparator class for process objects
friend class Anticipated_Compare;

private:    
    vector<int> cpuBursts;
    vector<int> ioBursts;
    vector<int> turnaroundTime;
    vector<int> tauEstimates; // Estimated CPU Burst times
    char processID;
    int waitTime;
    int arrivalTime; // Gets set once by constructor and never modified afterwards
    int CPU_Bursts_Completed; // Used to help iterate through cpu burst vector
    int burstElapsed; // Used to keep track of a burst that was preempted - MUST BE RESET AFTER EACH BURST COMPLETION FOR PREEMPTIVE ALGORITHMS

public:
    Process() {
        processID = 'A';
        waitTime = 0;
        arrivalTime = 0;
        tauEstimates.push_back(0);
        CPU_Bursts_Completed = 0;
        burstElapsed = 0;
    }

    /* arr is the arrival time and lam is used to calculate the initial tau */
    Process(char p, int arr, double lam) {
        processID = p;
        waitTime = 0;
        arrivalTime = arr;
        tauEstimates.push_back(ceil(1.0 / lam)); // the return value of ceil is a double which gets downcasted to an integer
        CPU_Bursts_Completed = 0;
        burstElapsed = 0;
    }

// ---------------GETTERS---------------------------------------

    const vector<int>& getCPU_Bursts() const {
        return cpuBursts;
    }

    const vector<int>& getIO_Bursts() const {
        return ioBursts;
    }

    const vector<int>& getTurnaround() const {
        return turnaroundTime;
    }

    const vector<int>& getTauEstimates() const {
        return tauEstimates;
    }

    int getCurrentCPUBurstTime() {
        return cpuBursts[CPU_Bursts_Completed];
    }

    int getCurrentIOBurstTime() {
        return ioBursts[CPU_Bursts_Completed];
    }

    char getProcessID() {
        return processID;
    }

    int getWait() {
        return waitTime;
    }

    int getArrival() {
        return arrivalTime;
    }

    int getNumCPUBurstsCompleted() {
        return CPU_Bursts_Completed;
    }

    int getBurstElapsed() {
        return burstElapsed;
    }

// ---------------MODIFIERS---------------------------------

    void addCPUBurst(int x) {
        cpuBursts.push_back(x);
    }

    void addIOBurst(int x) {
        ioBursts.push_back(x);
    }

    void addTurnaround(int x) {
        turnaroundTime.push_back(x);
    }

    void addWait(int x) {
        waitTime += x;
    }

    /* This function gets called to calculate all estimated CPU burst times 
        once simulation has generated random CPU Bursts */
    void calculateTau(double alph) {
        double tau;
        for (int i = 1; i < (int) cpuBursts.size(); i++) {
            tau = ceil((alph * cpuBursts[i - 1]) + ((1.0 - alph) * tauEstimates[i - 1]));
            tauEstimates.push_back(tau);
        }
    }

    void cpuBurstCompleted() {
        CPU_Bursts_Completed++;
    }

    void resetBurstElapsed() {
        burstElapsed = 0;
    }

    void addBurstElapsed(int x) {
        burstElapsed += x;
    }

    void decrementCPUBurst(int time) {
        cpuBursts[CPU_Bursts_Completed] -= time;
    }

    void decrementIOBurst(int time) {
        ioBursts[CPU_Bursts_Completed] -= time;
    }

// ---------------------OUTPUT---------------------------------------

    void printBursts() {
        if ((int) cpuBursts.size() == 1) {
            printf("Process %c (arrival time %d ms) %ld CPU burst (tau %dms)\n", processID, arrivalTime, cpuBursts.size(), tauEstimates[0]);
        } else {
            printf("Process %c (arrival time %d ms) %ld CPU bursts (tau %dms)\n", processID, arrivalTime, cpuBursts.size(), tauEstimates[0]);
        }
        for (int i = 0; i < (int) ioBursts.size(); i++) {
            printf("--> CPU burst %d ms --> I/O burst %d ms\n", cpuBursts[i], ioBursts[i]);
        }
        printf("--> CPU burst %d ms\n", cpuBursts[cpuBursts.size() - 1]);
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

class Anticipated_Compare {
public:
    bool operator() (Process const a, Process const b) {
        /* Same amount of remaining time left, therefore compare by process ID */
        if ((a.tauEstimates[a.CPU_Bursts_Completed] - a.burstElapsed) == (b.tauEstimates[b.CPU_Bursts_Completed] - b.burstElapsed)) {
            return a.processID > b.processID;
        } else {
            return (a.tauEstimates[a.CPU_Bursts_Completed] - a.burstElapsed) > (b.tauEstimates[b.CPU_Bursts_Completed] - b.burstElapsed);
        }
    }

};


#endif
