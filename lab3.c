#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "lab3.h"

typedef struct {  // A user-type structure used to hold info for the threads
    int** board; // Sudoku board
    int start_row; // Starting row for the thread
    int end_row;   // Ending row for the thread
    int result;  // a 0 for invalid, 1 for valid
} param_struct;

void* check_row(void* param) {
    param_struct* data = (param_struct*)param;
    int* seen = (int*)calloc(ROW_SIZE, sizeof(int));

    for (int i = data->start_row; i <= data->end_row; i++) {
        for (int j = 0; j < ROW_SIZE; j++) {
            int num = data->board[i][j];

            // Check if the number has already been seen in this row
            if (seen[num - 1] == 1) {
                data->result = 0; // Invalid row
                free(seen);
                return NULL;
            }

            seen[num - 1] = 1;
        }
    }

    data->result = 1; // Valid row
    free(seen);
    return NULL;
}

void* check_column(void* param) {
    param_struct* data = (param_struct*)param;
    int* seen = (int*)calloc(ROW_SIZE, sizeof(int));

    for (int i = 0; i < ROW_SIZE; i++) {
        for (int j = data->start_row; j <= data->end_row; j++) {
            int num = data->board[j][i];

            // Check if the number has already been seen in this column
            if (seen[num - 1] == 1) {
                data->result = 0; // Invalid column
                free(seen);
                return NULL;
            }

            seen[num - 1] = 1;
        }
    }

    data->result = 1; // Valid column
    free(seen);
    return NULL;
}

void* check_box(void* param) {
    param_struct* data = (param_struct*)param;
    int* seen = (int*)calloc(ROW_SIZE, sizeof(int));

    for (int i = data->start_row; i <= data->end_row; i++) {
        for (int j = 0; j < ROW_SIZE; j++) {
            int num = data->board[i][j];

            // Calculate the corresponding index in the "seen" array
            int index = (i % 3) * 3 + (j % 3);

            // Check if the number has already been seen in this box
            if (seen[index] == 1) {
                data->result = 0; // Invalid box
                free(seen);
                return NULL;
            }

            seen[index] = 1;
        }
    }

    data->result = 1; // Valid box
    free(seen);
    return NULL;
}

int** read_board_from_file(char* filename){
    // START - Variables
    FILE *fp = NULL;  // File pointer, called 'fp', instantiated to NULL
    int** board = (int**)malloc(ROW_SIZE * sizeof(int*)); // Use MALLOC to allocate memory for the 2D array of integers that is the board
    long file_size;  // Instantiates a variable to hold the numerical size of the file in the parameter
    // END   - Variables

    // CHECKING IF FILE EXISTS AND CAN BE READ    
    // Opens the file specified in the parameters of the function in read mode
    fp = fopen(filename, "r");
    if(fp == NULL) { // If the file doesn't exist, print some stuff
        fprintf(stderr, "File Location Equal to NULL, Unable To Access File.");
        return NULL;  // Stops function due to error thrown
    }

    // CHECKS FILE SIZE TO SEE IF IT IS VALID (Less than 160 chars)
    // file_size Calculations
    fseek(fp, 0, SEEK_END);  // Moves the pointer to the end of the file using fseek function
    file_size = ftell(fp);   // Sets file_size equal to the value of the pointer (which is at the end of the file)
    fseek(fp, 0, SEEK_SET);  // Sets the pointer back to the start of the file to allow for reading from the file later
    if (file_size > 160) { // If the file size is bigger than 99, throw error due to not meeting requirements
        printf("File size is greater than 160.");  // Prints to console what happened
        fclose(fp); // Closes file 
        return NULL;  // Stops function due to error thrown
    }
    
    // CHECKS IF BOARD EQUALS NULL
    // Board error check
    if (board == NULL) {
        fprintf(stderr, "There was an error in dynamically allocating memory for contents.");
        free(board); // Frees the memory used for contents
        fclose(fp); // Closes file 
        return NULL;  // Stops function due to error thrown
    }

    // Algorithm to populate the 2D board array
    for (int i = 0; i < ROW_SIZE; i++) { // Iterates thru each row and adds a row to board each iteration
        board[i] = (int*)malloc(ROW_SIZE * sizeof(int));
        for (int j = 0; j < ROW_SIZE; j++) { // Iterates thru each index in each row and ensures that it is valid
            if (fscanf(fp, "%d,", &board[i][j]) != 1) { // Throws error and closes file if invalid char is detected
                fprintf(stderr, "Error reading data from the file!");
                fclose(fp);
                return 1;
            }
        }
    }
    
    // Closing out of files and memory if everything worked
    fclose(fp); // Closes the file if no errors were thrown
    
    // should add each integer from the read file, ignoring each comma
    // Should have 81 members in the array, therefore the ending index should be 80

    return board;
}

int is_board_valid(int** board){
    /* the thread identifiers */
    pthread_t* tid[27];  // 27 threads, 9 rows, 9 columns, 9 boxes for validation
    pthread_attr_t attr;
    param_struct param[27];  // 27 params for the 27 threads
    int results[27];  // Stores results for each row, column, and box validation check

    // Initialize pthread attributes
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Init and spawn threads for row, column, and box validation
    for (int i = 0; i < 9; i++) {
        // Row Threads
        param[i].board = board;
        param[i].start_row = i;
        param[i].end_row = i;
        pthread_create(&tid[i], &attr, check_row, &param[i]);

        // Column Threads
        param[i + 9].board = board;
        param[i + 9].start_row = i;
        param[i + 9].end_row = i;
        pthread_create(&tid[i + 9], &attr, check_column, &param[i + 9]);

        // Box Threads
        param[i + 18].board = board;
        param[i + 18].start_row = i;
        param[i + 18].end_row = i;
        pthread_create(&tid[i + 18], &attr, check_box, &param[i + 18]);
    }

    // Waits for all threads to finish  
    for (int i = 0; i < 27; i++) {
        pthread_join(tid[i], NULL);
        results[i] = param[i].result;
    }

    // TO DO: Implement the validation of each row, column, box in the board using the 3 first functions in the file

    // Checks the results and returns whether the board is valid or not
    for (int i = 0; i < 27; i++) {
        if (results[i] == 0) {
            return 0; // Board is not valid
        }
    }
    
    return 1;  // returns 1 if the board is valid
}
