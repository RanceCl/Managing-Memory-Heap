/* mem.c
 * Roderick "Rance" White
 * roderiw
 * Lab4: Dynamic Memory Allocation
 * ECE 2230, Fall 2020
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "mem.h"

// Global variables required in mem.c only
// NEVER use DummyChunk in your allocation or free functions!!
static mem_chunk_t DummyChunk = {0, &DummyChunk};
static mem_chunk_t * Rover = &DummyChunk;   // one time initialization

// Global variables for print statistics functions
static int NumSbrkCalls = 0;                //Number of sbrk calls made
static int NumPages = 0;                    //Number of pages requested

// private function prototypes
void mem_validate(void);

/* function to request 1 or more pages from the operating system.
 *
 * new_bytes must be the number of bytes that are being requested from
 *           the OS with the sbrk command.  It must be an integer 
 *           multiple of the PAGESIZE
 *
 * returns a pointer to the new memory location.  If the request for
 * new memory fails this function simply returns NULL, and assumes some
 * calling function will handle the error condition.  Since the error
 * condition is catastrophic, nothing can be done but to terminate 
 * the program.
 *
 * You can update this function to match your design.  But the method
 * to test sbrk much not be changed.  
 */
mem_chunk_t *morecore(int new_bytes) 
{
    char *cp;
    mem_chunk_t *new_p;
    // preconditions that must be true for all designs
    assert(new_bytes % PAGESIZE == 0 && new_bytes > 0);
    assert(PAGESIZE % sizeof(mem_chunk_t) == 0);
    cp = sbrk(new_bytes);
    if (cp == (char *) -1)  /* no space available */
        return NULL;
    new_p = (mem_chunk_t *) cp;
    // You should add some code to count the number of calls
    // to sbrk, and the number of pages that have been requested
    NumSbrkCalls++; 
    NumPages += new_bytes/PAGESIZE;
    return new_p;
}

/* deallocates the space pointed to by return_ptr; it does nothing if
 * return_ptr is NULL.  
 *
 * This function assumes that the Rover pointer has already been 
 * initialized and points to some memory block in the free list.
 */
void Mem_free(void *return_ptr)
{
    // precondition
    assert(Rover != NULL && Rover->next != NULL);
    mem_chunk_t *Past=Rover, *p=Rover->next, *Retptr;

    Retptr = (mem_chunk_t *)return_ptr-1;

    //If there no next block yet
    if(Rover->next == Rover)
    {
        Retptr->next = Rover;
        Rover->next = Retptr;
    }
    /* Put memory where the Rover is pointing if not coalescing */
    else if(Coalescing != TRUE)
    {
        Retptr->next = Rover->next;
        Rover->next = Retptr;
    }

    /* Sort, if Coalescing */
    else
    {
        /* Scan through list for the original position of memory */
        do{
            //If address of return ptr is between two different ranges.
            if((Past + Past->size_units - 1 < Retptr) && (Retptr < p || p->size_units==0))
            {
                Retptr->next = p;
                Past->next = Retptr;
                break;
            }

            Past=Past->next;
            p=p->next;			                            //Shift to next block in list
        } while(p != Rover->next);

        /* Merge memory if coalescing and if the block is contiguous */
        //If the address after end of past address is the same as beginning of the next block address
        if(Past->size_units != 0 && ((Past + Past->size_units) == Retptr))
        {
            Past->size_units = Past->size_units + Retptr->size_units;   //Add unit sizes
            Past->next = Retptr->next;                                  //Point to memory after p
            Retptr=Past;                                                //In case future is also true
        }
        //If the address after end of current address is same as beginning of the current address
        if(p->size_units != 0 && ((Retptr + Retptr->size_units) == p))
        {
            Retptr->size_units = Retptr->size_units + p->size_units;    //Add unit sizes
            Retptr->next = p->next;                                     //Point to memory after p
        }

    }

    //Make Rover point to the memory that had return_ptr returned to it
    Rover = Retptr;
}

/* returns a pointer to space for an object of size nbytes, or NULL if the
 * request cannot be satisfied.  The memory is uninitialized.
 *
 * This function assumes that there is a Rover pointer that points to
 * some item in the free list.  
 */
void *Mem_alloc(const int nbytes)
{

    // precondition
    assert(nbytes > 0);
    assert(Rover != NULL && Rover->next != NULL);

    // Insert your code here to find memory block
    int nunits, new_bytes;
    mem_chunk_t *p=Rover, *q, *Current=Rover;

    /* Get the units needed from the bytes requested, add 1 for header */
    nunits=(nbytes/ sizeof(mem_chunk_t) ) + 1;	    // 1 unit is the size of a mem_chunk_t
    if((nbytes % sizeof(mem_chunk_t)) > 0)	    //If there's a remainder,
        nunits = nunits + 1;			    //add 1 to round up.

    /* Search for if a block of memory is in free list that is enough for allocation */
    do{
	/* If not looking for best fit, the first working space will do; break code after finding */
        if(Current->size_units >= nunits && SearchPolicy != BEST_FIT)
        {
            p=Current;
	    break;				    //Break loop
	}

	/* If looking for best fit, keep searching until the end of the loop for the best match*/
        //If we're looking for the best fit, look for the smallest fit size
        else if((p->size_units ==0 || Current->size_units < p->size_units) && Current->size_units >= nunits)
            p=Current;

        Current=Current->next;			    //Shift to next block in list
    } while(Current != Rover);

    /* If nothing of appropriate size could be found, add more to the list */
    //Units needed must be smaller than page size, since 1 unit is used for the header
    if(p->size_units < nunits)		            //If size is less than the units needed
    {
        //Move the Rover right before the dummy pointer
        while(Rover->next->size_units != 0) Rover = Rover->next;

	new_bytes = nunits * sizeof(mem_chunk_t);

        //First, add the rest needed to make the new_bytes a multiple of PAGESIZE
        if(new_bytes % PAGESIZE > 0) new_bytes = new_bytes + (PAGESIZE - (new_bytes % PAGESIZE));

        p = morecore(new_bytes);
        p->next = NULL;
        p->size_units = new_bytes / sizeof(mem_chunk_t);

        /* Putting new memory into the list */
        //If there is no next block yet
        if(Rover->next == Rover)
        {
            Rover->next = p;
            p->next = Rover;
        }
        //If there are already blocks present
        else 
        {
            //Place new memory after rover
            p->next = Rover->next;
            Rover->next = p;
        }
    }

    //Position Rover after block allocated from
    Rover=p->next;

    /* Check if block allocated from is now empty */
    while(Current->next != p) Current=Current->next;

    /* Carve out the memory needed from the block and leave the rest in the list */
    //Remove space from p
    p->size_units = p->size_units - nunits;     //Remove size from block of memory

    //If memory block is empty, remove it from list
    if(p->size_units == 0)
    {
        Current->next=Current->next->next;
        p->next=NULL;
    }

    p=p+p->size_units;                          //Shift pointer to header of memory to be sent
    p->size_units = nunits;
    p->next = NULL;
    q = p + 1;                                  //Move q to one unit past block header

    //Postconditions
    assert(p + 1 == q);                         // +1 means one unit or sizeof(mem_chunk_t)
    assert((p->size_units-1)*sizeof(mem_chunk_t) >= nbytes);
    assert((p->size_units-1)*sizeof(mem_chunk_t) < nbytes + sizeof(mem_chunk_t));
    assert(p->next == NULL);  // saftey first!
    return q;
}

/* prints stats about the current free list
 *
 * -- number of items in the linked list including dummy item
 * -- min, max, and average size of each item (in bytes)
 * -- total memory in list (in bytes)
 * -- number of calls to sbrk and number of pages requested
 *
 * A message is printed if all the memory is in the free list
 */
void Mem_stats(void)
{
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);
    mem_chunk_t *p = Rover;
    mem_chunk_t *start = p;

    int NumItems = 0, M = 0, MinSize=p->size_units, MaxSize=p->size_units, AvgSize = 0;

    /* If only the dummy chunk is present */
    if(Rover == Rover->next) 
    {
        NumItems = 1; 
        MinSize=0; 
        MaxSize=0; 
        AvgSize=0; 
    }
    else
    {
        //Change values if rover started at the dummy block
        if(p == (&DummyChunk))
        {
            MinSize=p->next->size_units; 
            MaxSize=p->next->size_units;
        }
        //While loop to scan list
        do {
            if(p != (&DummyChunk))
            {
                if(MinSize > p->size_units) MinSize = p->size_units;
                if(MaxSize < p->size_units) MaxSize = p->size_units;
                M = M + p->size_units;
            }
            NumItems = NumItems + 1;
            p=p->next;
        } while (p != start);

        /* Convert sizes to bytes */
        MinSize = MinSize * sizeof(mem_chunk_t);
        MaxSize = MaxSize * sizeof(mem_chunk_t);
        M = M * sizeof(mem_chunk_t);

        /* Calculations */
        AvgSize = M/(NumItems - 1);

    }
    mem_validate();

    printf("Number of items: %d\nMin Size: %d, Max Size: %d, Average Size: %d\nTotal Memory: %d\nsbrk Calls: %d, Total Pages: %d\n", NumItems, MinSize, MaxSize, AvgSize, M, NumSbrkCalls, NumPages);


    // One of the stats you must collect is the total number
    // of pages that have been requested using sbrk.
    // Say, you call this NumPages.  You also must count M,
    // the total number of bytes found in the free list 
    // (including all bytes used for headers).  If it is the case
    // that M == NumPages * PAGESiZE then print
    if(M == NumPages * PAGESIZE)
        printf("all memory is in the heap -- no leaks are possible\n");
}

/* print table of memory in free list 
 *
 * The print should include the dummy item in the list 
 *
 * A unit is the size of one mem_chunk_t structure
 */
void Mem_print(void)
{
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);

    mem_chunk_t *p = Rover;
    mem_chunk_t *start = p;

    do {
        printf("p=%p, size=%d (units), end=%p, next=%p %s\n", 
                p, p->size_units, p + p->size_units - 1, p->next, 
                p->size_units!=0?"":"<-- dummy");
        p = p->next;
    } while (p != start);
    printf("\n\n");

    mem_validate();
}

/* This is an experimental function to attempt to validate the free
 * list when coalescing is used.  It is not clear that these tests
 * will be appropriate for all designs.  If your design utilizes a different
 * approach, that is fine.  You do not need to use this function and you
 * are not required to write your own validate function.
 */
void mem_validate(void)
{
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);
    assert(Rover->size_units >= 0);
    int wrapped = FALSE;
    int found_dummy = FALSE;
    int found_rover = FALSE;
    mem_chunk_t *p, *largest, *smallest;

    // for validate begin at DummyChunk
    p = &DummyChunk;
    do {
        if (p->size_units == 0) {
            assert(found_dummy == FALSE);
            found_dummy = TRUE;
        } else {
            assert(p->size_units > 0);
        }
        if (p == Rover) {
            assert(found_rover == FALSE);
            found_rover = TRUE;
        }
        p = p->next;
    } while (p != &DummyChunk);
    assert(found_dummy == TRUE);
    assert(found_rover == TRUE);

    if (Coalescing) {
        do {
            if (p >= p->next) {
                // this is not good unless at the one wrap around
                if (wrapped == TRUE) {
                    printf("validate: List is out of order, already found wrap\n");
                    printf("first largest %p, smallest %p\n", largest, smallest);
                    printf("second largest %p, smallest %p\n", p, p->next);
                    assert(0);   // stop and use gdb
                } else {
                    wrapped = TRUE;
                    largest = p;
                    smallest = p->next;
                }
            } else {
                assert(p + p->size_units - 1 < p->next);
            }
            p = p->next;
        } while (p != &DummyChunk);
        assert(wrapped == TRUE);
    }
}
/* vi:set ts=8 sts=4 sw=4 et: */

