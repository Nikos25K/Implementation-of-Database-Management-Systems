# Assignment: Implementation of B+ Tree for Database Management

## Overview
The goal of this assignment is to understand the internal workings of Database Management Systems (DBMS) at the block level, specifically focusing on how indexing using B+ trees can improve system performance. The task involves implementing a set of functions to manage data files organized as B+ trees. These functions handle record insertion, retrieval, and index management, all built on top of a block-level library provided for the assignment.

The implementation is done in C, and the functions interact with the block-level library (`BF`) to manage blocks of data efficiently.

---

## Key Concepts
- **B+ Trees**: A self-balancing tree data structure that maintains sorted data and allows for efficient insertion, deletion, and retrieval operations.
- **Block-Level Management**: The implementation works at the block level, where each block contains a fixed number of records or index entries.
- **Indexing**: The B+ tree is used to index records based on a primary key (`id`), enabling faster search operations.

---

## Implementation Details
The following functions have been implemented as part of this assignment:

### 1. `BP_CreateFile(char *fileName)`
- Creates and initializes an empty B+ tree file with the given filename.
- The first block of the file contains metadata about the B+ tree, stored in a structure called `BPLUS_INFO`.
- Returns `0` on success, `-1` on failure.

### 2. `BP_OpenFile(char *fileName, int *file_desc)`
- Opens the B+ tree file with the given filename and reads the metadata from the first block.
- Returns a pointer to the `BPLUS_INFO` structure, which contains necessary information about the B+ tree.
- The `file_desc` parameter is updated with the file descriptor returned by the block-level `BF_OpenFile` function.

### 3. `BP_CloseFile(int file_desc, BPLUS_INFO* info)`
- Closes the B+ tree file identified by `file_desc`.
- The `info` structure is freed if the file is closed successfully.
- Returns `0` on success, `-1` on failure.

### 4. `BP_InsertEntry(int file_desc, BPLUS_INFO* bplus_info, Record record)`
- Inserts a new record into the B+ tree.
- The record is inserted into the appropriate leaf node, and the tree is rebalanced if necessary.
- If the `id` of the record already exists, the insertion is rejected with an appropriate message.
- Returns the block ID where the record was inserted on success, `-1` on failure.

### 5. `BP_GetEntry(int file_desc, BPLUS_INFO* header_info, int id, Record** result)`
- Searches for a record in the B+ tree with a key matching the given `id`.
- Starting from the root, the function traverses the tree to locate the relevant block containing the record.
- If a matching record is found, the `result` pointer is set to point to the record, and `0` is returned.
- If no matching record is found, `result` is set to `NULL`, and `-1` is returned.

---

## Data Structures

### `Record`
A structure representing a record in the B+ tree.
```c
typedef struct {
    int id;
    char name[20];
    char surname[20];
    char city[20];
} Record;
```

### `BPLUS_INFO`
A structure that holds metadata about the B+ tree, such as the root node, tree height, and other relevant information.

### `BP_block_info`
A structure that holds information about each block in the B+ tree, such as the number of records or index entries in the block and pointers to child blocks.

### Notes
- The first block of each B+ tree file contains metadata (`BPLUS_INFO`) that is crucial for the tree's management.
- The implementation avoids dynamic memory allocation (e.g., `malloc`) and instead uses pointers to specific memory regions.
- The `memcpy` function is used to copy record data into the appropriate memory locations.
- The primary key (`id`) is used for indexing, and duplicate keys are not allowed.

### Testing
The implementation can be tested using the provided `randomRecord()` function to generate sample records. A sample main function (`bp_main.c`) is provided in the examples folder to test the functionality of the implemented functions.

### Project Structure
The project is organized as follows:
- `bin`: Contains the executables generated during compilation.
- `build`: Contains all object files generated during compilation.
- `include`: Contains header files (`bf.h`, `bp_file.h`, `bp_indexnode.h`, `bp_datanode.h`).
- `lib`: Contains the block-level library (`libbf.so`).
- `src`: Contains the source code for the implementation.
- `examples`: Contains a sample main function (`bp_main.c`) for testing.

### Important Considerations
- Always call `BF_UnpinBlock` for blocks that are no longer needed to prevent the buffer from filling up.
- Use `BF_Block_SetDirty` for blocks that have been modified to ensure changes are written to disk.
- Do not modify the functions provided in the `bp_file.h` library, as this may result in a lower grade.

### Conclusion
This assignment provides a deep understanding of how B+ trees are used for indexing and managing records in a DBMS. The implementation demonstrates the ability to create, open, close, insert, and retrieve records in a B+ tree, while efficiently managing blocks of data using the provided block-level library.