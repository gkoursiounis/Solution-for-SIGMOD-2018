#ifndef EXECUTE_QUERY_H
#define EXECUTE_QUERY_H

#include "relation.h"
#include "query.h"
#include "job_scheduler.h"

middleman *initialize_middleman(uint32_t number_of_tables);
int filter_middle_bucket(predicate_filter *filter,
                         middle_list_bucket *bucket,
                         table* table,
                         middle_list *new_list);
int filter_original_table(predicate_filter *filter,
                          table* table,
                          middle_list *new_list);
relation *construct_relation_from_table(table * table, uint64_t column_id);
void construct_relation_from_middleman(middle_list_bucket *bucket,
                                       table *table,
                                       relation *rel,
                                       uint64_t column_id,
                                       uint64_t *counter);
int update_middle_bucket(lookup_table*lookup, middle_list_bucket *bucket, middle_list *updated_list);
//int update_middle_bucket(middle_list_bucket **lookup, middle_list_bucket *bucket, middle_list *updated_list);
int self_join_table(predicate_join *join, table* table, middle_list *list);

int self_join_middle_bucket(predicate_join *join,
                            table *table_r,
                            table *table_s,
                            middle_list_bucket *bucket_r,
                            middle_list_bucket *bucket_s,
                            middle_list *list_r,
                            middle_list *list_s,
                            middle_list *index_list,
                            uint32_t *r_index,
                            uint32_t *s_index,
                            uint32_t *counter);

middleman *execute_query(query *q, table_index* index, bool *sorting);
void calculate_sum(projection p, middle_list_bucket *bucket, table *table, uint64_t *sum);
void calculate_projections(query *q, table_index* index, middleman *m);

int execute_query_parallel(job_query_parameters* p);
int update_related_lists(uint32_t predicate_index, query *q, uint32_t **concatenated_tables,
                         middleman *m,int delete_r, int delete_s,
                         middle_list *result_R,
                         middle_list *result_S, middle_list *index_list);

#endif // EXECUTE_QUERY_H
