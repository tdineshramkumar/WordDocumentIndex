#include "linklist.h"

// this is a library function
Listptr initlist() {
	Listptr h = (Listptr) malloc(sizeof(List));
	h->start = NULL ;
	h->end = NULL ;
	return h ;
}

// creates a new node
Nodeptr newnode(int index, int count,Nodeptr next) {
	Nodeptr n = (Nodeptr) malloc(sizeof(Node));
	n->index = index ;
	n->count = count ;
	n->next = next ;
	return n ;
}

// inserts the element in decreasing order of count
void insertlist(Listptr *h, int index, int count) {
	if ((*h) == NULL) {
		printf("ERROR: Inserting into Uninitialized list.\n");
		exit(EXIT_FAILURE);
	}
	if ((*h)->start == NULL || (*h)->end == NULL ) {
		// if no element
		(*h)->start = newnode(index,count,NULL);
		(*h)->end = (*h)->start ;
	} 
	else {
		// if already elements exist then insert in order
		Nodeptr n1 ,n2 ;
		// if count greater than first element
		if ( (*h)->start->count <= count ) {
			(*h)->start = newnode(index,count,(*h)->start);
		}
		// if smaller than smallest
		else if ( (*h)->end->count >= count ) {
			(*h)->end->next = newnode(index,count,NULL);
			(*h)->end = (*h)->end->next ;
		}
		else {
			// if inbetween
			n1 = (*h)->start ; 
			while ( n1->count > count ) {
				// as long as value is greater move ahead
				n2 = n1 ; n1 = n2->next ;  
			}
			// now n1->count <= count
			// n2 newnode n1
			n2->next = newnode(index,count,n1);
		}
	}
}

void displaylist(Listptr *h) {
	if ((*h) == NULL) {
		printf("ERROR: Displaying into Uninitialized list.\n");
		exit(EXIT_FAILURE);
	}
	Nodeptr n = (*h)->start;
	int count = 0 ;
	if ( n == NULL ) {
		printf("NULL. (0)");
	}
	while( n != NULL ) {
		printf("[%d]%d ",n->index,n->count);
		n = n->next;
		count ++ ;
	}
	printf("(%d)\n",count);
}

void deletelist(Listptr *h) {
	if ((*h) == NULL) {
		printf("ERROR: Deleting into Uninitialized list.\n");
		exit(EXIT_FAILURE);
	}
	Nodeptr n = (*h)->start;
	Nodeptr n1 ;
	while( n!= NULL ){
		n1 = n ;
		n = n->next ;
		free (n1);
	}
	free(*h);
	(*h) = NULL ;
}

void mergelist(Listptr *h1, Listptr *h2) {
	if ((*h1) == NULL || (*h2) == NULL) {
		printf("ERROR: Merging into Uninitialized list.\n");
		exit(EXIT_FAILURE);
	}
	Nodeptr l2 = (*h2)->start;
	Nodeptr l1 = (*h1)->start ;
	// Insert at first
	if ( l1->count < l2->count ) {
		(*h1)->start = (*h2)->start ;
		Nodeptr tmp ;
		while ( l1->count < l2->count ) {
			tmp = l2 ;
			l2 = l2->next ;
			if ( l2 == NULL ) {
				// if all element of l2 is greater than l1 then
				tmp->next = l1 ;
				free(*h2);
				return ;
			}
		}
		// tmp->count > l1->count > l2->count 
		tmp->next = l1 ; // first element 
	}
	Nodeptr tmp = l1 ;
	while (1) {
		if ( l1->count >= l2->count ) {
			// always run the first time here
			tmp = l1 ;
			l1 = l1->next ; // move the first one ahead
		}
		else {
			tmp->next = l2 ;
			tmp = tmp->next ;
			l2 = l2->next ;
			tmp->next = l1 ; 
		}
		if (l1 == NULL ) {
			// if first one is null
			tmp->next = l2 ;
			(*h1)->end = (*h2)->end ;
			break;
		}
		else if (l2 == NULL) {
			break;
		}
	}
	free(*h2);
	(*h2) = NULL ;
}


// PACKING FUNCTIONS ...
// packlist function should pack the list into list of integers and return the size taken 
// it should delete the List and return number of nodes deleted
// head is also deleted ...
// Node it must be called of valid Listpointer (Initialized one only)
// It takes a pointer to integer thus does not move the pointer so you must move it manually based on return value
int packlist( Listptr *h, int *serial,  int *nodesdeleted)  {
	// FORMAT 
	// NODECOUNT | INDEX | COUNT | INDEX | COUNT | ...
	if ((*h) == NULL) {
		printf("ERROR: Packing an Uninitialized list.\n");
		exit(EXIT_FAILURE);
	}
	serial[0] = 0 ; // first value of serial always contains count
	(*nodesdeleted) = 0 ; // Initialize Nodes Deleted to zero
	int numint = 1 ; // no of integers looked from serial
	Nodeptr n = (*h)->start ; // Look at the first node
	Nodeptr tmp ;
	while ( n != NULL ) {
		serial[0] ++ ; // increment node count
		(*nodesdeleted) ++ ; // increment nodes deleted
		serial[ numint ] = n->index ;
		numint ++ ; // Inserting Index
		serial[ numint ] = n->count ;
		numint ++ ; // Inserted Count
		tmp = n ; 
		n = n->next ; // Look at next node
		free(tmp); // Delete the current node
	}
	free(*h); // free the header
	(*h) = NULL ;
	return numint ; // No of Serial Values used ..
}
// this function shows the contents of the list 
// Node it does not affect the pointers position
// To view the next one you must manually move the pointer 
// It returns the size of contents view
// THis is used to move the pointer to view the next list
int showpackedlist( int *serial )  {
	int nodecount = serial[0];
	int numint = 1 ; // number of serial elements looked at
	for (int i = 0 ; i < nodecount ; i ++ ) {
		int index = serial[numint] ;
		numint ++ ;
		int count = serial[numint] ;
		numint ++ ;
		printf("[%d]%d ",index, count );
	}
	printf("(PC:%d)\n", nodecount );
	return numint ; // Return number of serial numbers look at
}
// This function unpacks the serial, it creates a new List 
// It does not move the pointer it tells number of nodes inserted
// It returns the number of integer viewed to extract 
// U should manually move the pointer to extract the next list using it
// Input (*h) must be NULL else will get deleted
int unpacklist( Listptr *h, int *serial,int *nodesinserted) {
	// FORMAT 
	// NODECOUNT | INDEX | COUNT | INDEX | COUNT | ...
	if ((*h) != NULL) {
		printf("ERROR: Unpacking an INITIALIZED list.\n");
		exit(EXIT_FAILURE);
	}
	int nodecount = serial[0];
	//printf("UNPACK:nodecount %d\n",nodecount );
	int numint = 1 ; // number of serial elements looked at
	(*h) = initlist() ; 
	(*nodesinserted) = 0 ;
	for (int i = 0 ; i < nodecount ; i ++ ) {
		(*nodesinserted) ++;
		int index = serial[numint];
		numint ++ ;
		int count = serial[numint];
		numint ++ ;
		//printf("Node Inserting :%d Index:%d Count:%d \n",*nodesinserted,index,count);
		if ( (*h)->start == NULL || (*h)->end == NULL ) {
			// if first element
			(*h)->start = newnode(index,count,NULL);
			(*h)->end = (*h)->start ;
		}
		else {
			// if already elements exist
			// insert at end as it is in sorted order no need to sort
			(*h)->end->next = newnode(index,count,NULL);
			(*h)->end = (*h)->end->next ; // change the current end
		}
	}
	return numint ;
}

