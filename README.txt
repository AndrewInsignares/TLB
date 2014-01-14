README:

Description:
This project was to create a page table data structure using a multi-level page table. The first-level page table contains 1024 entries with each entry pointing to another page table. The second level has the same structure but points to a page frame number in memory. This information is kept organized and structured by using bit masking by setting “valid” and “null” bits. 

Run:

GCC compiler needed

In directory of files:

Compile tlb.c (only once):
gcc –m32 -c tlb.c 

Compile program:
gcc –m32 –o memory page.c tlb.o cpu.o mmu.o kernel.o 


OR:
make proj3

Output:

	Windows:
	proj3.exe
	
	Mac:
	proj3
