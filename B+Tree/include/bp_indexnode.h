#ifndef BP_INDEX_NODE_H
#define BP_INDEX_NODE_H
#include <record.h>
#include <bf.h>
#include <bp_file.h>

#define MAX_INDEXES (BF_BLOCK_SIZE - sizeof(BPLUS_NODE_TYPE) - 2 * sizeof(int)) / (2 * sizeof(int) ) - 1

typedef struct{
    BPLUS_NODE_TYPE type;       // Ο τύπος του κόμβου (δεδομένων ή ευρετηρίου)
    int block_id;               // Το id του block
    int index_count;            // Ο αριθμός των κλειδιών που περιέχει ο κόμβος (μέσω αυτού μπορούμε να βρούμε και τον αριθμό των παιδιών)

    int keys[MAX_INDEXES];      // Οι κλειδιά των ευρετηρίων
    int children[MAX_INDEXES + 1]; // Τα παιδιά στα οποία οδηγεί ο κόμβος  
} BPLUS_INDEX_NODE;

void initialize_index_node(char* block_data, int block_id, int max_index_count);
void add_index_to_node(BPLUS_INDEX_NODE* node, int key, int child_block_id);
void split_index_node(char* old_block, char* new_block, int* mid_key, int max_index_count);
int find_child_for_record(char* block_data, int id);

#endif