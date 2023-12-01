// START - IMPORTS
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "lab3.h"
// STOP  - IMPORTS



// START - Variable Instantiations
extern int** sudoku_board; // global var reference
int* validation; // array that keeps track of each iteration of validation being valid or not
// STOP  - Variable Instantiations



// Reading the board from file function
int** read_board_from_file(char* filename){
    sudoku_board = (int**)malloc(sizeof(int*)*ROW_SIZE); // dynamically allocates memory for the sodoku board to be read into
    FILE *fp = NULL; // File pointer = null to start
    for(int row = 0; row < ROW_SIZE; row++)
	    sudoku_board[row] = (int*)malloc(sizeof(int)*COL_SIZE); // allocates memory for the columns
    
    fp = fopen(filename,"r"); // Opens the file in read mode
    for(int x = 0; x < ROW_SIZE; x++) // nested for loop that copies the applicable values over
        for(int y = 0; y < COL_SIZE; y++) 
            fscanf(fp, "%d%*c", &sudoku_board[x][y]);

    fclose(fp); // closes file
    return sudoku_board; // returns the board
}



// Validating the board function
void* validate(void* arg){
    param_struct* seq = (param_struct*) arg; // structure holding parameters for the threads
    int validation_array[9] = {0,0,0,0,0,0,0,0,0}; // array holding the bits specifying if the related number was in the 
    int thread_id = seq->id; // the id of the thread
    int starting_column = seq->starting_col; // the starting column
    int ending_column = seq->ending_col; // the ending column
    int starting_row = seq->starting_row; // the starting row
    int ending_row = seq->ending_row; // the ending row
    int current_iter; // the current iteration
    validation[thread_id] = 1; // sets it to one to start out

    for(int row = starting_row; row <= ending_row; row++){ // nested for loop to iterate over all columns and rows
        for(int column = starting_column; column <= ending_column; column++){
            current_iter = sudoku_board[row][column];
            validation_array[current_iter - 1] = 1;
        }
    }
    
    for(int i = 0; i < 9; i++) {
        if(validation_array[i] != 1) { // sets the current thread's validation value to zero if it isn't one
        validation[thread_id] = 0;
        }
    }    
}



// Overarching validation function to be run in main.c 
int is_board_valid(){
    pthread_t* tid;  /* the thread identifiers */
    pthread_attr_t attr;
    tid = (pthread_t*) malloc(sizeof(int*) * NUM_OF_THREADS);
    param_struct* parameter = (param_struct*) malloc(sizeof(param_struct) * NUM_OF_THREADS);
    validation = (int*) malloc(sizeof(int) * 27); // allocates the memory for the 27 threads necessary
    int position = 0;
    
    for(int i = 0; i < ROW_SIZE; i++){
        parameter[position].id = position;
        parameter[position].starting_row = i;
        parameter[position].starting_col = 0;
        parameter[position].ending_row = i;
        parameter[position].ending_col = COL_SIZE - 1;

        pthread_create(&tid[position], NULL, validate, &parameter[position]);
        position++;
    }

    for(int i = 0; i < COL_SIZE; i++){
        parameter[position].id = position;
        parameter[position].starting_row = 0;
        parameter[position].starting_col = i;
        parameter[position].ending_row = COL_SIZE - 1;
        parameter[position].ending_col = i;

        pthread_create(&tid[position], NULL, validate, &parameter[position]);
        position++;
    }

    int r = 0, c = 0;
    for(int i = 0; i < NUM_OF_SUBGRIDS; i++){
        parameter[position].id = position;
        parameter[position].starting_row = r;
        parameter[position].starting_col = c;
        parameter[position].ending_row = r + 2;
        parameter[position].ending_col = c + 2;

        pthread_create(&tid[position], NULL, validate, &parameter[position]);
        position++;
        if(c == 6){
            r += 3;
            c = 0;
        } 
        else c += 3;
    }
    
    for(int i = 0; i< NUM_OF_THREADS; i++) 
        pthread_join(tid[i], NULL);
    
    for (int x = 0; x< NUM_OF_THREADS; x++)
        if(validation[x] != 1)return 0;

    free(validation);
    free(tid);
    return 1;
}
