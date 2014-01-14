#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "mmu.h"
#include "page.h"
#include "cpu.h"

/* The following machine parameters are being used:
   
   Number of bits in an address:  32
   Page size: 4KB

   Page Table Type:  2 level page table
   Size of first level page table: 1024 entries
   Size of first level Page Table Entry:  32 bits
   Size of each 2nd Level Page Table: 1024 entries
   Size of 2nd Level Page Table Entry: 32 bits

   Bits of address giving index into first level page table: 10
   Bits of address giving index into second level page table: 10
   Bits of address giving offset into page: 12

*/

/* Each entry of a 2nd level page table has
   the following:
     Present/Absent bit: 1 bit
     Page Frame: 20 bits
*/

/******THIS IS THE ENTRY var FOR SECOND LEVEL ******/
//first bit is PRESENT BIT

// This is the type definition for the 
// an entry in a second level page table

typedef unsigned int PT_ENTRY;

//when second level is created, use for loop over 1024 and set to PT_ENTRY

// This is declaration of the variable representing
// the first level page table

PT_ENTRY **first_level_page_table;

// This sets up the initial page table. The function
// is called by the MMU.
//
// Initially, all the entries of the first level 
// page table should be set to NULL. Later on, 
// when a new page is referenced by the CPU, the 
// second level page table for storing the entry
// for that new page should be created if it doesn't
// exist already. 

#define VBIT_MASK   0x80000000  //VBIT is leftmost bit of first word
#define VPAGE_MASK  0x000FFFFF            //lowest 20 bits of first word
#define RBIT_MASK   0x80000000  //RIT is leftmost bit of second word
#define MBIT_MASK   0x40000000  //MBIT is second leftmost bit of second word
#define PFRAME_MASK 0x000FFFFF            //lowest 20 bits of second word



void pt_initialize_page_table()
{
  //THIS IS FIRST WORKING SECTION
  /*
   *	similar to first assign initialization, init the FIRST PAGE
   *	set EVERY entry (1024 of em) to NULL
   *
   *	each entry will contain a pointer to a 2nd level page table, if 	
   *	referenced by the CPU, the second level is used to store the entry
   */
   
  //create first level page tabel using malloc() 
  first_level_page_table = (PT_ENTRY **)malloc(1024 * sizeof(PT_ENTRY*));

  //entries are treated as null as part of the compiler
  
  /* SECOND LEVEL
   *	will also contain 1024 entries
   *	each entry is a 32-bit word
   *	Present bit - (1bit)
   *	PageFrameNumber - (20bits)
   *
   *	ONLY CREATED WHEN NECESSARY
   *
   * 	Created in pt_update_pagetable()
   */
}



// for performing DIV by 1024 to index into the
// first level page table
#define DIV_FIRST_PT_SHIFT 10  



// for performing MOD  1024 to index in a 
// second level page table
#define MOD_SECOND_PT_MASK 0x3FF 

//If you choose to use the same representation of a TLB
//entry that I did, then these are masks that can be used to 
//select the various fields of a TLB entry.


BOOL page_fault;  //set to true if there is a page fault


//This is called when there is a TLB_miss.
// Using the page table, this looks up the page frame 
// corresponding to the specified virtual page.
// If the desired page is not present, the variable page_fault
// should be set to TRUE (otherwise FALSE).
PAGEFRAME_NUMBER pt_get_pageframe(VPAGE_NUMBER vpage)
{

    //THIS will occur in a page fault in the vpage doesn't occur
    //if no second level page table, or if present bit is 0
     
    int firstLevelIndex =  (vpage >> DIV_FIRST_PT_SHIFT);
		
	int secondLevelIndex = (vpage % MOD_SECOND_PT_MASK);	
	
	PT_ENTRY * secondLevel = first_level_page_table[firstLevelIndex];
   
    if(first_level_page_table[firstLevelIndex] != NULL) {
    	if( (secondLevel[secondLevelIndex] & VBIT_MASK) != 0) {
  		    page_fault = FALSE;
  		    return ((secondLevel[secondLevelIndex] & VPAGE_MASK));
		}
		
		//if( (secondLevel[secondLevelIndex] & VBIT_MASK) == 0) {
		//	printf("ENTRY DNE\n");
  		  //  page_fault = TRUE;
		//}
    	
    }
    
   // if(first_level_page_table[firstLevelIndex] == NULL) {
    //		printf("NULLLL");
	page_fault = TRUE;
    //}
    /*
    if( (entry2 & VPAGE_MASK) != 0 ) {
    		printf("PAGE EXISTS");
    }
    else{
    		printf("PAGEDNE");
	}
	*/
    
    //if page fualt -->should signal page fault has occured
    
    //return (vpage & PFRAME_MASK);
    //return the pageframe number corresponding to the desired page
}



// This inserts into the page table an entry mapping of the 
// the specified virtual page to the specified page frame.
// It might require the creation of a second-level page table
// to hold the entry, if it doesn't already exist.
void pt_update_pagetable(VPAGE_NUMBER vpage, PAGEFRAME_NUMBER pframe)
{
  /* SECOND LEVEL PAGE IS CREATED IF NEEDED
   *	will also contain 1024 entries
   *	each entry is a 32-bit word
   *	Present bit - (1bit)
   *	PageFrameNumber - (20bits)
   *
   *	ONLY CREATED when entry doesn't exists (Present Bit)
   *	2nd level page table is created with malloc()
   *
   *	The appropriate entry of the first level page table should then be 
   *	updated to point to the newly created second level page table
   * 	
   */
   int firstLevelIndex =  (vpage >> DIV_FIRST_PT_SHIFT);
		
   int secondLevelIndex = (vpage % MOD_SECOND_PT_MASK);	
	
   PT_ENTRY * secondLevel = first_level_page_table[firstLevelIndex];

   if(secondLevel == NULL){
				
  		//update first_level_page_table with new pointer entry1 to second level
  		secondLevel = (PT_ENTRY *)malloc(1024 * sizeof(PT_ENTRY));
  		
  		first_level_page_table[firstLevelIndex] = secondLevel;
   }
	//now second level exists
   if(secondLevel != NULL){
   		
   		//write in pageFrame
   		secondLevel[secondLevelIndex] = pframe;
   		
   		//set the present bit to 1
   		secondLevel[secondLevelIndex] = (secondLevel[secondLevelIndex] | VBIT_MASK);
   }
}


// This clears a page table entry by clearing its present bit.
// It is called by the OS (in kernel.c) when a page is evicted
// from a page frame.
void pt_clear_page_table_entry(VPAGE_NUMBER vpage)
{

	int firstLevelIndex =  (vpage >> DIV_FIRST_PT_SHIFT);
		
	int secondLevelIndex = (vpage % MOD_SECOND_PT_MASK);	
	
	PT_ENTRY * secondLevel = first_level_page_table[firstLevelIndex];
	
	if( first_level_page_table[firstLevelIndex] != NULL) {
   		
   		secondLevel[secondLevelIndex] = (secondLevel[secondLevelIndex] & ~VBIT_MASK);
		
	}		
}


