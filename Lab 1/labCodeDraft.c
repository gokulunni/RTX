
#include "rtx.h"
typedef struct __node_t {
    int size;
    struct __node_t *next;
    struct __node_t *prev;
} node_t;

int blockSize=0;
node_t *head=NULL;

int mem_init(size_t blk_size, int algo){
    int totalSizeOfMemory;
    head = mmap(NULL,totalSizeOfMemory,PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
    head->size = totalSizeOfMemory - sizeOf(node_t);
    head->next = NULL;
    head->prev = NULL;
    blockSize = blk_size;

    return 0; //Else -1 for failure
}

void *mem_alloc(size_t size){
    //iterate through linked list find a element bigger than size
    while(linked list not null){
        if( linkedList_Size>= size+ headerSize){
            
            if (linkedList_Size> size&& linkedList_Size > blockSize){
            //Split if size is bigger
                return split()
            }
            else if linkedList_size<blockSize{
                print out investigate this
            }
            else{
                //Otherwise return address
                return linedList.Address + sizeOf(node_t);
            }

        }

    }

}

void *split(size,linkedList){
    int splitSizeUnused= linkedList.Size()-size- sizeOf(node_t);
    //Store split size first (update linked list)

    //Create new linked list node for remaining split size

    //new node previous = linkedList.prev
    
    //new node.next = linkedList.next

    //Change pointers for linked list node before and linked list node after, to point to new node created

    
    return linkedList.Address + sizeOf(node_t); 
}

void mem_dealloc(void *ptr){
    // startingAddress= ptr- sizeOf(node_t)
    // endingAddress = ptr+ startingAddress.size()
    //Check if we need to set block size as bytes or as number of blocks

    //Iterate through the linked list

        //if address is equal startingAddress then extend the space of the node found

        //if address is equal endingAddress then extend the space of the deallocated- 
        //node then add to list and delete node that we took space from

        //ENSURE YOU ARE DEALING WITH FREE->deallocated->FREE

    return;
}


int mem_count_extfrag(size_t size){
    int counter=0;
    //Iterate through linked list
        
        //if node.size()< size {counter++}

    return counter;
}