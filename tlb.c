#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "tlb.h"
#include "cpu.h"
#include "mmu.h"
#include "kernel.h"

/* This is some of the code that I wrote. You may use any of this code
   you like, but you certainly don't have to.
*/

/* I defined the TLB as an array of entries,
   each containing the following:
   Valid bit: 1 bit
   Virtual Page: 20 bits
   Modified bit: 1 bit
   Reference bit: 1 bit
   Page Frame: 20 bits (of which only 18 are meaningful given 1GB RAM)
*/

//You can use a struct to get a two-word entry.
typedef struct {
  unsigned int vbit_and_vpage;  // 32 bits containing the valid bit and the 20bit
                                // virtual page number.
  unsigned int mr_pframe;       // 32 bits containing the modified bit, reference bit,
                                // and 20-bit page frame number
} TLB_ENTRY;



// This is the actual TLB array. It should be dynamically allocated
// to the right size, depending on the num_tlb_entries value 
// assigned when the simulation started running.

TLB_ENTRY *tlb;  

// This is the TLB size (number of TLB entries) chosen by the 
// user. 

unsigned int num_tlb_entries;

  //Since the TLB size is a power of 2, I recommend setting a
  //mask to perform the MOD operation (which you will need to do
  //for your TLB entry evicition algorithm, see below).
unsigned int mod_tlb_entries_mask;

//this must be set to TRUE when there is a tlb miss, FALSE otherwise.
BOOL tlb_miss; 


//If you choose to use the same representation of a TLB
//entry that I did, then these are masks that can be used to 
//select the various fields of a TLB entry.

#define VBIT_MASK   0x80000000  //VBIT is leftmost bit of first word
#define VPAGE_MASK  0x000FFFFF            //lowest 20 bits of first word
#define RBIT_MASK   0x80000000  //RIT is leftmost bit of second word
#define MBIT_MASK   0x40000000  //MBIT is second leftmost bit of second word
#define PFRAME_MASK 0x000FFFFF            //lowest 20 bits of second word


// Initialize the TLB (called by the mmu)
void tlb_initialize()
{

  //Here's how you can allocate a TLB of the right size
  tlb = (TLB_ENTRY *) malloc(num_tlb_entries * sizeof(TLB_ENTRY));
  
  //This is the mask to perform a MOD operation (see above)
  mod_tlb_entries_mask = num_tlb_entries - 1;  
  
  //Fill in rest here...
  
  //does anything have to be initialized? array already declared
  
  /*******clear all entries on completion*********/

  tlb_clear_all();
  

}

// This clears out the entire TLB, by clearing the
// valid bit for every entry.
void tlb_clear_all() 
{
  //printf("\nClear the entire TLB:\n");
  
  int count;
  int i;
  for(i = 0; i < num_tlb_entries; i++) {
		//mask each vbit to 1
  		tlb[i].vbit_and_vpage = (tlb[i].vbit_and_vpage & ~VBIT_MASK);
  		/*
  		if((tlb[i].vbit_and_vpage & VBIT_MASK) == 0){
  			printf("VBITS CLEARED");
  		}
  		*/
  }
  //printf("VBIT Clear successful\n");
}


//clears all the R bits in the TLB
void tlb_clear_all_R_bits() 
{
	
	//printf("Clearing all R Bits:\n");
	int i;
	for(i = 0; i < num_tlb_entries; i++) {
  
		//Clear RBITS
  		tlb[i].mr_pframe = (tlb[i].mr_pframe & ~RBIT_MASK);
  		
  		/*
  		if((tlb[i].mr_pframe & RBIT_MASK) == 0){
  			printf("RBITS CLEARED\n");
  		}
  		*/
 	 }
	//printf("RBITS cleared\n");
}


// This clears out the entry in the TLB for the specified
// virtual page, by clearing the valid bit for that entry.
// DONT GET TO YETT******
void tlb_clear_entry(VPAGE_NUMBER vpage) {
	
	//printf("ENTER CLEAR ENTRY****\n");
	int i;
	for ( i = 0; i < num_tlb_entries; i++) {
		if( (tlb[i].vbit_and_vpage & VPAGE_MASK) == vpage){
			tlb[i].vbit_and_vpage = (tlb[i].vbit_and_vpage & ~VBIT_MASK);
		}	
	}
}

// Returns a page frame number if there is a TLB hit. If there is a TLB
// miss, then it sets tlb_miss (see above) to TRUE.  It sets the R
// bit of the entry and, if the specified operation is a STORE,
// sets the M bit.

PAGEFRAME_NUMBER tlb_lookup(VPAGE_NUMBER vpage, OPERATION op)
{
	/* TLB LOOKUP IMP
	 *
	 * if there is a TLB entry for the virtual page  (vpage), set the RBIT
	 * set MBIT as well iff op == "STORE"
	 *
	 * return pageFrame
	 *
	 * if no entry for vpage , then set BOOL tlb_miss = TRUE; 
	 * 
	 */
	 int i;
	 int found = 0;
	 for( i = 0; i < num_tlb_entries; i++) {
	 
	 	//check if vbit is valid
	 	if( (tlb[i].vbit_and_vpage & VBIT_MASK) != 0){

	 		if( (tlb[i].vbit_and_vpage & VPAGE_MASK) == vpage){
	 			//if tlb entry (matches vpage) then set R bit	
	 			//mmu_modify_rbit_bitmap(vpage, (tlb[i].mr_pframe | RBIT_MASK));
	 			tlb[i].mr_pframe = (tlb[i].mr_pframe | RBIT_MASK);
	 			
	 			found = 1;
	 			tlb_miss = FALSE;
	 			
	 			if( op == STORE ){
	 				//printf("OPERATION STORE\n");
	 				//mmu_modify_mbit_bitmap(vpage, (tlb[i].mr_pframe | MBIT_MASK));
	 				tlb[i].mr_pframe = (tlb[i].mr_pframe | MBIT_MASK);
	 			}
	 			
	 			return (tlb[i].mr_pframe & PFRAME_MASK); 
	 		}
	 	  }	
	 	  
	  } //end for  
	  
	//check if entry was found  
	if( found == 0) {
		//printf("TLB MISS\n");
		tlb_miss = TRUE;
		
	}
	//tlb_miss = TRUE;  					SETS all tlbs tomiss without imp DUH
}

// Uses an NRU clock algorithm, where the first entry with
// either a cleared valid bit or cleared R bit is chosen.

int clock_hand = 0;  // points to next TLB entry to consider evicting


void tlb_insert(VPAGE_NUMBER new_vpage, PAGEFRAME_NUMBER new_pframe,
		BOOL new_mbit, BOOL new_rbit)
{

  // Starting at the clock_hand'th entry, find first entry to
  // evict with either valid bit  = 0 or the R bit = 0. If there
  // is no such entry, then just evict the entry pointed to by
  // the clock hand.
  //printf("TLB INSERT TIME\n");
  
  int i = clock_hand;
 do {
        // check the i th entry of the TLB to see if the V bit or R bit
        // is 0. If so, then break out of the loop.
        if( ((tlb[i].vbit_and_vpage & VBIT_MASK) == 0) || ((tlb[i].mr_pframe & RBIT_MASK) == 0) ){
        	//printf("VBIT IS VALID\n");
        	break;
        }

  i = (i+1) % num_tlb_entries;  //otherwise increment i, using the mod
                                //operator to wrap around correctly.
 } while (i != clock_hand);

  // Then, if the entry to evict has a valid bit = 1,
  // write the M and R bits of the entry back to the M and R
  // bitmaps, respectively, in the MMU (see mmu_modify_rbit_bitmap, etc.
  // in mmu.h)

 // if the V bit of tlb[i] is 1, then write the M & R bits back to the
    // MMU
    if( (tlb[i].vbit_and_vpage & VBIT_MASK) != 0) {
    	//printf("***WRITE BACK TO MMU***\n");
    	mmu_modify_rbit_bitmap((tlb[i].mr_pframe & PFRAME_MASK) , (tlb[i].mr_pframe & RBIT_MASK));
    	mmu_modify_mbit_bitmap((tlb[i].mr_pframe & PFRAME_MASK) , (tlb[i].mr_pframe & MBIT_MASK));
    }

  // Then, insert the new vpage, pageframe, M bit, and R bit into the
  // TLB entry that was just found (and possibly evicted).
    
  //write the new vpage, pframe, M, and R bits to tlb[i], and set its
     //valid bit    
     
    //sets vpage
    tlb[i].vbit_and_vpage = new_vpage;
	tlb[i].mr_pframe = new_pframe;
    //set valid bit
    tlb[i].vbit_and_vpage = (tlb[i].vbit_and_vpage | VBIT_MASK);

    //mmu_modify_pageframe_bitmap(new_pframe, 1);
    
    if( new_rbit == TRUE) {
    	tlb[i].mr_pframe = (tlb[i].mr_pframe | RBIT_MASK);
    } else {
    	tlb[i].mr_pframe = (tlb[i].mr_pframe & ~RBIT_MASK);
    }
    
    if( new_mbit == TRUE) {
    	tlb[i].mr_pframe = (tlb[i].mr_pframe | MBIT_MASK);
    } else {
    	tlb[i].mr_pframe = (tlb[i].mr_pframe & ~MBIT_MASK);
    }
    
  // Finally, set clock_hand to point to the next entry after the
  // entry found above.	
	
  //set clock_hand to (i + 1) % num_tlb_entries
  clock_hand = (i+1) % num_tlb_entries;
}


//Writes the M  & R bits in the each valid TLB
//entry back to the M & R MMU bitmaps.

void tlb_write_back()
{	
	//printf("VALID TB / WRITEBACK\n");
	int i;
	//PAGEFRAME_NUMBER pframe_num = tlb[i].mr_pframe;
	
	for ( i = 0; i < num_tlb_entries; i++ ) {
		
	 	if( (tlb[i].vbit_and_vpage & VBIT_MASK) != 0){
	
			mmu_modify_rbit_bitmap((tlb[i].mr_pframe & PFRAME_MASK) , (tlb[i].mr_pframe & RBIT_MASK));
			mmu_modify_mbit_bitmap((tlb[i].mr_pframe & PFRAME_MASK) , (tlb[i].mr_pframe & MBIT_MASK));
		
		}
	}
	
}

