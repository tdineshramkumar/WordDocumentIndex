#ifndef _LINK_LIST_
#define _LINK_LIST_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Each node in the list 
struct _node { 
	int index ; // the index of file
	int count ; // the number of occurance of word
	struct _node * next ; // the next file with decreasing count
};
// Each list 
struct _head {
	struct _node * start ; // ordered start
	struct _node * end ; // ordered end
};

typedef struct _node Node ;
typedef struct _node *Nodeptr ;
typedef struct _head List  ;
typedef struct _head *Listptr ;

Listptr initlist() ; // creates a new list (basically a new head structure)
void insertlist(Listptr *h, int index, int count); // inserts a new entry into list 
void displaylist(Listptr *h) ; // prints a list
void mergelist(Listptr *h1, Listptr *h2); // merges h1 and h2 into h1 and free h2
void deletelist(Listptr *h); // deletes a list


// PACKING FUNCTIONS ...
// packlist function should pack the list into list of integers and return the size taken 
// it should delete the List and return number of nodes deleted
// head is also deleted ...
// Node it must be called of valid Listpointer (Initialized one only)
// It takes a pointer to integer thus does not move the pointer so you must move it manually based on return value
int packlist( Listptr *h, int *serial,  int *nodedeleted) ;
// this function shows the contents of the list 
// Node it does not affect the pointers position
// To view the next one you must manually move the pointer 
// It returns the size of contents view
// THis is used to move the pointer to view the next list
int showpackedlist( int *serial ) ;  
// This function unpacks the serial, it creates a new List 
// It does not move the pointer it tells number of nodes inserted
// It returns the number of integer viewed to extract 
// U should manually move the pointer to extract the next list using it
// Input (*h) must be NULL else will get deleted
int unpacklist( Listptr *h, int *serial,int *nodesinserted) ;


#endif