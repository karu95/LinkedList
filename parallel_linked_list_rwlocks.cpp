#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <math.h>

#define MAX_VAL 50000

using namespace std;

// List node object struct.
struct list_node_s {
    int data;
    struct list_node_s *next_p;
};
struct thread_routine_params {
    int sample_size;
    int member_operations;
    int insert_operations;
    int delete_operations;
};
struct list_node_s *head_p = NULL; // Pointer for the head node of the linked list.
pthread_rwlock_t *linked_list_rwlock; // Pointer initialization for RW lock.

int thread_count; // No. of threads.

// Generate a given sized sorted linked list of random numbers. 
int gen_rand_list(int size);

// Run test samples on the linked list with different sample sizes and operation percentages.
int test_samples(int sample_size, double mem_percentage, double insert_percentage, double delete_percentage);

// Calculate mean when total sum and sample size is given.
double calc_mean(int sample_size, double x_sum);

// Calculate standard deviation when squared sum, mean and sample size is given.
double calc_std(int sample_size, double x_squard_sum, double mean);

// Calculate N when mean and standard deviation given.
long calc_n (double mean, double std);

// Empty the generated random linked list.
void free_list(struct list_node_s **head_pp);

// Check if the linked list is empty.
int is_Empty(struct list_node_s *head_p);

// Routine provided for a given thread.
void *thread_routine(void* thread_routine_params);

int main(int argc, char *argv[]) {
    int list_size = strtol(argv[1], NULL, 10);
    int sample_size = strtol(argv[2], NULL, 10);
    double mem_percentage = strtod(argv[3], NULL);
    double delete_percentage = strtod(argv[4], NULL);
    double insert_percentage = strtod(argv[5], NULL);
    thread_count = strtol(argv[6], NULL, 10);

    linked_list_rwlock = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t));

    pthread_rwlock_init(linked_list_rwlock, NULL);

    struct thread_routine_params *params = (struct thread_routine_params*) malloc(sizeof(struct thread_routine_params)); 

    params->member_operations = (sample_size*mem_percentage)/thread_count;
    params->insert_operations = (sample_size*insert_percentage)/thread_count;
    params->delete_operations = (sample_size*delete_percentage)/thread_count;
    params->sample_size = sample_size/thread_count;
    
    long test_sample_size = 10;
    long n;

    while(true) {

        double time_sum = 0.0;
        double time_squard_sum = 0.0;

        for (int i = 0; i < test_sample_size; i++) {
            gen_rand_list(list_size);
            pthread_t *thread_handles;
            thread_handles = (pthread_t*) malloc(thread_count*sizeof(pthread_t));
            clock_t begin_time = clock();
            for (int i = 0; i < thread_count; i++) {
                pthread_create(&thread_handles[i], NULL, thread_routine, (void*) params);
            }
            for (int i = 0; i < thread_count; i++) {
                pthread_join(thread_handles[i], NULL);
            }
            double time_taken = (double(clock () - begin_time)/1000000);
            free(thread_handles);
            time_sum += time_taken;
            time_squard_sum += pow(time_taken, 2);
            free_list(&head_p);
        }

        double x_bar = calc_mean(test_sample_size, time_sum); 
        double standard_dev = calc_std(test_sample_size, time_squard_sum, x_bar);
        n = calc_n(x_bar, standard_dev);
        long difference = n-test_sample_size;
        if (labs(difference) < 4) {
            cout << "Mean time taken for parallel operation using RW locks: " << x_bar << "\n";   
            cout << "STD for parallel operation using RW locks : " << standard_dev << "\n";   
            cout << "Sample size : " << test_sample_size << "\n";
            break;
        } else {
            if (n < 3) {
                test_sample_size = 3;
            } else {
                test_sample_size = n;
            }
        }
    }
}

double calc_mean(int sample_size, double x_sum) {

    return (x_sum/sample_size);
}

double calc_std(int sample_size, double x_squard_sum, double mean) {

    return sqrt((x_squard_sum/sample_size) - (pow(mean,2)));
}

long calc_n (double mean, double std) {

    return pow((100*1.96*std)/(5*mean), 2);
}

int memeber(int value, struct list_node_s *head_p) {
    struct list_node_s* curr_p = head_p;

    while(curr_p != NULL && curr_p->data < value){
        curr_p = curr_p->next_p;
    }

    if (curr_p == NULL || curr_p->data>value) {
        return 0;
    } else {
        return 1;
    }
}

int insert(int value, struct list_node_s **head_p) {
    struct list_node_s *curr_p = *head_p;
    struct list_node_s *prep_p = NULL;
    struct list_node_s *temp_p;

    while(curr_p != NULL && curr_p->data < value) {
        prep_p = curr_p;
        curr_p = curr_p->next_p;
    }

    if (curr_p == NULL || curr_p->data > value) {
        temp_p = (struct list_node_s*) malloc(sizeof(struct list_node_s));
        temp_p->data = value;
        temp_p->next_p = curr_p;
        if (prep_p == NULL) {
            *head_p = temp_p;
        } else {
            prep_p->next_p = temp_p;
        }
        return 1;
    } else {
        return 0;
    }
}

int delete_node(int value, struct list_node_s **head_p) {
    struct list_node_s *curr_p = *head_p;
    struct list_node_s *prep_p = NULL;

    while(curr_p != NULL && curr_p->data < value){
        prep_p = curr_p;
        curr_p = curr_p->next_p;   
    }

    if (curr_p == NULL || curr_p->data > value) {
        return 0;
    } else {
        if (prep_p == NULL) {
            *head_p = curr_p->next_p;
        } else {
            prep_p->next_p = curr_p->next_p;
        }
        return 1;
    }
}

int gen_rand_list(int size) {
    int count = 0;
    srand((unsigned) time(0));
    while(count<size) {
        if (insert(rand()%MAX_VAL, &head_p)==1) {
            count++;
        }
    }
}

void *thread_routine(void* params) {

    int mem_samples = ((struct thread_routine_params*) params)->member_operations;
    int insert_samples = ((struct thread_routine_params*) params)->insert_operations;
    int delete_samples = ((struct thread_routine_params*) params)->delete_operations;

    int operation;
    srand((unsigned) time(0));
    while(mem_samples > 0 || insert_samples > 0 || delete_samples > 0) {
        operation = rand() % 3;
        if (operation == 0 && mem_samples>0) {
            pthread_rwlock_rdlock(linked_list_rwlock);
            memeber(rand()%MAX_VAL, head_p);
            pthread_rwlock_unlock(linked_list_rwlock);
            mem_samples--;
        } else if (operation == 1 && insert_samples>0) {
            pthread_rwlock_wrlock(linked_list_rwlock);
            insert(rand()%MAX_VAL, &head_p);
            pthread_rwlock_unlock(linked_list_rwlock);
            insert_samples--;
        } else if (delete_samples>0){
            pthread_rwlock_wrlock(linked_list_rwlock);
            delete_node(rand()%MAX_VAL, &head_p);
            pthread_rwlock_unlock(linked_list_rwlock);
            delete_samples--;
        }
    }
    return 0;
}

void free_list(struct list_node_s **head_pp) {
    struct list_node_s *curr_p;
    struct list_node_s *succ_p;

    if (is_Empty(*head_pp)) return;
    curr_p = *head_pp;
    succ_p = curr_p->next_p;
    while (succ_p != NULL) {
        free(curr_p);
        curr_p = succ_p;
        succ_p = curr_p->next_p;
    }

    free(curr_p);
    *head_pp = NULL;
}


int is_Empty(struct list_node_s *head_p) {
    if (head_p == NULL)
        return 1;
    else
        return 0;
}