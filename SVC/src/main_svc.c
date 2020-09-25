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

#define HEAP_START 0x1000047c
#define HEAP_END   0x10008000
#define FREE_HEADER_SIZE 12
#define ALLOC_HEADER_SIZE 4
#define TOTAL_MEM_SIZE    (HEAP_END - HEAP_START - FREE_HEADER_SIZE)

/* Function Prototypes */
int coalescingTest(void);
int externalFragmentationTest(void);
int splitMergeTest(void);
int invalidArgs_memalloc_test(void);
int invalidArgs_memcountextfrag_test(void);
int invalidArgs_memdealloc_test(void);
int completeMemoryUsageTest(void);
int whiteBoxTest(void);

int total_tests = 7;
int (*tests[]) (void) = {/*coalescingTest,*/ externalFragmentationTest, splitMergeTest, 
                                     invalidArgs_memalloc_test, /*invalidArgs_memdealloc_test,*/ invalidArgs_memcountextfrag_test,
                                     completeMemoryUsageTest, whiteBoxTest};
char *test_names[] = {/*"coalescingTest",*/ "externalFragmentationTest", "splitMergeTest", 
                                     "invalidArgs_memalloc_test", /*"invalidArgs_memdealloc_test",*/ "invalidArgs_memcountextfrag_test",
                                     "completeMemoryUsageTest", "whiteBoxTest"};

int coalescingTest(void)
{
  void *pointers[3];
  int num_chunks = 3;
  int chunk_size = (TOTAL_MEM_SIZE/num_chunks) - ALLOC_HEADER_SIZE;
  
	int i;
  for(i = 0; i < num_chunks; i++)
  {
    pointers[i] = mem_alloc(chunk_size);
    if(pointers[i] == NULL)
    {
      printf("Error: Could not allocate %d bytes for chunk %d/%d\n", chunk_size, i+1, num_chunks);
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
	void *alloc_success = mem_alloc(TOTAL_MEM_SIZE)
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
      printf("Error: Could not allocate %d bytes\n", chunk_size);
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
	int fragments = mem_count_extfrag(TOTAL_MEM_SIZE + 1);
  if(fragments!= 1)
  {
		printf("Unexpected number of fragments: %d\n", fragments);
    return 0;
  }

  return 1;
}

int completeMemoryUsageTest(void)
{
  int num_allocs = 10;
  int alloc_sizes = (TOTAL_MEM_SIZE / num_allocs) - ALLOC_HEADER_SIZE;
  void* tmps[10 + 1];

  for (int i = 0; i < num_allocs; i++)
  {
    tmps[i] = mem_alloc(alloc_sizes);
		if(tmps[i] == NULL)
			printf("Error: Could not allocate %d bytes for chunk %d/%d\n", alloc_sizes, i+1, num_allocs);
		
		for(int j = i-1; j >= 0; j++)
		{
			mem_dealloc(tmps[j]);
		}
  }
	
	for(int i = 0; i < num_allocs; i++)
	{
		printf("Address: %x\n", tmps[i]);
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
      printf("Error: Could not allocate %d bytes\n", chunk_size);
      return 0;
    }
    num_allocs++;
		int fragments = mem_count_extfrag(TOTAL_MEM_SIZE + 1); //seems to enter infinite loop when traversing nodes in LL
    if(fragments != num_allocs + 1)
      return 0;
  }

  for(i = 0; i < num_chunks; i++)
  {
    mem_dealloc(pointers[i]);
    num_deallocs++;
		int fragments = mem_count_extfrag(TOTAL_MEM_SIZE + 1);
    if(fragments != num_allocs - num_deallocs + 1)
      return 0;
  }
  return 1;
}

int invalidArgs_memalloc_test(void)
{
	void *oversized_pointer = mem_alloc(TOTAL_MEM_SIZE + 1);
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
    return 0;

  return 1;
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
		//check that the first memory allocated starts at heap
		
		unsigned int address = (unsigned int) blocks[i];
		unsigned int expected = HEAP_START + 24 + ALLOC_HEADER_SIZE*(i+1) + (20 * i);
		if (address != expected)
			return 0;
	}
	
	for(int i = 0; i < 20; i++)
		mem_dealloc(blocks[i]);
  
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

	ret_val = mem_init(4, FIRST_FIT);
  if(ret_val == -1)
    return -1;

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

