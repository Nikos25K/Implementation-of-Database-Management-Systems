#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    // Create a CHUNK_Iterator to iterate over chunks from the input file
    CHUNK_Iterator chunkIterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);

    // Arrays to hold record iterators, current records, and flags for each chunk
    CHUNK_RecordIterator recordIterators[bWay];
    Record *currentRecords[bWay];
    bool hasMoreRecords[bWay];
    CHUNK *chunksArray[bWay];
    
    // Output buffer for records and its index
    Record outputBuffer[HP_GetMaxRecordsInBlock(output_FileDesc)];
    int outputBufferIndex = 0;

    // Flag to check if there are more records to process
    bool thereAreRecords = true;
    int minimumRecordIndex = -1;

    // Initialize chunks and record iterators
    for (int i = 0; i < bWay; i++) {
        chunksArray[i] = malloc(sizeof(CHUNK));
        if (!chunksArray[i]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        currentRecords[i] = malloc(sizeof(Record));
        if (!currentRecords[i]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        // Load the first chunk if available
        if (CHUNK_GetNext(&chunkIterator, chunksArray[i])) {
            currentRecords[i] = NULL;
            continue;
        }

        // Create record iterators for each chunk
        recordIterators[i] = CHUNK_CreateRecordIterator(chunksArray[i]);
        CHUNK_GetNextRecord(&recordIterators[i], currentRecords[i]);
    }

    // Main merging loop
    while(1){
        // Inner loop to process records and find the smallest record from each chunk
        while(1){
            thereAreRecords = false;

            // Check if any chunk has remaining records
            for(int i = 0; i < bWay; i++)
                if (currentRecords[i]){
                    thereAreRecords = true;
                    break;
                }

            if (!thereAreRecords)
                break;

            // Find the smallest record among the current records of the chunks
            minimumRecordIndex = -1;
            for(int i = 0; i < bWay; i++){
                if (!currentRecords[i])
                    continue;

                if (minimumRecordIndex == -1)
                    minimumRecordIndex = i;
                else if (shouldSwap(currentRecords[minimumRecordIndex], currentRecords[i]))
                    minimumRecordIndex = i;
            }

            // Insert the smallest record into the output file
            HP_InsertEntry(output_FileDesc, *currentRecords[minimumRecordIndex]);
            HP_Unpin(output_FileDesc, HP_GetIdOfLastBlock(output_FileDesc));

            int a = recordIterators[minimumRecordIndex].currentBlockId;
            
            // Move to the next record in the chosen chunk
            if (CHUNK_GetNextRecord(&recordIterators[minimumRecordIndex], currentRecords[minimumRecordIndex]))
                currentRecords[minimumRecordIndex] = NULL;

            int b = recordIterators[minimumRecordIndex].currentBlockId;
            // Unpin the block if we moved to a new block
            if (b != a)
                HP_Unpin(input_FileDesc, a);
        }

        // Reset flag for the next batch of records
        thereAreRecords = false;

        // Check if we need to load new chunks to continue processing
        for(int i = 0; i < bWay; i++){
            // Unpin blocks from the previous chunk
            for (int j = chunksArray[i]->from_BlockId; j <= chunksArray[i]->to_BlockId; j++)
                HP_Unpin(input_FileDesc, j);

            // Load the next chunk if available
            if (CHUNK_GetNext(&chunkIterator, chunksArray[i])) {
                currentRecords[i] = NULL;
                continue;
            }

            // Re-initialize record iterators and current records
            recordIterators[i] = CHUNK_CreateRecordIterator(chunksArray[i]);
            currentRecords[i] = malloc(sizeof(Record));
            if (!currentRecords[i]) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            // Check if the new chunk has records
            if(!CHUNK_GetNextRecord(&recordIterators[i], currentRecords[i]))
                thereAreRecords = true;
        }
        // Break if there are no more records to process
        if (!thereAreRecords)
            break;
    }

    // Free memory allocated for chunks and current records
    for (int i = 0; i < bWay; i++) {
        free(chunksArray[i]);
        free(currentRecords[i]);
    }

    // Unpin the last block of the output file
    HP_Unpin(output_FileDesc, HP_GetIdOfLastBlock(output_FileDesc));
}