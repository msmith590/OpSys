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

void generateProcesses(list<Process>& processes, long int s, int numP, int tail, double lam, double alph, int t_cs, bool print) {
    srand48(s); // Re-seeds the pseudo-random number generator
    vector<char> pid{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}; // vector containing usable process IDs
    vector<Process> p;
    int arrival = 0;
    int numBursts = 0;
    
    for (int i = 0; i < numP; i++) {
        arrival = floor(next_exp(tail, lam));
        p.push_back(Process(pid[i], arrival, lam, t_cs));
        numBursts = ceil(drand48() * 100);
        for (int j = 0; j < numBursts - 1; j++) {
            p[i].addCPUBurst(ceil(next_exp(tail, lam)));
            p[i].addIOBurst(ceil(next_exp(tail, lam)) * 10);
        }
        p[i].addCPUBurst(ceil(next_exp(tail, lam))); // Adds the last CPU Burst time without a following IO Burst
        p[i].calculateTau(alph);
        if (print) {
            p[i].printBursts();
        }
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

int nextEvent(list<Process>& incoming, vector<Process>& cpu, list<Process>& readyQ, list<Process>& io, int timer, bool switching, bool specific) {
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
    } else { // if CPU is empty, check ready queue for process to add and begin processing
        if (!readyQ.empty() && !switching) { 
            it = readyQ.begin();
            next = 0;
            rc = 1;
        }
    }
    if (!io.empty()) {
        it = io.begin();
        if (next > it->getCurrentIOBurstTime() || (next == it->getCurrentIOBurstTime() && rc == 1)) {
            next = it->getCurrentIOBurstTime();
            rc = 2;
        }
    }
    if (!incoming.empty()) {
        it = incoming.begin();
        if (next > (it->getArrival() - timer) || ((next == it->getArrival() - timer) && rc == 1)) {
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
        printf("Next event occurs in READY QUEUE\n");
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

void advanceWait(vector<Process>& cpu, list<Process>& readyQ, list<Process>& io, int elapse) {
   /* Function that advances time in the ready queue and io subsystem */
    for (Process& p : cpu) {
        p.cpuElapsed(elapse);
    }
    for (Process& p : readyQ) {
        p.addWait(elapse);
    }
    for (Process& p : io) {
        p.ioElapsed(elapse);
    }
}

bool addToIO(list<Process>& io, Process p) {
    /* Function that adds to and maintains a sorted list of Processes completing IO */
    if (p.numIOBursts() == 0) {
        fprintf(stderr, "Cannot add a terminated process to io subsystem!\n");
        abort();
    }
    if (io.empty()) {
        io.push_back(p);
        return true;
    } else {
        list<Process>::iterator it = io.begin();
        while (it != io.end()) {
            if (p.getCurrentIOBurstTime() < it->getCurrentIOBurstTime()) {
                io.insert(it, p);
                return true;
            } else if (p.getCurrentIOBurstTime() == it->getCurrentIOBurstTime()) {
                if (p.getProcessID() < it->getProcessID()) {
                    io.insert(it, p);
                    return true;
                }
            }
            it++;
        }
        io.insert(it, p);
        return true;
    }
    return false;
}

bool priorityAdd(list<Process>& readyQ, Process p) {
   /* Function that maintains a priority ordering in the ready queue based on tau estimates */
    if (p.numCPUBursts() == 0) {
        fprintf(stderr, "ERROR: Cannot add a terminated process back to the readyQ!\n");
        abort();
    }
    if (readyQ.empty()) {
        readyQ.push_back(p);
        return true;
    } else {
        list<Process>::iterator it = readyQ.begin();
        while (it != readyQ.end()) {
            if (p.getAdjustedTau() < it->getAdjustedTau()) {
                readyQ.insert(it, p);
                return true;
            } else if (p.getAdjustedTau() == it->getAdjustedTau()) {
                if (p.getProcessID() < it->getProcessID()) {
                    readyQ.insert(it, p);
                    return true;
                }
            }
            it++;
        }
        readyQ.insert(it, p);
        return true;
    }
    return false;
}

bool preempt(vector<Process>& cpu, Process p, bool switching, Process s) {
    if (cpu.empty()) {
        if (switching && (s.numCPUBursts() != s.numIOBursts())) {
            /* Another process s was previously selected to go into CPU...need to check if we swap this with our newly arriving process p */
            if (s.getAdjustedTau() <= p.getAdjustedTau()) {
                return false;
            } else {
                return true;
            }
        } else {
            return false;
        }
    } else if (cpu[0].getAdjustedTau() <= p.getAdjustedTau()) {
        return false;
    } else {
        return true;
    }
    return false;
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
    list<Process>::iterator it;
    vector<Process> terminated;
    Process p;
    int timer = 0;
    int elapse = 0;
    int temp = 0;
    bool switching = false; // used to perform logic in the case an event/events happens during context switch time

    printf("\ntime %dms: Simulator started for FCFS ", timer);
    printQ(readyQ);
    while (!incoming.empty() || !cpu.empty() || !readyQ.empty() || !io.empty())
    {
        if (switching) {
            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) + timer >= temp) {
                switching = false;
                advanceWait(cpu, readyQ, io, (temp - timer));
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                timer = temp;
                if (!(p.numCPUBursts() == 0) || !(p.numIOBursts() == 0)) {
                    if (p.numCPUBursts() == p.numIOBursts()) { // Process needs to complete io
                        addToIO(io, p);
                    } else {
                        cpu.push_back(p);
                        readyQ.pop_front();
                        printf("time %dms: Process %c started using the CPU for %dms burst ", timer, cpu[0].getProcessID(), cpu[0].getCurrentCPUBurstTime());
                        printQ(readyQ);
                        if (cpu.size() > 1) {
                            fprintf(stderr, "ERROR: Too many processes in the CPU!\n");
                            abort();
                        }
                    }
                }
            }
        }

        int next = nextEvent(incoming, cpu, readyQ, io, timer, switching, false);
        if (next == -1) {
            fprintf(stderr, "ERROR: while loop did not terminate correctly...\n");
            abort();
        } else if (next == 0) { // CPU event ==> process completing a cpu burst
            if (cpu.size() != 1) {
                fprintf(stderr, "ERROR: CPU has too many processes...\n");
                abort();
            }
            if (switching) {
                fprintf(stderr, "ERROR: Skipping an IO/Arrival event!\n");
                abort();
            }
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
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
            } else {
                cpu[0].completedCPU();
                printf("time %dms: Process %c terminated ", timer, cpu[0].getProcessID());
                printQ(readyQ);
                terminated.push_back(cpu[0]);
            }
            p = cpu[0]; // Temporarily holds process so that other time manipulations can occur
            cpu.clear();
            advanceWait(cpu, readyQ, io, elapse); // empty cpu
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            switching = true;

            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) < (t_cs / 2)) {
                // Corner case: check if another event happens during context switching out
                // When this is the case, do not add process p to io (if not terminated) yet
                // Do no increment timer by context switch out...let while loop execute again
                temp = timer + (t_cs / 2); // used to hold position of timer after context switch out for later comparisons
            } else {
                switching = false;
                elapse = t_cs / 2;
                timer += elapse; // time added for switching process out of cpu
                advanceWait(cpu, readyQ, io, elapse); // empty cpu
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                if (p.numIOBursts()) {
                    addToIO(io, p);
                }
            }
        } else if (next == 1) { // Ready queue event ==> switch process into cpu
            if (switching) {
                fprintf(stderr, "ERROR: Skipping an IO/Arrival event!\n");
                abort();
            }
            if (!cpu.empty()) {
                fprintf(stderr, "ERROR: Process stuck in the CPU!\n");
                abort();
            }
            it = readyQ.begin();
            p = *it;
            switching = true;

            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) < (t_cs / 2)) {
                temp = timer + (t_cs / 2);
            } else {
                readyQ.pop_front();
                switching = false;
                elapse = t_cs / 2;
                advanceWait(cpu, readyQ, io, elapse);
                timer += elapse;
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                cpu.push_back(p);
                printf("time %dms: Process %c started using the CPU for %dms burst ", timer, cpu[0].getProcessID(), cpu[0].getCurrentCPUBurstTime());
                printQ(readyQ);
            }
        } else if (next == 2) { // IO event ==> process completes IO Burst and is added back to ready queue
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
            timer += elapse;
            advanceWait(cpu, readyQ, io, elapse);
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            it = io.begin();
            it->completedIO();
            readyQ.push_back(*it);
            printf("time %dms: Process %c completed I/O; added to ready queue ", timer, it->getProcessID());
            printQ(readyQ);
            io.pop_front();
        } else if (next == 3) { // Initial arrival event ==> add process to ready queue
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
            timer += elapse;
            advanceWait(cpu, readyQ, io, elapse);
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            it = incoming.begin();
            readyQ.push_back(*it);
            printf("time %dms: Process %c arrived; added to ready queue ", timer, it->getProcessID());
            printQ(readyQ);
            incoming.pop_front();
        }
    }
    printf("time %dms: Simulator ended for FCFS ", timer);
    printQ(readyQ);
}

void SJF(list<Process>& incoming, int t_cs) {
    vector<Process> cpu;
    list<Process> readyQ;
    list<Process> io;
    list<Process>::iterator it;
    vector<Process> terminated;
    Process p;
    int timer = 0;
    int elapse = 0;
    int temp = 0;
    bool switching = false; // used to perform logic in the case an event/events happens during context switch time

    printf("\ntime %dms: Simulator started for SJF ", timer);
    printQ(readyQ);
    while (!incoming.empty() || !cpu.empty() || !readyQ.empty() || !io.empty())
    {
        if (switching) {
            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) + timer >= temp) {
                switching = false;
                advanceWait(cpu, readyQ, io, (temp - timer));
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                timer = temp;
                if (!(p.numCPUBursts() == 0) || !(p.numIOBursts() == 0)) {
                    if (p.numCPUBursts() == p.numIOBursts()) { // Process needs to complete io
                        addToIO(io, p);
                    } else {
                        cpu.push_back(p);
                        it = readyQ.begin();
                        if (it->getProcessID() == p.getProcessID()) {
                            readyQ.pop_front();
                        }
                        printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst ", timer, cpu[0].getProcessID(), cpu[0].getCurrentTau(), cpu[0].getCurrentCPUBurstTime());
                        printQ(readyQ);
                        if (cpu.size() > 1) {
                            fprintf(stderr, "ERROR: Too many processes in the CPU!\n");
                            abort();
                        }
                    }
                }
            }
        }

        int next = nextEvent(incoming, cpu, readyQ, io, timer, switching, false);
        if (next == -1) {
            fprintf(stderr, "ERROR: while loop did not terminate correctly...\n");
            abort();
        } else if (next == 0) { // CPU event ==> process completing a cpu burst
            if (cpu.size() != 1) {
                fprintf(stderr, "ERROR: CPU has too many processes...\n");
                abort();
            }
            if (switching) {
                fprintf(stderr, "ERROR: Skipping an IO/Arrival event!\n");
                abort();
            }
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
            timer += elapse;
            if ((cpu[0].cpuElapsed(elapse) == 0) && cpu[0].numIOBursts()) {
                int prevTau = cpu[0].getCurrentTau();
                cpu[0].completedCPU();
                if (cpu[0].numIOBursts() > 1) {
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d bursts to go ", timer, cpu[0].getProcessID(), prevTau, cpu[0].numCPUBursts());
                    printQ(readyQ);
                } else {
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d burst to go ", timer, cpu[0].getProcessID(), prevTau, cpu[0].numCPUBursts());
                    printQ(readyQ);
                }
                printf("time %dms: Recalculated tau for process %c: old tau %dms; new tau %dms ", timer, cpu[0].getProcessID(), prevTau, cpu[0].getCurrentTau());
                printQ(readyQ);
                printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", timer, cpu[0].getProcessID(), timer + (t_cs / 2) + cpu[0].getCurrentIOBurstTime());
                printQ(readyQ);
            } else {
                cpu[0].completedCPU();
                printf("time %dms: Process %c terminated ", timer, cpu[0].getProcessID());
                printQ(readyQ);
                terminated.push_back(cpu[0]);
            }
            p = cpu[0]; // Temporarily holds process so that other time manipulations can occur
            cpu.clear();
            advanceWait(cpu, readyQ, io, elapse); // empty cpu
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            switching = true;

            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) < (t_cs / 2)) {
                // Corner case: check if another event happens during context switching in
                // When this is the case, do not add process p to io (if not terminated) yet
                // Do no increment timer by context switch out...let while loop execute again
                temp = timer + (t_cs / 2); // used to hold position of timer after context switch out for later comparisons
            } else {
                switching = false;
                elapse = t_cs / 2;
                timer += elapse; // time added for switching process out of cpu
                advanceWait(cpu, readyQ, io, elapse); // empty cpu
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                if (p.numIOBursts()) {
                    addToIO(io, p);
                }
            }
        } else if (next == 1) { // Ready queue event ==> switch process into cpu
            if (switching) {
                fprintf(stderr, "ERROR: Skipping an IO/Arrival event!\n");
                abort();
            }
            if (!cpu.empty()) {
                fprintf(stderr, "ERROR: Process stuck in the CPU!\n");
                abort();
            }
            it = readyQ.begin();
            p = *it;
            switching = true;

            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) < (t_cs / 2)) {
                if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) != 0) {
                    readyQ.pop_front();
                }
                temp = timer + (t_cs / 2);
            } else {
                readyQ.pop_front();
                switching = false;
                elapse = t_cs / 2; // context switch in
                advanceWait(cpu, readyQ, io, elapse); // empty cpu
                timer += elapse;
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                cpu.push_back(p);
                printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst ", timer, cpu[0].getProcessID(), cpu[0].getCurrentTau(), cpu[0].getCurrentCPUBurstTime());
                printQ(readyQ);
            }
        } else if (next == 2) { // IO event ==> process completes IO Burst and is added back to ready queue
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
            timer += elapse;
            advanceWait(cpu, readyQ, io, elapse);
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            it = io.begin();
            it->completedIO();
            priorityAdd(readyQ, *it);
            printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ", timer, it->getProcessID(), it->getCurrentTau());
            printQ(readyQ);
            io.pop_front();
        } else if (next == 3) { // Initial arrival event ==> add process to ready queue
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
            timer += elapse;
            advanceWait(cpu, readyQ, io, elapse);
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            it = incoming.begin();
            priorityAdd(readyQ, *it);
            printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ", timer, it->getProcessID(), it->getCurrentTau());
            printQ(readyQ);
            incoming.pop_front();
        }
    }
    printf("time %dms: Simulator ended for SJF ", timer);
    printQ(readyQ);
}

void SRT(list<Process>& incoming, int t_cs) {
    vector<Process> cpu;
    list<Process> readyQ;
    list<Process> io;
    list<Process>::iterator it;
    vector<Process> terminated;
    Process p;
    int timer = 0;
    int elapse = 0;
    int temp = 0;
    bool switching = false; // used to perform logic in the case an event/events happens during context switch time
    bool preempting = false; // when this is true, we are going to be removing a running process in the CPU
    bool multiple = false; // used to handle corner case where an io burst completion or initial arrival event occur duing preemption

    printf("\ntime %dms: Simulator started for SRT ", timer);
    printQ(readyQ);
    while (!incoming.empty() || !cpu.empty() || !readyQ.empty() || !io.empty()) 
    {

        if (switching) {
            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) + timer >= temp) {
                switching = false;
                advanceWait(cpu, readyQ, io, (temp - timer));
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                timer = temp;
                if (!(p.numCPUBursts() == 0) || !(p.numIOBursts() == 0)) {
                    if (multiple) {
                        priorityAdd(readyQ, p);
                        multiple = false;
                    } else if (p.numCPUBursts() == p.numIOBursts()) { // Process needs to complete io
                        addToIO(io, p);
                        if (preempting) {
                            fprintf(stderr, "Cannot preempt an already completed process!\n");
                            abort();
                        }
                    } else {
                        cpu.push_back(p);
                        it = readyQ.begin();
                        if (it->getProcessID() == p.getProcessID()) {
                            readyQ.pop_front();
                        }
                        if (cpu[0].getPartialComplete() == 0) {
                            printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst ", timer, cpu[0].getProcessID(), cpu[0].getCurrentTau(), cpu[0].getCurrentCPUBurstTime());
                        } else {
                            printf("time %dms: Process %c (tau %dms) started using the CPU for remaining %dms of %dms burst ", timer, cpu[0].getProcessID(), 
                            cpu[0].getCurrentTau(), cpu[0].getCurrentCPUBurstTime(), cpu[0].getOriginalCPUBurst());
                        }
                        printQ(readyQ);
                        if (cpu.size() > 1) {
                            fprintf(stderr, "ERROR: Too many processes in the CPU!\n");
                            abort();
                        }

                        if (preempting) {
                            /* The following code checks to see if we should preempt a process that just got switched into CPU and has not yet begun processing */
                            printf("time %dms: Process %c (tau %dms) will preempt %c ", timer, it->getProcessID(), it->getCurrentTau(), p.getProcessID());
                            printQ(readyQ);
                            switching = true;
                            p = cpu[0];
                            p.addPreemption();
                            cpu.clear();
                            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) < (t_cs / 2)) {
                                /* The following checks to see if there are any io/initial arrival events that may occur within context switch out*/
                                temp = timer + (t_cs / 2); // used to hold position of timer after context switch out for later comparisons
                                multiple = true;
                            } else {
                                switching = false;
                                preempting = false;
                                elapse = t_cs / 2;
                                timer += elapse; // time added for switching process out of cpu
                                advanceWait(cpu, readyQ, io, elapse); // empty cpu
                                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                                priorityAdd(readyQ, p);
                            }
                        }
                    }
                }
            }
        }

        int next = nextEvent(incoming, cpu, readyQ, io, timer, switching, false);
        if (next == -1) {
            fprintf(stderr, "ERROR: while loop did not terminate correctly...\n");
            abort();
        } else if (next == 0) { // CPU event ==> process completing a cpu burst
            if (cpu.size() != 1) {
                fprintf(stderr, "ERROR: CPU has too many processes...\n");
                abort();
            }
            if (switching) {
                fprintf(stderr, "ERROR: Skipping an IO/Arrival event!\n");
                abort();
            }
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
            timer += elapse;
            if ((cpu[0].cpuElapsed(elapse) == 0) && cpu[0].numIOBursts()) {
                int prevTau = cpu[0].getCurrentTau();
                cpu[0].completedCPU();
                if (cpu[0].numIOBursts() > 1) {
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d bursts to go ", timer, cpu[0].getProcessID(), prevTau, cpu[0].numCPUBursts());
                    printQ(readyQ);
                } else {
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d burst to go ", timer, cpu[0].getProcessID(), prevTau, cpu[0].numCPUBursts());
                    printQ(readyQ);
                }
                printf("time %dms: Recalculated tau for process %c: old tau %dms; new tau %dms ", timer, cpu[0].getProcessID(), prevTau, cpu[0].getCurrentTau());
                printQ(readyQ);
                printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", timer, cpu[0].getProcessID(), timer + (t_cs / 2) + cpu[0].getCurrentIOBurstTime());
                printQ(readyQ);
            } else {
                cpu[0].completedCPU();
                printf("time %dms: Process %c terminated ", timer, cpu[0].getProcessID());
                printQ(readyQ);
                terminated.push_back(cpu[0]);
            }
            p = cpu[0]; // Temporarily holds process so that other time manipulations can occur
            cpu.clear();
            advanceWait(cpu, readyQ, io, elapse); // empty cpu
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            switching = true;

            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) < (t_cs / 2)) {
                // Corner case: check if another event happens during context switching in
                // When this is the case, do not add process p to io (if not terminated) yet
                // Do no increment timer by context switch out...let while loop execute again
                temp = timer + (t_cs / 2); // used to hold position of timer after context switch out for later comparisons
            } else {
                switching = false;
                elapse = t_cs / 2;
                timer += elapse; // time added for switching process out of cpu
                advanceWait(cpu, readyQ, io, elapse); // empty cpu
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                if (p.numIOBursts()) {
                    addToIO(io, p);
                }
            }
        } else if (next == 1) { // Ready queue event ==> switch process into cpu
            if (switching) {
                fprintf(stderr, "ERROR: Skipping an IO/Arrival event!\n");
                abort();
            }
            if (!cpu.empty()) {
                fprintf(stderr, "ERROR: Process stuck in the CPU!\n");
                abort();
            }
            it = readyQ.begin();
            p = *it;
            switching = true;

            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) < (t_cs / 2)) {
                if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) != 0) {
                    /* Preserves a preferred output style of showing all processes on readyQ before a same-time procedure occurs */
                    readyQ.pop_front();
                }
                temp = timer + (t_cs / 2);
            } else {
                readyQ.pop_front();
                switching = false;
                elapse = t_cs / 2; // context switch in
                advanceWait(cpu, readyQ, io, elapse); // empty cpu
                timer += elapse;
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                cpu.push_back(p);
                if (cpu[0].getPartialComplete() == 0) {
                    printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst ", timer, cpu[0].getProcessID(), cpu[0].getCurrentTau(), cpu[0].getCurrentCPUBurstTime());
                } else {
                    printf("time %dms: Process %c (tau %dms) started using the CPU for remaining %dms of %dms burst ", timer, cpu[0].getProcessID(), 
                    cpu[0].getCurrentTau(), cpu[0].getCurrentCPUBurstTime(), cpu[0].getOriginalCPUBurst());
                }
                printQ(readyQ);
            }
        } else if (next == 2) { // IO event ==> process completes IO Burst and is added back to ready queue
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
            timer += elapse;
            advanceWait(cpu, readyQ, io, elapse);
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            it = io.begin();
            it->completedIO();
            priorityAdd(readyQ, *it);
            if (preempt(cpu, *it, switching, p)) {
                if (switching) { /* At this point, we know: cpu is empty and there is a process already pulled from ready queue to get switched in */
                    printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ", timer, it->getProcessID(), it->getCurrentTau());
                    printQ(readyQ);
                } else { /* CPU has an actively running process that will get preempted by arriving process */
                    printf("time %dms: Process %c (tau %dms) completed I/O; preempting %c ", timer, it->getProcessID(), it->getCurrentTau(), cpu[0].getProcessID());
                    printQ(readyQ);
                }
                preempting = true;
            } else {
                printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue ", timer, it->getProcessID(), it->getCurrentTau());
                printQ(readyQ); 
            }
            io.pop_front();

        } else if (next == 3) { // Initial arrival event ==> add process to ready queue
            elapse = nextEvent(incoming, cpu, readyQ, io, timer, switching, true);
            timer += elapse;
            advanceWait(cpu, readyQ, io, elapse);
            // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
            it = incoming.begin();
            priorityAdd(readyQ, *it);
            if (preempt(cpu, *it, switching, p)) {
                if (switching) { /* At this point, we know: cpu is empty and there is a process already pulled from ready queue to get switched in */
                    printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ", timer, it->getProcessID(), it->getCurrentTau());
                    printQ(readyQ);
                } else { /* CPU has an actively running process that will get preempted by arriving process */
                    printf("time %dms: Process %c (tau %dms) arrived; preempting %c ", timer, it->getProcessID(), it->getCurrentTau(), cpu[0].getProcessID());
                    printQ(readyQ);
                }
                preempting = true;
            } else {
                printf("time %dms: Process %c (tau %dms) arrived; added to ready queue ", timer, it->getProcessID(), it->getCurrentTau());
                printQ(readyQ); 
            }
            incoming.pop_front();
        }
        
        if (preempting && !switching) {
            /* In this instance cpu is not in an active context switch */
            switching = true;
            p = cpu[0];
            p.addPreemption();
            cpu.clear();
            if (nextEvent(incoming, cpu, readyQ, io, timer, switching, true) < (t_cs / 2)) {
                temp = timer + (t_cs / 2); // used to hold position of timer after context switch out for later comparisons
                multiple = true;
            } else {
                switching = false;
                preempting = false;
                elapse = t_cs / 2;
                timer += elapse; // time added for switching process out of cpu
                advanceWait(cpu, readyQ, io, elapse); // empty cpu
                // ----------------------TIME ELAPSED FOR ALL OTHER PROCESSES----------------------------------
                priorityAdd(readyQ, p);
            }
        }
    }
    printf("time %dms: Simulator ended for SRT ", timer);
    printQ(readyQ);
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
    generateProcesses(processes, seed, numProc, upperBound, lambda, alpha, time_cs, true);
    FCFS(processes, time_cs);
    processes.clear();

    generateProcesses(processes, seed, numProc, upperBound, lambda, alpha, time_cs, false);
    SJF(processes, time_cs);
    processes.clear();

    generateProcesses(processes, seed, numProc, upperBound, lambda, alpha, time_cs, false);
    SRT(processes, time_cs);
    processes.clear();


    return EXIT_SUCCESS;
}