/* llist.c                   
 * Roderick "Rance" White
 * roderiw
 * ECE 2230 Fall 2020
 * MP4
 *
 * Propose: Holds various functions to be used by the functions within the ids_support.c file. 
 * These functions can create and destroy doubly linked lists, add and remove data pointers 
 * from these lists, return data pointers from these lists based on certain information, and 
 *access information within them, if indicated.
 *
 * Assumptions: Many details are incomplete.  
 *
 * Bugs: Many details have not been implemented.
 *
 */

#include <stdio.h>
#include <string.h>


#include <stdlib.h>
#include <assert.h>

#include "llist.h"        // defines public functions for list ADT

// definitions for private constants used in llist.c only

#define LLIST_SORTED    999999
#define LLIST_UNSORTED -888888

// prototypes for private functions used in llist.c only 

void llist_debug_validate(llist_t *L);

llist_elem_t * FindMax(llist_t *A, llist_elem_t *m, llist_elem_t *n);
void HalfList(llist_t *A, llist_t *L, llist_t *R);
void MergeList(llist_t *A, llist_t *L, llist_t *R);
void SelectionSort(llist_t *A, llist_elem_t *m, llist_elem_t *n);
void IterativeSelectionSort(llist_t *A, llist_elem_t *m, llist_elem_t *n);
void MergeSort(llist_t *A);

/* ----- below are the functions  ----- */

/* Obtains a pointer to an element stored in the specified list, at the
 * specified list position
 * 
 * list_ptr: pointer to list-of-interest.  A pointer to an empty list is
 *           obtained from llist_construct.
 *
 * pos_index: position of the element to be accessed.  Index starts at 0 at
 *            head of the list, and incremented by one until the tail is
 *            reached.  Can also specify LLPOSITION_FRONT and LLPOSITION_BACK
 *
 * return value: pointer to element accessed within the specified list.  A
 * value NULL is returned if the pos_index does not correspond to an element in
 * the list.
 */
data_t * llist_access(llist_t *list_ptr, int pos_index)
{
    int count=0;
    llist_elem_t *L;
 
    // debugging function to verify that the structure of the list is valid
//    llist_debug_validate(list_ptr);

    /* handle special cases.
     *   1.  The list is empty
     *   2.  Asking for the head 
     *   3.  Asking for the tail
     *   4.  specifying a position that is out of range.  This is not defined
     *       to be an error in this function, but instead it is assumed the 
     *       calling function correctly specifies the position index
     */
    if (llist_entries(list_ptr) == 0) {
        return NULL;  // list is empty
    }
    else if (pos_index == LLPOSITION_FRONT || pos_index == 0) {
        return list_ptr->ll_front->data_ptr;
    }
    else if (pos_index == LLPOSITION_BACK || pos_index == llist_entries(list_ptr) - 1) {
        return list_ptr->ll_back->data_ptr;
    }
    else if (pos_index < 0 || pos_index >= llist_entries(list_ptr))
        return NULL;   // does not correspond to position in list

    // loop through the list until find correct position index
    L=list_ptr->ll_front;
    for (count = 0; count < pos_index; count++) {
        L = L->ll_next;
    }
    // already verified that pos_index should be valid so L better not be null
    assert(L != NULL);
    assert(L->data_ptr != NULL);
    return L->data_ptr;
}

/* Allocates a new, empty list 
 *
 * If the comparison function is NULL, then the list is unsorted.
 *
 * Otherwise, the list is initially assumed to be sorted.  Note that if 
 * list_insert is used the list is changed to unsorted.  
 *
 * The field sorted can only take values LLIST_SORTED or LLIST_UNSORTED
 *
 * Use llist_destruct to remove and deallocate all elements on a list 
 * and the header block.
 */
llist_t * llist_construct(int (*fcomp)(const data_t *, const data_t *))
{
    llist_t *L;
    L = (llist_t *) malloc(sizeof(llist_t));
    L->ll_front = NULL;
    L->ll_back = NULL;
    L->ll_entry_count = 0;
    L->compare_fun = fcomp;
    if (fcomp == NULL)
        L->ll_sorted_state = LLIST_UNSORTED;
    else
        L->ll_sorted_state = LLIST_SORTED;

//    llist_debug_validate(L);
    return L;
}

/* Finds an element in a list and returns a pointer to it.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: element against which other elements in the list are compared.
 * 
 * fcomp: function to test if elem_ptr is a match to an item in the list.
 *        Returns 0 if they match and any value not equal to 0 if they do not match.
 *
 * NOTICE: pos_index is returned and is not an input value!
 *
 * The function returns a pointer to the matching element with lowest index if
 * a match if found.  If a match is not found the return value is NULL.
 *
 * The function also returns the integer position of matching element with the
 *           lowest index.  If a matching element is not found, the position
 *           index that is returned should be -1. 
 *
 * pos_index: used as a return value for the position index of matching element
 *
 */
data_t * llist_elem_find(llist_t *list_ptr, data_t *elem_ptr, int *pos_index,
        int (*fcomp)(const data_t *, const data_t *))
{
//    llist_debug_validate(list_ptr);

    //If the list is empty
    if (llist_entries(list_ptr) == 0) 
    {
        *pos_index = -1;
        return NULL;
    }

    int i=0, Compare;
    llist_elem_t *CurrElem;

    CurrElem=list_ptr->ll_front;

    Compare=fcomp(elem_ptr, CurrElem->data_ptr);

    /* Compare each data pointer until a match is made or the end of the list is reached. */
    //Loops until the end of the list is reached or a match has been made.
    while(i<list_ptr->ll_entry_count-1 && Compare != 0)
    {
        i=i+1;
        CurrElem=CurrElem->ll_next;
        Compare=fcomp(elem_ptr, CurrElem->data_ptr);
    }

    //If a match has been made by the end.
    if(Compare == 0)
    {
        *pos_index = i;         //i is the index of the variable
        return CurrElem->data_ptr;
    }

    else
    {
        *pos_index = -1;
        return NULL;
    }
}

/* Deallocates the contents of the specified list, releasing associated memory
 * resources for other purposes.
 */
void llist_destruct(llist_t *list_ptr)
{
    // the first line must validate the list 
//    llist_debug_validate(list_ptr);

    llist_elem_t *NextElem, *CurrElem=list_ptr->ll_front;

    /* While loop to free all information within the list */
    while(CurrElem != NULL)
    {
        NextElem=CurrElem->ll_next;
        free(CurrElem->data_ptr);
        free(CurrElem);
        CurrElem=NULL;
        CurrElem = NextElem;
    }
    free(list_ptr);                 //Free list
}

/* Inserts the specified data element into the specified list at the specified
 * position.
 *
 * llist_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * pos_index: numeric position index of the element to be inserted into the 
 *            list.  Index starts at 0 at head of the list, and incremented by 
 *            one until the tail is reached.  The index can also be equal
 *            to LLPOSITION_FRONT or LLPOSITION_BACK (these are special negative 
 *            values use to provide a short cut for adding to the head
 *            or tail of the list).
 *
 * If pos_index is greater than the number of elements currently in the list, 
 * the element is simply appended to the end of the list (no additional elements
 * are inserted).
 *
 * Note that use of this function results in the list to be marked as unsorted,
 * even if the element has been inserted in the correct position.  That is, on
 * completion of this subroutine the llist_ptr->ll_sorted_state must be equal 
 * to LLIST_UNSORTED.
 */

void llist_insert(llist_t *list_ptr, data_t *elem_ptr, int pos_index)
{
    assert(list_ptr != NULL);

    int i=1;

    llist_elem_t *NewElem, *PrevElem, *NextElem;

    /* New elem_ptr must be put into an newly allocated llist_elem_t and have values initialized */
    NewElem = (llist_elem_t *) malloc(sizeof(llist_elem_t));
    NewElem->data_ptr = elem_ptr;
    NewElem->ll_prev = NULL;
    NewElem->ll_next = NULL;

    /* If list is empty */
    if(list_ptr->ll_entry_count == 0)
    {
        list_ptr->ll_front = NewElem;
        list_ptr->ll_back = NewElem;
    }

    /* If pos_index indicates to the start of the list, shift list and make NewElem the ll_front */
    else if(pos_index==0 || pos_index==LLPOSITION_FRONT)
    {
        NewElem->ll_next = list_ptr->ll_front;          //NewElem's next will point to the current ll_front
        list_ptr->ll_front->ll_prev = NewElem;          //Current ll_front's prev shall now point to NewElem
        list_ptr->ll_front = NewElem;                    //Make NewElem the new ll_front
    }

    /* If pos_index indicates to the end of the list, shift list and make NewElem the ll_back */
    else if(pos_index > list_ptr->ll_entry_count || pos_index==LLPOSITION_BACK)
    {
        NewElem->ll_prev = list_ptr->ll_back;           //NewElem's prev will point to the current ll_back
        list_ptr->ll_back->ll_next = NewElem;           //Current ll_back's next shall now point to NewElem
        list_ptr->ll_back = NewElem;                    //Make NewElem the new ll_back
    }

    /* Otherwise, insert the new element at the specific index */
    else if(pos_index > 0)
    {
        PrevElem=list_ptr->ll_front;
        NextElem=list_ptr->ll_front->ll_next;

        /* Shifts along the elements being pointed to until the index is reached */
        for(i=1; i<pos_index; i++)
        {
            PrevElem=NextElem;
            NextElem=NextElem->ll_next;
        }

        /* Set the nexts and prevs of each entry to their new position */
        PrevElem->ll_next = NewElem;
        NextElem->ll_prev = NewElem;
        NewElem->ll_prev = PrevElem;
        NewElem->ll_next = NextElem;
    }

    //Increment up the entry count
    list_ptr->ll_entry_count=list_ptr->ll_entry_count + 1;
    

    // the last three lines of this function must be the following 
    if (list_ptr->ll_sorted_state == LLIST_SORTED) 
        list_ptr->ll_sorted_state = LLIST_UNSORTED;
//    llist_debug_validate(list_ptr);
}


/* Inserts the element into the specified sorted list at the proper position,
 * as defined by the compare_fun function pointer found in the header block.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * If you use llist_insert_sorted, the list preserves its sorted nature.
 *
 * If you use llist_insert, the list will be considered to be unsorted, even
 * if the element has been inserted in the correct position.
 *
 * If the list is not sorted and you call llist_insert_sorted, this subroutine
 * should generate a system error and the program should immediately stop.
 *
 * The comparison procedure must accept two arguments (A and B) which are both
 * pointers to elements of type data_t.  The comparison procedure returns an
 * integer code which indicates the precedence relationship between the two
 * elements.  The integer code takes on the following values:
 *    1: A should be closer to the list head than B
 *   -1: B should be closer to the list head than A
 *    0: A and B are equal in rank
 *
 * Note: if the element to be inserted is equal in rank to an element already
 *       in the list, the newly inserted element will be placed after all the 
 *       elements of equal rank that are already in the list.
 */
void llist_insert_sorted(llist_t *list_ptr, data_t *elem_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->ll_sorted_state == LLIST_SORTED);

    // insert your code here

    int Compare=0, i=0;
    llist_elem_t *NewElem, *PrevElem, *NextElem;
    /* New elem_ptr must be put into an newly allocated llist_elem_t and have values initialized */
    NewElem = (llist_elem_t *) malloc(sizeof(llist_elem_t));
    NewElem->data_ptr = elem_ptr;
    NewElem->ll_prev = NULL;
    NewElem->ll_next = NULL;

    /* If list is empty */
    if(list_ptr->ll_entry_count == 0)
    {
        list_ptr->ll_front = NewElem;
        list_ptr->ll_back = NewElem;
    }

    else
    {
        NextElem=list_ptr->ll_front;
        Compare=list_ptr->compare_fun(NewElem->data_ptr, NextElem->data_ptr);     //Compares the first data pointer

        while(Compare != 1 && NextElem != NULL)
        {
            i=i+1;
            PrevElem=NextElem;
            NextElem=NextElem->ll_next;

            /* End loop before compare if NextElem points to NULL */
            if(NextElem == NULL)
                break;

            Compare=list_ptr->compare_fun(NewElem->data_ptr,NextElem->data_ptr);     //Compares the data pointers
        }

        //If the end has been reached, count up one more time to make sure it is last
        if(NextElem == NULL)
            i=i+1;

        /* If it is at the beginning of the list */
        if(i==0)
        {
            NewElem->ll_next = list_ptr->ll_front;
            list_ptr->ll_front->ll_prev = NewElem;
            list_ptr->ll_front = NewElem;
        }

        /* If it at the end of the list, shift list and make NewElem the ll_back */
        else if(i>=list_ptr->ll_entry_count)
        {
            NewElem->ll_prev = list_ptr->ll_back;
            list_ptr->ll_back->ll_next = NewElem;
            list_ptr->ll_back = NewElem;
        }

        /* Otherwise, place new element in its new position */
        else
        {
            PrevElem->ll_next = NewElem;
            NextElem->ll_prev = NewElem;
            NewElem->ll_prev = PrevElem;
            NewElem->ll_next = NextElem;
        }


    }

    //Increment up the entry count
    list_ptr->ll_entry_count=list_ptr->ll_entry_count + 1;

    // the last line checks if the new list is correct 
//    llist_debug_validate(list_ptr);
}

/* Removes an element from the specified list, at the specified list position,
 * and returns a pointer to the element.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * pos_index: position of the element to be removed.  Index starts at 0 at
 *            head of the list, and incremented by one until the tail is
 *            reached.  Can also specify LLPOSITION_FRONT and LLPOSITION_BACK
 *
 * Attempting to remove an element at a position index that is not contained in
 * the list will result in no element being removed, and a NULL pointer will be
 * returned.
 */
data_t * llist_remove(llist_t *list_ptr, int pos_index)
{
//    llist_debug_validate(list_ptr);


    int i=1;
    llist_elem_t *RemovedElem, *PrevElem, *NextElem;
    data_t *RemovedDat;

    /* If the list is empty or if the index isn't contained in the list. */
    if (llist_entries(list_ptr) == 0 || pos_index >= list_ptr->ll_entry_count) 
    {
        return NULL;
    }

    //If there's only one element left in the list
    else if(llist_entries(list_ptr) == 1)
    {
        RemovedElem=list_ptr->ll_front;                 //Holding the element to be returned

        //Set pointers to and from removed element to NULL
        list_ptr->ll_front = NULL;
        list_ptr->ll_back = NULL;
    }

    /* If pos_index indicates to the start of the list */
    else if(pos_index==0 || pos_index==LLPOSITION_FRONT)
    {
        NextElem=list_ptr->ll_front->ll_next;           //New front placeholder
        RemovedElem=list_ptr->ll_front;                 //Holding the element to be returned
        list_ptr->ll_front = NextElem;                  //Replaces the old front element

        //Set pointers to and from removed element to NULL
        list_ptr->ll_front->ll_prev = NULL;
        RemovedElem->ll_next = NULL;
    }

    /* If pos_index indicates to the end of the list */
    else if(pos_index >= list_ptr->ll_entry_count-1 || pos_index==LLPOSITION_BACK)
    {
        PrevElem=list_ptr->ll_back->ll_prev;            //New back placeholder
        RemovedElem=list_ptr->ll_back;                  //Holding the element to be returned
        list_ptr->ll_back = PrevElem;                  //Replaces the old back element

        //Set pointers to and from removed element to NULL
        list_ptr->ll_back->ll_next = NULL;
        RemovedElem->ll_prev = NULL;
    }

    /* Otherwise, remove the new element at the specific index */
    else if(pos_index > 0)
    {
        RemovedElem=list_ptr->ll_front;
        NextElem=list_ptr->ll_front->ll_next;

        /* Shifts along the elements being pointed to until the index is reached */
        for(i=1; i<=pos_index; i++)
        {
            PrevElem=RemovedElem;
            RemovedElem=NextElem;
            NextElem=NextElem->ll_next;
        }

        /* Changes the next and previous pointers of the next and previous elements. */
        PrevElem->ll_next=NextElem;
        NextElem->ll_prev=PrevElem;

        /* Sets the previous and next pointers in the removed element to NULL. */
        RemovedElem->ll_next = NULL;
        RemovedElem->ll_prev = NULL;
    }

    //Increment down the entry count
    list_ptr->ll_entry_count=list_ptr->ll_entry_count - 1;

    RemovedDat = RemovedElem->data_ptr;
    free(RemovedElem);
//    return RemovedElem->data_ptr;

    return RemovedDat;
}


/* Obtains the length of the specified list, that is, the number of elements
 * that the list contains.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * Returns an integer equal to the number of elements stored in the list.  An
 * empty list has a size of zero.
 */
int llist_entries(llist_t *list_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->ll_entry_count >= 0);
    return list_ptr->ll_entry_count;
}


















/* Sorts a list of alerts based on specific sort_type that is sent by the
 * function.
 */
void llist_sort(llist_t *list_ptr, int sort_type, int (*fcomp)(const data_t *, const data_t *))
{
    llist_t *TempList = NULL;
    data_t *TempElem = NULL;

    list_ptr->compare_fun = fcomp;


    /* Insertion Sort */
    if(sort_type == 1)
    {
        TempList=llist_construct(fcomp);        //Create a temporary list for sorting
        //Until old list is empty
        while(list_ptr->ll_front != NULL)
        {
            TempElem=llist_remove(list_ptr,0);  //Removes current front of list, which is shifting
            llist_insert_sorted(TempList, TempElem);    //Puts removed element into sorted list
        }
        //Adjust pointers from list to the newly sorted list
        list_ptr->ll_front = TempList->ll_front;
        list_ptr->ll_back = TempList->ll_back;
        list_ptr->ll_entry_count = TempList->ll_entry_count;
        //Set TempList's pointers to NULL so TempList may be freed, and then free TempList
        TempList->ll_front = NULL;
        TempList->ll_back = NULL;
        TempList->ll_entry_count = 0;
        llist_destruct(TempList);
        printf("Size: %d\n", list_ptr->ll_entry_count);
//        free(TempElem);
    }

    /* Selection Sort */
    else if(sort_type == 2)
    {
        SelectionSort(list_ptr, list_ptr->ll_front, list_ptr->ll_back);
    }


    /* Iteritive Selection Sort */
    else if(sort_type == 3)
    {
        IterativeSelectionSort(list_ptr, list_ptr->ll_front, list_ptr->ll_back);
    }

    /* Merge Sort */
    else if(sort_type == 4)
    {
        MergeSort(list_ptr);
    }
    list_ptr->ll_sorted_state = LLIST_SORTED;
    llist_debug_validate(list_ptr);
}






llist_elem_t * FindMax(llist_t *A, llist_elem_t *m, llist_elem_t *n)   /* assume m<n */
{
	llist_elem_t *i = m;      /* i is a elem pointer that visits all positions from m to n */
	llist_elem_t *j = m;        /* j is a pointer that saves the position of the largest */


	do {
		i=i->ll_next;                      /* advance i to point to next number */

                /* if elem at i > largest previous elem at j */
		if (A->compare_fun(i->data_ptr,j->data_ptr) == 1) {
			j = i;              /* save the current i in j */
		}
	} while (i != n);           /* stop when all i in m:n have been tested */

	return j;                       /* return j == pointer to the largest elem */
}


void SelectionSort(llist_t *A, llist_elem_t *m, llist_elem_t *n)
{
	llist_elem_t *MaxPosition;      /* MaxPosition is the elem of A's biggest data_ptr  */
	data_t *temp;                     /* temp is used to exchange items in A */


	if (m != n) {               /* if there is more than one number to sort */
  	    /* Let MaxPosition be the index of the element that must be first in m through n */
   	    MaxPosition = FindMax(A,m,n);

            /* Exchange m's data_ptr and MaxPosition's data_ptr */
            temp = m->data_ptr;
            m->data_ptr = MaxPosition->data_ptr;
            MaxPosition->data_ptr = temp;

  	    /* SelectionSort the elements within positions m+1 through n */
    	    SelectionSort(A, m->ll_next, n);
	}
}




void IterativeSelectionSort(llist_t *A, llist_elem_t *m, llist_elem_t *n)
{
    llist_elem_t *MaxPosition, *i = m;
    data_t *temp;

    while (m < n) {
   
        i = m; 
        MaxPosition = m;

        do {
            i=i->ll_next;
            if ( A->compare_fun(i->data_ptr,MaxPosition->data_ptr) == 1 ) MaxPosition = i;
        } while (i != n);

        temp = m->data_ptr; m->data_ptr = MaxPosition->data_ptr; MaxPosition->data_ptr = temp;

        m=m->ll_next;
    }
}




void HalfList(llist_t *A, llist_t *L, llist_t *R)
{
    int OrigSize = A->ll_entry_count;
    data_t *TempElem = NULL;

    /* Separate half of list into the left list. */
    while(A->ll_entry_count > OrigSize/2)
    {
            TempElem=llist_remove(A,0);  //Removes current front of list, which is shifting
            llist_insert(L, TempElem, LLPOSITION_BACK);    //Puts removed element into list
    } 

    /* Separate half of list into the right list. */
    while(A->ll_entry_count !=0)
    {
            TempElem=llist_remove(A,0);  //Removes current front of list, which is shifting
            llist_insert(R, TempElem, LLPOSITION_BACK);    //Puts removed element into list
    } 

}

void MergeList(llist_t *A, llist_t *L, llist_t *R)
{
    data_t *TempL = NULL, *TempR = NULL;
    int Compare;

    TempL = llist_remove(L, 0);
    TempR = llist_remove(R, 0);
    while(TempL != NULL && TempR != NULL)
    {


        Compare = A->compare_fun(TempL,TempR);

        if (Compare == 1) {          /* if elem on left should be before elem on right then */
            llist_insert(A, TempL, LLPOSITION_BACK);
            TempL=llist_remove(L, 0);
        }
        else {
            llist_insert(A, TempR, LLPOSITION_BACK);
            TempR = llist_remove(R, 0);
        }
    }

    /* Put rest of left list into merged list. */
    while(TempL != NULL)
    {
            llist_insert(A, TempL, LLPOSITION_BACK);    //Puts removed element into list
            TempL=llist_remove(L,0);  //Removes current front of list, which is shifting

    } 

    /* Put rest of right list into merged list. */
    while(TempR != NULL)
    {
            llist_insert(A, TempR, LLPOSITION_BACK);    //Puts removed element into list
            TempR=llist_remove(R,0);  //Removes current front of list, which is shifting

    }

}


void MergeSort(llist_t *A)
{
    llist_t *L = NULL;
    llist_t *R = NULL;

    if (A->ll_entry_count > 1) {
        /* break the List into two half-lists, L = LeftList and R = RightList) */
        L=llist_construct(A->compare_fun);
        R=llist_construct(A->compare_fun);

        HalfList(A, L, R);

        /* (sort the LeftList using MergeSort(L)) */
        MergeSort(L);
        /* (sort the RightList using MergeSort(R)) */
        MergeSort(R);
        /* (merge L and R into a single sorted List) */
        MergeList(A, L, R);

        llist_destruct(L);
        llist_destruct(R);
    } //else {




//        (do nothing, since the list is already sorted)
//    }
}












/* This function verifies that the pointers for the two-way linked list are
 * valid, and that the list size matches the number of items in the list.
 *
 * If the linked list is sorted it also checks that the elements in the list
 * appear in the proper order.
 *
 * The function produces no output if the two-way linked list is correct.  It
 * causes the program to terminate and print a line beginning with "Assertion
 * failed:" if an error is detected.
 *
 * The checks are not exhaustive, so an error may still exist in the
 * list even if these checks pass.
 *
 * YOU MUST NOT CHANGE THIS FUNCTION.  WE USE IT DURING GRADING TO VERIFY THAT
 * YOUR LIST IS CONSISTENT.
 */
void llist_debug_validate(llist_t *L)
{
    llist_elem_t *N;
    int count = 0;
    assert(L != NULL);
    if (L->ll_front == NULL)
        assert(L->ll_back == NULL && L->ll_entry_count == 0);
    if (L->ll_back == NULL)
        assert(L->ll_front == NULL && L->ll_entry_count == 0);
    if (L->ll_entry_count == 0)
        assert(L->ll_front == NULL && L->ll_back == NULL);
    assert(L->ll_sorted_state == LLIST_SORTED || L->ll_sorted_state == LLIST_UNSORTED);

    if (L->ll_entry_count == 1) {
        assert(L->ll_front == L->ll_back && L->ll_front != NULL);
        assert(L->ll_front->ll_next == NULL && L->ll_front->ll_prev == NULL);
        assert(L->ll_front->data_ptr != NULL);
    }
    if (L->ll_front == L->ll_back && L->ll_front != NULL)
        assert(L->ll_entry_count == 1);
    if (L->ll_entry_count > 1) {
        assert(L->ll_front != L->ll_back && L->ll_front != NULL && L->ll_back != NULL);
        N = L->ll_front;
        assert(N->ll_prev == NULL);
        while (N != NULL) {
            assert(N->data_ptr != NULL);
            if (N->ll_next != NULL) assert(N->ll_next->ll_prev == N);
            else assert(N == L->ll_back);
            count++;
            N = N->ll_next;
        }
        assert(count == L->ll_entry_count);
    }
    if (L->ll_sorted_state == LLIST_SORTED && L->ll_front != NULL) {
        N = L->ll_front;
        while (N->ll_next != NULL) {
            assert(L->compare_fun(N->data_ptr, N->ll_next->data_ptr) != -1);
            N = N->ll_next;
        }
    }
}
/* commands for vim. ts: tabstop, sts: softtabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
