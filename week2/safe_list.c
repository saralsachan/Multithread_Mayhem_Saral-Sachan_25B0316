#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

typedef struct Node{
    int value;
    struct Node* next;
}Node;

typedef struct{
    Node* head;
    pthread_mutex_t lock; //coarse grained lock protecting whole list
}ThreadSafeList;

ThreadSafeList list;

void list_init(void){
    list.head = NULL;   
    pthread_mutex_init(&list.lock, NULL);
}

void list_insert(int val){
    Node* newNode = (Node*) malloc(sizeof(Node));
    newNode->value = val;

    pthread_mutex_lock(&list.lock);
    newNode->next = list.head;
    list.head = newNode;
    pthread_mutex_unlock(&list.lock);
}

int list_contains(int val){
    pthread_mutex_lock(&list.lock);

    Node* curr = list.head;
    while(curr!=NULL){
        if(curr->value == val){
            pthread_mutex_unlock(&list.lock);
            return 1; // found
        }
        curr = curr->next;
    } 
    pthread_mutex_unlock(&list.lock); //realease the lock in case not found too
    //Always release the lock before returning from the function
    return 0; //not found
}

void list_remove(int val){

    pthread_mutex_lock(&list.lock);
    Node* curr = list.head;
    Node* prev = NULL;

    while(curr!=NULL){
        if(curr->value == val){
            if(prev == NULL){
                list.head = curr->next; //removing the head
            }
            else{
                prev->next = curr->next; //remove middle curr
            }
            free(curr);
            break; //deleting the first occurence
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock(&list.lock);
}

void list_destroy(void){ //void argument means no arguments. Why not leave it empty? Bcs in C , the compiler may not raise any error if you even pass arguments
    
    pthread_mutex_lock(&list.lock);
    Node* curr = list.head;
    while(curr!=NULL){
        Node* temp = curr;
        curr = curr->next;
        free(temp);
    }
    list.head = NULL;
    pthread_mutex_unlock(&list.lock);
    pthread_mutex_destroy(&list.lock);

}

///STRESS TEST
void* worker_thread(void* arg) {
    // Each thread performs 1000 operations
    for (int i = 0; i < 1000; i++) {
        int val = rand() % 100; // Choose values randomly from a small range (0-99)
        int op = rand() % 3;    // Randomly pick an operation
        
        if (op == 0) {
            list_insert(val);
        } else if (op == 1) {
            list_contains(val);
        } else {
            list_remove(val);
        }
    }
    return NULL;
}    

int main(){
    list_init();

    int num_threads = 8;
    pthread_t threads[num_threads];

    for(int i=0; i<num_threads; i++){
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Stress test completed without crashing.\n");
    
    // Test basic verification
    list_insert(42);
    int contains_42 = list_contains(42);
    int contains_150 = list_contains(150); // Outside our 0-99 insertion range
    
    printf("list_contains(42)  [Just Inserted] : %d\n", contains_42);
    printf("list_contains(150) [Never Inserted]: %d\n", contains_150);
    
    list_destroy();

    
}