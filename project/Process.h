#ifndef _PROCESS_
#define _PROCESS_

#include <vector>
#include <cmath>
#include <stdio.h>

using namespace std;

class Process {

friend class Arrival_Compare; // friendly comparator class for process objects
// friend class Anticipated_Compare;

private:    
    vector<int> cpuBursts;
    vector<int> ioBursts;
    vector<int> tauEstimates; // Estimated CPU Burst times
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

    const vector<int>& getCPU_Bursts() const {
        return cpuBursts;
    }

    const vector<int>& getIO_Bursts() const {
        return ioBursts;
    }

    const vector<int>& getTauEstimates() const {
        return tauEstimates;
    }

    char getProcessID() {
        return processID;
    }

    int getArrival() {
        return arrivalTime;
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
        for (int i = 1; i < (int) cpuBursts.size(); i++) {
            tau = ceil((alph * cpuBursts[i - 1]) + ((1.0 - alph) * tauEstimates[i - 1]));
            tauEstimates.push_back(tau);
        }
    }

// ---------------------OUTPUT---------------------------------------

    void printBursts() {
        if ((int) cpuBursts.size() == 1) {
            printf("Process %c: arrival time %dms; tau %dms; %ld CPU burst:\n", processID, arrivalTime, tauEstimates[0], cpuBursts.size());
        } else {
            printf("Process %c: arrival time %dms; tau %dms; %ld CPU bursts:\n", processID, arrivalTime, tauEstimates[0], cpuBursts.size());
        }
        for (int i = 0; i < (int) ioBursts.size(); i++) {
            printf("--> CPU burst %dms --> I/O burst %dms\n", cpuBursts[i], ioBursts[i]);
        }
        printf("--> CPU burst %dms\n", cpuBursts[cpuBursts.size() - 1]);
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
