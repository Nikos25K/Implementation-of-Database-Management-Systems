#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"

/* Determines if two records should be swapped during sorting. */
bool shouldSwap(Record *rec1, Record *rec2) {
    // First compare by surname
    int surnameComparison = strcmp(rec1->surname, rec2->surname);
    if (surnameComparison != 0)
        return surnameComparison > 0;

    // If surnames are equal, compare by name
    int nameComparison = strcmp(rec1->name, rec2->name);
    if (nameComparison != 0)
        return nameComparison >= 0;

    // If names are equal, compare by city
    int cityComparison = strcmp(rec1->city, rec2->city);
    if (cityComparison != 0)
        return cityComparison > 0;

    // If cities are equal, compare by id
    return rec1->id > rec2->id;
}


/* Partition the array around the pivot. */
int partition(Record *records, int low, int high) {
    Record pivot = records[high]; // Choose the last element as the pivot
    int i = low - 1;              // Index of smaller element

    for (int j = low; j < high; j++)
        // Check if the current record is less than or equal to the pivot
        if (!shouldSwap(&records[j], &pivot)) {
            i++; // Increment index of smaller element
            Record temp = records[i];
            records[i] = records[j];
            records[j] = temp;
        }

    // Swap the pivot element with the element at i+1
    Record temp = records[i + 1];
    records[i + 1] = records[high];
    records[high] = temp;

    return i + 1;
}

/* QuickSort implementation for records. */
void quickSort(Record *records, int low, int high) {
    if (low < high) {
        // Partition the array and get the pivot index
        int pi = partition(records, low, high);

        // Recursively sort the elements before and after the partition
        quickSort(records, low, pi - 1);
        quickSort(records, pi + 1, high);
    }
}

/* Sorts records within a CHUNK in ascending order using quicksort. */
void sort_Chunk(CHUNK *chunk) {
    CHUNK_RecordIterator iterator = CHUNK_CreateRecordIterator(chunk);
    int totalRecords = chunk->recordsInChunk;

    // Allocate memory for all records in the chunk
    Record *records = (Record *)malloc(sizeof(Record) * totalRecords);

    // Load all records from the chunk into memory
    int index = 0;
    Record tempRecord;
    while (CHUNK_GetNextRecord(&iterator, &tempRecord) == 0)
        records[index++] = tempRecord;

    // Perform quicksort on the records
    quickSort(records, 0, totalRecords - 1);

    // Write sorted records back into the chunk
    iterator = CHUNK_CreateRecordIterator(chunk);
    index = 0;
    while (CHUNK_GetNextRecord(&iterator, &tempRecord) == 0) {
        CHUNK_UpdateIthRecord(chunk, index, records[index]);
        index++;
    }

    // Free allocated memory
    free(records);
}

/* Sorts the contents of a file in chunks. */
void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    CHUNK chunk;

    // Process each chunk individually
    while (CHUNK_GetNext(&iterator, &chunk) == 0){
        sort_Chunk(&chunk);

        // Unpin all blocks of the chunk
        for (int i = chunk.from_BlockId; i <= chunk.to_BlockId; i++)
            HP_Unpin(file_desc, i);
    }
}