#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include <bp_indexnode.h>

// Αρχικοποίηση του index node με τα δεδομένα
void initialize_index_node(char* block_data, int block_id, int max_index_count){
    BPLUS_INDEX_NODE* node = (BPLUS_INDEX_NODE*)block_data;
    node->type = BPLUS_INDEX;
    node->block_id = block_id;
    node->index_count = 0;

    // Αρχικοποίηση των keys σε -1
    for (int i = 0; i < max_index_count; i++)
        node->keys[i] = -1;

    // Αρχικοποίηση των children σε -1
    for (int i = 0; i < max_index_count + 1; i++)
        node->children[i] = -1;

    // Αντιγραφή των δεδομένων στο block
    memcpy(block_data, node, sizeof(BPLUS_INDEX_NODE));
}

// Επιστρέφει τη θέση που πρέπει να μπει το index στον κόμβο
// Χρησιμοποιεί δυαδική αναζήτηση
int find_position_for_index(BPLUS_INDEX_NODE* node, int key) {
    int left = 0;
    int right = node->index_count - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (node->keys[mid] == key)
            return mid;
        else if (node->keys[mid] < key)
            left = mid + 1;
        else
            right = mid - 1;
    }

    return left;
}

// Επιστρέφει τη θέση που πρέπει να μπει το index στον κόμβο ή -1 αν υπάρχει ήδη
void add_index_to_node(BPLUS_INDEX_NODE* node, int key, int child_block_id) {

    // Βρίσκουμε τη θέση που πρέπει να μπει το index
    int pos = find_position_for_index(node, key);

    // Κάνουμε shift τα keys προς τα δεξιά
    for (int i = node->index_count; i > pos; i--)
        node->keys[i] = node->keys[i - 1];

    // Κάνουμε shift τα children προς τα δεξιά
    for (int i = node->index_count + 1; i > pos + 1; i--)
        node->children[i] = node->children[i - 1];

    // Εισάγουμε το index
    node->keys[pos] = key;
    node->children[pos + 1] = child_block_id;
    node->index_count++;
}


void split_index_node(char* old_block, char* new_block, int* mid_key, int max_index_count) {
    // Κάνουμε cast τα blocks σε BPLUS_INDEX_NODE
    BPLUS_INDEX_NODE* old_node = (BPLUS_INDEX_NODE*)old_block;
    BPLUS_INDEX_NODE* new_node = (BPLUS_INDEX_NODE*)new_block;

    // Βρίσκουμε το σημείο διαίρεσης
    int split_point = (max_index_count + 1) / 2;

    // Αποθηκεύουμε το key που θα μεταφερθεί στον γονικό κόμβο
    *mid_key = old_node->keys[split_point];

    // Αντιγράφουμε τα keys και τα children στο νέο block από το split point και μετά
    for (int i = split_point + 1; i < old_node->index_count; i++) {
        new_node->keys[i - (split_point + 1)] = old_node->keys[i];
        new_node->children[i - (split_point + 1)] = old_node->children[i];
    }

    // Αντιγράφουμε το τελευταίο child
    new_node->children[new_node->index_count] = old_node->children[old_node->index_count];

    // Ενημερώνουμε τον αριθμό των keys στους κόμβους
    new_node->index_count = old_node->index_count - (split_point + 1);
    old_node->index_count = split_point;

    // Καθορίζουμε σε ποιον κόμβο θα μπει το νέο index
    if (*mid_key < old_node->keys[0])
        add_index_to_node(old_node, *mid_key, new_node->children[0]);
    else
        add_index_to_node(new_node, *mid_key, new_node->children[0]);

    // Αντιγράφουμε τα δεδομένα στα blocks
    memcpy(old_block, old_node, sizeof(BPLUS_INDEX_NODE));
    memcpy(new_block, new_node, sizeof(BPLUS_INDEX_NODE));
}

// Επιστρέφει το block_id του παιδιού που πρέπει να ακολουθήσουμε για να βρούμε το record
int find_child_for_record(char* block_data, int id) {
    BPLUS_INDEX_NODE* index_node = (BPLUS_INDEX_NODE*)block_data;

    // Βρίσκουμε τη θέση που πρέπει να μπει το record
    int pos = find_position_for_index(index_node, id);

    // Αν ο δείκτης ισούται με το id, τότε το παιδί που ακολουθούμε είναι το δεξιό
    if (index_node->keys[pos] == id)
        return index_node->children[pos + 1];
    else
        return index_node->children[pos];
}