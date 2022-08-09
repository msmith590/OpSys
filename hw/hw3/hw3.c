/* hw3.c
 *  Author: Martin Smith
 *  CSCI 4210
 *  07/28/2022
 *  Goldschmidt/Plum
 *
 *  Submitty score: 50/50
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

/*  Diagram showing move order and possible moves
 *   from starting point S
 *
 *   +---+---+---+---+---+
 *   |   | 1 |   | 8 |   |
 *   +---+---+---+---+---+
 *   | 2 |   |   |   | 7 |
 *   +---+---+---+---+---+
 *   |   |   | S |   |   |
 *   +---+---+---+---+---+
 *   | 3 |   |   |   | 6 |
 *   +---+---+---+---+---+
 *   |   | 4 |   | 5 |   |
 *   +---+---+---+---+---+
*/

extern int next_thread_id;
extern int max_squares;
extern int total_tours;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief This struct stores the required argument information for child threads that are
 * created when multiple moves exist on a Knight's Tour
 * 
 */
typedef struct
{
    int** board;
    int move;
    int dimensions[2]; // dimensions[0] = m; dimensions[1] = n;
    int position[2]; // position[0] = r; position[1] = c;
    int threadID;
} boardData_t;

/**
 * @brief This struct stores two arrays that, when interpreted together, represent possible
 * future positions. The function pathfinder(..) inserts the integer values for both arrays
 * when determining future paths.
 * 
 */
typedef struct
{
    int row[8];
    int col[8];
} nextMoves_t;

void* diverge(void* arg);

/* Function that checks to see if a string can be fully converted into an integer */
int isInteger(char* string) {
    int i = 0, valid = 1; // valid is set to true initially (represented by a value of 1)
    while (*(string + i) != '\0') {
        if (isdigit(*(string + i)) == 0) {
            valid = 0;
            break;
        }
        i++;
    }
    return valid;
}

/* Debugging function that prints out board contents */
void printBoard(int** board, int m, int n) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d  ", board[i][j]);
        }
        printf("\n");
    }
}

/* Memory cleanup function for boardData_t types */
void freeBoard(boardData_t* arg) {
    for (int i = 0; i < arg->dimensions[0]; i++) {
        free(arg->board[i]);
    }
    free(arg->board);
    free(arg);
}

/**
 * The pathfinder(..) function checks a board for valid moves from a provided
 * starting position. While checking for possibilities, all possible moves
 * are stored in a predefined nextMoves_t struct called possiblePos, which
 * is used in the runTour(..) function to move Sonny
*/
int pathfinder(int m, int n, int r, int c, int** board, nextMoves_t* possiblePos)
{ // Assumes that we have a valid board and r, c are valid
    int moves = 0;
    for (int i = 0; i < 8; i++) {
        possiblePos->row[i] = -1;
        possiblePos->col[i] = -1;
    }

    if ((r - 2) >= 0 && (c - 1) >= 0)
    { /* Move 1 (see diagram above) */
        if (board[r - 2][c - 1] == 0)
        {
            moves++;
            possiblePos->row[0] = r - 2;
            possiblePos->col[0] = c - 1;
        }
    }
    if ((r - 1) >= 0 && (c - 2) >= 0)
    { /* Move 2 (see diagram above) */
        if (board[r - 1][c - 2] == 0)
        {
            moves++;
            possiblePos->row[1] = r - 1;
            possiblePos->col[1] = c - 2;
        }
    }
    if ((r + 1) < m && (c - 2) >= 0)
    { /* Move 3 (see diagram above) */
        if (board[r + 1][c - 2] == 0)
        {
            moves++;
            possiblePos->row[2] = r + 1;
            possiblePos->col[2] = c - 2;
        }
    }
    if ((r + 2) < m && (c - 1) >= 0)
    { /* Move 4 (see diagram above) */
        if (board[r + 2][c - 1] == 0)
        {
            moves++;
            possiblePos->row[3] = r + 2;
            possiblePos->col[3] = c - 1;
        }
    }
    if ((r + 2) < m && (c + 1) < n)
    { /* Move 5 (see diagram above) */
        if (board[r + 2][c + 1] == 0)
        {
            moves++;
            possiblePos->row[4] = r + 2;
            possiblePos->col[4] = c + 1;
        }
    }
    if ((r + 1) < m && (c + 2) < n)
    { /* Move 6 (see diagram above) */
        if (board[r + 1][c + 2] == 0)
        {
            moves++;
            possiblePos->row[5] = r + 1;
            possiblePos->col[5] = c + 2;
        }
    }
    if ((r - 1) >= 0 && (c + 2) < n)
    { /* Move 7 (see diagram above) */
        if (board[r - 1][c + 2] == 0)
        {
            moves++;
            possiblePos->row[6] = r - 1;
            possiblePos->col[6] = c + 2;
        }
    }
    if ((r - 2) >= 0 && (c + 1) < n)
    { /* Move 8 (see diagram above) */
        if (board[r - 2][c + 1] == 0)
        {
            moves++;
            possiblePos->row[7] = r - 2;
            possiblePos->col[7] = c + 1;
        }
    }

    return moves;
}

/* Function that copies board data from src over to dest */
void boardDataCopy(boardData_t* src, boardData_t* dest) {
    int** newBoard = calloc(src->dimensions[0], sizeof(int*));
    for (int i = 0; i < src->dimensions[0]; i++) {
        newBoard[i] = calloc(src->dimensions[1], sizeof(int));
        for (int j = 0; j < src->dimensions[1]; j++) {
            newBoard[i][j] = src->board[i][j];
        }
    }
    dest->board = newBoard;
    dest->dimensions[0] = src->dimensions[0];
    dest->dimensions[1] = src->dimensions[1];
    dest->move = src->move;
    dest->position[0] = src->position[0];
    dest->position[1] = src->position[1];
    dest->threadID = src->threadID;
}

/* Function that moves Sonny to position (r, c) and updates move counter */
void updateBoard(boardData_t* data, int r, int c) {
    data->move++;
    data->position[0] = r;
    data->position[1] = c;
    if (data->board[r][c] == 0) {
        data->board[r][c] = data->move;
    } else {
        fprintf(stderr, "ERROR: updateBoard() failed!\n");
        abort();
    }
}

/**
 * Function that performs action of finding a knight's tour. All threads will call
 * this function to simulate moving Sonny across their boards. When multiple moves
 * are detected, a child thread is created for EACH possible move, and parent thread
 * can block until child threads have terminated.
 */
void runTour(boardData_t* data) {
    nextMoves_t* possibleMoves = calloc(1, sizeof(nextMoves_t));
    int* threadReturn; // Holds the thread ID of the terminated thread
    int boardSize = data->dimensions[0] * data->dimensions[1];
    
    while (data->move < boardSize) {
        int numMoves = pathfinder(data->dimensions[0], data->dimensions[1], data->position[0], data->position[1], data->board, possibleMoves);
        
        if (numMoves > 1) { // Multiple moves detected
            if (data->threadID == 0) {
                printf("MAIN: %d possible moves after move #%d; creating %d child threads...\n", numMoves, data->move, numMoves);
            } else {
                printf("T%d: %d possible moves after move #%d; creating %d child threads...\n", data->threadID, numMoves, data->move, numMoves);
            }
            pthread_t tid[numMoves]; // array the keeps track of thread IDs
            int tid_counter = 0;
            for (int i = 0; i < 8; i++) {
                if (possibleMoves->row[i] != -1 && possibleMoves->col[i] != -1) {
                    boardData_t* new = calloc(1, sizeof(boardData_t));
                    boardDataCopy(data, new);
                    updateBoard(new, possibleMoves->row[i], possibleMoves->col[i]);
                    if (pthread_create(&tid[tid_counter], NULL, diverge, new) != 0) {
                        fprintf(stderr, "ERROR: pthread_create() failed!\n");
                        abort();
                    }
                    #ifdef NO_PARALLEL
                    pthread_join(tid[tid_counter], (void**) &threadReturn);
                    if (data->threadID == 0) {
                        printf("MAIN: T%d joined\n", *threadReturn);
                    } else {
                        printf("T%d: T%d joined\n", data->threadID, *threadReturn);
                    }
                    free(threadReturn);
                    #endif
                    tid_counter++;
                }
            }

            #ifndef NO_PARALLEL
            for (int i = 0; i < tid_counter; i++) {
                pthread_join(tid[i], (void**) &threadReturn);
                if (data->threadID == 0) {
                    printf("MAIN: T%d joined\n", *threadReturn);
                } else {
                    printf("T%d: T%d joined\n", data->threadID, *threadReturn);
                }
                free(threadReturn);
            }
            #endif

            break; // at this point, parent thread no longer needs to continue processing
        } else if (numMoves == 1) {
            for (int i = 0; i < 8; i++) {
                if (possibleMoves->row[i] != -1 && possibleMoves->col[i] != -1) {
                    updateBoard(data, possibleMoves->row[i], possibleMoves->col[i]);
                    break;
                }
            }
        } else { // numMoves == 0 but move number is less than boardSize, implying a dead end
            pthread_mutex_lock(&mutex);
            {
                if (data->move > max_squares) {
                    max_squares = data->move;
                    if (data->threadID == 0) {
                        printf("MAIN: Dead end at move #%d; updated max_squares\n", data->move);
                    } else {
                        printf("T%d: Dead end at move #%d; updated max_squares\n", data->threadID, data->move);
                    }
                } else {
                    if (data->threadID == 0) {
                        printf("MAIN: Dead end at move #%d\n", data->move);
                    } else {
                        printf("T%d: Dead end at move #%d\n", data->threadID, data->move);
                    }
                }
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
    }

    if (data->move == (data->dimensions[0] * data->dimensions[1])) { // Full Knight's tour found
        pthread_mutex_lock(&mutex);
        {
            total_tours++;
            max_squares = data->move;
            printf("T%d: Sonny found a full knight's tour; incremented total_tours\n", data->threadID);
        }
        pthread_mutex_unlock(&mutex);
    }

    free(possibleMoves);
}

/* Thread function that all child threads execute to continue simulation */
void* diverge(void* arg) {
    boardData_t* version = (boardData_t*) arg;
    
    // Assign a new thread ID to current thread (synchronized)
    pthread_mutex_lock(&mutex);
    {
        version->threadID = next_thread_id;
        next_thread_id++;
    }
    pthread_mutex_unlock(&mutex);
    
    runTour(version);
    int* tid = calloc(1, sizeof(int));
    *tid = version->threadID;

    freeBoard(version);
    pthread_exit(tid);
    return NULL;
}

/* Driver function for entire simulation that creates starting board */
int simulate( int argc, char * argv[] ) {
    setvbuf( stdout, NULL, _IONBF, 0 ); // disables buffered output for more predictable results

    if (argc != 5) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: a.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    }

#ifdef DEBUG_MODE
    int intTest = isInteger(*(argv + 1));
    printf("Argument provided for m: %s\n", *(argv + 1));
    printf("Integer? (1 means yes, 0 means no): %d\n", intTest);

    intTest = isInteger(*(argv + 2));
    printf("Argument provided for n: %s\n", *(argv + 2));
    printf("Integer? (1 means yes, 0 means no): %d\n", intTest);

    intTest = isInteger(*(argv + 3));
    printf("Argument provided for r: %s\n", *(argv + 3));
    printf("Integer? (1 means yes, 0 means no): %d\n", intTest);

    intTest = isInteger(*(argv + 4));
    printf("Argument provided for c: %s\n", *(argv + 4));
    printf("Integer? (1 means yes, 0 means no): %d\n", intTest);
#endif

    int m, n, r, c; // m == # of rows; n == # of columns; r == starting row of knight; c == starting column of knight

    if ((isInteger(argv[1]) && atoi(argv[1]) > 2) && (isInteger(argv[2]) && atoi(argv[2]) > 2) && // Validates inputs provided
        (isInteger(argv[3]) && atoi(argv[3]) >= 0 && atoi(argv[3]) < atoi(argv[1])) &&
        (isInteger(argv[4]) && atoi(argv[4]) >= 0 && atoi(argv[4]) < atoi(argv[2]))) {
        m = atoi(argv[1]);
        n = atoi(argv[2]);
        r = atoi(argv[3]);
        c = atoi(argv[4]);
    } else {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: a.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    }

    printf("MAIN: Solving Sonny's knight's tour problem for a %dx%d board\n", m, n);
    printf("MAIN: Sonny starts at row %d and column %d (move #1)\n", r, c);

    // Initial board creation
    int** board = calloc(m, sizeof(int*));
    for (int i = 0; i < m; i++) {
        *(board + i) = calloc(n, sizeof(int));
    }
    board[r][c] = 1; // Starting position
    boardData_t* initial = calloc(1, sizeof(boardData_t));
    initial->board = board;
    initial->dimensions[0] = m;
    initial->dimensions[1] = n;
    initial->position[0] = r;
    initial->position[1] = c;
    initial->move = 1;
    initial->threadID = 0;

    runTour(initial);
    pthread_mutex_lock(&mutex);
    {
        if (total_tours == 0) {
            if (max_squares == 1) {
                printf("MAIN: Search complete; best solution(s) visited %d square out of %d\n", max_squares, m * n);
            } else {
                printf("MAIN: Search complete; best solution(s) visited %d squares out of %d\n", max_squares, m * n);
            }
        } else {
            if (total_tours == 1) {
                printf("MAIN: Search complete; found %d possible path to achieving a full knight's tour\n", total_tours);
            } else {
                printf("MAIN: Search complete; found %d possible paths to achieving a full knight's tour\n", total_tours);
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    freeBoard(initial);

    return EXIT_SUCCESS;
}
