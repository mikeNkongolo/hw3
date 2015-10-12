/***************************************************************************
 *
 mike
 *
 ***************************************************************************/

/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
 */

#include "mm_alloc.h"

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

/* Your final implementation should comment out this macro. */
/* #define MM_USE_STUBS */
#define ASSERT     assert
#define align4(x)  (((((x)-1)>>2)<<2)+4)


/* Global variables                                                      */
s_block_ptr base = NULL; /* beginning point of heap                @HW3A */
s_block_ptr last;        /* last visited block                     @HW3A */

/* Internal functions                                              @HW3A */

/* Split block according to size, b must exist */
/* void split_block (s_block_ptr b, size_t s); */

/* Try fusing block with neighbors             */
/* s_block_ptr fusion(s_block_ptr b);          */

/* Get the block from addr                     */
/* s_block_ptr get_block (void *p);            */

/* Add a new block at the of heap,
 * return NULL if things go wrong
 */
/* s_block_ptr extend_heap (s_block_ptr last , size_t s); */


/* Traverse the chunks list and stop when find a free block with enough space */
static s_block_ptr find_block(size_t size)    /*                @HW3A */
{  
  s_block_ptr p = base;
  for (; p != NULL; p = p->next)              /*                @HW3A */
   {
      last = p;
      if (p->free && p->size >= size)         /*                @HW3A */
          return p;                           /*                @HW3A */
   }
    
  return NULL;                                /*                @HW3A */
}

/* Invoke sbrk to extend break point                            @HW3A */
static s_block_ptr extend_heap (size_t size)                 /* @HW3A */
{    
    /* Current break is address of the new block                      */
    s_block_ptr new_block = (s_block_ptr)sbrk(0);            /* @HW3A */
    if ((int)sbrk(size + S_BLOCK_SIZE) == -1)                /* @HW3A */ 
        return NULL;                                         /* @HW3A */
    new_block->size = size;                                  /* @HW3A */
    new_block->ptr  = new_block->data;                       /* @HW3A */
    new_block->free = TRUE;                                  /* @HW3A */
     
    if (base == NULL) /* heap is empty                          @HW3A */
    {
        new_block->prev = new_block->prev = NULL;  /*           @HW3A */
        base = new_block;                          /*           @HW3A */
    }
    else 
    {
        ASSERT(last->next == NULL);                /*           @HW3A */
        new_block->next = NULL;                    /*           @HW3A */
        new_block->prev = last;                    /*           @HW3A */
        last->next = new_block;                    /*           @HW3A */
    }
    printf("%x %x %d\n", new_block, new_block->ptr, new_block->size); 
    return new_block;                              /*           @HW3A */
}

static s_block_ptr split_block(s_block_ptr p, size_t new_size)/*@HW3A */
{
    s_block_ptr new_block = NULL;              
    if (p->size >= new_size + S_BLOCK_SIZE + 4)
    {
        /* Shorten the block'size to new_size */
        p->size = new_size;  

        /* Define a new block on the address p->data+size            */
        new_block = (s_block_ptr)(p->data + new_size);
        new_block->size = p->size - new_size - S_BLOCK_SIZE;
        new_block->ptr = new_block->data;
        new_block->free = TRUE;

        /* Insert the new block after block p */
        new_block->next = p->next;
        new_block->prev = p;
        if (p->next)
            p->next->prev = new_block;
        p->next = new_block;
    }

    return new_block;
}

static s_block_ptr fusion_block(s_block_ptr pb)
{
    ASSERT(pb->free == TRUE); 

    /* If p's next block exists and p's block is also free, 
     * then merge p and p's next block.                    */
    if (pb->next && pb->next->free) 
    {
      pb->size = pb->size + BLOCK_SIZE + pb->next->size;
      if (pb->next->next)
        pb->next->next->prev = pb;
      pb->next = pb->next->next; 		
    }

    return (pb);
}


static s_block_ptr get_block(void *p)
{
    /* If p is a valid block address, convert it to a s_block_ptr */
    char *tmp;
    tmp = (char*)p;
    return (s_block_ptr)(tmp - BLOCK_SIZE);
}

static int is_valid_block_addr (void *p)
{
   s_block_ptr pb = get_block(p);
   if (base)
   {
     if(p > base && p < sbrk(0))
	return pb->ptr == p;
   }

   return FALSE;
}

static void copy_block(s_block_ptr src, s_block_ptr dst)
{
  int *sdata, *ddata; 
  size_t i;
  sdata = src->ptr;
  ddata = dst->ptr;
  for (i = 0; i * 4 < src->size && i * 4 < dst->size; i++)
    ddata[i] = sdata[i];
}

/*********************** Interfaces ***********************/
void* mm_malloc(size_t size)
{
#ifdef MM_USE_STUBS
    return calloc(1, size);
#else
    /* Align the requested size */
    size_t s = align4(size);    
    s_block_ptr pb;

    /* no block is allocated yet */
    if (base == NULL) 
    {
       pb = extend_heap(s);
       if (pb == NULL)
          return NULL;
       base = pb;
    }
    /* else already allocated blocks */
    else 
    {
       /* Find first fitting block */
       pb = find_block(s);
       if (pb == NULL)
        {
          pb = extend_heap(s);
          if (pb == NULL)
            return NULL;
        }
       else
        {
          if (pb->size - s >= S_BLOCK_SIZE + 4)
            split_block(pb, s);
        }
    }
        
    pb->free = FALSE;
    return pb->ptr;
#endif
}

void* mm_realloc(void* ptr, size_t size)
{
#ifdef MM_USE_STUBS
    return realloc(ptr, size);
#else
    size_t s;
    void *newp;
    s_block_ptr pb, new;

    /* exclude some special conditions */
    if (ptr == NULL)
        return mm_malloc(size);
    
    if ((pb=get_block(ptr)) != NULL)
    {
        s = align4(size);
        if (pb->size >= s)
         {
           /* If space is enough, try to split current block */
           if (pb->size - s >= (BLOCK_SIZE + 4))
             split_block(pb, s);
         }
    }
    else 
    {
        /* Try fusion with next if possible */
        if (pb->next && pb->next->free
            && (pb->size + BLOCK_SIZE + pb->next->size) >= s)
        {
           fusion_block(pb);
           if (pb->size - s >= BLOCK_SIZE + 4)
             split_block(pb, s);
        }
        else
        {
           /* no way to get enough space on current node, have to malloc new memory */
           newp = mm_malloc(s);
           if (newp == NULL)
               return NULL;
           new = get_block(newp);
           copy_block(pb, new);
           mm_free(pb);
           return (newp);  
        }

        return (pb);
    }
    
    return (NULL);
#endif
}

void mm_free(void* ptr)
{
#ifdef MM_USE_STUBS
    free(ptr);
#else
    /* #error Not implemented. */
    s_block_ptr pb;
    /* If the pointer ptr is valid and get a non-NULL block address */
    if ((pb=get_block(ptr)) != NULL)
    {
       pb->free = TRUE;  
       /* Step backward and fusion two blocks */
       if(pb->prev && pb->prev->free)
         fusion_block(pb->prev);
       /* Also try fusion with the next block */
       if(pb->next)
         fusion_block(pb);
       /* Else we are the last pointer, free the end of heap   */
       else
       {
          /* If we are also the head pointer, no more block    */
          if (pb->prev == NULL)
              base = NULL;
          /* Else release the end of heap                      */
          else
              pb->prev->next = NULL;     
          brk(pb);
       }
    }
    
#endif
}


