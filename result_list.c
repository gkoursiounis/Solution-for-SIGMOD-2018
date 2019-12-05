#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "result_list.h"

/**
 * The Bucket inside a node of the result list.
 * Contains a 2d array of row ids (uint64_t) and an index (unsigned int)
 * to the next empty space in the array.
 */
typedef struct result_list_bucket
{
    uint64_t row_ids[RESULT_LIST_BUCKET_SIZE][2];
    unsigned int index_to_add_next;
} result_list_bucket;

/**
 * The node of the result list.
 * Contains a bucket with the row ids and a pointer to the next node.
 */
typedef struct result_list_node
{
    result_list_bucket bucket;
    result_list_node* next;
} result_list_node;

/**
 * The result list.
 * Contains pointers to the head and tail nodes (for O(1) append)
 * and a node counter.
 */
typedef struct result_list
{
    result_list_node* head; //The first node of the list
    result_list_node* tail; //The last node of the list
    unsigned int number_of_nodes; //Counter of the buckets;
} result_list;

/**
 * Creates and initializes a new empty result list node (with the bucket)
 * @return result_list_node* The new node
 */
struct result_list_node* create_result_list_node()
{
    //Create the node
    result_list_node* new_node;
    new_node=malloc(sizeof(result_list_node));
    if(new_node==NULL)
    {
        perror("result_list_node: error in malloc");
        return NULL;
    }
    //Initialize empty
    new_node->next=NULL;
    new_node->bucket.index_to_add_next=0;
    return new_node;
}

/**
 * Checks if the bucket given is full
 * @param result_list_bucket The bucket to check
 * @return int 1 if the bucket is full else 0
 */
int is_result_list_bucket_full(result_list_bucket* bucket)
{
    return bucket->index_to_add_next==RESULT_LIST_BUCKET_SIZE ? 1 : 0;
}

/**
 * Appends the rowids given to the bucket.
 * @param result_list_bucket* the bucket
 * @param uint64_t r_row_id
 * @param uint64_t s_row_id
 * @return 0 if successful 1 else
 */
int append_to_bucket(result_list_bucket* bucket, uint64_t r_row_id, uint64_t s_row_id)
{
    if(!is_result_list_bucket_full(bucket))
    {
        bucket->row_ids[bucket->index_to_add_next][ROWID_R_INDEX]=r_row_id;
        bucket->row_ids[bucket->index_to_add_next][ROWID_S_INDEX]=s_row_id;
        bucket->index_to_add_next++;
        return 0;
    }
    return 1;
}
/**
 * Prints the contents of the bucket (index and array)
 * @param result_list_bucket the bucket to print
 * @param FILE* Where the output will be printed
 */
void print_bucket(result_list_bucket* bucket,FILE*output)
{
    //Print the array inside the bucket
    //printf("Index to add next: %u\n", bucket->index_to_add_next);
    for(unsigned int i=0; i<bucket->index_to_add_next; i++)
    {
        fprintf(output,"RowIdR: %" PRIu64 " RowIdS: %" PRIu64 "\n", bucket->row_ids[i][ROWID_R_INDEX], bucket->row_ids[i][ROWID_S_INDEX]);
    }
}

result_list* create_result_list()
{
    //Create the list
    result_list* new_list;
    new_list=malloc(sizeof(result_list));
    if(new_list==NULL)
    {

        perror("create_result_list(): error in malloc");
        return NULL;
    }
    //Initialize the list to be empty
    new_list->head=NULL;
    new_list->tail=NULL;
    new_list->number_of_nodes=0;
    return new_list;
}

void delete_result_list(result_list* list)
{
    if(list==NULL)
    {
        printf("delete_result_list: NULL list pointer\n");
        return;
    }
    result_list_node* temp=list->head;
    //Delete all the nodes
    while(list->head!=NULL)
    {
        //printf("Nodes: %u\n", list->number_of_nodes);
        list->head=temp->next;
        free(temp);
        temp=list->head;
        list->number_of_nodes--;
        //printf("Node Deleted\n");
    }
    free(list);
    printf("List Deleted\n");
}

void print_result_list(result_list* list,FILE*output)
{
    //printf("%" PRIu64 "\n", sizeof(result_list));
    //printf("%" PRIu64 "\n", sizeof(result_list_node));
    //printf("%" PRIu64 "\n", sizeof(result_list_bucket));
    if(list==NULL)
    {
        fprintf(stderr,"print_result_list: NULL list pointer\n");
        return;
    }
    unsigned int index=0;
    result_list_node*temp=list->head;
    fprintf(output,"Number Of Records: %"PRIu64"\n",result_list_get_number_of_records(list));
    fprintf(output,"Number Of Buckets: %u\n", list->number_of_nodes);
    while(temp!=NULL)//Visit all the nodes and print them
    {
        fprintf(output,"Bucket Index: %u\n", index);
        print_bucket(&(temp->bucket),output);
        index++;
        temp=temp->next;
    }
}

int append_to_list(result_list* list, uint64_t r_row_id, uint64_t s_row_id)
{
    if(list->head==NULL)//Create the first node
    {
        list->head=create_result_list_node();
        if(list->head==NULL)
        {
            return 1;
        }
        //No need to check
        if(append_to_bucket(&list->head->bucket, r_row_id, s_row_id))
        {
            printf("append_to_list: Error cannot add to empty bucket\n");
            return 2;
        }
        list->tail=list->head;
        list->number_of_nodes++;
    }
    else//Add to the tail
    {
        if(list->tail==NULL||list->tail->next!=NULL)
        {
            printf("append_to_list: error of the list\n");
            return 3;
        }
        else
        {
            if(append_to_bucket(&list->tail->bucket, r_row_id, s_row_id))
            {//Full Bucket
                list->tail->next=create_result_list_node();
                //No Need To Check
                list->tail=list->tail->next;
                if(append_to_bucket(&list->tail->bucket, r_row_id, s_row_id))
                {
                    printf("append_to_list: Error cannot add to empty bucket\n");
                    return 4;
                }
                list->number_of_nodes++;
            }
        }
    }
    return 0;
}

int is_result_list_empty(result_list* list)
{
    //return list->Head==NULL ? 1 : 0;
    return list->number_of_nodes==0 ? 1 : 0;
}

unsigned int result_list_get_number_of_buckets(result_list* list)
{
    return list->number_of_nodes;
}

uint64_t result_list_get_number_of_records(result_list* list)
{
    if(list==NULL||list->number_of_nodes==0||list->tail==NULL)
    {
        return 0;
    }
    return ((list->number_of_nodes-1)*RESULT_LIST_BUCKET_SIZE+list->tail->bucket.index_to_add_next);
}
