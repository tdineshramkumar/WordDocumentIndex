#include "trie.h"

Trieptr tmptrie = NULL , maintrie = NULL ; // <---------------------------- DECLARATIONS
Trienodeptr newtrienode() {
	Trienodeptr t = (Trienodeptr) malloc(sizeof(Trienode));
	for (int i = 0 ; i < 26 ; i ++) {
		t->data[i] = NULL ;
		t->next[i] = NULL ;
		t->count[i] = 0 ;
	}
	return t;
}
Trieptr newtrie(int index,int NODECOUNT,int HEADCOUNT,int TRIECOUNT) {
	Trieptr t = (Trieptr) malloc(sizeof(Trie));
	t->index = index ;
	t->NODECOUNT = NODECOUNT ;
	t->HEADCOUNT = HEADCOUNT ;
	t->TRIECOUNT = TRIECOUNT ;
	t->WORDDEPTH = 0 ;
	t->start = NULL ;
	return t ;
}


// this function resets the trie and pushs current value to linklist
void pushtolist(Trieptr *t, Trienodeptr tn) {
	if ( tn == NULL ) return ;
	// if not null
	for (int i = 0 ; i < 26 ; i ++) {
		if ( tn->count[i] > 0 ) {
			// if element exist
			if ( tn->data[i] == NULL ) // if no list yet create one
				(*t)->HEADCOUNT++ , tn->data[i] = initlist(); // increment head count
			(*t)->NODECOUNT++ ,insertlist(&(tn->data[i]),(*t)->index, tn->count[i]); // increment node count and insert to list
			tn->count[i] = 0 ; // reset counter of that node
		}
		if ( tn->next[i] != NULL ) pushtolist(t, tn->next[i]); 
	}
}

// this function creates a new trie if null and initializes it 
// else already existing then packs the current trie into linklist and resets the trie and changes index

void inittrie(Trieptr *t, int index) {
	if (*t == NULL) {
		// if called first time create a new trie ..
		*t = newtrie(index,0,0,0);
	}
	else {
		// Push the current index counts into List
		// Reset the Count
		pushtolist(t,(*t)->start);
		// Reset the INDEX
		(*t)->index = index ;
	}
}

// if c is in A-Za-z it returns a value in  0-25 as r
// if no return false
bool chartoint(char c, int *r) {
	if ( c >= 'A' && c <= 'Z' ) {
		*r = c - 'A' ;
		return true;
	}
	else if ( c >= 'a' && c <= 'z' ) {
		*r = c - 'a' ;
		return true ;
	}
	return false ;
}
void inserttrie(Trieptr *t, char *s) {
	if ( (*t) == NULL ) {
		printf("ERROR Inserting into UNINITIALIZED trie\n");
		exit(EXIT_FAILURE);
	}
	if ( s  == NULL || strlen(s) == 0)  {
		printf("NOTE: Empty String Not Inserted\n");
	}
	if ((*t)->start == NULL) {
		// if no first element
		(*t)->TRIECOUNT ++ , (*t)->start = newtrienode();  // create one 
		(*t)->WORDDEPTH = 1 ;
	}
	if ( (*t)->WORDDEPTH < strlen(s) ) (*t)->WORDDEPTH = strlen(s);
	// Assuming valid characters only given ..
	Trienodeptr tn = (*t)->start ; // get the first one 
	int r ; // the current alphabet index
	for (int i = 0 ; i < strlen(s) ; i ++) {
		if ( chartoint(s[i],&r) ) {
			if ( i == ( strlen(s)-1 ) ) {
				// if last one 
				(tn->count[r]) ++;
				return ;
			}
			if (tn->next[r] == NULL)  { // if no trie existed before
				(*t)->TRIECOUNT ++  ; 
				tn->next[r] = newtrienode(); 
			}// create a new trie node and increment count
			tn = tn->next[r] ; // move to next trie
		}
		else {
			printf("ERROR: Not Inserted [%s] Not VALID Form.\n",s);
			exit(EXIT_FAILURE);
		}
	}
}

void showstrwithcount(Trieptr *t, Trienodeptr tn, char *s,int l) {
	if ( tn == NULL ) return ;
	for (int i = 0 ; i < 26 ;  i ++) {
		s[l] = i + 'a' ;
		s[l+1] = '\0' ;
		if ( tn->count[i] > 0 || tn->data[i] != NULL )
			printf("%s: ", s);
		if ( tn->count[i] > 0) {
			printf("{[%d]%d} ",(*t)->index,tn->count[i]);
			if ( tn->data[i] == NULL ) printf("\n") ;
		}	
		if ( tn->data[i] != NULL )
			displaylist( &(tn->data[i]) );
		if ( tn->next[i] != NULL ) 
			showstrwithcount(t, tn->next[i], s,l+1);
	}
}
void displaytrieheader(Trieptr *t) {
	if ( (*t) == NULL ) {
		printf("ERROR: Displaying an UNINITIALIZED trie\n");
		exit(EXIT_FAILURE);
	}
  char s[(*t)->WORDDEPTH+3] ;
  printf("WORDDEPTH:%d\n", (*t)->WORDDEPTH );
  printf("NODECOUNT:%d\nHEADCOUNT:%d\nTRIECOUNT:%d\nWORDDEPTH:%d\n",
  	(*t)->NODECOUNT,(*t)->HEADCOUNT,(*t)->TRIECOUNT, (*t)->WORDDEPTH);
}
// this function prints the trie
void displaytrie(Trieptr *t) {
	if ( (*t) == NULL ) {
		printf("ERROR: Displaying an UNINITIALIZED trie\n");
		exit(EXIT_FAILURE);
	}
  char s[(*t)->WORDDEPTH+3] ;
  printf("WORDDEPTH:%d\n", (*t)->WORDDEPTH );
  printf("NODECOUNT:%d\nHEADCOUNT:%d\nTRIECOUNT:%d\nWORDDEPTH:%d\n",
  	(*t)->NODECOUNT,(*t)->HEADCOUNT,(*t)->TRIECOUNT, (*t)->WORDDEPTH);
  if ( (*t)->index == 0 ) {
  	// NOT YET INITIALIZED WITH FILE DEFAULT index
  	printf("DEFAULT INDEX 0.\n");
  }
  else {
  	printf("CURRENT INDEX:%d \n" ,(*t)->index);
  }
  showstrwithcount(t,(*t)->start,s,0);
}

int packtrienode( Trienodeptr tn , int *serial ) ;
// THis function takes a trie and packs all lists into newly allocated serial and removes the lists
// And returns the size of serial and also it unsets the number of nodes and heads
// It keeps the current Trie as such
// Node t must not be empty
int packtrie( Trieptr *t, int **serial) {
	if ( (*t) == NULL ) {
		printf("ERROR: PACKING an UNINITIALIZED trie\n");
		exit(EXIT_FAILURE);
	}
	int sizeofserial = 4  + 2 * (*t)->NODECOUNT + (*t)->HEADCOUNT + 2 * (*t)->TRIECOUNT ;
	(*serial) = (int *)malloc(sizeof(int) * sizeofserial); // Allocate size of serial
	int * tmpserial = (*serial); // POINTS TO FIRST OF SERIAL
	// set the initial header information
	tmpserial[0] = (*t)->NODECOUNT ;
	tmpserial[1] = (*t)->HEADCOUNT ;
	tmpserial[2] = (*t)->TRIECOUNT ;
	tmpserial[3] = (*t)->WORDDEPTH ;
	// get the first trie
	Trienodeptr tn = (*t)->start ;
	(*t)->NODECOUNT = 0 ;
	(*t)->HEADCOUNT = 0 ;
	int used = packtrienode( tn, tmpserial + 4 );
	if ( used != ( sizeofserial - 4 ) ) {
		printf("ERROR: IMBALANCE IN USAGE %d\n", used - ( sizeofserial - 4 ) );
		exit(EXIT_FAILURE);
	}
	return sizeofserial ;
}
// this function packs a trie node and returns size of serial buffer it took
// use it to insert next trie
int packtrienode( Trienodeptr tn , int *serial ) {
	// NEXT (STRUCTURE) | DATA (LIST) | {LIST} 
	if (tn == NULL) return 0 ; // if empty node
	serial[0] = 0;
	serial[1] = 0;
	for (int i = 0 ; i < 26; i ++) {
		if ( tn->next[i] != NULL ) serial[0] = serial[0] | ( 1 << i ) ; // Set up the structure
		if ( tn->data[i] != NULL ) serial[1] = serial[1] | ( 1 << i ) ; // Set up data availability
	}
	int numint = 2;
	// NOW NEED TO SET UP EACH LIST
	for (int i = 0; i < 26; i ++) {
		if ( tn->data[i] != NULL ) {
			int nodesdeleted ; // NOT used
			// pack the list and delete the list 
			// used contains the number of serial capacity used 
			int used = packlist( &(tn->data[i]), serial+numint, &nodesdeleted );
			numint = numint + used ;   
		}
	}
	// NOW INSERT ITS CHILDREN
	for (int i = 0 ; i < 26 ; i ++) {
		if ( tn->next[i] != NULL ){
			int used = packtrienode( tn->next[i], serial+numint );
			numint = numint + used ;
		}
	}
	// NOTE : No deleting of trie node
	return numint ;
} 
// This function takes a serial pointer and 
// And displays the contents.
// And returns the size of serial data explored
int showpackedtrie( int *serial ) {
	printf("\t********** PACKED TRIE ********** \n");
	printf("NODECOUNT:%d HEADCOUNT:%d TRIECOUNT:%d WORDDEPTH:%d \n", 
		serial[0], serial[1], serial[2], serial[3] );
	int NODECOUNT = serial[0];
	int HEADCOUNT = serial[1];
	int TRIECOUNT = serial[2];
	int WORDDEPTH = serial[3];
	int numint = 4 ; // Seen four
	for ( int i = 0 ; i < TRIECOUNT ; i ++ ) {
		int structure = serial[numint];
		numint++ ;
		int data = serial[numint];
		numint++ ;
		printf("STRUCTURE:%d ",structure);
		for (int j = 0 ; j < 26 ; j ++) {
			( (structure & ( 1 << j)) != 0) ? printf("%c",'a'+j) : printf("0") ;
		}
		printf("\nDATA:%d ",data );
		for (int j = 0 ; j < 26 ; j ++) {
			( (data & ( 1 << j)) != 0) ? printf("%c",'a'+j) : printf("0") ;
		}
		printf("\n");
		for (int j = 0 ; j < 26; j ++) {
			if ( (data & ( 1 << j  )) != 0 ) {
				int nodecount = serial[numint];
				numint++ ;
				printf("(%d) ", nodecount );
				for (int m = 0 ; m < nodecount ; m ++ ) {
					int index = serial[numint];
					numint ++ ;
					int count = serial[numint];
					numint ++ ;
					printf("[%d]%d ",index,count);
				}
				printf("\n");
			}
		}
	}

	printf("\t********** DONE ********** \n");
	return numint ;
}
// This function unpacks a trie and merges the contents the lists of existing trie
// with the new one.
// It returns the size of serial data viewed
int unpacktrienode( Trieptr *t, Trienodeptr tn , int *serial ) ;
int unpacktrie( Trieptr *t, int *serial ) {
	if ( *t == NULL ) {
		// New Trie
		inittrie(t,0);
	}
	int NODECOUNT = serial[0];
	int HEADCOUNT = serial[1];
	int TRIECOUNT = serial[2];
	int WORDDEPTH = serial[3];
	int numint = 4 ; // Seen three
	// UPDATE WORD DEPTH
	if ((*t)->WORDDEPTH < WORDDEPTH ) (*t)->WORDDEPTH = WORDDEPTH ;
	if ((*t)->start == NULL) {
		// if no first element
		(*t)->TRIECOUNT ++, (*t)->start = newtrienode();  // create one 
	}
	if ( NODECOUNT == 0 || HEADCOUNT == 0 || TRIECOUNT == 0 ) {
		// EMPTY TRIE -> USED TO HANDLE ERROR IN MASTER
		printf("EMPTY PAYLOAD RECVD.\n");
		return numint; 
	}
	// Now first element exist
	int used = unpacktrienode( t, (*t)->start , serial + numint);
	numint = numint + used ;
	int sizeofserial = 4 + 2*NODECOUNT + HEADCOUNT + 2*TRIECOUNT ;
	if ( numint != sizeofserial ) {
		printf("ERROR: INVALID TRIE UNMATCHED SIZE EXPECTED:%d RECVD:%d.\n",sizeofserial,numint);
		exit(EXIT_FAILURE);
	}
	return numint ;
}
// this function unpacks a trie node and returns size of serial buffer it took
// use it to insert next trie
int unpacktrienode( Trieptr *t, Trienodeptr tn , int *serial ) {
	if ( tn == NULL ) { 
		printf("ERROR: Trying to UNPACK NULL node.\n");
		exit(EXIT_FAILURE);
	} 
	int numint = 0 ;
	int structure = serial[numint];
	numint++ ;
	int data = serial[numint];
	numint++ ;
	for ( int j = 0 ; j < 26 ; j ++ ) {
		if ( (data & ( 1 << j)) != 0 ) {
			// if Data available 
			int nodesinserted ; 
			// NOTE WRITE LATER TO MERGE THE TWO OPERATIONS 
			Listptr h = NULL ;
			int used = unpacklist(&h, serial+numint,&nodesinserted);
			numint = numint + used ; // update seen count
			// THIS CREATES A NEW LIST
			if ( tn->data[j] == NULL ) {
				tn->data[j] = h ; // Add that list
				(*t)->HEADCOUNT ++ ; // New Head Added
				(*t)->NODECOUNT += nodesinserted ;
			}
			else {
				// If list already exist there
				mergelist(&(tn->data[j]),&h) ; // merge the existing and new list
				(*t)->NODECOUNT += nodesinserted ; // increment the node count
			}
		} 
	}
	// NOW THE CHILD NODES
	for (int i = 0 ; i < 26 ; i ++ ) {
		if ( (structure & ( 1 << i )) != 0 ) {
			// Then Look At next Trie
			if ( tn->next[i] == NULL ) {
				// if previously no trie
				(*t)->TRIECOUNT ++ ; // increment trie count
				tn->next[i] = newtrienode() ; // create a new trie node
			}

			int used = unpacktrienode(t,tn->next[i],serial+numint);
			numint = numint + used ; // move the seen count
		}
	}
	// Return Seen Count
	return numint ;
}
void mergetrienode ( Trieptr *t1 , Trienodeptr tn1, Trienodeptr tn2) ;
void mergetrie(Trieptr *t1, Trieptr *t2) {
	// Merges two tries
	if ( *t2 == NULL ) {
		printf("ERROR: Merging UNINITIALIZED trie.\n");
		exit(EXIT_FAILURE);
	}
	if  ( *t1 == NULL ) { // <------------------------- MODIFIED
		//if first one is not yet initialized
		*t1 = newtrie(0,0,0,0);
		(*t1)->TRIECOUNT ++ ;
		(*t1)->WORDDEPTH ++ ;
		(*t1)->start = newtrienode();
	}

	// Add all nodes from t2 to t1
	(*t1)->NODECOUNT += (*t2)->NODECOUNT ;
	if ( (*t1)->WORDDEPTH < (*t2)->WORDDEPTH ) 
		(*t1)->WORDDEPTH = (*t2)->WORDDEPTH ;
	// RESET COUNTS IN T2
	(*t2)->NODECOUNT = 0 ; // Use this to increment node count of 1
	(*t2)->HEADCOUNT = 0 ;
	// MERGE LISTS IN EACH TRIE NODE
	if ( (*t2)->start == NULL ) {
		// Nothing in it .. (empty file)
		return ;
	}
	mergetrienode( t1, (*t1)->start, (*t2)->start ); 

}
// Merges Nodes in One Trie to another
void mergetrienode ( Trieptr *t1 , Trienodeptr tn1, Trienodeptr tn2) {
	if ( tn1 == NULL || tn2 == NULL ) {
		printf("ERROR: Merging NULL nodes.\n");
		exit(EXIT_FAILURE);
	}
	for (int i = 0 ; i < 26; i ++) {
		if ( tn2->data[i] != NULL ) {
			// if a list is present in TN1
			if ( tn1->data[i] == NULL ) {
				// if no list in TN1
				(*t1)->HEADCOUNT ++ ; // Since we creating a NEW LIST
				tn1->data[i] = tn2->data[i] ; // Make it point to tn2's data
				tn2->data[i] = NULL ; // Make it point to NULL 
			}
			else {
				// if list is present in TN2
				mergelist( &(tn1->data[i]), &(tn2->data[i]) );
				// this automatically deletes tn2->data[i] ;
				// Just for sake of it
				tn2->data[i] = NULL ;
			}
		}
	}
	for (int i = 0 ; i < 26; i ++) {
		if ( tn2->next[i] != NULL ) {
			if ( tn1->next[i] == NULL ) {
				// if first one has no trie
				tn1->next[i] = newtrienode();
				(*t1)->TRIECOUNT ++ ;
			}
			mergetrienode(t1,tn1->next[i],tn2->next[i]);
		}
	}
}

int triesize( Trieptr * t) {
	if (*t == NULL) return 0;
	int size = 4  + 
		2 * (*t)->NODECOUNT + 
		(*t)->HEADCOUNT + 
		2 * (*t)->TRIECOUNT ;
	return size ;
}

int nodecounts( Trieptr * t) {
	if (*t == NULL) return 0;
	int size = (*t)->NODECOUNT;
	return size ;
}
int avgnodecounts( Trieptr * t) {
	if (*t == NULL ) return 0;
	if ((*t)->HEADCOUNT == 0 ) return 0;
	int size =( (*t)->NODECOUNT / (*t)->HEADCOUNT );
	return size ;
}