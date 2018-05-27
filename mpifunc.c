#include "main.h"
#include <mpi.h>
#include <stdio.h>

int world_size ; // contains number of processes 
int world_rank ; // contains the current rank of process
int sendto ; // This contains whom to send to 
int recvcount ; // This contains how many will send // THIS CONTAINS MAX RECV COUNT
int currrecvcount; // CUrrent value of recv count
void mpi_initialize() {
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&world_size);
	MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
	int levels[world_size] ;
	// compute levels of all processes
	for (int rank=0; rank < world_size; rank++) { 
		levels[rank] = 0 ;
		for (int i=1; i< 2*world_size; i*=2) 
			if ( (rank % i) == 0 ) 
				levels[rank] ++;
	}
	int level = levels[world_rank];
	int maxprev = 0;
	for (int i = world_rank + 1; i < world_size; i ++) {
		if ( levels[i] < level && maxprev < levels[i] ) { // if level is less that its level and more that previously encountered levels 
			maxprev = levels[i] ; // adjust the previously maximum level
			recvcount  ++ ; // increment recv from count
		}
	}
	currrecvcount = recvcount ;
	for (int i= world_rank-1; i >= 0 ; i--) {// For each rank less than current
		if ( levels[i] > level ) // if its level is greater than currents level (the closest one)
		{	
			sendto=i;  // then send to it
			break ;	
		}
	}
}
bool isgrandmaster() {
	if ( world_rank == 0) return true ;
	else return false ;
}
bool ismaster() {
	if ( world_rank != 0 && recvcount > 0) return true ;
	else return false ;
}
bool isslave() {
	if ( world_rank != 0 && recvcount == 0) return true ;
	else return false ;
}
void mpi_finalize() {
	MPI_Finalize();
}
void mergeremainingfiles() {
	//printf("MERGE REMAINING FILES.\n");
	if ( isslave() ) {

	//printf("MERGE REMAINING SLAVE FILES.\n");
		// If slave and all files indexed put the current file into lists...
		inittrie(&tmptrie,0); // <------------------------------------NOTE THE INDEX IF ANY PROBLEM CHANGE OR REMOVE IT
	}
	else {
		// if not slave push the current file into lists
		//printf("INIT TRIE BEFORE. size: %d\n", triesize(&tmptrie));
		inittrie(&tmptrie,0); // <------------------------------------NOTE THE INDEX IF ANY PROBLEM CHANGE OR REMOVE IT
		//printf("INIT TRIE SUCCESSFULL. size: %d\n", triesize(&tmptrie));
	//printf("MERGE REMAINING MASTER FILES.\n");
		mergetrie(&maintrie,&tmptrie);
	}

}



///////////////// SEND FUNCTIONS 
int *serial = NULL;
#define TRIEMSG 0 
#define FILEMSG 1
MPI_Request sendrequest ;
bool ismsgsent = false ;
void sendfiles() {
	ismsgsent = true ; // just for starting ...
	if ( isslave() ) {
		// if slave single trie ...
		int size = packtrie( &tmptrie , &serial);
		MPI_Isend((void *)serial,size,MPI_INT,sendto,TRIEMSG,MPI_COMM_WORLD,&sendrequest);
	}
	else if ( ismaster() ){
		// For masters pack main trie and send ...
		int size = packtrie( &maintrie , &serial);
		MPI_Isend((void *)serial,size,MPI_INT,sendto,TRIEMSG,MPI_COMM_WORLD,&sendrequest);
	}
	else {
		printf("ERROR: GRANDMASTER DOES NOT SEND MESSAGE\n");
		exit(EXIT_FAILURE);
	}

}

bool sendpending() {
	if ( !ismsgsent ) return false ; // if message has yet been sent
	int flag ;
	MPI_Test( &sendrequest, &flag, MPI_STATUS_IGNORE) ;
	if ( flag ) { // if sent 
		if ( serial != NULL )  {
			// Previous allocation
			free(serial); // free it ...
			serial = NULL ;
		}
		return false ; // Send completed
	}
	return true ; // Send is pending
}

void waitonpendingsend(){
	if ( !ismsgsent ) return ; // if no message is sent return 
	MPI_Wait(&sendrequest,MPI_STATUS_IGNORE);
}

// Four stages NOFILES, HASFILEBUTNOYREAD, READING, READ
bool isrcvingfiles = false ;
int *rcvserial = NULL ;
MPI_Request recvfilerequest ;
bool receivedfiles() {
	int flag; 
	MPI_Status status;
	if ( !isrcvingfiles ) {
		// if no trie message previous recvd
		MPI_Iprobe( MPI_ANY_SOURCE, TRIEMSG, MPI_COMM_WORLD, &flag, &status );
		// Probe for a message
		if ( flag ){ // if some message is there
			isrcvingfiles = true ;
			if ( rcvserial != NULL ) 
				free(rcvserial) ; // if any existing message free it
			int count;
			//int recvtype = status.MPI_TAG ;
			int src = status.MPI_SOURCE ;
			MPI_Get_count(&status,MPI_INT,&count);
			rcvserial = (int *) malloc(sizeof(int) *count);
			MPI_Irecv( rcvserial, count,MPI_INT, src, TRIEMSG, MPI_COMM_WORLD, &recvfilerequest );// initiate recv
			return false ;
		}
		else {
			isrcvingfiles = false ;
			return false ;
		}
	}
	if ( isrcvingfiles ) {
		MPI_Test( &recvfilerequest, &flag, MPI_STATUS_IGNORE) ;
		if ( flag ) {
			// if recv complete
			isrcvingfiles = false ; // it can look for some other request
			return true ;
		}
		else {
			return false ;
		}
	}

	
		//int unpacktrie( Trieptr *t, int *serial ) 
}
void mergercvdtomainfiles(){
	unpacktrie(&maintrie, rcvserial); // Unpack rcv data to main trie
	free(rcvserial); // free the rcvserial
	rcvserial = NULL ; // set pointer to null
}

bool isrcvingfilenames = false ;
int rcvfilenameslength ;
char *rcvfilenameserial = NULL ;
int recvfrom ;
MPI_Request recvfilenamerequest ;
bool receivedfilenames() {
	int flag; 
	MPI_Status status;
	if ( !isrcvingfilenames ) {
		// if no trie message previous recvd
		MPI_Iprobe( MPI_ANY_SOURCE, FILEMSG, MPI_COMM_WORLD, &flag, &status );
		// Probe for a message
		if ( flag ){ // if some message is there
			isrcvingfilenames = true ;
			if ( rcvfilenameserial != NULL ) 
				free(rcvfilenameserial) ; // if any existing message free it
			int count;
			//int recvtype = status.MPI_TAG ;
			int src = status.MPI_SOURCE ;
			recvfrom = src ;
			MPI_Get_count(&status,MPI_BYTE,&count);
			rcvfilenameserial = (char *) malloc(sizeof(char) *count);
			MPI_Irecv( rcvfilenameserial, count,MPI_BYTE, src, FILEMSG, MPI_COMM_WORLD, &recvfilenamerequest );// initiate recv
			rcvfilenameslength = count ; // set file name length
		}
		else {
			isrcvingfilenames = false ;
			return false ;
		}
	}
	if ( isrcvingfilenames ) {
		MPI_Test( &recvfilenamerequest, &flag, MPI_STATUS_IGNORE) ;
		if ( flag ) {
			// if recv complete
			isrcvingfilenames = false ; // it can look for some other request
			return true ;
		}
		else {
			return false ;
		}
	}

}

static char *filenames = NULL ;
int filenameslength = 0;
// This function MUST only be called after recvfilenames function 
void combinefilenames(){ // <-------------------------------- ERROR IN THIS FNCTIONS
	//printf("ERROR: combinefilenames not rectified \n");
	//exit(EXIT_FAILURE);
	if (filenames == NULL || filenameslength == 0) {
		filenames = rcvfilenameserial ;
		filenameslength = rcvfilenameslength ;
		rcvfilenameserial = NULL ;
		printf("[%d] COMBINING FILES WITH NULL \n",world_rank);
	}
	else  {
		char *tmp = (char *)malloc((sizeof(char)) * (filenameslength + rcvfilenameslength - 4)); // -1 to move common first header and merge them 
		memcpy(tmp, filenames, filenameslength);
		int *t1 = (int *) tmp;
		int *t2 = (int *) rcvfilenameserial ;
		printf("[%d] COMBINING FILES (%d) + (%d) \n", world_rank, t1[0], t2[0] );
		t1[0] += t2[0] ; // increment the count
		// NOTE: RCVFILENAMESERIAL + 4 , RCVFILENAMELENGTH -4 
		/// >>>> MODIFIED ......
		memcpy(tmp + filenameslength, rcvfilenameserial+ 4, rcvfilenameslength - 4); // Copy from rcvd serial file names to 
		free(rcvfilenameserial);
		free(filenames);
		rcvfilenameserial = NULL ;
		filenameslength = filenameslength + rcvfilenameslength - 4; // UPDATE CURRENT FILE NAMES LENGTH
		filenames = tmp ; // UPDATE FILE NAMES
	}
}

extern int totalnameslength ; //<----------------------------------- NOTE: DEFINED IN FILE.C
extern int numregfiles ;

char * filenamesserial ;
// NOTE ONLY ONE SEND OF EITHER FILES OR FILENEMAES
// IF MASTER AFTER ALL OTHER FILENAMES COMBINED
void sendfilenames() {
	if ( isslave() ) {
		// if it is slave then send its own files
		// 3 -> NUMBER OF PROCESSES | RANK | NUM FILES
		int totalserialsize = 3 * sizeof(int) + sizeof(char)*totalnameslength + sizeof(int)* numregfiles ;
		filenamesserial = (char *) malloc( totalserialsize);
		void *buffer = (void *) filenamesserial ;
		int *tmp = (int *) filenamesserial ;
		tmp[0] = 1 ; // initially number of processes is 1
		tmp[1] = world_rank ; // set its rank
		tmp[2] = numregfiles ; // set numregfiles
		int size = 12 ; // offset now is 12 (3 ints)
		struct dirent *de ;
		DIR * dir = opendir(dirname);
		char * c ;
		while ( ( de = readdir(dir) ) != NULL ) {
			if ( de->d_type == DT_REG ) {
				void * t1 = (buffer + size) ;
				tmp = (int *) t1 ;
				tmp[0] = strlen(de->d_name) ;
				size += 4 ;
				t1 = (buffer + size);
				c = (char *) t1 ;
				memcpy(c,de->d_name,tmp[0]);
				size +=tmp[0];
			}
		}
		MPI_Isend((void *)filenamesserial,totalserialsize,MPI_BYTE,sendto,FILEMSG,MPI_COMM_WORLD,&sendrequest);
		closedir(dir);

	}
	else if ( ismaster( )){
		// combine all till now and send
		int totalserialsize = filenameslength + 2 * sizeof(int) + sizeof(char)*totalnameslength + sizeof(int)* numregfiles ;
		filenamesserial = (char *) malloc( totalserialsize);
		for (int i = 0 ; i < totalserialsize ; i ++) {
			filenamesserial[i] = 0 ;
		}
		void *buffer = (void *) filenamesserial ;
		int *tmp = (int *) filenamesserial ;
		int *tmp2 = (int *) filenames;
		tmp[0] = 1 + tmp2[0]; // initially number of processes is 1
		tmp[1] = world_rank ; // set its rank
		tmp[2] = numregfiles ; // set numregfiles
		int size = 12 ; // offset now is 12 (3 ints)
		struct dirent *de ;
		DIR * dir = opendir(dirname);
		char * c ;
		while ( ( de = readdir(dir) ) != NULL ) {
			if ( de->d_type == DT_REG ) {
				void * t1 = (buffer + size) ;
				tmp = (int *) t1 ;
				tmp[0] = strlen(de->d_name) ;
				size += 4 ;
				t1 = (buffer + size);
				c = (char *) t1 ;
				memcpy(c,de->d_name,tmp[0]);
				size +=tmp[0];
			}
		}
		
		memcpy( (void *)filenamesserial + size , (void *)filenames + 4, filenameslength - 4 );
		MPI_Isend((void *)filenamesserial,totalserialsize,MPI_BYTE,sendto,FILEMSG,MPI_COMM_WORLD,&sendrequest);
		closedir(dir);
	}
	else {
		// if grand master
		printf("ERROR: GRANDMASTER cannot send file names.\n");
		exit(EXIT_FAILURE);
	}
}


void decrementfilerecvs() {
	currrecvcount--;
}


bool canrecvfiles() {
	if ( currrecvcount > 0) return true ;
	else return false ;
}
