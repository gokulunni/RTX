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

#define TOTAL_MEM_SIZE (ImageRW_IRAM1ZILIMIT - 0x10008000);

/* Function Prototypes */
int coalescingTest();
int externalFragmentationTest();
int splitMergeTest();
int invalidArgs_memalloc_test();
int invalidArgs_memcountextfrag_test();
int invalidArgs_memdealloc_test();
int completeMemoryUsageTest();

int tests_passed;
int total_tests = 7;
void (*tests[total_tests]) (void) = {coalescingTest, externalFragmentationTest, splitMergeTest, 
                                     invalidArgs_memalloc_test, invalidArgs_memdealloc_test, invalidArgs_memcountextfrag_test,
                                     completeMemoryUsageTest};

int coalescingTest()
{
  void *pointers[3];
  int num_chunks = 3;
  int chunk_size = TOTAL_MEM_SIZE/num_chunks;
  
  for(int i = 0; i < num_chunks; i++)
  {
    pointers[i] = mem_alloc(chunk_size);
    if(pointers[i] == NULL)
    {
      printf("Error: Could not allocate %d bytes\n", chunk_size);
      return 0;
    }
  }

  for(int i = 0; i < num_chunks; i++)
  {
    mem_dealloc(pointers[i]);
  }

  //Try to alloc a large chunk and see if successful to verify coalescing
  if(mem_alloc(TOTAL_MEM_SIZE))
  {
    return 1;
  }

  return 0;
}

int externalFragmentationTest()
{
  int num_chunks = 10;
  int chunk_size = 10;
  void *pointers[num_chunks];
  for(int i = 0; i < num_chunks; i++)
  {
    pointers[i] = mem_alloc(chunk_size);
    if(pointers[i] == NULL)
    {
      printf("Error: Could not allocate %d bytes\n", chunk_size);
      return 0;
    }
  }

  for(int i = 0; i < 10; i++)
  {
    mem_dealloc(pointers[i]);
  }

  if(mem_count_extfrag(TOTAL_MEM_SIZE + 1) != 1)
  {
    return 0;
  }

  return 1;
}

int splitMergeTest()
{
  int num_allocs = 0;
  int num_deallocs = 0;
  int chunk_size = 10;
  int num_chunks = 10;
  void *pointers[num_chunks];

  for(int i = 0; i < num_chunks; i++)
  {
    pointers[i] = mem_alloc(chunk_size);
    if(pointers[i] == NULL)
    {
      printf("Error: Could not allocate %d bytes\n", chunk_size);
      return 0;
    }
    num_allocs++;
    if(mem_count_extfrag(TOTAL_MEM_SIZE + 1) != num_allocs + 1)
      return 0;
  }

  for(int i = 0; i < num_chunks; i++)
  {
    mem_dealloc(pointers[i]);
    num_deallocs++;
    if(mem_count_extfrag(TOTAL_MEM_SIZE + 1) != num_allocs - num_deallocs + 1)
      return 0;
  }
  return 1;
}

int invalidArgs_memalloc_test()
{
  if(mem_alloc(TOTAL_MEM_SIZE + 1) != NULL || mem_alloc(-1) != NULL)
    return 0;

  return 1;
}

int invalidArgs_memcountextfrag_test()
{

  //what return value should we expect?
  if (mem_count_extfrag(NULL) != NULL)
    return 0;

  return 1;
}

int invalidArgs_memdealloc_test()
{
  //How can we verify dealloc???

  //double free -> undefined behaviour
  void* tmp = mem_alloc(1);
  mem_dealloc(tmp);
  mem_dealloc(tmp) != NULL

  //Free on stack pointer
  void *stack_pointer;
  mem_dealloc(stack_pointer);

  return 1;
}

int completeMemoryUsageTest()
{
  int num_allocs = 10;
  int alloc_sizes = TOTAL_MEM_SIZE / num_allocs;
  void* tmps[num_allocs + 1];

  for (int i = 0; i < num_allocs; i++)
  {
    tmps[i] = mem_alloc(alloc_sizes);
  }

  //check to see if the next allocation goes through
  if (tmps[num_allocs] = mem_alloc(alloc_sizes) != NULL)
    return 0;

  for (int i = 0; i < num_allocs; i++)
  {
    mem_dealloc(tmps[i]);
  }
  return 1;
}

int main()
{
   
  U32 ret_val = 1234;

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

  printf("G04_test: START");  
  for(int i = 0; i < total_tests; i++)
  {
    if((*tests[i])())
    {
      printf("G04_test: test %d OK\n", i);
      passed++;
    }
    else
      printf("G04_test: test %d FAIL\n", i);
  }

  printf("%d/%d OK\n", passed, total_tests);
  printf("%d/%d FAIL\n", total_tests-passed, total_tests);
  printf("G04_test: END");
  
  /* printf has been retargeted to use the UART1,
     check putc function in uart_polling.c.
  */
  return 0;  
}
