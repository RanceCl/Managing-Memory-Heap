# makefile for MP4
# Version: 1
#
# -lm is used to link in the math library
#
# -Wall turns on all warning messages 
#
comp = gcc
comp_flags = -g -Wall 
comp_libs = -lm  

lab4 : llist.o mem.o lab4.o
	$(comp) $(comp_flags) llist.o mem.o lab4.o -o lab4 $(comp_libs)

llist.o : llist.c datatypes.h llist.h
	$(comp) $(comp_flags) -c llist.c

mem.o : mem.c mem.h
	$(comp) $(comp_flags) -c mem.c

lab4.o : lab4.c datatypes.h llist.h mem.h
	$(comp) $(comp_flags) -c lab4.c

clean :
	rm -f *.o lab4 core

