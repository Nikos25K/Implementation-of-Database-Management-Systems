# Assignment: Implementation of Heap File Management in a Database System

## Overview
This assignment focuses on understanding the internal workings of Database Management Systems (DBMS) at both the block and record levels. The goal is to implement a set of functions that manage heap files, which are files organized as a collection of records without any specific ordering. The implementation is done at the block level, using a provided block-level library.

The assignment involves creating and managing heap files, inserting records into these files, and retrieving records based on a specific key. The implementation is done in C, and the functions are designed to interact with the block-level library provided.

---

## Key Concepts
- **Heap Files**: Files organized as a collection of records without any specific ordering.
- **Block-Level Management**: The assignment requires working at the block level, where each block contains a fixed number of records.
- **Record Management**: Functions are implemented to insert and retrieve records from the heap files.

---

## Implementation Details
The following functions have been implemented as part of this assignment:

### 1. `HP_CreateFile(char *fileName)`
- Creates and initializes an empty heap file with the given filename.
- The first block of the file contains metadata about the heap file, stored in a structure called `HP_Info`.
- Returns `0` on success, `-1` on failure.

### 2. `HP_OpenFile(char *fileName, int *file_desc)`
- Opens the heap file with the given filename and reads the metadata from the first block.
- Returns a pointer to the `HP_Info` structure, which contains necessary information about the file.
- The `file_desc` parameter is updated with the file descriptor returned by the block-level `BF_OpenFile` function.

### 3. `HP_CloseFile(int file_desc, HP_info* header_info)`
- Closes the heap file identified by `file_desc`.
- The `header_info` structure is freed if the file is closed successfully.
- Returns `0` on success, `-1` on failure.

### 4. `HP_InsertEntry(int file_desc, HP_info* header_info, Record record)`
- Inserts a new record into the heap file.
- The record is inserted into the appropriate block, and the metadata in `header_info` is updated.
- Returns the block ID where the record was inserted on success, `-1` on failure.

### 5. `HP_GetAllEntries(int file_desc, int id)`
- Retrieves and prints all records from the heap file that have a specific `id` value.
- Returns the number of blocks read during the search on success, `-1` on failure.

---

## Data Structures

### `Record`
A structure representing a record in the heap file.
```c
typedef struct {
    int id;
    char name[20];
    char surname[20];
    char city[20];
} Record;
```

### `HP_Info`
A structure that holds metadata about the heap file, such as the ID of the last block and the number of records per block.

### `HP_block_info`
A structure that holds information about each block, such as the number of records in the block and a pointer to the next block.

### Notes
- The first block of each heap file contains metadata (`HP_Info`) that is crucial for the file management.
- The implementation avoids dynamic memory allocation (e.g., `malloc`) and instead uses pointers to specific memory regions.
- The `memcpy` function is used to copy record data into the appropriate memory locations.
- No primary key checks are performed, as the heap file does not enforce any specific ordering.