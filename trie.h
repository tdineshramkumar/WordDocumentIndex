#ifndef _TRIE_
#define _TRIE_
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linklist.h"

struct _trie_node { 
	Listptr data[26]; // the list of word frequencies
	int count[26]; // current word frequency
	struct _trie_node *next[26]; // next elements of the trie
} ;

struct _trie_head {
	struct _trie_node * start;
	int index;
	int WORDDEPTH ; // contains the maximum length of word
	int NODECOUNT; // contains number of nodes in the trie
	int HEADCOUNT; // contains number of lists in the trie
	int TRIECOUNT; // contains number of tries in the trie
};


typedef struct _trie_node Trienode;
typedef struct _trie_node * Trienodeptr;
typedef struct _trie_head Trie;
typedef struct _trie_head *Trieptr;

// this function creates a new trie if null and initializes it 
// else already existing then packs the current trie into linklist and resets the trie and changes index
void inittrie(Trieptr *t, int index) ;  
// this function increments count of string given in current index 
void inserttrie(Trieptr *t, char *s); 
// this function prints the trie
void displaytrie(Trieptr *t);

// THis function takes a trie and packs all lists into newly allocated serial and removes the lists
// And returs the size of serial and also it unsets the number of nodes and heads
// It keeps the current Trie as such
// Node t must not be empty
int packtrie( Trieptr *t, int **serial) ;
// This function takes a serial pointer and 
// And displays the contents.
// And returns the size of serial data explored
int showpackedtrie( int *serial );
// This function unpacks a trie and merges the contents the lists of existing trie
// with the new one.
// It returns the size of serial data viewed
int unpacktrie( Trieptr *t, int *serial );


// This function takes lists from t2 and merges it with lists in t1 and removes them from t1
// It keeps the current index ... 
// t2 is Not Deleted
void mergetrie(Trieptr *t1, Trieptr *t2);

// THis function returns the size of trie
int triesize( Trieptr * t);

int nodecounts( Trieptr * t);
int avgnodecounts( Trieptr * t);
void displaytrieheader(Trieptr *t);
bool chartoint(char c, int *r);
// Note YET IMPLEMENTED
// Later take care of stop words
void inserttriestopword(Trieptr *t, char *s);
#endif