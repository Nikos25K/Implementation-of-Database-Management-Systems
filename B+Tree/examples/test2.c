// FILE: second/examples/test_test2.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "bf.h"
#include "bp_file.h"
#include "bp_datanode.h"
#include "bp_indexnode.h"

#define FILE_NAME "data.db"
#define RECORDS_NUM 200

void custom_exit(char* message){
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Το συγκεκριμένο πρόγραμμα δημιουργεί 200 τυχαίες εγγραφές και τις εισάγει στο αρχείο data.db
// Στη συνέχεια εμφανίζει τις εγγραφές αυτές σε αντίστροφη σειρά
// Αν κάποια εγγραφή δεν βρεθεί, το πρόγραμμα τερματίζεται (λόγω της custom_exit)

int main() {
    BF_Init(LRU);
    BP_CreateFile(FILE_NAME);
    int file_desc;
    BPLUS_INFO* info = BP_OpenFile(FILE_NAME, &file_desc);

    Record record;
    int check;
    int same_key = 0;

    for (int i = 0; i < RECORDS_NUM; i++) {
        record = randomRecord();
        record.id = i;
        check = BP_InsertEntry(file_desc, info, record);
        if (check != 0)
            same_key++;
    }

    int length_of_nulls = 0;
    Record* result;
    for (int i = 0; i < RECORDS_NUM; i++) {
        BP_GetEntry(file_desc, info, i, &result);
        if (result == NULL)
            length_of_nulls++;
    }

    if (length_of_nulls == same_key)
        printf("Success: All records with same id found\n");
    else
        custom_exit("Error: Some records with same id not found");

    printf("About to show all the records generated in reverse order\n");
    printf("If any of the BP_GetEntry calls fails, the program ends\n");
    sleep(10);

    for (int i = RECORDS_NUM -1; i >= 0 ; i--) {
        BP_GetEntry(file_desc, info, i, &result);
        if (result == NULL)
            custom_exit("Error: Record not found");
        else
            printRecord(*result);
    }

    BP_CloseFile(file_desc, info);
    BF_Close();

    return 0;
}