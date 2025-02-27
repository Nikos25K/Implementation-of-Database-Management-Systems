#include <merge.h>
#include <stdio.h>
#include <string.h>
#include "chunk.h"


/* Creates a ChunkIterator for efficient traversal of chunks within a file. */
CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator iterator;
    iterator.file_desc = fileDesc;
    iterator.current = 1; // Start from the first block (assumed)
    iterator.lastBlocksID = HP_GetIdOfLastBlock(fileDesc);
    iterator.blocksInChunk = blocksInChunk;
    return iterator;
}

/* Retrieves the next CHUNK in the sequence as per the provided CHUNK_Iterator. */
int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
    if (iterator->current > iterator->lastBlocksID)
        return -1; // No more chunks

    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->current;
    chunk->to_BlockId = iterator->current + iterator->blocksInChunk - 1;

    if (chunk->to_BlockId > iterator->lastBlocksID)
        chunk->to_BlockId = iterator->lastBlocksID;

    chunk->blocksInChunk = chunk->to_BlockId - chunk->from_BlockId + 1;
    chunk->recordsInChunk = 0;

    for (int i = chunk->from_BlockId; i <= chunk->to_BlockId; i++)
        chunk->recordsInChunk += HP_GetRecordCounter(iterator->file_desc, i);

    iterator->current += iterator->blocksInChunk;
    return 0;
}

/* Retrieves the ith record from a CHUNK of blocks in a heap file. */
int CHUNK_GetIthRecordInChunk(CHUNK *chunk, int i, Record *record) {
    if (i >= chunk->recordsInChunk || i < 0)
        return -1; // Invalid index

    int recordsPerBlock = HP_GetMaxRecordsInBlock(chunk->file_desc);
    int blockOffset = i / recordsPerBlock;
    int recordOffset = i % recordsPerBlock;

    int targetBlockId = chunk->from_BlockId + blockOffset;

    return HP_GetRecord(chunk->file_desc, targetBlockId, recordOffset, record);
}

/* Updates the ith record in a chunk. */
int CHUNK_UpdateIthRecord(CHUNK *chunk, int i, Record record) {
    if (i >= chunk->recordsInChunk || i < 0)
        return -1; // Invalid index

    int recordsPerBlock = HP_GetMaxRecordsInBlock(chunk->file_desc);
    int blockOffset = i / recordsPerBlock;
    int recordOffset = i % recordsPerBlock;

    int targetBlockId = chunk->from_BlockId + blockOffset;

    return HP_UpdateRecord(chunk->file_desc, targetBlockId, recordOffset, record);
}

/* This function is used to print the records within a chunk. */
void CHUNK_Print(CHUNK chunk) {
    for (int blockId = chunk.from_BlockId; blockId <= chunk.to_BlockId; blockId++)
        HP_PrintBlockEntries(chunk.file_desc, blockId);
}

/* Creates a record iterator for efficient traversal within a CHUNK. */
CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {
    CHUNK_RecordIterator iterator;
    iterator.chunk = *chunk;
    iterator.currentBlockId = chunk->from_BlockId;
    iterator.cursor = 0;
    return iterator;
}

/* Function to get the next record from the iterator. */
int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record *record) {
    if (iterator->currentBlockId > iterator->chunk.to_BlockId)
        return -1; // No more records

    if (HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId, iterator->cursor, record) == 0) {
        iterator->cursor++;
        if (iterator->cursor >= HP_GetRecordCounter(iterator->chunk.file_desc, iterator->currentBlockId)) {
            iterator->cursor = 0;
            iterator->currentBlockId++;
        }
        return 0;
    }

    return -1; // Error retrieving record
}