#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include <bp_datanode.h>

// Αρχικοποίηση του data node με τα δεδομένα
void initialize_data_node(char* block_data, int block_id, int max_records_count){
    BPLUS_DATA_NODE* node = (BPLUS_DATA_NODE*)block_data;
    node->type = BPLUS_LEAF;
    node->block_id = block_id;
    node->record_count = 0;
    node->next_block_id = -1;

    // Αρχικοποίηση των records σε -1
    for (int i = 0; i < max_records_count; i++)
        node->records[i].id = -1;

    // Αντιγραφή των δεδομένων στο block
    memcpy(block_data, node, sizeof(BPLUS_DATA_NODE));
}

// Εκτύπωση των δεδομένων του data node
void print_data_node(BPLUS_DATA_NODE* node){
    for (int i = 0; i < node->record_count; i++)
        if (node->records[i].id != -1)
            printf("Record %d: %d %s %s\n", i, node->records[i].id, node->records[i].name, node->records[i].surname);
}


void split_data_node(char* old_block, char* new_block, int* mid_record_id, Record new_record, int max_records_count, int new_block_id){
    // Κάνουμε cast τα blocks σε BPLUS_DATA_NODE
    BPLUS_DATA_NODE* old_node = (BPLUS_DATA_NODE*)old_block;
    BPLUS_DATA_NODE* new_node = (BPLUS_DATA_NODE*)new_block;

    // Βρίσκουμε το σημείο διαίρεσης
    int split_point = (max_records_count + 1) / 2;

    // Αντιγράφουμε τα records στο νέο block από το split point και μετά
    for (int i = split_point; i < old_node->record_count; i++)
        new_node->records[i - split_point] = old_node->records[i];

    // Ενημερώνουμε τον αριθμό των records στους κόμβους
    new_node->record_count = old_node->record_count - split_point;
    old_node->record_count = split_point;

    new_node->block_id = new_block_id;

    // Καθορίζουμε σε ποιον κόμβο θα μπει το νέο record
    if (new_record.id < new_node->records[0].id)
        insert_record_to_data_node(old_block, new_record);
    else
        insert_record_to_data_node(new_block, new_record);

    // Αποθηκεύουμε το record που θα μεταφερθεί στον γονικό κόμβο
    *mid_record_id = new_node->records[0].id;

    // Αντιγράφουμε τα δεδομένα στα blocks
    memcpy(old_block, old_node, sizeof(BPLUS_DATA_NODE));
    memcpy(new_block, new_node, sizeof(BPLUS_DATA_NODE));
}

// Επιστρέφει τη θέση που πρέπει να μπει το record στον κόμβο και ελέγχει για διπλότυπα
// Χρησιμοποιεί δυαδική αναζήτηση
int find_position_for_record(BPLUS_DATA_NODE* node, int id) {
    int left = 0;
    int right = node->record_count - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (node->records[mid].id == id)
            return mid;
        else if (node->records[mid].id < id)
            left = mid + 1;
        else
            right = mid - 1;
    }

    return left;
}

// Επιστρέφει τη θέση που πρέπει να μπει το record στον κόμβο ή -1 αν υπάρχει ήδη
int record_pos_in_node(BPLUS_DATA_NODE* node, int id) {
    int pos = find_position_for_record(node, id);

    // Ελέγχουμε αν το record υπάρχει ήδη
    if (pos < node->record_count && node->records[pos].id == id)
        return -1;

    return pos;
}

// Εισαγωγή ενός record στον κόμβο
BP_INSERT_STATUS insert_record_to_data_node(char* block_data, Record record) {
    BPLUS_DATA_NODE* node = (BPLUS_DATA_NODE*)block_data;

    // Βρίσκουμε τη θέση που πρέπει να μπει το record
    int pos_to_insert = record_pos_in_node(node, record.id);

    // Ελέγχουμε αν το record υπάρχει ήδη
    if (pos_to_insert == -1)
        return BP_LEAF_HAS_DUPLICATE;

    // Ελέγχουμε αν ο κόμβος είναι γεμάτος
    if (node->record_count == MAX_RECORDS)
        return BP_LEAF_FULL;

    // Κάνουμε shift τα records προς τα δεξιά
    for (int i = node->record_count; i > pos_to_insert; i--)
        node->records[i] = node->records[i - 1];

    // Εισάγουμε το record
    node->records[pos_to_insert] = record;
    node->record_count++;

    // Αντιγράφουμε τα δεδομένα στο block και επιστρέφουμε επιτυχές status
    memcpy(block_data, node, sizeof(BPLUS_DATA_NODE));
    return BP_OK;
}

// Ενημέρωση του metadata block με το νέο root id
void update_metadata_node(BPLUS_INFO* header_info, int fd, int new_root_id) {
    BF_Block* info_block;
    BF_Block_Init(&info_block);
    BF_GetBlock(fd, 0, info_block);

    // Ενημερώνεται και η δομή που έχει τα metadata (BPLUS_INFO) αλλά και το block
    header_info->root_block_id = new_root_id;
    memcpy(BF_Block_GetData(info_block), header_info, sizeof(BPLUS_INFO));

    BF_Block_SetDirty(info_block);
    BF_UnpinBlock(info_block);
    BF_Block_Destroy(&info_block);
}