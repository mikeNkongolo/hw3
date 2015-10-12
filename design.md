# Malloc Design Document

## Data Struct
   * struct s_block
      - size    size_t
      - next    struct s_block*
      - prev    struct s_block*
      - ptr     void *
      - data[0] char* 
   * base void *   : starting point of heap
   * last s_block *: last visited block in FINd_block

## Interfaces
   * **void* mm_malloc(size_t size)**
       
      - Correctness Constraints
         - **malloc** allocates at least the number of bytes requested;
         - The pointer returned by malloc points to an allocated space where the program can read or write successfully;
         - No other call to malloc will allocate this space or any portion of it, unless the pointer has been freed before;
         - **malloc** should be tractable: malloc must terminate in as soon as possible;
         - The returned pointer should be NULL in case of failure.

      - Implementation
         - Align the requested size
         - If no block is allocated yet(base is not initialized
         -    Allocate block with aligned size by invoking *sbrk*
         -    Update base pointer
         - Else
         -    Traverse the chunks list and stop when find a free block with enough space
         -    If not able to find a fitting chunk
         -       Invoke extend_heap
         -       Update last visited pointer
         -    Else
         -       If the fitting chunk is wide enough to hold the requested size plus a new chunk(at least BLOCK_SIZE + 4)
         -         Split the block and insert a new block in the list
         - If allocate successfully 
         -    Update the fitting block's free flag to 0
         -    Set block's ptr to block's point 
         - Return the fitting block  

   * **void mm_free(void *p)**

      - Correctness Constraints
         - Find the correct chunk to be freed;
         - Avoid space fragmentation;
         - Allocator must combine two contiguous free chunks to achieve its allocation needs(fusion neighbors in one bigger chunk);
         - Avoid calls to sbrk whenever necessary.

      - Implementation
         - If the pointer p is valid and get a non-NULL block address)
         -   Mark the block free
         -   If previous block exists and is free, step backward and fusion two blocks
         -   Also try fusion with the next block
         -   If we are the last pointer, release memory.
         -   If there is no more block in the list, go back the original state(set base to NULL)
         - Else
         -   Silently do nothing??

   * **mm_realloc(void *pb, int size)**

      - Correctness Constraints
         - The new region is suitably aligned for requested size
         - workable and efficient
      - Implementation
         - If the size doesn't change, or the extra-available size(do to alignment constraint, or if the remainning size was too small to split) is sufficient
         -   Do nothing
         - If shrink the block
         -   Try a split
         - If the next block is free and provide enough space
         -   Fusion and try to split if necessary


   * Internal functions
          
      - **s_block_ptr find_block(size_t size)**
         - Traverse the chunks list and stop when find a free block with enough space
         - If the fitting block found, then
         -   Return the block
         - Else
         -   Return NULL
          
      - **s_block_tr extend_heap(size_t size)**
         - Invoke sbrk to extend break point
         - If sbrk fails, return NULL
         - Initialize the new block's members
         - If heap is empty(base==NULL), then
         -   Assign current break point to base point
         - Else  
         -   Append the new block to last block
         - Return the new node
          
      - **s_block_ptr split_block(s_block_ptr p, size_t new_size)**
         - If the block's size is greater or equal with (new_size + BLOCK_SIZE + 4)
         -    Shorten the block'size to new_size
         -    Define a new block on the address p->data+size
         -    Insert the new block after block p
         - Return the new block's address
              
      - **s_block_ptr get_block(void* p)**
         - If p is a valid addr, then
         -   Return  (s_block_ptr)p
         - Else return NULL
            
      - **int is_addr_valid(void *p)**
         - Verify if p in not NULL
         - Verify if [p, aligned(p+BLOCK_SIZE+p->size)] is within range of mapped addresses
         - Verify if p's ptr member is equal to p itself
            
      - **void fusion_block(s_block_ptr p)**
         - Verify p is a free block
         - If p's next block exists and p's block is also free, then
         -   Merge p and p's next block

      - **void copy_block(s_block_ptr src, s_block_ptr dst)** : Copy data from block to block
         - convert src and dst to int*
         - copy data from dst to src int by int
         
   * Others
      - MACRO: #define align4(x) (((((x) -1) > >2) < <2)+4)
       
## Test Design 
   * Function Test
      - Simple test: invoke mm_malloc to allocate 4 byes and use mm_free to free it
      - Allocate zero byte: invoke mm_malloc to allocate 0 byes and use mm_free to free it
      - Allocate negative number of memory: invoke mm_malloc to allocate negative number of memory  
      - Verify the first fitting algorithm works correctly
      - Allocate some memory to point array with random sizes, then free the points, verify break point is the orignal point
      - Extra test: invoke malloc and free in multi-threading environment
      
   * System Test    
      - Limitation test
      - Recovery Test
      - Stress Test
      - Longevity
   * Performance Test    
      - Run benchmark

## Improvement(Bonus)
   * Threadsafe: protect the datastructures such that multiple threads can make allocations at the same time.  
   * Defer allocations larger than a page size to mmap with anonymous map 
   * Try out allocation algorithm: the buddy allocator
   * Implement realloc properly to extend the current allocation chunk if possible
     
## Questions
   * Why requested size of malloc needs to be aligned?  
      
## Reference
   * Malloc Tutorial 
     http://www.inf.udec.cl/~leo/Malloc_tutorial.pdf

   * A Scalable Concurrent malloc(3) Implementation for FreeBSD
     http://people.freebsd.org/~jasone/jemalloc/bsdcan2006/jemalloc.pdf

   * Buddy memory allocation
     https://en.wikipedia.org/wiki/Buddy_memory_allocation

   * You can defer allocations larger than a page size to mmap
      A free interval  tree?
      mmap(2)

   * 浅析linux内核内存管理之buddy system
      http://blog.csdn.net/hsly_support/article/details/7483113

    

