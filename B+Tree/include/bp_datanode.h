#ifndef BP_DATANODE_H
#define BP_DATANODE_H
#include <record.h>
#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>

#define MAX_RECORDS ((BF_BLOCK_SIZE - sizeof(BPLUS_NODE_TYPE) - 3 * sizeof(int)) / sizeof(Record))


typedef struct {
    BPLUS_NODE_TYPE type;           // Ο τύπος του κόμβου (δεδομένων ή ευρετηρίου)
    int block_id;                    // Το id του block
    int record_count;                // Ο αριθμός των εγγραφών που περιέχει ο κόμβος
    int next_block_id;                // Το id του επόμενου block

    Record records[MAX_RECORDS];    // Οι εγγραφές του κόμβου
} BPLUS_DATA_NODE;


void initialize_data_node(char* block_data, int block_id, int max_records_count);
void print_data_node(BPLUS_DATA_NODE* node);
void split_data_node(char* old_block, char* new_block, int* mid_record_id, Record new_record, int max_records_count, int new_block_id);

int find_position_for_record(BPLUS_DATA_NODE* node, int id);
BP_INSERT_STATUS insert_record_to_data_node(char* node, Record record);

void update_metadata_node(BPLUS_INFO* header_info, int fd, int new_root_id);

#endif 