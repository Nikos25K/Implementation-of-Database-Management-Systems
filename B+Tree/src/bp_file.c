#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_datanode.h"
#include "bp_indexnode.h"

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
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


// Δημιουργία του αρχείου του B+ δέντρου
int BP_CreateFile(char *fileName) {

    // Δημιουργία του αρχείου
    CALL_BF(BF_CreateFile(fileName));
    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc));

    // Δημιουργία και αρχικοποίηση του πρώτου block
    BF_Block *block;
    init_and_allocate_block(file_desc, &block);

    // Δημιουργία της δομής BPLUS_INFO
    BPLUS_INFO bplus_info;
    bplus_info.root_block_id = -1;
    bplus_info.last_block_id = 0;
    bplus_info.max_index_count = MAX_INDEXES;
    bplus_info.max_records_count = MAX_RECORDS;

    // Αντιγραφή της δομής BPLUS_INFO στα δεδομένα του block
    memcpy(BF_Block_GetData(block), &bplus_info, sizeof(BPLUS_INFO));

    BF_Block_SetDirty(block);
    BF_Block_Destroy(&block);
    return 0;
}


BPLUS_INFO* BP_OpenFile(char *fileName, int *file_desc) {
    CALL_BF(BF_OpenFile(fileName, file_desc));

    // Αρχικοποίηση και ανάγωση του πρώτου block (metadata block)
    BF_Block *block;
    init_and_get_block(*file_desc, 0, &block);

    // Δέσμευση μνήμης για τη δομή BPLUS_INFO
    BPLUS_INFO *bplus_info = malloc(sizeof(BPLUS_INFO));
    if (bplus_info == NULL) {
        finalizeBlock(block, false);
        CALL_BF(BF_CloseFile(*file_desc));
        return NULL;
    }


    // Αντιγραφή των πληροφοριών του B+ δέντρου από το block
    memcpy(bplus_info, BF_Block_GetData(block), sizeof(BPLUS_INFO));
    finalizeBlock(block, false);

    return bplus_info;
}


int BP_CloseFile(int file_desc,BPLUS_INFO* info){  
    // Αρχικοποίηση και αποδέσμευση του πρώτου block
    BF_Block *block;
    init_and_get_block(file_desc, 0, &block);
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);

    // Κλείσιμο του αρχείου και αποδέσμευση της μνήμης
    CALL_BF(BF_CloseFile(file_desc));
    free(info);
    return 0;
}

// Κόμβος για την υλοποίηση της στοίβας
struct snode {
    int blocks_num;
    struct snode *next;
};
typedef struct snode snode;


// Δομή στοίβας
typedef struct {
    snode *head;
    snode *tail;
    int size;
} stack;

// Δημιουργία στοίβας
stack *create() {
    stack *tmp = malloc(sizeof(stack));
    tmp->head = NULL;
    tmp->tail = NULL;
    tmp->size = 0;
    return tmp;
}

// Δημιουργία κόμβου
snode* create_node(int blocks_num) {
    snode* tmp = malloc(sizeof(snode));
    tmp->blocks_num = blocks_num;
    tmp->next = NULL;
    return tmp;
}

// Εισαγωγή στοιχείου στη στοίβα
void push(stack *s, int blocks_num) {
    snode *tmp = create_node(blocks_num);
    if (s->size == 0)
        s->head = tmp;
    else
        s->tail->next = tmp;

    s->tail = tmp;
    s->size++;
}

// Αφαίρεση στοιχείου από τη στοίβα
int pop(stack *s) {
    if (s->size == 0)
        return -1;

    snode *tmp = s->head;
    if (s->size == 1) {
        s->head = NULL;
        s->tail = NULL;
    } else {
        while (tmp->next != s->tail)
            tmp = tmp->next;

        s->tail = tmp;
        tmp = tmp->next;
        s->tail->next = NULL;
    }
    int bl_num = tmp->blocks_num;
    free(tmp);
    s->size--;
    return bl_num;
}

// Αποδέσμευση της στοίβας
void Destroystack(stack *s) {
    while (s->size > 0)
        pop(s);
    free(s);
}


void handle_root_split(int file_desc, BPLUS_INFO* bplus_info, int mid_record_id, int left_block_id, int right_block_id) {
    // Δημιουργία ενός νέου root block
    BF_Block* new_root;
    init_and_allocate_block(file_desc, &new_root);
    char* new_root_data = BF_Block_GetData(new_root);
    bplus_info->last_block_id++;

    // Αρχικοποίηση του root ως index node
    initialize_index_node(new_root_data, bplus_info->last_block_id, bplus_info->max_index_count);

    // Προσθήκη των παιδιών στον root
    BPLUS_INDEX_NODE* root = (BPLUS_INDEX_NODE*)new_root_data;
    root->keys[0] = mid_record_id;
    root->children[0] = left_block_id;
    root->children[1] = right_block_id;
    root->index_count = 1;

    // Αποθήκευση του root block
    finalizeBlock(new_root, true);

    // Ανανέωση των μεταδεδομένων του B+ δέντρου
    update_metadata_node(bplus_info, file_desc, bplus_info->last_block_id);
}


void propagate_split_upwards(stack* parent_stack, int mid_record_id,
                            int left_block_id, int right_block_id, int file_desc, BPLUS_INFO* bplus_info) {

    // Επαναφορά του stack
    while (parent_stack->size > 0) {
        int parent_block_id = pop(parent_stack);

        // Ανάγνωση του γονικού block
        BF_Block* block;
        BF_Block_Init(&block);
        CALL_BF(BF_GetBlock(file_desc, parent_block_id, block));
        char* block_data = BF_Block_GetData(block);
        BPLUS_INDEX_NODE* parent = (BPLUS_INDEX_NODE*)block_data;

        // Ελέγχουμε αν το γονικό block έχει χώρο για ένα ακόμα index
        if (parent->index_count < bplus_info->max_index_count) {
            add_index_to_node(parent, mid_record_id, right_block_id);
            finalizeBlock(block, true);
            return;
        }

        // Διαφορετικά, χωρίζουμε το γονικό block

        // Αρχικά, δημιουργούμε ένα νέο index node
        int new_mid_key;
        BF_Block* new_index_block;
        init_and_allocate_block(file_desc, &new_index_block);

        // Αρχικοποίηση του νέου index node
        char* new_index_data = BF_Block_GetData(new_index_block);
        initialize_index_node(new_index_data, bplus_info->last_block_id + 1, bplus_info->max_index_count);

        // Χωρίζουμε το γονικό block
        split_index_node(block_data, new_index_data, &new_mid_key, bplus_info->max_index_count);
        bplus_info->last_block_id++;

        BPLUS_INDEX_NODE* new_parent = (BPLUS_INDEX_NODE*)new_index_data;

        // Καθορίζουμε σε ποιον κόμβο θα προστεθεί το index
        if (mid_record_id > new_mid_key)
            add_index_to_node(new_parent, mid_record_id, right_block_id);
        else
            add_index_to_node(parent, mid_record_id, right_block_id);

        // Ανανεώνουμε τα παιδιά του γονικού block
        parent->children[parent->index_count] = left_block_id;
        new_parent->children[0] = right_block_id;

        // Αποθηκεύουμε τα block
        memcpy(BF_Block_GetData(block), parent, sizeof(BPLUS_INDEX_NODE));
        finalizeBlock(block, true);

        memcpy(BF_Block_GetData(new_index_block), new_parent, sizeof(BPLUS_INDEX_NODE));
        finalizeBlock(new_index_block, true);

        // Ενημέρωση του mid key για το επόμενο split
        mid_record_id = new_mid_key;
        right_block_id = bplus_info->last_block_id;
    }

    // Αν το stack είναι άδειο, προωθούμε το split στο root
    handle_root_split(file_desc, bplus_info, mid_record_id, left_block_id, right_block_id);
}



int BP_InsertEntry(int file_desc, BPLUS_INFO* bplus_info, Record record) {
    // Βρίσκουμε τον αριθμό των blocks του αρχείου
    int num_of_blocks = 0;
    CALL_BF(BF_GetBlockCounter(file_desc, &num_of_blocks));

    // Αν το δέντρο είναι άδειο (υπάρχει μόνο το metadata block)
    if (num_of_blocks == 1) {
        BF_Block* root;
        init_and_allocate_block(file_desc, &root);

        // Δημιουργία του root ως leaf node
        char* block_data = BF_Block_GetData(root);
        initialize_data_node(block_data, num_of_blocks, bplus_info->max_records_count);
        insert_record_to_data_node(block_data, record);

        // Ενημέρωση των μεταδεδομένων του B+ δέντρου
        bplus_info->last_block_id++;
        update_metadata_node(bplus_info, file_desc, num_of_blocks);

        finalizeBlock(root, true);
        return 0;
    }

    // Αν το δέντρο δεν είναι άδειο
    stack* stack = create();
    int current_block_id = bplus_info->root_block_id;
    BF_Block* block;
    BF_Block_Init(&block);

    // Αναζήτηση του κατάλληλου block για την εισαγωγή της εγγραφής (record)
    while (1) {
        CALL_BF(BF_GetBlock(file_desc, current_block_id, block));
        char* block_data = BF_Block_GetData(block);
        BPLUS_NODE_TYPE type = *((BPLUS_NODE_TYPE*)block_data);

        // Αν έχουμε φτάσει σε leaf node τερματίζουμε την αναζήτηση
        if (type == BPLUS_LEAF)
            break; 

        // Αν έχουμε φτάσει σε index node συνεχίζουμε την αναζήτηση
        push(stack, current_block_id);
        current_block_id = find_child_for_record(block_data, record.id);
    }

    // Προσπαθούμε να προσθέσουμε την εγγραφή στο leaf node
    char* block_data = BF_Block_GetData(block);
    BP_INSERT_STATUS status = insert_record_to_data_node(block_data, record);

    // Αν η εγγραφή προστέθηκε επιτυχώς τελειώνουμε την εισαγωγή
    if (status == BP_OK) {
        finalizeBlock(block, true);
        Destroystack(stack);
        return 0;
    }
    // Αν υπάρχει διπλότυπη εγγραφή τερματίζουμε την εισαγωγή
    else if (status == BP_LEAF_HAS_DUPLICATE) {
        fprintf(stderr, "Error: Record with id %d already exists\n", record.id);
        finalizeBlock(block, false);
        Destroystack(stack);
        return -1;
    }

    // Κρατάμε την εγγραφή που πρέπει να εισαχθεί στον γονικό κόμβο
    int mid_record_id;

    // Δημιουργία ενός νέου leaf node
    BF_Block* new_leaf_block;
    init_and_allocate_block(file_desc, &new_leaf_block);
    char* new_leaf_data = BF_Block_GetData(new_leaf_block);
    bplus_info->last_block_id++;

    // Χωρίζουμε το leaf node
    split_data_node(block_data, new_leaf_data, &mid_record_id, record, bplus_info->max_records_count, bplus_info->last_block_id);

    finalizeBlock(block, true);
    finalizeBlock(new_leaf_block, true);

    BPLUS_DATA_NODE* new_leaf = (BPLUS_DATA_NODE*)new_leaf_data;
    BPLUS_DATA_NODE* leaf = (BPLUS_DATA_NODE*)block_data;

    int left_block_id = leaf->block_id;
    int right_block_id = new_leaf->block_id;

    // Αν το root είναι το μόνο block του αρχείου ή όχι
    if (stack->size == 0)
        handle_root_split(file_desc, bplus_info, mid_record_id, left_block_id, right_block_id);
    else
        propagate_split_upwards(stack, mid_record_id, left_block_id, right_block_id, file_desc, bplus_info);

    Destroystack(stack);
    return 0;
}




int BP_GetEntry(int file_desc, BPLUS_INFO* header_info, int id, Record** result) {
    // Ξεκινάμε την αναζήτηση από το root block
    int current_block_id = header_info->root_block_id;
    BF_Block* block;
    BF_Block_Init(&block);

    while (1) {
        CALL_BF(BF_GetBlock(file_desc, current_block_id, block));
        char* block_data = BF_Block_GetData(block);

        BPLUS_NODE_TYPE type = *((BPLUS_NODE_TYPE*)block_data);
        // Αν έχουμε φτάσει σε index node συνεχίζουμε την αναζήτηση
        if (type == BPLUS_INDEX) {
            current_block_id = find_child_for_record(block_data, id);
            continue;
        }

        // Αν έχουμε φτάσει σε leaf node, τερματίζουμε την αναζήτηση και αναζητούμε την εγγραφή
        BPLUS_DATA_NODE* leaf = (BPLUS_DATA_NODE*)block_data;

        // Ψάχνουμε την θέση της εγγραφής στο leaf node
        int position = find_position_for_record(leaf, id);
        if (leaf->records[position].id == id)
            // Αν βρεθεί η εγγραφή την επιστρέφουμε
            *result = &(leaf->records[position]);
        else
            *result = NULL;

        break;
    }

    // Αποδέσμευση του block
    finalizeBlock(block, false);
    return *result == NULL ? -1 : 0;
}
