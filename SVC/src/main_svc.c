/**
 * @brief:  main_svc.c, demonstrate svc as a gateway to os functions
 * @author: Yiqing Huang
 * @date:   2020/09/09
 * NOTE: Standard C library is not allowed in the final kernel code.
 *       A tiny printf function for embedded application development
 *       taken from http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
 *       and is configured to use UART1 to output.
 */

#include <LPC17xx.h>
#include "rtx.h"
#include "printf.h"
#include "uart_polling.h"


#ifdef RAM_TARGET
#define IROM_BASE  0x10000000
#else
#define IROM_BASE  0x0
#endif

/* #define HEAP_START 0x1000047c */
#define HEAP_END   0x10008000
#define FREE_HEADER_SIZE 12
#define ALLOC_HEADER_SIZE 4

extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
unsigned int heap_start;
int total_mem_size;

/* Function Prototypes */
int coalescingTest(void);
int externalFragmentationTest(void);
int splitMergeTest(void);
int invalidArgs_memalloc_test(void);
int invalidArgs_memcountextfrag_test(void);
int invalidArgs_memdealloc_test(void);
int completeMemoryUsageTest(void);
int whiteBoxTest(void);
int varyingSizesTest(void);
int randomOrderTest(void);

int total_tests = 5;
int (*tests[]) (void) = {/*coalescingTest,*/ externalFragmentationTest, /*splitMergeTest, */
                                     invalidArgs_memalloc_test, /*invalidArgs_memdealloc_test,*/ invalidArgs_memcountextfrag_test,
                                     /*completeMemoryUsageTest, */ whiteBoxTest, varyingSizesTest, randomOrderTest};
char *test_names[] = {/*"coalescingTest",*/ "externalFragmentationTest", /*"splitMergeTest", */
                                     "invalidArgs_memalloc_test", /*"invalidArgs_memdealloc_test",*/ "invalidArgs_memcountextfrag_test",
                                    /*"completeMemoryUsageTest", */ "whiteBoxTest", "varyingSizesTest", "randomOrderTest"};

int coalescingTest(void)
{
  void *pointers[3];
  int num_chunks = 3;
  int chunk_size = (total_mem_size - FREE_HEADER_SIZE/num_chunks) - ALLOC_HEADER_SIZE;
  
	int i;
  for(i = 0; i < num_chunks; i++)
  {
    pointers[i] = mem_alloc(chunk_size);
    if(pointers[i] == NULL)
    {
     #ifdef DEBUG_1
      printf("Error: Could not allocate %d bytes for chunk %d/%d\n", chunk_size, i+1, num_chunks);
      #endif
			for(int j = i-1; j >= 0; j++)
			{
				mem_dealloc(pointers[j]);
			}
			return 0;
    }
  }

  for(i = 0; i < num_chunks; i++)
  {
    mem_dealloc(pointers[i]);
  }

  //Try to alloc a large chunk and see if successful to verify coalescing
  void *alloc_success = mem_alloc(total_mem_size - FREE_HEADER_SIZE - ALLOC_HEADER_SIZE);
  
  if(alloc_success)
  {
    return 1;
  }

  return 0;
}

int externalFragmentationTest(void)
{
  int num_chunks = 10;
  int chunk_size = 10;
  void *pointers[10];
	
	int i;
  for(i = 0; i < num_chunks; i++)
  {
    pointers[i] = mem_alloc(chunk_size);
    if(pointers[i] == NULL)
    {
		#ifdef DEBUG_1
      	printf("Error: Could not allocate %d bytes\n", chunk_size);
      	#endif
			for(int j = i-1; j >= 0; j++)
			{
				mem_dealloc(pointers[j]);
			}
      return 0;
    }
  }

  for(i = 0; i < 10; i++)
  {
    mem_dealloc(pointers[i]);
  }
	int fragments = mem_count_extfrag(total_mem_size + 1);
  if(fragments!= 1)
  {
  		#ifdef DEBUG_1
		printf("Unexpected number of fragments (completeMemoryUsageTest): %d\n", fragments);
		#endif
    return 0;
  }

  return 1;
}

int completeMemoryUsageTest(void)
{
  int num_allocs = 10;
  int alloc_sizes = (total_mem_size - FREE_HEADER_SIZE/ num_allocs) - ALLOC_HEADER_SIZE;
  void* tmps[10 + 1];

  for (int i = 0; i < num_allocs; i++)
  {
    tmps[i] = mem_alloc(alloc_sizes);
		if(tmps[i] == NULL){
			#ifdef DEBUG_1
			printf("Error (completeMemoryUsageTest): Could not allocate %d bytes for chunk %d/%d\n", alloc_sizes, i+1, num_allocs);
			#endif
		}
		
		for(int j = i-1; j >= 0; j++)
		{
			mem_dealloc(tmps[j]);
		}
  }
	
	for(int i = 0; i < num_allocs; i++)
	{
		#ifdef DEBUG_1
		printf("Address (completeMemoryUsageTest): %x\n", tmps[i]);
		#endif
	}

  //check to see if the next allocation goes through
	tmps[num_allocs] = mem_alloc(alloc_sizes);
  if (tmps[num_allocs] != NULL)
    return 0;

  for (int i = 0; i < num_allocs; i++)
  {
    mem_dealloc(tmps[i]);
  }
  return 1;
}

int splitMergeTest(void)
{
  int num_allocs = 0;
  int num_deallocs = 0;
  int chunk_size = 10;
  int num_chunks = 10;
  void *pointers[10];

	int i;
  for(i = 0; i < num_chunks; i++)
  {
    pointers[i] = mem_alloc(chunk_size);
    if(pointers[i] == NULL)
    {
      #ifdef DEBUG_1
      printf("Error (splitMergeTest): Could not allocate %d bytes\n", chunk_size);
      #endif
      return 0;
    }
    num_allocs++;
		int fragments = mem_count_extfrag(total_mem_size + 1); //seems to enter infinite loop when traversing nodes in LL
    if(fragments != num_allocs + 1)
      return 0;
  }

  for(i = 0; i < num_chunks; i++)
  {
    mem_dealloc(pointers[i]);
    num_deallocs++;
		int fragments = mem_count_extfrag(total_mem_size + 1);
    if(fragments != num_allocs - num_deallocs + 1)
      return 0;
  }
  return 1;
}

int invalidArgs_memalloc_test(void)
{
	void *oversized_pointer = mem_alloc(total_mem_size + 1);
	void *invalid_pointer = mem_alloc(-1);
  if(oversized_pointer != NULL || invalid_pointer != NULL)
    return 0;

  return 1;
}

int invalidArgs_memcountextfrag_test(void)
{

  //what return value should we expect?
	int ret_val = mem_count_extfrag(NULL);
  if (ret_val != NULL)
    return 1;

  return 0;
}

int invalidArgs_memdealloc_test(void)
{
  //double free -> undefined behaviour
  void* tmp = mem_alloc(1);
  mem_dealloc(tmp);
  int status = mem_dealloc(tmp);
	if(status)
			return 0;

  //Free on stack pointer
	int stack_var = 5;
  int *stack_pointer = &stack_var;
  status = mem_dealloc(stack_pointer);
	if(status)
		return 0;

  return 1;
}

int whiteBoxTest(void)
{
	void* blocks[20];
	for(int i = 0; i < 20; i++)
	{
		blocks[i] = mem_alloc(20);
		//Ensure that there is only 1 fragmentation 
		
		unsigned int address = (unsigned int) blocks[i];
		int fragments= mem_count_extfrag(32000);
		if (fragments!=1){
			//We should always have one fragmentation until colesing
			return 0;
		}
	}
	
	//Deallocate some random block inbetween
	mem_dealloc(blocks[4]);
	
	int fragments= mem_count_extfrag(32000);
	if (fragments!=2){
		//We should always have two fragments now that we deallocated a block
		return 0;
	}

	
	for(int i = 0; i < 20; i++)
		if(i!=4){
			mem_dealloc(blocks[i]);
		}
	return 1;
}

int varyingSizesTest(void)
{
	void *pointers[69];
	int blocks = 69;
	for(int i = 0; i < 69; i++)
	{
		pointers[i] = mem_alloc((i+1)*2);
		if(pointers[i] == NULL)
		{
			#ifdef DEBUG_1
			printf("Error (varyingSizeTest): Could not allocate %d bytes for fragment %d\n", i*2, i);
			#endif
			for(int j = i-1; j >= 0; j--)
			{
				mem_dealloc(pointers[j]);
			}
      return 0;
		}
	}
	
	int frags = mem_count_extfrag(total_mem_size+1);
	if(frags != 1)
	{
		#ifdef DEBUG_1
		printf("Unexpected Result (varyingSizeTest): expected %d fragment but found %d\n", blocks, frags);
		#endif
		return 0;
	}
	
	for(int j = 0; j < 69; j++)
	{
			mem_dealloc(pointers[j]);
	}
		
	frags = mem_count_extfrag(total_mem_size+1);
	if(frags != 1)
	{
		#ifdef DEBUG_1
		printf("Unexpected Result (varyingSizeTest): expected 1 fragment but found %d\n", frags);
		#endif
		return 0;
	}
		
		
		return 1;
	
}

int randomOrderTest(void)
{
  void* pointer = mem_alloc(10);
  mem_dealloc(pointer);
  pointer = mem_alloc(20);

  if (pointer == NULL)
  {
    #ifdef DEBUG_1
    printf("(randomOrderTest) could not re-allocate memory\n");
    return 0;
    #endif
  }

  void* pointer1 = mem_alloc(30);
  mem_dealloc(pointer);
  mem_dealloc(pointer1);

  if (pointer != NULL || pointer1 != NULL)
  {
    #ifdef DEBUG_1
    printf("(randomOrderTest) could not deallocate memory on what was re-allocated\n");
    return 0;
    #endif
  }


  return 1;
}

int main()
{
   
  int ret_val;
	int passed = 0;
	
  SystemInit();  /* initialize the system */
  __disable_irq();
  uart_init(1);  /* uart1 uses polling for output */
  init_printf(NULL, putc);
  __enable_irq();
  
  #ifdef DEBUG_1
	printf("Dereferencing Null to get inital SP = 0x%x\r\n", *(U32 *)(IROM_BASE));
	printf("Derefrencing Reset vector to get intial PC = 0x%x\r\n", *(U32 *)(IROM_BASE + 4));
  printf("We are at privileged level, so we can access SP.\r\n"); 
	printf("Read MSP = 0x%x\r\n", __get_MSP());
	printf("Read PSP = 0x%x\r\n", __get_PSP());
	
	/* transit to unprivileged level, default MSP is used */
  __set_CONTROL(__get_CONTROL() | BIT(0));
  printf("We are at unprivileged level, we cannot access SP.\r\n");
	printf("Cannot read MSP = 0x%x\r\n", __get_MSP());
	printf("Cannot read PSP = 0x%x\r\n", __get_PSP());
#endif
	
	ret_val = mem_init(4, FIRST_FIT);
  if(ret_val == -1)
    return -1;

	/* Set heap start address, and total size */
	heap_start = Image$$RW_IRAM1$$ZI$$Limit + 4;
	total_mem_size = HEAP_END - heap_start;;
	
	/*Begin test */
  printf("G04_test: START\n");  
  for(int i = 0; i < total_tests; i++)
  {
    if((*tests[i])())
    {
      printf("G04_test: test %d (%s) OK\n", i+1, test_names[i]);
      passed++;
    }
    else
      printf("G04_test: test %d (%s) FAIL\n", i+1, test_names[i]);
  }

  printf("%d/%d OK\n", passed, total_tests);
  printf("%d/%d FAIL\n", total_tests-passed, total_tests);
  printf("G04_test: END\n");
  
  /* printf has been retargeted to use the UART1,
     check putc function in uart_polling.c.
  */
  return 0;  
}

