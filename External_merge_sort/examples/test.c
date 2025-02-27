#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merge.h"

#define FILE_NAME "data.db"
#define OUTPUT_FILE "results.txt"

FILE* open_file(const char* filename, const char* mode);
void remove_out_db_files();
int createAndPopulateHeapFile(char* filename, int recordsNum);
void sortPhase(int file_desc, int chunkSize);
void mergePhases(int inputFileDesc, int chunkSize, int bWay, int* fileCounter);
int nextOutputFile(int* fileCounter);
void write_to_file(const char* filename, const char* content);

int main() {
    int chunkSizes[] = {5, 10}; // Different chunk sizes to test
    int bWays[] = {2, 10}; // Different bWay values to test
    int recordsNum[] = {100, 1000, 30000}; // Different number of records to test
    int fileIterator = 0;
    clock_t start, end;
    double cpu_time_used;
    char output[1024];

    BF_Init(LRU);

    FILE* file = open_file(OUTPUT_FILE, "w");

    // Write header to output file
    fprintf(file, "Experiment Results:\n");
    fprintf(file, "-------------------------------------------------------------\n");
    fprintf(file, "%7s | %10s | %4s | %13s | %6s\n", "Records", "Chunk Size", "bWay", "Time (seconds)", "Passes");
    fprintf(file, "-------------------------------------------------------------\n");

    for (int i = 0; i < sizeof(chunkSizes) / sizeof(chunkSizes[0]); i++)
        for (int j = 0; j < sizeof(bWays) / sizeof(bWays[0]); j++)
            for(int k = 0; k < sizeof(recordsNum) / sizeof(recordsNum[0]); k++) {
                
                remove_out_db_files();
                int chunkSize = chunkSizes[i];
                int bWay = bWays[j];
                fileIterator = 0;

                // Create and populate heap file
                int file_desc = createAndPopulateHeapFile(FILE_NAME, recordsNum[k]);

                // Measure time for sorting and merging
                start = clock();
                sortPhase(file_desc, chunkSize);
                mergePhases(file_desc, chunkSize, bWay, &fileIterator);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

                // Write results to output file
                fprintf(file, "%7d | %10d | %4d | %13.6f | %6d\n", recordsNum[k], chunkSize, bWay, cpu_time_used, fileIterator);
            }

    fprintf(file, "-------------------------------------------------------------\n");
    fclose(file);

    return 0;
}

FILE* open_file(const char* filename, const char* mode) {
    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }
    return file;
}

void remove_out_db_files() {
    char command[50];
    sprintf(command, "rm -f out*.db");
    int ret = system(command);
    if (ret == -1) {
        perror("Error removing out*.db files");
        exit(1);
    }
}

int createAndPopulateHeapFile(char* filename, int recordsNum) {
    remove(filename);
    HP_CreateFile(filename);

    int file_desc;
    HP_OpenFile(filename, &file_desc);

    Record record;
    srand(12569874);
    for (int id = 0; id < recordsNum; ++id) {
        record = randomRecord();
        HP_InsertEntry(file_desc, record);
    }

    return file_desc;
}

/*Performs the sorting phase of external merge sort algorithm on a file specified by 'file_desc', using chunks of size 'chunkSize'*/
void sortPhase(int file_desc, int chunkSize) { 
    sort_FileInChunks(file_desc, chunkSize);
}

/* Performs the merge phase of the external merge sort algorithm using chunks of size 'chunkSize' and 'bWay' merging. The merge phase may be performed in more than one cycles.*/
void mergePhases(int inputFileDesc, int chunkSize, int bWay, int* fileCounter) {
    int outputFileDesc;
    while (chunkSize <= HP_GetIdOfLastBlock(inputFileDesc)) {
        outputFileDesc = nextOutputFile(fileCounter);
        merge(inputFileDesc, chunkSize, bWay, outputFileDesc);
        HP_CloseFile(inputFileDesc);
        chunkSize *= bWay;
        inputFileDesc = outputFileDesc;
    }
    HP_CloseFile(outputFileDesc);
}

/*Creates a sequence of heap files: out0.db, out1.db, ... and returns for each heap file its corresponding file descriptor. */
int nextOutputFile(int* fileCounter) {
    char mergedFile[50];
    sprintf(mergedFile, "out%d.db", (*fileCounter)++);
    int file_desc;
    HP_CreateFile(mergedFile);
    HP_OpenFile(mergedFile, &file_desc);
    return file_desc;
}