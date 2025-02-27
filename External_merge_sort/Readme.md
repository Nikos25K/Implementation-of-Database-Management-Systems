# Assignment: Implementation of External Merge Sort Algorithm

## Overview
The goal of this assignment is to implement an external merge sort algorithm for sorting records stored in a heap file. The algorithm consists of two main phases: the **sorting phase** and the **merging phase**. During the sorting phase, records are divided into sorted chunks (runs), and during the merging phase, these chunks are merged iteratively until a single sorted file is produced.

The implementation builds on the heap file management library (`HP_`) from the first assignment and introduces new libraries for managing chunks (`CHUNK_`) and performing the merge operation (`MERGE_`).

---

## Key Concepts
- **External Merge Sort**: An algorithm used to sort large datasets that do not fit entirely in memory. It works by dividing the data into smaller chunks, sorting them, and then merging the sorted chunks.
- **Chunks (Runs)**: Sorted segments of the dataset that are produced during the sorting phase.
- **b-Way Merge**: A merging strategy where `b` sorted chunks are merged at a time to produce larger sorted chunks.

---

## Implementation Details
The following functions and structures have been implemented as part of this assignment:

### Heap File Library (`HP_`)
The heap file library provides functions for managing records in a heap file. These functions are used to read, write, and update records, as well as manage blocks in the file.

#### Key Functions:
- **`HP_CreateFile(char *fileName)`**: Creates and initializes an empty heap file.
- **`HP_OpenFile(char *fileName, int *file_desc)`**: Opens a heap file and returns its file descriptor.
- **`HP_CloseFile(int file_desc)`**: Closes a heap file.
- **`HP_InsertEntry(int file_desc, Record record)`**: Inserts a record into the heap file.
- **`HP_GetRecord(int file_desc, int blockId, int cursor, Record* record)`**: Retrieves a record from a specific block and position.
- **`HP_UpdateRecord(int file_desc, int blockId, int cursor, Record record)`**: Updates a record in a specific block and position.
- **`HP_Unpin(int file_desc, int blockId)`**: Releases a block from memory.
- **`HP_PrintAllEntries(int file_desc)`**: Prints all records in the heap file.
- **`HP_GetRecordCounter(int file_desc, int blockId)`**: Retrieves the number of records in a specific block.
- **`HP_GetIdOfLastBlock(int file_desc)`**: Returns the ID of the last block in the heap file.
- **`HP_GetMaxRecordsInBlock(int file_desc)`**: Retrieves the maximum number of records that can fit in a block.
- **`HP_PrintBlockEntries(int file_desc, int blockId)`**: Prints all records in a specific block.

---

### Chunk Library (`CHUNK_`)
The chunk library provides functions and structures for managing chunks of records during the sorting and merging phases.

#### Key Structures:
- **`CHUNK`**: Represents a chunk of records, including the file descriptor, starting and ending block IDs, and the number of records and blocks in the chunk.
- **`CHUNK_Iterator`**: An iterator for traversing chunks within a file.
- **`CHUNK_RecordIterator`**: An iterator for traversing records within a chunk.

#### Key Functions:
- **`CHUNK_CreateIterator(int fileDesc, int blocksInChunk)`**: Creates an iterator for traversing chunks in a file.
- **`CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK* chunk)`**: Retrieves the next chunk in the sequence.
- **`CHUNK_GetIthRecordInChunk(CHUNK* chunk, int i, Record* record)`**: Retrieves the `i`-th record from a chunk.
- **`CHUNK_UpdateIthRecord(CHUNK* chunk, int i, Record record)`**: Updates the `i`-th record in a chunk.
- **`CHUNK_Print(CHUNK chunk)`**: Prints all records in a chunk.
- **`CHUNK_CreateRecordIterator(CHUNK *chunk)`**: Creates a record iterator for traversing records within a chunk.
- **`CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record* record)`**: Retrieves the next record from the iterator.

---

### Merge Library (`MERGE_`)
The merge library provides the core functionality for merging sorted chunks during the merge phase of the external merge sort algorithm.

#### Key Functions:
- **`merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc)`**: Merges every `b` chunks of size `chunkSize` from the input file into the output file. This function uses `CHUNK_Iterator` and `CHUNK_RecordIterator` internally.

---

## Algorithm Workflow
1. **Sorting Phase**:
   - Divide the heap file into smaller chunks.
   - Sort each chunk individually (using an in-memory sorting algorithm).
   - Write the sorted chunks back to disk.

2. **Merging Phase**:
   - Perform a `b`-way merge on the sorted chunks.
   - Repeat the merge process until only one sorted chunk remains.

---

## Notes
- The first block (block 0) of the heap file contains metadata, while records start from block 1.
- The `HP_UpdateRecord` and `HP_GetRecord` functions are recommended for simplifying the implementation.
- Always use `HP_Unpin` to release blocks that are no longer needed.
- Use `BF_Block_SetDirty` to mark modified blocks for writing to disk.

---

## Testing
The implementation can be tested using the provided heap file functions and sample data. A sample `main` function can be used to generate random records and test the sorting and merging phases.

---

## Project Structure
The project is organized as follows:
- **bin**: Contains the executables generated during compilation.
- **build**: Contains all object files generated during compilation.
- **include**: Contains header files for the heap file, chunk, and merge libraries.
- **lib**: Contains the block-level library (`libbf.so`).
- **src**: Contains the source code for the implementation.
- **examples**: Contains a sample `main` function for testing.

---

## Important Considerations
- Ensure that `HP_Unpin` is called for blocks that are no longer needed to prevent memory leaks.
- Use `BF_Block_SetDirty` for blocks that have been modified to ensure changes are written to disk.
- Avoid modifying the functions provided in the `HP_` library, as this may result in a lower grade.

---

## Conclusion
This assignment provides a deep understanding of how external merge sort algorithms work in the context of database systems. The implementation demonstrates the ability to sort large datasets efficiently using a combination of in-memory sorting and multi-way merging, while managing blocks of data using the provided block-level library.
