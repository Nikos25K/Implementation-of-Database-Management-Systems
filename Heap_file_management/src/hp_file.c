#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define HP_ERROR -1
#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    exit(code);             \
  }                         \
}


// Βοηθητική συνάρτηση για την αρχικοποίηση και τη δέσμευση ενός block
int init_and_allocate_block(int file_desc, BF_Block **block) {
    BF_Block_Init(block);
    return BF_AllocateBlock(file_desc, *block);
}

// Βοηθητική συνάρτηση για την αρχικοποίηση και την ανάγνωση ενός block
int init_and_get_block(int file_desc, int block_num, BF_Block **block) {
    BF_Block_Init(block);
    return BF_GetBlock(file_desc, block_num, *block);
}

// Βοηθητική συνάρτηση για την τελική επεξεργασία ενός block
void finalizeBlock(BF_Block *block, bool setDirty) {
    if (setDirty)
        BF_Block_SetDirty(block);

    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
}

// Βοηθητική συνάρτηση για τον υπολογισμό του αριθμού των εγγραφών σε ένα block
int get_record_count_of_block(char *data, int max_records_per_block) {
    int filled_bytes = 0;
    int record_size = sizeof(Record);

    // Όσο υπάρχει χώρος στο block και δεν έχει φτάσει το μέγιστο αριθμό εγγραφών
    // και δεν έχει φτάσει το τέλος του block
    while ((filled_bytes + record_size) <= BF_BLOCK_SIZE &&
           filled_bytes / record_size < max_records_per_block &&
           *(data + filled_bytes) != '\0') {
        filled_bytes += record_size;
    }

    return filled_bytes / record_size;
}


// Βοηθητική συνάρτηση για την εισαγωγή μιας εγγραφής σε ένα block
int insert_record_to_block(char *data, Record record, int current_records) {
    int offset = current_records * sizeof(Record);
    memcpy(data + offset, &record, sizeof(Record));

    // Επιστροφή του αριθμού των εγγραφών στο block μετά την εισαγωγή
    return current_records + 1;
}


int HP_CreateFile(char *fileName) {

    // Δημιουργία του αρχείου fileName
    CALL_BF(BF_CreateFile(fileName));
    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc));

    // Δημιουργία και αρχικοποίηση του πρώτου block
    BF_Block *block;
    init_and_allocate_block(file_desc, &block);

    // Δημιουργία της δομής HP_info( θα περιέχει τον αριθμό των εγγραφών ανά block)
    HP_info hp_info;
    hp_info.record_count_per_block = BF_BLOCK_SIZE / sizeof(Record);
    memcpy(BF_Block_GetData(block), &hp_info, sizeof(HP_info));

    // Το block έχει τροποποιηθεί
    finalizeBlock(block, true);
    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc) {
    CALL_BF(BF_OpenFile(fileName, file_desc));

    // Δημιουργία της δομής HP_info προς επιστροφή
    HP_info *hp_info = malloc(sizeof(HP_info));
    if (!hp_info) {
        CALL_BF(BF_CloseFile(*file_desc));
        return NULL;
    }

    BF_Block *block;
    init_and_get_block(*file_desc, 0, &block);

    // Ανάγνωση του πρώτου block που περιέχει τη δομή HP_info
    memcpy(hp_info, BF_Block_GetData(block), sizeof(HP_info));
    finalizeBlock(block, false);

    return hp_info;
}

int HP_CloseFile(int file_desc, HP_info* hp_info) {
    // Unpin το block που περιέχει τις πληροφορίες
    BF_Block *block;
    init_and_get_block(file_desc, 0, &block);
    finalizeBlock(block, false);

    // Κλείσιμο του αρχείου και αποδέσμευση της μνήμης
    CALL_BF(BF_CloseFile(file_desc));
    free(hp_info);
    return 0;
}

int HP_InsertEntry(int file_desc, HP_info* hp_info, Record record) {
    BF_Block *block;

    // Παίρνουμε τον αριθμό των blocks
    int num_blocks;
    CALL_BF(BF_GetBlockCounter(file_desc, &num_blocks));

    int record_size = sizeof(Record);

    // Αν δεν υπάρχει κανένα block πέραν του hp_info, δημιουργούμε το πρώτο
    if (num_blocks == 1) {
        init_and_allocate_block(file_desc, &block);
        char *data = BF_Block_GetData(block);
        insert_record_to_block(data, record, 0);
        finalizeBlock(block, true);
        return 1;
    }

    // Ανάγνωση του τελευταίου block
    init_and_get_block(file_desc, num_blocks - 1, &block);
    char *data = BF_Block_GetData(block);

    // Υπολογισμός του αριθμού των εγγραφών στο block
    int current_records = get_record_count_of_block(data,  hp_info->record_count_per_block);

    // Αν υπάρχει χώρος στο block, εισάγουμε την εγγραφή
    if (current_records < hp_info->record_count_per_block) {
        insert_record_to_block(data, record, current_records);
        finalizeBlock(block, true);
        return num_blocks - 1;
    }

    // Δημιουργία νέου block
    finalizeBlock(block, false);
    init_and_allocate_block(file_desc, &block);
    data = BF_Block_GetData(block);

    // Εισαγωγή της εγγραφής στο νέο block
    insert_record_to_block(data, record, 0);
    finalizeBlock(block, true);
    return num_blocks;
}

int HP_GetAllEntries(int file_desc, HP_info* hp_info, int value) {
    BF_Block *block;

    // Παίρνουμε τον αριθμό των blocks
    int num_blocks;
    CALL_BF(BF_GetBlockCounter(file_desc, &num_blocks));

    int record_size = sizeof(Record);

    // Διάβασμα των blocks
    int blocksRead = 0;
    Record record;

    for (int i = 1; i < num_blocks; i++) {
        init_and_get_block(file_desc, i, &block);
        char *data = BF_Block_GetData(block);

        // Υπολογισμός του αριθμού των εγγραφών στο block
        int current_records = get_record_count_of_block(data, hp_info->record_count_per_block);

        // Εκτύπωση των εγγραφών που έχουν τιμή value
        for (int j = 0; j < current_records; j++) {
            memcpy(&record, data + j * record_size, record_size);
            if (record.id == value)
                printRecord(record);
        }

        blocksRead++;
        finalizeBlock(block, false);
    }
    return blocksRead ? blocksRead : -1;
}