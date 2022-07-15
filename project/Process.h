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
    vector<int> partialComplete; // Tracks elapsed cpu burst times (for preemptive algorithms only)
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

    int getPartialComplete() {
        /* Funciton that returns the partial completed cpu burst time */
        if (cpuBursts.size() == 0) {
            fprintf(stderr, "ERROR: All CPU Bursts completed...should not be comparing partial completeness anymore!\n");
            abort();
        }
        return partialComplete[partialComplete.size() - cpuBursts.size()];
    }

    int getOriginalCPUBurst() {
        /* Function that returns the original length of the cpu burst by adding elapsed time to remaining time */
        return this->getPartialComplete() + this->getCurrentCPUBurstTime();
    }

    int getCurrentTau() {
        /* Function that returns the current estimated cpu burst time for a process */
        if (cpuBursts.size() == 0) {
            fprintf(stderr, "ERROR: All CPU Bursts completed...cannot provide next tau estimate!\n");
            abort();
        }
        return tauEstimates[tauEstimates.size() - cpuBursts.size()];
    }

    int getAdjustedTau() {
        /* Function that returns an adjusted tau value based on the known elapsed time */
        if (cpuBursts.size() == 0) {
            fprintf(stderr, "ERROR: All CPU Bursts completed...cannot provide next adjusted tau estimate!\n");
            abort();
        }
        return this->getCurrentTau() - this->getPartialComplete();
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

    bool isCompleted() {
        if (cpuBursts.size() == 0 && ioBursts.size() == 0) {
            return true;
        } else {
            return false;
        }
    }

// ---------------MODIFIERS---------------------------------

    void addCPUBurst(int x) {
        cpuBursts.push_back(x);
        turnaround.push_back(x + cs);
        wait.push_back(0);
        preemptions.push_back(0);
        partialComplete.push_back(0);
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

    void partialElapsed(int e) {
        if (cpuBursts.size() == 0) {
            fprintf(stderr, "ERROR: Process terminated...cannot increment partial complete anymore!\n");
            abort();
        } else {
            partialComplete[partialComplete.size() - cpuBursts.size()] += e;
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
        partialElapsed(e);
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
        /* Function that increments the preemption counter for a cpu burst */
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

// ---------------------STATISTICS----------------------------------

    int totalCPU() {
        /* Function that sums all CPU Burst times for a process */
        int sum = 0;
        if (cpuBursts.size() == 0 && partialComplete.size() > 0) { // end of simulation
            for (int i = 0; i < (int) partialComplete.size(); i++) {
                sum += partialComplete[i];
            }
        } else if (cpuBursts.size() == partialComplete.size() && cpuBursts.size() > 0) { // beginning of simulation
            list<int>::iterator it = cpuBursts.begin();
            while (it != cpuBursts.end()) {
                sum += *it;
            }
        } else { // middle of simulation -- DO NOT CALL DURING EXECUTION OF PREEMPTIVE ALGORITHMS
            for (int i = 0; i < (int) (partialComplete.size() - cpuBursts.size()); i++) {
                sum += partialComplete[i];
            }
            list<int>::iterator it = cpuBursts.begin();
            while (it != cpuBursts.end()) {
                sum += *it;
            }
        }
        return sum;
    }

    double avrgCPU() {
        /* Function that returns average CPU burst time */
        double average = this->totalCPU();
        if (cpuBursts.size() == 0 && partialComplete.size() > 0) { // end of simulation
            average = (average / partialComplete.size());
        } else if (cpuBursts.size() == partialComplete.size() && cpuBursts.size() > 0) { // beginning of simulation
            average = (average / cpuBursts.size());
        } else { // middle of simulation -- DO NOT CALL DURING EXECUTION OF PREEMPTIVE ALGORITHMS
            average = (average / cpuBursts.size());
        }
        return average;
    }

    double avrgWait() {
        /* Function that returns average wait time */
        double average = 0;
        for (int i = 0; i < (int) wait.size(); i++) {
            average += wait[i];
        }
        average = (average / wait.size());
        return average;
    }

    double avrgTurnaround() {
        /* Function that returns average turnaround time */
        double average = 0;
        for (int i = 0; i < (int) turnaround.size(); i++) {
            average += turnaround[i];
        }
        average = (average / turnaround.size());
        return average;
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
