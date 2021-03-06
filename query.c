#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "query.h"

query* create_query(void)
{
    query* q=NULL;
    q=malloc(sizeof(query));
    if(q==NULL)
    {
        perror("create_query: error in malloc");
        return NULL;
    }
    q->number_of_predicates=0;
    q->number_of_projections=0;
    q->number_of_tables=0;
    q->predicates=NULL;
    q->projections=NULL;
    q->table_ids=NULL;
    return q;
}

void delete_query(query*q)
{
    if(q==NULL)
    {
        fprintf(stderr, "delete_query: NULL query pointer\n");
        return;
    }
    if(q->number_of_predicates!=0&&q->predicates==NULL)
    {
        fprintf(stderr, "delete_query: NULL predicates pointer but counter !=0\n");
    }
    if(q->number_of_projections!=0&&q->projections==NULL)
    {
        fprintf(stderr, "delete_query: NULL projection pointer but counter !=0\n");
    }
    if(q->number_of_tables!=0&&q->table_ids==NULL)
    {
        fprintf(stderr, "delete_query: NULL table_ids pointer but counter !=0\n");
    }
    if(q->predicates!=NULL)
    {
        for(uint32_t i=0; i<q->number_of_predicates; i++)
        {
            if(q->predicates[i].p!=NULL)
            {
                free(q->predicates[i].p);
                q->predicates[i].p=NULL;
            }
        }
        free(q->predicates);
        q->predicates=NULL;
    }
    if(q->projections!=NULL)
    {
        free(q->projections);
        q->projections=NULL;
    }
    if(q->table_ids!=NULL)
    {
        free(q->table_ids);
        q->table_ids=NULL;
    }
    free(q);
    q=NULL;
}

/**
 * Parse the string representation of the predicate
 * @param char* The string representation of the predicate
 * @param predicate* Pointer to a predicate to write the data
 * @return 0 on succes
 */
int parse_predicate(char* token, predicate*p)
{
    if(token==NULL||p==NULL||token[0]=='\0')
    {
        fprintf(stderr, "parse_predicate: NULL parameter\n");
        return -1;
    }
    //Find the type of predicate by counting the dots
    uint32_t dots=0;
    predicate_filter_type p_f_type=Not_Specified;
    uint32_t i=0;
    while(token[i]!='\0')
    {
        if(token[i]=='.')
        {
            dots++;
        }
        else if(token[i]=='=')
        {
            if(p_f_type==Not_Specified)
            {
                p_f_type=Equal;
            }
            else if(p_f_type==Less)
            {
                p_f_type=Less_Equal;
            }
            else if(p_f_type==Greater)
            {
                p_f_type=Greater_Equal;
            }
            else
            {
                fprintf(stderr, "analyze_query: wrong operation symbol %s\n", token);
                return -2;
            }
        }
        else if(token[i]=='<')
        {
            if(p_f_type==Not_Specified)
            {
                p_f_type=Less;
            }
            else
            {
                fprintf(stderr, "analyze_query: wrong operation symbol %s\n", token);
                return -3;
            }
        }
        else if(token[i]=='>')
        {
            if(p_f_type==Not_Specified)
            {
                p_f_type=Greater;
            }
            else if(p_f_type==Less)
            {
                p_f_type=Not_Equal;
            }
            else
            {
                fprintf(stderr, "analyze_query: wrong operation symbol %s\n", token);
                return -4;
            }
        }
        i++;
    }
    if(dots==2&&p_f_type==Equal)
    {//Join predicate
        p->type=Join;
        p->p=malloc(sizeof(predicate_join));
        if(p->p==NULL)
        {
            perror("parse_predicate: predicate join malloc error\n");
            return -5;
        }
        if(sscanf(token, "%"PRIu32".%"PRIu64"=%"PRIu32".%"PRIu64, &(((predicate_join*) p->p)->r.table_id), &(((predicate_join*) p->p)->r.column_id), &(((predicate_join*) p->p)->s.table_id), &(((predicate_join*) p->p)->s.column_id))!=4)
        {
            fprintf(stderr, "parse_predicate: predicate join sscanf error %s\n", token);
            return -6;
        }
    }
    else if(dots==1)
    {
        p->type=Filter;
        p->p=malloc(sizeof(predicate_filter));
        if(p->p==NULL)
        {
            perror("parse_predicate: predicate filter malloc error\n");
            return -7;
        }
        ((predicate_filter*) p->p)->filter_type=p_f_type;
        if(((predicate_filter*) p->p)->filter_type==Less)
        {
            if(sscanf(token, "%"PRIu32".%"PRIu64"<%"PRIu64, &((predicate_filter*) p->p)->r.table_id, &((predicate_filter*) p->p)->r.column_id, &((predicate_filter*) p->p)->value)!=3)
            {
                fprintf(stderr, "parse_predicate: predicate join sscanf error %s\n", token);
                return -8;
            }
        }
        else if(((predicate_filter*) p->p)->filter_type==Less_Equal)
        {
            if(sscanf(token, "%"PRIu32".%"PRIu64"<=%"PRIu64, &((predicate_filter*) p->p)->r.table_id, &((predicate_filter*) p->p)->r.column_id, &((predicate_filter*) p->p)->value)!=3)
            {
                fprintf(stderr, "parse_predicate: predicate join sscanf error %s\n", token);
                return -9;
            }
        }
        else if(((predicate_filter*) p->p)->filter_type==Equal)
        {
            if(sscanf(token, "%"PRIu32".%"PRIu64"=%"PRIu64, &((predicate_filter*) p->p)->r.table_id, &((predicate_filter*) p->p)->r.column_id, &((predicate_filter*) p->p)->value)!=3)
            {
                fprintf(stderr, "parse_predicate: predicate join sscanf error %s\n", token);
                return -10;
            }
        }
        else if(((predicate_filter*) p->p)->filter_type==Not_Equal)
        {
            if(sscanf(token, "%"PRIu32".%"PRIu64"<>%"PRIu64, &((predicate_filter*) p->p)->r.table_id, &((predicate_filter*) p->p)->r.column_id, &((predicate_filter*) p->p)->value)!=3)
            {
                fprintf(stderr, "parse_predicate: predicate join sscanf error %s\n", token);
                return -11;
            }
        }
        else if(((predicate_filter*) p->p)->filter_type==Greater)
        {
            if(sscanf(token, "%"PRIu32".%"PRIu64">%"PRIu64, &((predicate_filter*) p->p)->r.table_id, &((predicate_filter*) p->p)->r.column_id, &((predicate_filter*) p->p)->value)!=3)
            {
                fprintf(stderr, "parse_predicate: predicate join sscanf error %s\n", token);
                return -12;
            }
        }
        else if(((predicate_filter*) p->p)->filter_type==Greater_Equal)
        {
            if(sscanf(token, "%"PRIu32".%"PRIu64">=%"PRIu64, &((predicate_filter*) p->p)->r.table_id, &((predicate_filter*) p->p)->r.column_id, &((predicate_filter*) p->p)->value)!=3)
            {
                fprintf(stderr, "parse_predicate: predicate join sscanf error %s\n", token);
                return -13;
            }
        }
        else
        {
            fprintf(stderr, "parse_predicate: predicate wrong format %s\n", token);
            return -14;
        }
    }
    else
    {
        fprintf(stderr, "parse_predicate: predicate format error %s\n", token);
        return -15;
    }
    return 0;
}

/**
 * Removes the delimiter characters at the start/end of the str
 * and then the duplicates inside the str
 * Example:(Input "---This--is----a-test----",'-', Result"This-is-a-test")
 * @param char* The str
 * @param char The delimiter character
 */
void remove_extra_chars(char* str, char delimiter)
{
    if(str==NULL)
    {
        return;
    }
    size_t i=0;
    size_t j=0;
    //Remove leading spaces
    while(str[i]!='\0'&&str[i]==delimiter)
    {
        i++;
    }
    while(str[j+i]!='\0')
    {
        str[j]=str[j+i];
        j++;
    }
    str[j]=str[j+i];
    //Remove trailing spaces
    i=0;
    j=0;
    size_t len=strlen(str);
    if(len>0)
    {
        size_t end_index=len-1;
        while(i<len)
        {
            if(str[end_index-i]!=delimiter)
            {
                break;
            }
            i++;
        }
        str[end_index-i+1]='\0';
    }
    //Remove duplicate spaces
    i=0;
    j=0;
    int space=0;
    while(str[i]!='\0')
    {
        if(str[i]!=delimiter)
        {
            str[j]=str[i];
            j++;
            space=0;
        }
        else if(space==0)
        {
            str[j]=str[i];
            j++;
            space++;
        }
        else
        {
            space++;
        }
        i++;
    }
    str[j]='\0';
}

int analyze_query(char*query_str, query*q)
{
    if(query_str==NULL)
    {
        fprintf(stderr, "analyze_query: NULL query str parameter\n");
        return -1;
    }
    if(q==NULL)
    {
        fprintf(stderr, "analyze_query: NULL query pointer parameter\n");
        return -2;
    }
    if(q->number_of_predicates!=0||q->number_of_projections!=0||q->number_of_tables!=0||
       q->predicates!=NULL||q->projections!=NULL||q->table_ids!=NULL)
    {
        fprintf(stderr, "analyze_query: NULL query parameter not empty\n");
        return -3;
    }
    //Check for illegal characters and for 2 |
    short pipe_counter=0;
    for(uint32_t i=0; query_str[i]!='\0'; i++)
    {
        if(query_str[i]<'0'||query_str[i]>'9')//Not a digit
        {
            if(query_str[i]!='&'&&query_str[i]!='.'&&query_str[i]!=' '&&query_str[i]!='>'&&query_str[i]!='<'&&query_str[i]!='=')
            {
                if(query_str[i]=='|')
                {
                    pipe_counter++;
                }
                else
                {
                    fprintf(stderr, "analyze_query: illegal character: %c\n", query_str[i]);
                    return -4;
                }
            }
        }
    }
    if(pipe_counter!=2)
    {
        fprintf(stderr, "analyze_query: wrong number of \'|\' expected: 2 gived: %hd \n", pipe_counter);
        return -5;
    }
    //Tokenize the string with delimiter the |
    char *save_query_ptr;
    char* token=strtok_r(query_str, "|", &save_query_ptr);
    int i=0;
    if(token==NULL)
    {
        fprintf(stderr, "analyze_query: error with strtok_r table token\n");
        return -6;
    }
    else
    {
        //Parse tables
        uint32_t spaces=0;
        //Remove the extra spaces
        remove_extra_chars(token, ' ');
        //Check for illegal characters in the table token
        for(uint32_t i=0; token[i]!='\0'; i++)
        {
            if(token[i]<'0'||token[i]>'9')
            {//Not a digit check if space
                if(token[i]==' ')
                {
                    spaces++;
                }
                else
                {
                    fprintf(stderr, "analyze_query: illegal character in tables token: %c\n", token[i]);
                    return -7;
                }
            }
        }
        if(token[0]=='\0')
        {
            fprintf(stderr, "analyze_query: missing numbers at the tables token%s\n", token);
            return -8;
        }
        if(spaces==0)
        {//Only a table parse the number
            q->number_of_tables=1;
            q->table_ids=malloc(sizeof(uint32_t));
            if(q->table_ids==NULL)
            {
                perror("analyze_query: table malloc error\n");
                return -9;
            }
            if(sscanf(token, "%"PRIu32"", &q->table_ids[0])!=1)
            {
                fprintf(stderr, "analyze_query: tables sscanf error %s\n", token);
                return -10;
            }
        }
        else
        {//Tokenize the string with delimiter the ' '
            q->number_of_tables=spaces+1;
            q->table_ids=malloc(sizeof(uint32_t)*q->number_of_tables);
            if(q->table_ids==NULL)
            {
                perror("analyze_query: table malloc error\n");
                return -11;
            }
            //Tokenize the string with delimiter the |
            char *save_query_ptr2;
            char* subtoken=strtok_r(token, " ", &save_query_ptr2);
            int j=0;
            while(subtoken!=NULL)
            {
                if(sscanf(subtoken, "%"PRIu32"", &q->table_ids[j])!=1)
                {
                    fprintf(stderr, "analyze_query: tables sscanf error %s\n", subtoken);
                    return -12;
                }
                j++;
                subtoken=strtok_r(NULL, " ", &save_query_ptr2);
            }
        }
    }
    //Get predicates token
    token=strtok_r(NULL, "|", &save_query_ptr);
    i++;
    if(token==NULL)
    {
        fprintf(stderr, "analyze_query: error with strtok_r predicate token\n");
        return -13;
    }
    else
    {
        //Parse predicates
        //Remove the extra spaces
        remove_extra_chars(token, ' ');
        size_t token_size=strlen(token)+1;
        if(token_size==0)
        {
            fprintf(stderr, "analyze_query: token_size == 0\n");
            return -14;
        }
        char *token_copy=malloc(sizeof(char)*token_size);
        if(token_copy==NULL)
        {
            perror("analyze_query: char* malloc error\n");
            return -14;
        }
        strncpy(token_copy, token, token_size);
        remove_extra_chars(token_copy, '&');
        if(strncmp(token_copy, token, token_size)!=0)
        {
            fprintf(stderr, "analyze_query: predicate error with & %s\n", token);
            free(token_copy);
            return -13;
        }
        free(token_copy);
        token_copy=NULL;
        bool number=false;
        bool dot=false;
        bool seperator=false;
        bool operation_symbol=false;
        uint32_t predicate_counter=0;
        //Check for illegal characters in the predicate token
        for(uint32_t i=0; token[i]!='\0'; i++)
        {
            if(token[i]<'0'||token[i]>'9')
            {//Not a digit check if dot or & or <, =, >
                if(token[i]=='.')
                {
                    dot=true;
                }
                else if(token[i]=='&')
                {
                    seperator=true;
                    predicate_counter++;
                }
                else if(token[i]=='<')
                {
                    operation_symbol=true;
                }
                else if(token[i]=='=')
                {
                    operation_symbol=true;
                }
                else if(token[i]=='>')
                {
                    operation_symbol=true;
                }
                else
                {
                    fprintf(stderr, "analyze_query: illegal character in predicate token: %c\n", token[i]);
                    return -14;
                }
            }
            else
            {
                number=true;
            }
        }
        if(token[0]=='\0')
        {//empty predicate
            fprintf(stderr, "analyze_query: Empty Predicate\n");
            return -15;
        }
        else if(number&&dot&&operation_symbol)
        {
            if(seperator)
            {//Greater than one predicates
                q->number_of_predicates=predicate_counter+1;
                q->predicates=malloc(sizeof(predicate)*q->number_of_predicates);
                if(q->predicates==NULL)
                {
                    perror("analyze_query: predicate malloc error\n");
                    return -16;
                }
                for(uint32_t i=0; i<q->number_of_predicates; i++)
                {
                    q->predicates[i].p=NULL;
                }
                //Tokenize the string with delimiter the '&'
                char *save_query_ptr2;
                char* subtoken=strtok_r(token, "&", &save_query_ptr2);
                int j=0;
                while(subtoken!=NULL)
                {
                    if(parse_predicate(subtoken, &q->predicates[j])!=0)
                    {
                        return -17;
                    }
                    j++;
                    subtoken=strtok_r(NULL, "&", &save_query_ptr2);
                }
            }
            else
            {//One predicate
                q->number_of_predicates=1;
                q->predicates=malloc(sizeof(predicate));
                if(q->predicates==NULL)
                {
                    perror("analyze_query: predicate malloc error\n");
                    return -18;
                }
                q->predicates[0].p=NULL;
                //Find the type of predicate by counting the dots
                if(parse_predicate(token, &q->predicates[0])!=0)
                {
                    return -19;
                }
            }
        }
        else
        {
            fprintf(stderr, "analyze_query: predicate token has wrong format%s\n", token);
            return -20;
        }
    }
    //Get the pjojection token
    token=strtok_r(NULL, "|", &save_query_ptr);
    i++;
    if(token==NULL)
    {
        fprintf(stderr, "analyze_query: error with strtok_r projection token\n");
        return -21;
    }
    else
    {
        //Parse projections
        uint32_t spaces=0;
        uint32_t dots=0;
        uint32_t numbers=0;
        //Remove the extra spaces
        remove_extra_chars(token, ' ');
        //Check for illegal characters in the table token
        for(uint32_t i=0; token[i]!='\0'; i++)
        {
            if(token[i]<'0'||token[i]>'9')
            {//Not a digit check if space of dot
                if(token[i]==' ')
                {
                    spaces++;
                }
                else if(token[i]=='.')
                {
                    dots++;
                }
                else
                {
                    fprintf(stderr, "analyze_query: illegal character in projection token: %c\n", token[i]);
                    return -22;
                }
            }
            else
            {
                numbers++;
            }
        }
        if(numbers==0)
        {
            fprintf(stderr, "analyze_query: missing numbers at the projection token %s\n", token);
            return -23;
        }
        if(spaces==0)
        {//Only a table parse the number
            q->number_of_projections=1;
            q->projections=malloc(sizeof(projection));
            if(q->projections==NULL)
            {
                perror("analyze_query: projections malloc error\n");
                return -24;
            }
            if(sscanf(token, "%"PRIu32".%"PRIu64"", &q->projections[0].column_to_project.table_id, &q->projections[0].column_to_project.column_id)!=2)
            {
                fprintf(stderr, "analyze_query: projections sscanf error %s\n", token);
                return -25;
            }
        }
        else if(dots==spaces+1)
        {//Tokenize the string with delimiter the ' '
            q->number_of_projections=dots;
            q->projections=malloc(sizeof(projection)*q->number_of_projections);
            if(q->projections==NULL)
            {
                perror("analyze_query: projections malloc error\n");
                return -26;
            }
            //Tokenize the string with delimiter the ' '
            char *save_query_ptr2;
            char* subtoken=strtok_r(token, " ", &save_query_ptr2);
            int j=0;
            while(subtoken!=NULL)
            {
                if(sscanf(subtoken, "%"PRIu32".%"PRIu64"", &q->projections[j].column_to_project.table_id, &q->projections[j].column_to_project.column_id)!=2)
                {
                    fprintf(stderr, "analyze_query: projection sscanf error %s\n", subtoken);
                    return -27;
                }
                j++;
                subtoken=strtok_r(NULL, " ", &save_query_ptr2);
            }
        }
        else
        {
            fprintf(stderr, "analyze_query: projections format error %s\n", token);
            return -28;
        }
    }
    return 0;
}

/**
 * Prints the table id and column number
 * @param Pointer to a table_column
 */
void print_table_column(table_column* tc)
{
    if(tc!=NULL)
    {
        printf("Table Index: %"PRIu32"\n", tc->table_id);
        printf("Column Index: %"PRIu64"\n", tc->column_id);
    }
    else
    {
        fprintf(stderr, "print_table_column: NULL Parameter\n");
    }
}

/**
 * Prints the predicate type
 * @param predicate_type
 */
void print_predicate_type(predicate_type pt)
{
    printf("Predicate Type: ");
    if(pt==Filter)
    {
        printf("Filter");
    }
    else if(pt==Join)
    {
        printf("Join");
    }
    else if(pt==Self_Join)
    {
        printf("Self Join");
    }
    else
    {
        printf("ERROR");
    }
    printf("\n");
}

/**
 * Prints the predicate filter type
 * @param predicate_filter_type
 */
void print_predicate_filter_type(predicate_filter_type ptf)
{
    printf("Predicate Filter Type: ");
    if(ptf==Not_Specified)
    {
        printf("Not Specified");
    }
    else if(ptf==Less)
    {
        printf("Less (<)");
    }
    else if(ptf==Less_Equal)
    {
        printf("Less Equal (<=)");
    }
    else if(ptf==Equal)
    {
        printf("Equal (=)");
    }
    else if(ptf==Not_Equal)
    {
        printf("Not Equal (<>)");
    }
    else if(ptf==Greater)
    {
        printf("Greater (>)");
    }
    else if(ptf==Greater_Equal)
    {
        printf("Greater Equal (>=)");
    }
    printf("\n");
}

/**
 * Prints the predicate join data
 * @param predicate_join*
 */
void print_predicate_join(predicate_join* pj)
{
    if(pj!=NULL)
    {
        printf("R:\n");
        print_table_column(&(pj->r));
        printf("S:\n");
        print_table_column(&(pj->s));
    }
    else
    {
        fprintf(stderr, "print_predicate_join: NULL Parameter\n");
    }
}

/**
 * Prints the predicate_filter data
 * @param predicate_filter*
 */
void print_predicate_filter(predicate_filter* pf)
{
    if(pf!=NULL)
    {
        print_predicate_filter_type(pf->filter_type);
        printf("R:\n");
        print_table_column(&(pf->r));
        printf("Value: %"PRIu64"\n", pf->value);
    }
    else
    {
        fprintf(stderr, "print_predicate_filter: NULL Parameter\n");
    }
}

/**
 * Prints the predicate data
 * @param predicate*
 */
void print_predicate(predicate* p)
{
    if(p!=NULL)
    {
        print_predicate_type(p->type);
        if(p->type==Filter)
        {
            print_predicate_filter((predicate_filter*) p->p);
        }
        else if(p->type==Join||p->type==Self_Join)
        {
            print_predicate_join((predicate_join*) p->p);
        }
        else
        {
            printf("Empty Predicate Type\n");
        }
    }
    else
    {
        fprintf(stderr, "print_predicate: NULL Parameter\n");
    }
}

/**
 * Prints the projection data
 * @param projection*
 */
void print_projection(projection* p)
{
    if(p!=NULL)
    {
        print_table_column(&(p->column_to_project));
    }
    else
    {
        fprintf(stderr, "print_projection: NULL Parameter\n");
    }
}

void print_query(query* q)
{
    if(q!=NULL)
    {
        printf("Number of tables: %"PRIu32"\n", q->number_of_tables);
        if(q->table_ids!=NULL)
        {
            for(uint32_t i=0; i<q->number_of_tables; i++)
            {
                printf("Table ID: %"PRIu32": %"PRIu32"\n", i, q->table_ids[i]);
            }
        }
        printf("Number of predicates: %"PRIu32"\n", q->number_of_predicates);
        if(q->predicates!=NULL)
        {
            for(uint32_t i=0; i<q->number_of_predicates; i++)
            {
                print_predicate(&q->predicates[i]);
            }
        }
        printf("Number of projections: %"PRIu32"\n", q->number_of_projections);
        if(q->projections!=NULL)
        {
            for(uint32_t i=0; i<q->number_of_projections; i++)
            {
                print_projection(&q->projections[i]);
            }
        }
    }
    else
    {
        fprintf(stderr, "print_query: NULL Parameter\n");
    }
}

void print_query_like_an_str(query* q)
{
    if(q==NULL||q->number_of_tables==0||q->number_of_predicates==0||
       q->number_of_projections==0||q->table_ids==NULL||q->predicates==NULL||
       q->projections==NULL)
    {
        fprintf(stderr, "print_query_like_an_str: Error with the query\n");
        return;
    }
    for(uint32_t i=0; i<q->number_of_tables; i++)
    {
        printf(" %"PRIu32, q->table_ids[i]);
    }
    printf("|");
    for(uint32_t i=0; i<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Filter)
        {
            printf("%"PRIu32".%"PRIu64, ((predicate_filter*) (q->predicates[i].p))->r.table_id, ((predicate_filter*) (q->predicates[i].p))->r.column_id);
            if(((predicate_filter*) (q->predicates[i].p))->filter_type==Less)
            {
                printf("<");
            }
            else if(((predicate_filter*) (q->predicates[i].p))->filter_type==Less_Equal)
            {
                printf("<=");
            }
            else if(((predicate_filter*) (q->predicates[i].p))->filter_type==Equal)
            {
                printf("=");
            }
            else if(((predicate_filter*) (q->predicates[i].p))->filter_type==Not_Equal)
            {
                printf("<>");
            }
            else if(((predicate_filter*) (q->predicates[i].p))->filter_type==Greater)
            {
                printf(">");
            }
            else if(((predicate_filter*) (q->predicates[i].p))->filter_type==Greater_Equal)
            {
                printf(">=");
            }
            else
            {
                printf("Empty Predicate Filter Type\n");
                return;
            }
            printf("%"PRIu64, ((predicate_filter*) (q->predicates[i].p))->value);
        }
        else if(q->predicates[i].type==Join||q->predicates[i].type==Self_Join)
        {
            printf("%"PRIu32".%"PRIu64"=%"PRIu32".%"PRIu64, ((predicate_join*) (q->predicates[i].p))->r.table_id, ((predicate_join*) (q->predicates[i].p))->r.column_id,
                   ((predicate_join*) (q->predicates[i].p))->s.table_id, ((predicate_join*) (q->predicates[i].p))->s.column_id);
        }
        else
        {
            printf("Empty Predicate Type\n");
            return;
        }
        if(i+1<q->number_of_predicates)
        {
            printf("&");
        }
    }
    printf("|");
    for(uint32_t i=0; i<q->number_of_projections; i++)
    {
        printf("%"PRIu32".%"PRIu64" ", q->projections[i].column_to_project.table_id, q->projections[i].column_to_project.column_id);
    }
    printf("\n");
}

/**
 * Compares the contents of the two predicates and returns true if are the same
 * @param predicate*
 * @param predicate*
 * @return bool true if equal
 */
bool compare_predicates(predicate* p1, predicate* p2)
{
    if(p1==NULL||p1->p==NULL||p2==NULL||p2->p==NULL)
    {
        fprintf(stderr, "compare_predicates: Error with the parameters\n");
        return false;
    }
    if(p1->type!=p2->type)
    {
        return false;
    }
    if(p1->type==Join||p1->type==Self_Join)
    {
        if((((predicate_join*) (p1->p))->r.table_id==((predicate_join*) (p2->p))->r.table_id&&
            ((predicate_join*) (p1->p))->r.column_id==((predicate_join*) (p2->p))->r.column_id&&
            ((predicate_join*) (p1->p))->s.table_id==((predicate_join*) (p2->p))->s.table_id&&
            ((predicate_join*) (p1->p))->s.column_id==((predicate_join*) (p2->p))->s.column_id)||
           (((predicate_join*) (p1->p))->r.table_id==((predicate_join*) (p2->p))->s.table_id&&
            ((predicate_join*) (p1->p))->r.column_id==((predicate_join*) (p2->p))->s.column_id&&
            ((predicate_join*) (p1->p))->s.table_id==((predicate_join*) (p2->p))->r.table_id&&
            ((predicate_join*) (p1->p))->s.column_id==((predicate_join*) (p2->p))->r.column_id))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(((predicate_filter*) (p1->p))->r.table_id!=((predicate_filter*) (p2->p))->r.table_id||
           ((predicate_filter*) (p1->p))->r.column_id!=((predicate_filter*) (p2->p))->r.column_id||
           ((predicate_filter*) (p1->p))->filter_type!=((predicate_filter*) (p2->p))->filter_type||
           ((predicate_filter*) (p1->p))->value!=((predicate_filter*) (p2->p))->value)
        {
            return false;
        }
    }
    return true;
}

/**
 * Swaps the two predicates
 * @param predicate*
 * @param predicate*
 */
void swap_predicates(predicate* p1, predicate* p2)
{
    predicate t=*p1;
    *p1=*p2;
    *p2=t;
}

int validate_query(query*q, table_index* ti)
{
    //Check the parameters
    if(q==NULL||ti==NULL||ti->num_tables==0||ti->tables==NULL||
       q->number_of_tables==0||q->number_of_predicates==0||
       q->number_of_projections==0||q->table_ids==NULL||q->predicates==NULL||
       q->projections==NULL)
    {
        fprintf(stderr, "validate_query: Error with the parameters\n");
        return -1;
    }
    //Add two arrays one uint32_t and one bool
    uint32_t* table_indexes=malloc(sizeof(uint32_t)*q->number_of_tables);
    if(table_indexes==NULL)
    {
        perror("validate_query: table_indexes malloc error");
        return -2;
    }
    bool* table_used=malloc(sizeof(bool)*q->number_of_tables);
    if(table_used==NULL)
    {
        free(table_indexes);
        perror("validate_query: table_indexes malloc error");
        return -3;
    }
    for(uint32_t i=0; i<q->number_of_tables; i++)
    {
        table_indexes[i]=i;
        table_used[i]=false;
    }
    //Verify the table ids and check for duplicates
    for(uint32_t i=0; i<q->number_of_tables; i++)
    {
        if(get_table(ti, q->table_ids[i])==NULL)
        {
            free(table_indexes);
            free(table_used);
            fprintf(stderr, "validate_query: Table index: %"PRIu32" not in tables\n", q->table_ids[i]);
            return -4;
        }
        //Check for duplicates
        //        if(table_indexes[i]==i)//Not a duplicate
        //        {
        //            for(uint32_t j=i+1; j<q->number_of_tables; j++)
        //            {
        //                if(q->table_ids[i]==q->table_ids[j])
        //                {//Duplicate table Ids
        //                    printf("Duplicate table ids\n");
        //                    table_indexes[j]=i;
        //                }
        //            }
        //        }
    }
    //Verify the predicates
    for(uint32_t i=0; i<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Join)
        {
            //Check if the r table id exists
            if(q->number_of_tables<=((predicate_join*) q->predicates[i].p)->r.table_id)
            {
                free(table_indexes);
                free(table_used);
                fprintf(stderr, "validate_query: Table r id in predicate join not in the table array\n");
                return -5;
            }
            //Change the table r index if duplicate
            if(((predicate_join*) q->predicates[i].p)->r.table_id!=table_indexes[((predicate_join*) q->predicates[i].p)->r.table_id])
            {
                ((predicate_join*) q->predicates[i].p)->r.table_id=table_indexes[((predicate_join*) q->predicates[i].p)->r.table_id];
            }
            table_used[((predicate_join*) q->predicates[i].p)->r.table_id]=true;
            //Check if the r column exists
            if(((predicate_join*) q->predicates[i].p)->r.column_id>=get_table(ti, q->table_ids[((predicate_join*) q->predicates[i].p)->r.table_id])->columns)
            {
                free(table_indexes);
                free(table_used);
                fprintf(stderr, "validate_query: Table r column in predicate join does not exist\n");
                return -6;
            }
            //Check if the s table id exists
            if(q->number_of_tables<=((predicate_join*) q->predicates[i].p)->s.table_id)
            {
                free(table_indexes);
                free(table_used);
                fprintf(stderr, "validate_query: Table s id in predicate join not in the table array\n");
                return -7;
            }
            //Change the table s index if duplicate
            if(((predicate_join*) q->predicates[i].p)->s.table_id!=table_indexes[((predicate_join*) q->predicates[i].p)->s.table_id])
            {
                ((predicate_join*) q->predicates[i].p)->s.table_id=table_indexes[((predicate_join*) q->predicates[i].p)->s.table_id];
            }
            table_used[((predicate_join*) q->predicates[i].p)->s.table_id]=true;
            //Check if the s column exists
            if(((predicate_join*) q->predicates[i].p)->s.column_id>=get_table(ti, q->table_ids[((predicate_join*) q->predicates[i].p)->s.table_id])->columns)
            {
                free(table_indexes);
                free(table_used);
                fprintf(stderr, "validate_query: Table s column in predicate join does not exist\n");
                return -8;
            }
            //Check if it is a self join
            if((((predicate_join*) q->predicates[i].p)->r.table_id)==((predicate_join*) q->predicates[i].p)->s.table_id)
            {
                q->predicates[i].type=Self_Join;
                //Check if the columns id are equal
                if((((predicate_join*) q->predicates[i].p)->r.column_id)==((predicate_join*) q->predicates[i].p)->s.column_id)
                {
                    if(i!=q->number_of_predicates)
                    {
                        swap_predicates(&q->predicates[i], &q->predicates[q->number_of_predicates-1]);
                    }
                    free(q->predicates[q->number_of_predicates-1].p);
                    q->predicates[q->number_of_predicates-1].p=NULL;
                    q->number_of_predicates--;
                    i--;
                }
            }
        }
        else if(q->predicates[i].type==Filter)
        {
            //Check if the r table id exists
            if(q->number_of_tables<=((predicate_filter*) q->predicates[i].p)->r.table_id)
            {
                free(table_indexes);
                free(table_used);
                fprintf(stderr, "validate_query: Table r id in predicate filter not in the table array\n");
                return -9;
            }
            //Change the table r index if duplicate
            if(((predicate_filter*) q->predicates[i].p)->r.table_id!=table_indexes[((predicate_filter*) q->predicates[i].p)->r.table_id])
            {
                ((predicate_filter*) q->predicates[i].p)->r.table_id=table_indexes[((predicate_filter*) q->predicates[i].p)->r.table_id];
            }
            table_used[((predicate_filter*) q->predicates[i].p)->r.table_id]=true;
            //Check if the r column exists
            if(((predicate_filter*) q->predicates[i].p)->r.column_id>=get_table(ti, q->table_ids[((predicate_filter*) q->predicates[i].p)->r.table_id])->columns)
            {
                free(table_indexes);
                free(table_used);
                fprintf(stderr, "validate_query: Table r column in predicate filter does not exist\n");
                return -10;
            }
        }
    }
    //Verify the projections
    for(uint32_t i=0; i<q->number_of_projections; i++)
    {
        if(q->number_of_tables<=q->projections[i].column_to_project.table_id)
        {
            free(table_indexes);
            free(table_used);
            fprintf(stderr, "validate_query: projection table id not in the table array\n");
            return -11;
        }
        //Change the table index if duplicate
        if(q->projections[i].column_to_project.table_id!=table_indexes[q->projections[i].column_to_project.table_id])
        {
            q->projections[i].column_to_project.table_id=table_indexes[q->projections[i].column_to_project.table_id];
        }
        table_used[q->projections[i].column_to_project.table_id]=true;
        if(q->projections[i].column_to_project.column_id>=get_table(ti, q->table_ids[q->projections[i].column_to_project.table_id])->columns)
        {
            free(table_indexes);
            free(table_used);
            fprintf(stderr, "validate_query: projection column does not exist\n");
            return -12;
        }
    }
    //Check if all the tables are used
    //If not remove them and change the indexes in the predicates and projections if needed
    for(uint32_t i=0; i<q->number_of_tables; i++)
    {
        if(table_used[i]==false)//Table not used
        {
            for(uint32_t j=i+1; j<q->number_of_tables; j++)
            {
                if(table_indexes[i]<=table_indexes[j]&&table_indexes[j]>0)
                {
                    table_indexes[j]--;
                }
            }
        }
    }
    //Change the predicates and the projections
    for(uint32_t i=0; i<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Join||q->predicates[i].type==Self_Join)
        {
            //Change the table r index if needed
            if(((predicate_join*) q->predicates[i].p)->r.table_id!=table_indexes[((predicate_join*) q->predicates[i].p)->r.table_id])
            {
                ((predicate_join*) q->predicates[i].p)->r.table_id=table_indexes[((predicate_join*) q->predicates[i].p)->r.table_id];
            }
            //Change the table s index if needed
            if(((predicate_join*) q->predicates[i].p)->s.table_id!=table_indexes[((predicate_join*) q->predicates[i].p)->s.table_id])
            {
                ((predicate_join*) q->predicates[i].p)->s.table_id=table_indexes[((predicate_join*) q->predicates[i].p)->s.table_id];
            }
        }
        else if(q->predicates[i].type==Filter)
        {
            //Change the table r index if needed
            if(((predicate_filter*) q->predicates[i].p)->r.table_id!=table_indexes[((predicate_filter*) q->predicates[i].p)->r.table_id])
            {
                ((predicate_filter*) q->predicates[i].p)->r.table_id=table_indexes[((predicate_filter*) q->predicates[i].p)->r.table_id];
            }
        }
    }
    for(uint32_t i=0; i<q->number_of_projections; i++)
    {
        //Change the table index if needed
        if(q->projections[i].column_to_project.table_id!=table_indexes[q->projections[i].column_to_project.table_id])
        {
            q->projections[i].column_to_project.table_id=table_indexes[q->projections[i].column_to_project.table_id];
        }
    }
    //Remove duplicate predicates
    for(uint32_t i=0; i<q->number_of_predicates; i++)
    {
        for(uint32_t j=i+1; j<q->number_of_predicates; j++)
        {
            if(compare_predicates(&q->predicates[i], &q->predicates[j]))
            {
                if(j!=q->number_of_predicates)
                {
                    swap_predicates(&q->predicates[j], &q->predicates[q->number_of_predicates-1]);
                }
                free(q->predicates[q->number_of_predicates-1].p);
                q->predicates[q->number_of_predicates-1].p=NULL;
                q->number_of_predicates--;
                j--;
            }
        }
    }
    uint32_t removed=0;
    for(uint32_t i=0; i<q->number_of_tables; i++)
    {
        if(table_used[i+removed]==false)//Table not used
        {
            uint32_t z=i;
            for(uint32_t j=z+1; j<q->number_of_tables; j++, z++)
            {
                q->table_ids[z]=q->table_ids[j];
            }
            q->number_of_tables--;
            removed++;
            i--;
        }
    }
    free(table_indexes);
    free(table_used);
    return 0;
}

/**
 * Swaps the table.columns in a join or self join predicate
 * (1.0=0.2 -> 0.2=1.0)
 * @param predicate*
 */
void swap_tc_in_predicate(predicate* p)
{
    if(p==NULL||p->type!=Join)
    {
        fprintf(stderr, "swap_tc_in_predicate: Error with the parameters\n");
        return;
    }
    table_column temp;
    temp.table_id=((predicate_join*) p->p)->r.table_id;
    temp.column_id=((predicate_join*) p->p)->r.column_id;
    ((predicate_join*) p->p)->r.table_id=((predicate_join*) p->p)->s.table_id;
    ((predicate_join*) p->p)->r.column_id=((predicate_join*) p->p)->s.column_id;
    ((predicate_join*) p->p)->s.table_id=temp.table_id;
    ((predicate_join*) p->p)->s.column_id=temp.column_id;
}

//Helper list for counting the table.columns in join predicates
typedef struct counter_node counter_node;

typedef struct counter_node
{
    uint32_t counter;
    table_column tc;
    counter_node* next;
} counter_node;

typedef struct counter_list
{
    uint32_t number_of_nodes;
    counter_node* head;
} counter_list;

/**
 * Creates and returns a new empty counter node
 * @return counter_node* or Null if malloc fails
 */
counter_node* new_counter_node(void)
{
    counter_node* new_node=malloc(sizeof(counter_node));
    if(new_node==NULL)
    {
        perror("new_counter_node: malloc error");
        return NULL;
    }
    new_node->counter=0;
    new_node->next=NULL;
    new_node->tc.table_id=0;
    new_node->tc.column_id=0;
    return new_node;
}

/**
 * Prints the data of all the nodes in the list
 * @param counter_list* The list to print
 */
void print_counter_list(counter_list* cl)
{
    if(cl==NULL)
    {
        printf("NULL Parameter\n");
        return;
    }
    counter_node* temp=cl->head;
    printf("List Counter: %"PRIu32"\n", cl->number_of_nodes);
    uint32_t counter=0;
    while(temp!=NULL)
    {
        printf("%"PRIu32": Table Index: %"PRIu32" Column Index: %"PRIu64"\n", counter, temp->tc.table_id, temp->tc.column_id);
        printf("Counter: %"PRIu32"\n", temp->counter);
        temp=temp->next;
        counter++;
    }
}

/**
 * Appends a new table.column in the list or if it already exists increases
 * the counter
 * @param counter_list* The counter list
 * @param table_column* The table.column
 * @return 0 if successful
 */
int counter_list_append(counter_list*list, table_column* tc/*,predicate_join* p*/)
{
    if(list==NULL||tc==NULL/*||p==NULL*/)
    {
        return -1;
    }
    if(list->head==NULL)
    {
        list->head=new_counter_node();
        if(list->head==NULL)
        {
            return -2;
        }
        list->number_of_nodes++;
        list->head->counter++;
        list->head->tc.table_id=tc->table_id;
        list->head->tc.column_id=tc->column_id;
        return 0;
    }
    counter_node* temp=list->head;
    counter_node* temp_to_add=list->head;
    while(temp!=NULL)
    {
        temp_to_add=temp;
        if(temp->tc.table_id==tc->table_id&&temp->tc.column_id==tc->column_id)
        {//Already in the list
            temp->counter++;
            return 0;
        }
        temp=temp->next;
    }
    temp_to_add->next=new_counter_node();
    if(temp_to_add->next==NULL)
    {
        return -6;
    }
    list->number_of_nodes++;
    temp_to_add->next->counter++;
    temp_to_add->next->tc.table_id=tc->table_id;
    temp_to_add->next->tc.column_id=tc->column_id;
    return 0;
}

/**
 * Removes the table.column in the list or if the counter is greater than 1
 * decreases the counter
 * @param counter_list* The counter list
 * @param table_column* The table.column
 * @return 0 if successful
 */
int counter_list_remove(counter_list*list, table_column* tc)
{
    if(list==NULL||tc==NULL)
    {
        return -1;
    }
    if(list->head!=NULL)
    {
        if(list->head->tc.table_id==tc->table_id&&list->head->tc.column_id==tc->column_id)
        {//Found in head
            list->head->counter--;
            if(list->head->counter==0)
            {//Delete the node
                counter_node*temp=list->head->next;
                free(list->head);
                list->head=temp;
                list->number_of_nodes--;
            }
            return 0;
        }
    }
    counter_node* temp=list->head;
    while(temp->next!=NULL)
    {
        if(temp->next->tc.table_id==tc->table_id&&temp->next->tc.column_id==tc->column_id)
        {//Found in the list
            temp->next->counter--;
            if(temp->next->counter==0)
            {//Remove The Node
                counter_node*temp_d=temp->next->next;
                free(temp->next);
                temp->next=temp_d;
                list->number_of_nodes--;
            }
            return 0;
        }
        temp=temp->next;
    }
    return -1;
}

/**
 * Returns the counter of the table.column given
 * @param counter_list*
 * @param table_column*
 * @return uint32_t The counter or 0
 */
uint32_t counter_list_get_counter(counter_list* list, table_column* tc)
{
    if(list==NULL||tc==NULL)
    {
        return 0;
    }
    counter_node* temp=list->head;
    while(temp!=NULL)
    {
        if(temp->tc.table_id==tc->table_id&&temp->tc.column_id==tc->column_id)
        {//Found in the list
            return temp->counter;
        }
        temp=temp->next;
    }
    return 0;
}

/**
 * Creates and returns an empty counter list
 * @return counter_list* of NULL if malloc fails
 */
counter_list* create_counter_list(void)
{
    //Create the list
    counter_list* new_list;
    new_list=malloc(sizeof(counter_list));
    if(new_list==NULL)
    {
        perror("create_counter_list(): error in malloc");
        return NULL;
    }
    //Initialize the list to be empty
    new_list->head=NULL;
    new_list->number_of_nodes=0;
    return new_list;
}

/**
 * Deletes all the nodes of the counter list
 * @param counter_list*
 */
void delete_counter_list(counter_list* list)
{
    if(list==NULL)
    {
        printf("delete_counter_list: NULL list pointer\n");
        return;
    }
    counter_node* temp=list->head;
    //Delete all the nodes
    while(list->head!=NULL)
    {
        list->head=temp->next;
        free(temp);
        temp=list->head;
        list->number_of_nodes--;
    }
    free(list);
}


double power(double x, uint64_t y)
{
 // printf("in power: x=%f  y=%d\n", x,y);
  double total = 1;
  for(uint64_t i = 0; i < y; i++)
  {
    total *= x;
  }
  return total;
}

int statistics_filters(query *q, predicate_filter *filter, table_index *index)
{

  if(q == NULL || filter == NULL || index == NULL)
  {
    fprintf(stderr, "statistics_filters: null parameters\n");
    return 1;
  }

  table *table = get_table(index, q->table_ids[filter->r.table_id]);
  if(table == NULL)
  {
    fprintf(stderr, "statistics_filters: Table not found\n");
    return 2;
  }

  //Case A. Filter '='
  if(filter->filter_type == Equal)
  {
    uint64_t initial_f_A = table->columns_stats[filter->r.column_id].f_A;

    if(table->over_n[filter->r.column_id])
    {
      int8_t b = table->distinct_vals[filter->r.column_id][((filter->value - table->columns_stats[filter->r.column_id].i_A) % N)/8];
      int position = ((filter->value - table->columns_stats[filter->r.column_id].i_A) % N)%8;

      switch(position)
      {
        case 0:
          b = b & 0x80;
          break;
        case 1:
          b = b & 0x40;
          break;
        case 2:
          b = b & 0x20;
          break;
        case 3:
          b = b & 0x10;
          break;
        case 4:
          b = b & 0x08;
          break;
        case 5:
          b = b & 0x04;
          break;
        case 6:
          b = b & 0x02;
          break;
        case 7:
          b = b & 0x01;
          break;
      }

      if(b > 0)
      {
        table->columns_stats[filter->r.column_id].f_A = 
            ((double)(table->columns_stats[filter->r.column_id].f_A))/table->columns_stats[filter->r.column_id].d_A;

        table->columns_stats[filter->r.column_id].d_A = 1;
      }
      else
      {
        table->columns_stats[filter->r.column_id].f_A = 0;
        table->columns_stats[filter->r.column_id].d_A = 0;
      }

      table->columns_stats[filter->r.column_id].i_A = filter->value;
      table->columns_stats[filter->r.column_id].u_A = filter->value;
    }
    else
    {
      int8_t b = table->distinct_vals[filter->r.column_id][(filter->value - table->columns_stats[filter->r.column_id].i_A)/8];
      int position = (filter->value - table->columns_stats[filter->r.column_id].i_A)%8;

      switch(position)
      {
        case 0:
          b = b & 0x80;
          break;
        case 1:
          b = b & 0x40;
          break;
        case 2:
          b = b & 0x20;
          break;
        case 3:
          b = b & 0x10;
          break;
        case 4:
          b = b & 0x08;
          break;
        case 5:
          b = b & 0x04;
          break;
        case 6:
          b = b & 0x02;
          break;
        case 7:
          b = b & 0x01;
          break;
      }

      if(b > 0)
      {
        table->columns_stats[filter->r.column_id].f_A = 
            ((double)(table->columns_stats[filter->r.column_id].f_A))/table->columns_stats[filter->r.column_id].d_A;

        table->columns_stats[filter->r.column_id].d_A = 1;
      }
      else
      {
        table->columns_stats[filter->r.column_id].f_A = 0;
        table->columns_stats[filter->r.column_id].d_A = 0;
      }

      table->columns_stats[filter->r.column_id].i_A = filter->value;
      table->columns_stats[filter->r.column_id].u_A = filter->value;
    }

    //update the rest columns of the table
    for(uint64_t i = 0; i < table->columns; i++)
    {
      if(i == filter->r.column_id)
        continue;

      double parenthesis = power((1 - ((double)table->columns_stats[filter->r.column_id].f_A)/initial_f_A),
          table->columns_stats[i].f_A/table->columns_stats[i].d_A); 

      table->columns_stats[i].d_A = (uint64_t) (table->columns_stats[i].d_A * (double)(1 - parenthesis) + 0.5);

      table->columns_stats[i].f_A = table->columns_stats[filter->r.column_id].f_A;
    }
  }
  //Case B. Filter '<' or '<='
  else if(filter->filter_type == Less_Equal || filter->filter_type == Less)
  {
    //keep initial u_A, i_A, f_A values
    uint64_t initial_f_A = table->columns_stats[filter->r.column_id].f_A;
    uint64_t initial_u_A = table->columns_stats[filter->r.column_id].u_A;
    uint64_t initial_i_A = table->columns_stats[filter->r.column_id].i_A;

    //if filter->value < u_A (max) then update..else keep the initial u_A
    if(filter->value < table->columns_stats[filter->r.column_id].u_A)
      table->columns_stats[filter->r.column_id].u_A = filter->value;

    //if filter->value < i_A (min) then set min = max = 0..else keep the initial i_A
    if(filter->value < table->columns_stats[filter->r.column_id].i_A)
    {
      table->columns_stats[filter->r.column_id].u_A = 0;
      table->columns_stats[filter->r.column_id].i_A = 0;
    }

    //update d_A
    table->columns_stats[filter->r.column_id].d_A = table->columns_stats[filter->r.column_id].d_A * 
      ((double)(table->columns_stats[filter->r.column_id].u_A - table->columns_stats[filter->r.column_id].i_A)/(initial_u_A - initial_i_A));

    //update f_A
    table->columns_stats[filter->r.column_id].f_A = table->columns_stats[filter->r.column_id].f_A * 
      ((double)(table->columns_stats[filter->r.column_id].u_A - table->columns_stats[filter->r.column_id].i_A)/(initial_u_A - initial_i_A));     
  
    //update the rest columns of the table
    for(uint64_t i = 0; i < table->columns; i++)
    {
      if(i == filter->r.column_id)
        continue;

      double parenthesis = power((1 - ((double)table->columns_stats[filter->r.column_id].f_A)/initial_f_A),
          table->columns_stats[i].f_A/table->columns_stats[i].d_A); 

      table->columns_stats[i].d_A = (uint64_t) (table->columns_stats[i].d_A * (double)(1 - parenthesis));

      table->columns_stats[i].f_A = table->columns_stats[filter->r.column_id].f_A;
    }
  }
  //Case C. Filter '>' or '>='
  else if(filter->filter_type == Greater_Equal || filter->filter_type == Greater)
  {
    //keep initial u_A, i_A, f_A values
    uint64_t initial_f_A = table->columns_stats[filter->r.column_id].f_A;
    uint64_t initial_u_A = table->columns_stats[filter->r.column_id].u_A;
    uint64_t initial_i_A = table->columns_stats[filter->r.column_id].i_A;

    //if filter->value > i_A (min) then update..else keep the initial i_A
    if(filter->value > table->columns_stats[filter->r.column_id].i_A)
      table->columns_stats[filter->r.column_id].i_A = filter->value;

    //if filter->value > u_A (max) then set min = max = 0..else keep the initial u_A
    if(filter->value > table->columns_stats[filter->r.column_id].u_A)
    {
      table->columns_stats[filter->r.column_id].u_A = 0;
      table->columns_stats[filter->r.column_id].i_A = 0;
    }

    //update d_A
    table->columns_stats[filter->r.column_id].d_A = table->columns_stats[filter->r.column_id].d_A * 
      ((double)(table->columns_stats[filter->r.column_id].u_A - table->columns_stats[filter->r.column_id].i_A)/(initial_u_A - initial_i_A));

    //update f_A
    table->columns_stats[filter->r.column_id].f_A = table->columns_stats[filter->r.column_id].f_A * 
      ((double)(table->columns_stats[filter->r.column_id].u_A - table->columns_stats[filter->r.column_id].i_A)/(initial_u_A - initial_i_A));     
  
    //update the rest columns of the table
    for(uint64_t i = 0; i < table->columns; i++)
    {//printf("\n\nneighbors\n");
      if(i == filter->r.column_id)
        continue;

      double parenthesis = power((1 - ((double)table->columns_stats[filter->r.column_id].f_A)/initial_f_A),
          table->columns_stats[i].f_A/table->columns_stats[i].d_A); 

      table->columns_stats[i].d_A = (uint64_t) (table->columns_stats[i].d_A * (double)(1 - parenthesis));

      table->columns_stats[i].f_A = table->columns_stats[filter->r.column_id].f_A;
    }
  }

  return 0;
}

int statistics_self_joins(query *q, predicate_join *join, table_index *index)
{
  table *table = get_table(index, q->table_ids[join->r.table_id]);
  if(table == NULL)
  {
    fprintf(stderr, "execute_query: Table not found\n");
    return 1;
  }

  //Case A. autocorrelation
  if(join->r.column_id == join->s.column_id)
  {
    table->columns_stats[join->r.column_id].f_A = (uint64_t)((double) (table->columns_stats[join->r.column_id].u_A)*
      (table->columns_stats[join->r.column_id].u_A))/(table->columns_stats[join->r.column_id].u_A - table->columns_stats[join->r.column_id].i_A + 1);
  
    for(uint64_t i = 0; i < table->columns; i++)
    {
      if(i == join->r.column_id)
        continue;

      table->columns_stats[i].f_A = table->columns_stats[join->r.column_id].f_A;
    }
  }
  //Case B. R.A = R.B
  else
  {
    uint64_t initial_f_A = table->columns_stats[join->r.column_id].f_A;

    uint64_t val_A = table->columns_stats[join->r.column_id].i_A;
    uint64_t val_B = table->columns_stats[join->s.column_id].i_A;

    //new i_A, new i_B = max(initial i_A, initial i_B)
    table->columns_stats[join->r.column_id].i_A = (val_A > val_B) ? val_A : val_B;
    table->columns_stats[join->s.column_id].i_A = (val_A > val_B) ? val_A : val_B;

    val_A = table->columns_stats[join->r.column_id].u_A;
    val_B = table->columns_stats[join->s.column_id].u_A;

    //new u_A, new u_B = min(initial u_A, initial u_B)
    table->columns_stats[join->r.column_id].u_A = (val_A < val_B) ? val_A : val_B;
    table->columns_stats[join->s.column_id].u_A = (val_A < val_B) ? val_A : val_B;

    //new f_A = initial f / n, where n = new u_A - new i_A + 1
    table->columns_stats[join->r.column_id].f_A = (uint64_t)((double) (table->columns_stats[join->r.column_id].f_A))/
        (table->columns_stats[join->r.column_id].u_A - table->columns_stats[join->r.column_id].i_A + 1);

    //new f_B = new f_A
    table->columns_stats[join->s.column_id].f_A = table->columns_stats[join->r.column_id].f_A;

    //d_A
    double parenthesis = power((1 - ((double)table->columns_stats[join->r.column_id].f_A)/initial_f_A),
          table->columns_stats[join->r.column_id].f_A/table->columns_stats[join->r.column_id].d_A); 

    table->columns_stats[join->r.column_id].d_A = 
        (uint64_t) (table->columns_stats[join->r.column_id].d_A * (double)(1 - parenthesis));

    table->columns_stats[join->s.column_id].d_A = table->columns_stats[join->r.column_id].d_A;

    //update the rest columns of the table
    for(uint64_t i = 0; i < table->columns; i++)
    {
      if(i == join->r.column_id)
        continue;

      double parenthesis = power((1 - ((double)table->columns_stats[join->r.column_id].f_A)/initial_f_A),
          table->columns_stats[i].f_A/table->columns_stats[i].d_A); 

      table->columns_stats[i].d_A = (uint64_t) (table->columns_stats[i].d_A * (double)(1 - parenthesis));

      table->columns_stats[i].f_A = table->columns_stats[join->r.column_id].f_A;
    }
  }

  return 0;
}


int calculate_join(query *q, predicate_join *join, 
                   table_index *index, statistics *results, 
                   uint64_t f_A, uint64_t i_A, uint64_t u_A, bool exists)
{
  table *table_r = get_table(index, q->table_ids[join->r.table_id]);
  if(table_r == NULL)
  {
    fprintf(stderr, "execute_query: Table not found\n");
    return 1;
  }

  table *table_s = get_table(index, q->table_ids[join->s.table_id]);
  if(table_s == NULL)
  {
    fprintf(stderr, "execute_query: Table not found\n");
    return 1;
  }

  if(exists)
  {
    //new i_A, new i_B = max(initial i_A, initial i_B)
    results->i_A = (i_A > table_s->columns_stats[join->s.column_id].i_A) ? 
        i_A : table_s->columns_stats[join->s.column_id].i_A;

    //new u_A, new u_B = min(initial u_A, initial u_B)
    results->u_A = (u_A < table_s->columns_stats[join->s.column_id].u_A) ? 
        u_A : table_s->columns_stats[join->s.column_id].u_A;
  }
  else
  {
    uint64_t val_A = table_r->columns_stats[join->r.column_id].i_A;
    uint64_t val_B = table_s->columns_stats[join->s.column_id].i_A;

    //new i_A, new i_B = max(initial i_A, initial i_B)
    results->i_A = (val_A > val_B) ? val_A : val_B;

    val_A = table_r->columns_stats[join->r.column_id].u_A;
    val_B = table_s->columns_stats[join->s.column_id].u_A;

    //new u_A, new u_B = min(initial u_A, initial u_B)
    results->u_A = (val_A < val_B) ? val_A : val_B;
  }

  //new f_A = (initial f_A * initial f_B) / n
  results->f_A = ((double)(f_A * table_s->columns_stats[join->s.column_id].f_A))/(results->u_A - results->i_A + 1);

  return 0;
    
}


uint32_t convert_to_binary(uint32_t decimal)
{
  if(decimal == 0)
    return 8;
  else if(decimal == 1)
    return 4;
  else if(decimal == 2)
    return 2;
  else if(decimal == 3)
    return 1;
  else
    return 0;
}


uint32_t convert_to_decimal(uint32_t binary)
{
  if(binary == 8)
    return 0;
  else if(binary == 4)
    return 1;
  else if(binary == 2)
    return 2;
  else if(binary == 1)
    return 3;
  else
    return 0;
}


/**
 * BIG ENDIAN
 * 8 -> 1st most significant byte is 1, else 0 -> table with index 0
 * 4 -> 2st most significant byte is 1, else 0 -> table with index 1
 * 2 -> 3st most significant byte is 1, else 0 -> table with index 2
 * 1 -> 4st most significant byte is 1, else 0 -> table with index 3
 */
best_order join_enumeration(query *q, table_index *index, neighbor_list *nl)
{
  best_tree btree;

  //A.1 allocate memory for best tree
  btree.relations = malloc(BEST_TREE_SIZE * sizeof(uint64_t));
  if(btree.relations == NULL)
  {
    perror("join_enumeration: malloc error");
    //return 1;
  }  

  btree.join_stats = malloc(BEST_TREE_SIZE * sizeof(ui_stats));
  if(btree.join_stats == NULL)
  {
    perror("join_enumeration: malloc error");
   // return 1;
  } 

  btree.order = malloc(BEST_TREE_SIZE * sizeof(best_order));
  if(btree.order == NULL)
  {
    perror("join_enumeration: malloc error");
   // return 1;
  } 

  //A.2 best tree initialization
  for(int i = 0; i < BEST_TREE_SIZE; i++)  
  {
    btree.relations[i] = -1;
    for(int j = 0; j < MAX_QUERY_NUM; j++)  
      btree.order[i].array[j] = -1;

    btree.join_stats[i].r_table_id = 0;
    btree.join_stats[i].r_column_id = 0;
    btree.join_stats[i].s_table_id = 0;
    btree.join_stats[i].s_column_id = 0;
    btree.join_stats[i].i_A = 0;
    btree.join_stats[i].u_A = 0;
  }

  //A.3 initalize best tree with the sets of 1 element R_j, j at {0,1,2,3}
  //bid endian
  int8_t pos = 8;  
  for(uint32_t i = 0; i < q->number_of_tables; i++)  
  {
    table *table = get_table(index, q->table_ids[i]);
    if(table == NULL)
    {
      fprintf(stderr, "join_enumeration: Table not found\n");
    //  return 2;
    }

    //every column of the table has the same f_A so we can choose whoever we want
    btree.relations[pos] = table->columns_stats[0].f_A;
    btree.order[pos].array[0] = i;
    pos = pos >> 1;
  }

  uint8_t *members_in_S = malloc(6 * sizeof(uint8_t));
  if(members_in_S == NULL)
  {
    fprintf(stderr, "join_enumeration: malloc error\n");
  //  return 1;
  }

  //B. Join enumeration algorithm
  uint8_t members = 0;
  for(uint32_t i = 0; i < q->number_of_tables; i++) 
  {
    if(i == 0)
    {
      //S with 1 member (in order to become 2)
      members_in_S[0] = 8;
      members_in_S[1] = 4;
      members_in_S[2] = 2;
      members_in_S[3] = 1;

      members = 4;
    }
    else if(i == 1)
    {
      //S with 2 members (in order to become 3)
      members_in_S[0] = 3;
      members_in_S[1] = 5;
      members_in_S[2] = 6;
      members_in_S[3] = 9;
      members_in_S[4] = 10;
      members_in_S[5] = 12;

      members = 6;
    }
    else if(i == 2)
    {
      //S with 3 members (in order to become 4)
      members_in_S[0] = 7;
      members_in_S[1] = 11;
      members_in_S[2] = 13;
      members_in_S[3] = 14;

      members = 4;
    }
    else if(i == 3)
    {
      members_in_S[0] = 15;
      members = 1;
    }

    for(uint8_t m = 0; m < members; m++)
    {
      int8_t S = members_in_S[m];

      if((int64_t)btree.relations[S] == -1)
      {
        continue;
      }

      int8_t temp_S = S << 4;

      //B.1 find neighbors of every relation in S
      for(int8_t j = 0; j < 4; j++)
      {
        //B.2 if temp_S < 0 then the 1st bit is 1 and index->tables[j] participates in S
        if(temp_S < 0)
        {
          
          for(int k = 0; k < nl->neighbors_num[j]; k++)
          {
            //convert neighbor position to binary repensentation
            int8_t neighbor_pos = convert_to_binary(nl->neighbors_list[j][k].table_id);

            //B.3 if neighbor is not already in S then add it to the neighbors list
            if((neighbor_pos & S) == 0)
            {
              predicate_join pj;
              statistics results;

              results.i_A = 0;
              results.u_A = 0;
              results.f_A = 0;
              results.d_A = 0;

              int res = 0;

              pj.r.table_id = j;
              pj.r.column_id = nl->neighbors_list[j][k].my_column_id;

              pj.s.table_id = nl->neighbors_list[j][k].table_id;
              pj.s.column_id = nl->neighbors_list[j][k].column_id;

              //B.4 search if r or s (table_id, column_id) have been joined before
              if((btree.join_stats[S].r_table_id == pj.r.table_id && 
                  btree.join_stats[S].r_column_id == pj.r.column_id )||
                 (btree.join_stats[S].s_table_id == pj.s.table_id && 
                  btree.join_stats[S].s_column_id == pj.s.column_id) ||
                 (btree.join_stats[S].s_table_id == pj.r.table_id && 
                  btree.join_stats[S].s_column_id == pj.r.column_id )||
                 (btree.join_stats[S].r_table_id == pj.s.table_id && 
                  btree.join_stats[S].r_column_id == pj.s.column_id)
                )
              {
                res = calculate_join(q, &pj, index, &results, btree.relations[S], btree.join_stats[S].i_A, 
                    btree.join_stats[S].u_A, true);
              }
              else
              {
                res = calculate_join(q, &pj, index, &results, btree.relations[S], btree.join_stats[S].i_A, 
                    btree.join_stats[S].u_A, false);
              }

              if(res)
              {
                fprintf(stderr, "join_enumeration: error in calculate_join\n");
          //      return 1;
              }

              int8_t new_S = S | neighbor_pos;

              //B.5 compare cost
              if(((int64_t) btree.relations[new_S] == -1) || btree.relations[new_S] > results.f_A)
              {
                btree.relations[new_S] = results.f_A;

                btree.join_stats[new_S].i_A = results.i_A;
                btree.join_stats[new_S].u_A = results.u_A;

                btree.join_stats[new_S].r_table_id = j;
                btree.join_stats[new_S].r_column_id = nl->neighbors_list[j][k].my_column_id;

                btree.join_stats[new_S].s_table_id = nl->neighbors_list[j][k].table_id;
                btree.join_stats[new_S].s_column_id = nl->neighbors_list[j][k].column_id;

                for(int z = 0; z < MAX_QUERY_NUM; z++)
                  btree.order[new_S].array[z] = btree.order[S].array[z];

                btree.order[new_S].array[i+1] = nl->neighbors_list[j][k].table_id;
              }
            }
          }
        }

        temp_S = temp_S << 1;
      }

    }
  }

  //B.6 find optimal solution
  best_order best_result;
  uint64_t min = 0;
  bool first = true;
  for(uint8_t m = 0; m < members; m++)
  {
    int8_t S = members_in_S[m];

    if((int64_t)btree.relations[S] == -1)
      continue;

    if(first)
    {
      min = btree.relations[S];
      best_result = btree.order[S];
      first = false;
    }
    else
    {
      if(btree.relations[S] < min)
      {
        min = btree.relations[S];
        best_result = btree.order[S];
      }
    }
  }

  //print optimal result
  //printf("Best Tree Result (f_A): %" PRIu64 "\tTables: ", min);

  //for(int z = 0; z < q->number_of_tables; z++)
  //{
  //  printf("%" PRIu8 " ", best_result.array[z]);
  //}
  //printf("\n");

  //free memory
  free(btree.relations);
  free(btree.join_stats);
  free(btree.order);
  free(members_in_S);

  return best_result;
}


int optimize_query(query*q, table_index* ti)
{
  //Check the parameters
  if(q==NULL||ti==NULL||ti->num_tables==0||ti->tables==NULL||
     q->number_of_tables==0||q->number_of_predicates==0||
     q->number_of_projections==0||q->table_ids==NULL||q->predicates==NULL||
     q->projections==NULL)
  {
    fprintf(stderr, "optimize_query: Error with the parameters\n");
    return -1;
  }

  //create a temporary table index for storing the changes of the filters/joins
  table_index *temp_index = malloc(sizeof(table_index));
  if(temp_index == NULL)
  {
    perror("optimize_query: malloc error");
    return -2;
  }

  temp_index->num_tables = q->number_of_tables;
  temp_index->tables = malloc(temp_index->num_tables * sizeof(table));
  if(temp_index->tables == NULL)
  {
    perror("optimize_query: malloc error");
    return -2;
  }

  for(uint64_t i = 0; i < q->number_of_tables; i++)
  {
    table *table = get_table(ti, q->table_ids[i]);
    if(table == NULL)
    {
      fprintf(stderr, "execute_query: Table not found\n");
      return -1;
    }

    temp_index->tables[i].table_id = table->table_id;
    temp_index->tables[i].rows = table->rows;
    temp_index->tables[i].columns = table->columns;

    temp_index->tables[i].columns_stats = malloc(temp_index->tables[i].columns * sizeof(statistics));
    if(temp_index->tables[i].columns_stats == NULL)
    {
      perror("optimize_query: malloc error");
      return -2;
    }

    memcpy(temp_index->tables[i].columns_stats, table->columns_stats, temp_index->tables[i].columns * sizeof(statistics));

    temp_index->tables[i].num_vals = malloc(temp_index->tables[i].columns * sizeof(uint64_t));
    if(temp_index->tables[i].num_vals == NULL)
    {
      perror("table_from_file: malloc error");
      return -5;
    }

    memcpy(temp_index->tables[i].num_vals, table->num_vals, temp_index->tables[i].columns * sizeof(uint64_t));

    temp_index->tables[i].over_n = malloc(temp_index->tables[i].columns * sizeof(bool));
    if(temp_index->tables[i].over_n == NULL)
    {
      perror("table_from_file: malloc error");
      return -6;
    }

    memcpy(temp_index->tables[i].over_n, table->over_n, temp_index->tables[i].columns * sizeof(bool));

    temp_index->tables[i].distinct_vals = malloc(temp_index->tables[i].columns * sizeof(int8_t *));
    if(temp_index->tables[i].distinct_vals == NULL)
    {
      perror("table_from_file: malloc error");
      return -7;
    }

    for(uint64_t j = 0; j < temp_index->tables[i].columns; j++)
    {
      temp_index->tables[i].distinct_vals[j] = malloc(temp_index->tables[i].num_vals[j] * sizeof(int8_t));
      if(temp_index->tables[i].distinct_vals[j] == NULL)
      {
        fprintf(stderr, "table_from_file: malloc error\n");
        return -5;
      }

      memcpy(temp_index->tables[i].distinct_vals[j], table->distinct_vals[j], temp_index->tables[i].num_vals[j] * sizeof(int8_t));
    }
  }

  //initialize neighbors list
  neighbor_list nl;
  nl.neighbors_list = malloc(MAX_QUERY_NUM *sizeof(neighbor_data *));
  if(nl.neighbors_list == NULL)
  {
    perror("optimize_query: malloc error");
    return -2;
  }

  for(int i = 0; i < MAX_QUERY_NUM; i++)
  {
    nl.neighbors_list[i] = malloc(MAX_QUERY_NUM * sizeof(neighbor_data));
    if(nl.neighbors_list[i] == NULL)
    {
      perror("optimize_query: malloc error");
      return -2;
    }
  }

  nl.neighbors_num = malloc(MAX_QUERY_NUM * sizeof(int));
  if(nl.neighbors_num == NULL)
  {
    perror("optimize_query: malloc error");
    return -2;
  }

  for(int i = 0; i < MAX_QUERY_NUM; i++)
    nl.neighbors_num[i] = 0;

  //First put the filters and count the join/self joins
  uint32_t j=0;
  for(uint32_t i=0; i<q->number_of_predicates; i++)
  {
    //If Filter move to beginning
    if(q->predicates[i].type==Filter)
    {

      //update statistics
      if(statistics_filters(q, (predicate_filter *) q->predicates[i].p, temp_index))
      {
        fprintf(stderr, "optimize_query: error in statistics_filters\n");
        return -4;
      }

      swap_predicates(&q->predicates[j], &q->predicates[i]);
      j++;
    }
    else if(q->predicates[i].type==Join)//Count the table.rowid pairs
    {
      predicate_join *join = q->predicates[i].p;

      //search if r is related with s in the neighbors list
      bool flag = 1;
      for(int i = 0; i < nl.neighbors_num[join->r.table_id]; i++)
      {
        if(nl.neighbors_list[join->r.table_id][i].table_id == join->s.table_id)
        {
          flag = 0;
          break;
        }
      }

      //if r and s are not related then add each one in the other's list
      if(flag)
      {
        int pos = nl.neighbors_num[join->r.table_id];
        nl.neighbors_list[join->r.table_id][pos].table_id = join->s.table_id;
        nl.neighbors_list[join->r.table_id][pos].column_id = join->s.column_id;
        nl.neighbors_list[join->r.table_id][pos].my_column_id = join->r.column_id;
        nl.neighbors_num[join->r.table_id]++;

        pos = nl.neighbors_num[join->s.table_id];
        nl.neighbors_list[join->s.table_id][pos].table_id = join->r.table_id;
        nl.neighbors_list[join->s.table_id][pos].column_id = join->r.column_id;
        nl.neighbors_list[join->s.table_id][pos].my_column_id = join->s.column_id;
        nl.neighbors_num[join->s.table_id]++;
      }
    }
  }

  //Then the self joins
  for(uint32_t i=j; i<q->number_of_predicates; i++)
  {
      //If Filter move to Beginning
      if(q->predicates[i].type==Self_Join)
      {
        //update statistics
        if(statistics_self_joins(q, (predicate_join *) q->predicates[i].p, temp_index))
        {
          fprintf(stderr, "optimize_query: error in statistics_self_joins\n");
          return -7;
        }

        swap_predicates(&q->predicates[j], &q->predicates[i]);
        j++;
      }
  }

  //join enumeration
  int order_counter = 0;
  best_order order = join_enumeration(q, temp_index, &nl);

  //based on the results change the predicate order
  for(uint32_t i = 0; i < q->number_of_tables-1; i++)
  {
    for(uint32_t k = j; k < q->number_of_predicates; k++)
    {
      bool r = false, s = false;

      if(q->predicates[k].type == Join && 
             (((predicate_join *)q->predicates[k].p)->r.table_id == order.array[order_counter] ||
             ((predicate_join *)q->predicates[k].p)->r.table_id == order.array[order_counter + 1]))
      {
        r = true;
      }

      if(q->predicates[k].type == Join && 
             (((predicate_join *)q->predicates[k].p)->s.table_id == order.array[order_counter] ||
             ((predicate_join *)q->predicates[k].p)->s.table_id == order.array[order_counter + 1]))
      {
        s = true;
      }

      if(r == true && s == true)
      {
        swap_predicates(&q->predicates[j], &q->predicates[k]);
        j++;
      }
      else if(r == true && s == false)
      {
        for(int m = order_counter; m >= 0; m--)
        {
          if(((predicate_join *)q->predicates[k].p)->s.table_id == order.array[m])
          {
            swap_predicates(&q->predicates[j], &q->predicates[k]);
            j++;
            break;
          }
        }
      }
      else if(r == false && s == true)
      {
        for(int m = order_counter; m >= 0; m--)
        {
          if(((predicate_join *)q->predicates[k].p)->r.table_id == order.array[m])
          {
            swap_predicates(&q->predicates[j], &q->predicates[k]);
            j++;
            break;
          }
        }
      }
    }

    order_counter++;
  }

  //memory free
  for(uint64_t i = 0; i < q->number_of_tables; i++)
  {
    free(temp_index->tables[i].columns_stats);
    free(temp_index->tables[i].num_vals);
    free(temp_index->tables[i].over_n);

    for(uint64_t j = 0; j < temp_index->tables[i].columns; j++)
    {
      free(temp_index->tables[i].distinct_vals[j]);
    }
    free(temp_index->tables[i].distinct_vals);
  }
  free(temp_index->tables);
  free(temp_index);

  for(int i = 0; i < MAX_QUERY_NUM; i++)
  {
    free(nl.neighbors_list[i]);
  }
  free(nl.neighbors_list);
  free(nl.neighbors_num);

  return 0;
   
}


int create_sort_array(query*q, bool**t_c_to_sort)
{
    //Check the parameters
    if(q==NULL||t_c_to_sort==NULL||
       *t_c_to_sort!=NULL||q->number_of_tables==0||q->number_of_predicates==0||
       q->number_of_projections==0||q->table_ids==NULL||q->predicates==NULL||
       q->projections==NULL)
    {
        fprintf(stderr, "create_sort_array: Error with the parameters\n");
        return -1;
    }
    bool joined_tables[q->number_of_tables][q->number_of_tables];
    //Array that keeps which table have become one
    //^Used to idendify self joins after two or more tables have joined
    //Initialize to 0
    for(uint32_t i=0; i<q->number_of_tables; i++)
    {
        for(uint32_t j=0; j<q->number_of_tables; j++)
        {
            joined_tables[i][j]=false;
        }
    }
    //Find the hidden self joins
    //And count the real joins
    uint32_t join_counter=0;
    for(uint32_t i=0; i<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Join)
        {
            //Check if the arrays have been joined
            if(joined_tables[((predicate_join*) (q->predicates[i].p))->r.table_id][((predicate_join*) (q->predicates[i].p))->s.table_id])
            {//Joined
                q->predicates[i].type=Self_Join; //Set it as Self Join
            }
            else
            {//Not Joined update the array
                join_counter++;
                joined_tables[((predicate_join*) (q->predicates[i].p))->r.table_id][((predicate_join*) (q->predicates[i].p))->s.table_id]=true;
                joined_tables[((predicate_join*) (q->predicates[i].p))->s.table_id][((predicate_join*) (q->predicates[i].p))->r.table_id]=true;
                for(uint32_t j=0; j<q->number_of_tables; j++)
                {
                    if(j!=((predicate_join*) (q->predicates[i].p))->r.table_id)
                    {
                        if(joined_tables[((predicate_join*) (q->predicates[i].p))->r.table_id][j])
                        {//Old Join found Update
                            joined_tables[((predicate_join*) (q->predicates[i].p))->s.table_id][j]=true;
                            joined_tables[j][((predicate_join*) (q->predicates[i].p))->s.table_id]=true;
                        }
                    }
                }
                for(uint32_t j=0; j<q->number_of_tables; j++)
                {
                    if(j!=((predicate_join*) (q->predicates[i].p))->s.table_id)
                    {
                        if(joined_tables[((predicate_join*) (q->predicates[i].p))->s.table_id][j])
                        {//Old Join found Update
                            joined_tables[((predicate_join*) (q->predicates[i].p))->r.table_id][j]=true;
                            joined_tables[j][((predicate_join*) (q->predicates[i].p))->r.table_id]=true;
                        }
                    }
                }
            }
        }
    }
    //Check if all tables all joined in the end
    if(join_counter<q->number_of_tables-1)
    {
        printf("The query requires cartesian product\n");
        return -5;
    }
    //Create the bool table
    if(join_counter==0)//No joins one table
    {
        join_counter=1;
    }
    bool* bool_array=malloc(sizeof(bool)*join_counter*2);
    if(bool_array==NULL)
    {
        perror("create_sort_array: malloc error");
        return -4;
    }
    uint32_t temp_index=0;
    for(uint32_t i=0; i<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Join)
        {
            temp_index=i;
            break;
        }
    }
    for(uint32_t i=temp_index+1; i<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Join)
        {
            if((((predicate_join*) (q->predicates[temp_index].p))->s.table_id==((predicate_join*) (q->predicates[i].p))->s.table_id&&
            ((predicate_join*) (q->predicates[temp_index].p))->s.column_id==((predicate_join*) (q->predicates[i].p))->s.column_id)||
               (((predicate_join*) (q->predicates[temp_index].p))->s.table_id==((predicate_join*) (q->predicates[i].p))->r.table_id&&
            ((predicate_join*) (q->predicates[temp_index].p))->s.column_id==((predicate_join*) (q->predicates[i].p))->r.column_id))
            {
                swap_tc_in_predicate(&q->predicates[temp_index]);
            }
            temp_index=i;
        }
    }
    table_column* last_sorted=NULL;
    uint32_t bool_counter=0;
    for(uint32_t i=0; i<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Join)
        {
            bool_array[bool_counter]=true;
            if(last_sorted!=NULL&&last_sorted->table_id==((predicate_join*) (q->predicates[i].p))->r.table_id&&
               last_sorted->column_id==((predicate_join*) (q->predicates[i].p))->r.column_id)
            {//Already sorted
                bool_array[bool_counter]=false;
            }
            bool_counter++;
            bool_array[bool_counter]=true;
            if(last_sorted!=NULL&&last_sorted->table_id==((predicate_join*) (q->predicates[i].p))->s.table_id&&
               last_sorted->column_id==((predicate_join*) (q->predicates[i].p))->s.column_id)
            {//Already sorted
                bool_array[bool_counter]=false;
            }
            bool_counter++;
            last_sorted=&((predicate_join*) (q->predicates[i].p))->r;
        }
    }
    *t_c_to_sort=bool_array;
    //    return join_counter*2;
    return 0;
}

/**
 * Moves the predicate in the query from index_start to index_end and shifts 
 * all the other predicates accordingly
 * @param query*
 * @param uint32_t
 * @param uint32_t
 */
void move_predicate(query*q, uint32_t index_start, uint32_t index_end)
{
    if(q==NULL||q->number_of_predicates==0||q->predicates==NULL||
       q->number_of_predicates<index_start||q->number_of_predicates<index_end||
       index_end==index_start)
    {
        return;
    }
    if(index_start<index_end)
    {//Move -> in the array
        for(uint32_t i=index_start; i<index_end; i++)
        {
            swap_predicates(&q->predicates[i], &q->predicates[i+1]);
        }
    }
    else
    {//Move <- in the array
        for(uint32_t i=index_start; i>index_end; i--)
        {
            swap_predicates(&q->predicates[i], &q->predicates[i-1]);
        }
    }
}

int optimize_query_memory(query*q)
{
    //The create bool must be called first
    //Check the parameters
    if(q==NULL||q->number_of_tables==0||q->number_of_predicates==0||
       q->number_of_projections==0||q->table_ids==NULL||q->predicates==NULL||
       q->projections==NULL)
    {
        fprintf(stderr, "optimize_query_after_bool_array: Error with the parameters\n");
        return -1;
    }
    //First move the filters as much to the right as you can before it is needed
    uint32_t move_counter=0;
    for(uint32_t i=0; i<q->number_of_predicates&&move_counter<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Filter)
        {
            //Find the max position the filter is needed before a join/self join
            uint32_t filter_pos=i;
            for(uint32_t j=i+1; j<q->number_of_predicates; j++)
            {
                if(q->predicates[j].type==Join)
                {
                    if((((predicate_join*) (q->predicates[j]).p)->r.table_id==((predicate_filter*) (q->predicates[i].p))->r.table_id)||
                       (((predicate_join*) (q->predicates[j]).p)->s.table_id==((predicate_filter*) (q->predicates[i].p))->r.table_id))
                    {
                        break;
                    }
                    else
                    {
                        filter_pos++;
                    }
                }
                else
                {
                    filter_pos++;
                }
            }
            if(filter_pos!=i)
            {
                move_predicate(q, i, filter_pos);
                i--;
                move_counter++;
            }
        }
    }
    for(uint32_t i=0; i<q->number_of_predicates; i++)
    {
        if(q->predicates[i].type==Self_Join&&((predicate_join*) (q->predicates[i].p))->r.table_id==((predicate_join*) (q->predicates[i].p))->s.table_id)
        {
            //Find the max position the self join is needed before a join/self join
            uint32_t self_join_pos=i;
            for(uint32_t j=i+1; j<q->number_of_predicates; j++)
            {
                if(q->predicates[j].type==Join)
                {
                    if((((predicate_join*) (q->predicates[j]).p)->r.table_id==((predicate_join*) (q->predicates[i].p))->r.table_id)||
                       (((predicate_join*) (q->predicates[j]).p)->s.table_id==((predicate_join*) (q->predicates[i].p))->r.table_id))
                    {
                        break;
                    }
                    else
                    {
                        self_join_pos++;
                    }
                }
                else
                {
                    self_join_pos++;
                }
            }
            if(self_join_pos!=i)
            {
                move_predicate(q, i, self_join_pos);
                i--;
            }
        }
    }
    //Then the self joins to the left after the joins
//    for(uint32_t i=q->number_of_predicates-1; i>0; i--)
//    {
//        //If i a hidden self join
//        if(q->predicates[i].type==Self_Join&&((predicate_join*) (q->predicates[i].p))->r.table_id!=((predicate_join*) (q->predicates[i].p))->s.table_id)
//        {
//            //Find the min position after the join
//            uint32_t self_join_pos=i;
//            for(uint32_t j=i-1; j>0; j--)
//            {
//                if(q->predicates[j].type==Join)
//                {
//                    if((((predicate_join*) (q->predicates[j]).p)->r.table_id==((predicate_join*) (q->predicates[i].p))->r.table_id)||
//                       (((predicate_join*) (q->predicates[j]).p)->s.table_id==((predicate_join*) (q->predicates[i].p))->s.table_id)||
//                       (((predicate_join*) (q->predicates[j]).p)->r.table_id==((predicate_join*) (q->predicates[i].p))->s.table_id)||
//                       (((predicate_join*) (q->predicates[j]).p)->s.table_id==((predicate_join*) (q->predicates[i].p))->r.table_id))
//                    {
//                        break;
//                    }
//                    else
//                    {
//                        self_join_pos--;
//                    }
//                }
//                else
//                {
//                    self_join_pos--;
//                }
//            }
//            if(i!=self_join_pos)
//            {
//                move_predicate(q, i, self_join_pos);
//                i++;
//            }
//        }
//    }
    return 0;
}
