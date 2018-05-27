#include "main.h"


// char dirname[] = "files/" ; // <----------------------------MODIFY IT LATER
const int SUFFICIENTWORKTHRESHOLD = 1000000 ; // <----------------------------MODIFY IT LATER
const int SUFFICIENTMAINSIZE = 10000000 ;// <----------------------------MODIFY IT LATER
const int TLS = 1000 ; // <-------------------------- THRESHOLD ON LIST SIZE
#define MAX_WORD_LEN 100
int totalnameslength = 0 ; //  It stores the total size of all file names
int numregfiles = 0;

#define SHIFT_INDEX 24

DIR * dir = NULL; 
// Put the slash
const char ALL [] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
void readfile( const char *filename ) {
	//printf("READFILE: %s\n",filename );
	if ( filename == NULL ) {
		printf("ERROR: File Name not mentioned.\n");
		exit(EXIT_FAILURE);
	}
	FILE *fp = fopen(filename,"r");
	if (fp == NULL) {
		printf("ERROR: Unable to open file %s\n", filename );
		perror("");
		exit(EXIT_FAILURE);
	}

	//printf("READFILE: OPENED %s\n",filename );
	char str[MAX_WORD_LEN];
	char *s ;
	int index = ((world_rank << SHIFT_INDEX) + numregfiles) ; // <----------------------------MODIFY IT LATER
	//printf("[%d](%d)",index,numregfiles);
	inittrie(&tmptrie,index);// <-----------------------DEPENDENCY ON TRIE
	while (fscanf(fp,"%99s",str)!= EOF) {
		s = str ;
		int len = strlen(str);
		//printf("[%s]\n",str);
		while ( s < (str + len) ) {
			int x = strspn(s,ALL);
			s[x] = '\0' ;
			if ( strlen(s) > 0) { 
				// printf("%s\n",s);
			//	printf("INSERTING :%s\n",s );
				inserttrie(&tmptrie,s); // <-----------------------DEPENDENCY ON TRIE
			}
			s = s + x + 1;
		}
	}
}
bool isdirectoryclosed = false ;
bool indexfile()  {
	if ( !isdirectoryclosed ) {
		if ( dir == NULL ) {
		// if directory not yet opened 
			dir = opendir(dirname);
			if ( dir == NULL ) {
				perror("ERROR: Unable to open directory.\n");
				exit(EXIT_FAILURE);
			}
		}

		struct dirent *de ;
		while ( ( de = readdir(dir) ) != NULL ) {
			if ( de->d_type == DT_REG ) {
				char *file = (char *)malloc(sizeof(char) * ( strlen(dirname) + strlen(de->d_name) + 10 )) ;
				strcpy( file, dirname );
				strcat( file, de->d_name );
				totalnameslength += strlen(de->d_name) ;
				//printf("READING FILE NAME: %s.\n", file);
				
				readfile(file);
				numregfiles ++ ;
				return true ;
			}
		}

		isdirectoryclosed = true;
		closedir(dir);
		dir = NULL ;
		return false ;
	}
	else {
		return false ;
	} 
}
static char ***filenames = NULL ;
static int *filenamescount ;//////////// <-------------------------- NOTE: WORKING ONLY FOR ROOT
bool nyindexedgmfiles = true ;
void indexfilenames() {
	if (dir != NULL) {
		printf("ERROR: TRYING TO SERIALIZE FILES ON OPEN DIRECTORY.\n");
		exit(EXIT_FAILURE);
	}
	dir = opendir(dirname);
	// THIS IS DONE TO ENSURE THE SLAVE FILES WHEN COMES FIRST THEN IT THAT ONE ALLOCATE
	if ( filenames == NULL ) {
		filenames = (char ***) malloc(sizeof(char **) * world_size);
		filenamescount = (int *) malloc(sizeof(int ) * world_size);
		for (int i = 0 ; i < world_size ; i ++ )
			filenamescount[i] = 0 ;
	}

	// NOTE THIS FUNCTION ONLY CALLED IN GRANDMASTER
	struct dirent *de ;
	int count = 0;
	filenamescount[0] = numregfiles ;
	filenames[0] = (char **) malloc(sizeof(char *) * numregfiles); 
	while ( ( de = readdir(dir) ) != NULL ) {
		if ( de->d_type == DT_REG ) {
			filenames[0][count] = (char *) malloc(sizeof(char)* (strlen(de->d_name) + 5) );
			strcpy( filenames[0][count], de->d_name);
			count ++;
		}
	}
	nyindexedgmfiles = false ;
}
void showfilenames() {
	for (int i = 0 ; i < world_size; i ++) {
		for (int j = 0 ; j < filenamescount[i] ; j ++) {
			printf("%d. %s\n", i, filenames[i][j] );
		}
	}
}
bool notyetindexedfilenames() {
	return nyindexedgmfiles ;
}
extern int rcvfilenameslength ; // <---------------------------- NOTE: DEFINED IN MPIFUNC.C
extern char *rcvfilenameserial ;
extern int recvfrom ;
// NOTE: MUST BE CALLED ONLY AFTER RECV FILES
void indexrecvfilenames() {
	// THIS IS DONE TO ENSURE THE SLAVE FILES WHEN COMES FIRST THEN IT THAT ONE ALLOCATE
	if ( filenames == NULL ) {
		filenames = (char ***) malloc(sizeof(char **) * world_size);
		filenamescount = (int *) malloc(sizeof(int ) * world_size);
		for (int i = 0 ; i < world_size ; i ++ )
			filenamescount[i] = 0 ;
	}

	int *tmp = (int *)	rcvfilenameserial;
	if ( rcvfilenameserial == NULL) {
		printf("ERROR: rcvfilenameserial is NULL.\n");
		exit(EXIT_FAILURE);
	}
	int numprocesses = tmp[0];
	int size = 4 ;
	void * buffer = (void *) rcvfilenameserial ;
	for (int i = 0 ; i < numprocesses; i ++) {
		void * t1 = ( buffer + size );// move to current
		tmp = (int *) t1 ; // convert it to integer
		int rank = tmp[0] ;
		int count = tmp[1] ;
		filenamescount[rank] = count ;
		printf("grandmaster indexed %d files from %d sender [%d] total[%d] size[%d].\n",count,rank,recvfrom,rcvfilenameslength,size);
	//printf("RECV FILE NAMES LENGTH:%d NUMPROCESSES:%d RANK:%d COUNT:%d \n", rcvfilenameslength,numprocesses,rank,count);
		filenames[rank] = (char **) malloc(sizeof(char *) * count);
		size += 8 ; // read two integer
		for ( int j = 0 ; j < count ; j ++ ) {
			t1 = ( buffer + size );
			tmp = (int *) t1 ; 
			int slen = tmp[0];
			size += 4 ; // read one integer
			t1 = buffer + size ; // Move to string
			filenames[rank][j] = (char *) malloc(sizeof(char) * (slen + 5) );
			memcpy(filenames[rank][j], t1, slen);
			filenames[rank][j][slen] = '\0' ; // end of string marker..
			//printf("FILENAME:%d %s\n",j,filenames[rank][j] );
			size += slen ; // read a string ...
		}
	}
	free(rcvfilenameserial);
	rcvfilenameserial = NULL ;
	rcvfilenameslength = 0 ;
}

bool nomorefiles() {
	return isdirectoryclosed ;
}

// LATER CHANGE THESE TRIESIZE FUNCTION TO RETURN THE SIZE OF NODES
// NOT THE SIZE OF CURRENT TRIE AS IT IS USED FOR SENDING
// bool sufficientwork() {
// 	//if ( triesize( &tmptrie ) > SUFFICIENTWORKTHRESHOLD ) {
// 	if ( nodecounts( &tmptrie ) > SUFFICIENTWORKTHRESHOLD ) {
// 		printf("[%d] sufficientwork: TMPTRIE: %d NODECOUNT:%d.\n",world_rank, triesize(&tmptrie), nodecounts(&tmptrie));
// 		return true ;
// 	}  // <-----------------------DEPENDENCY ON TRIE
// 	else return false ;
// }
bool sufficientwork() {
	if ( avgnodecounts( &tmptrie ) > TLS ) {
		printf("[%d] sufficientwork: TMPTRIE: %d NODECOUNT:%d.\n",world_rank, triesize(&tmptrie), nodecounts(&tmptrie));
		return true ;
	}  // <-----------------------DEPENDENCY ON TRIE
	else return false ;
}
extern int recvcount ; //////// <-------------------- DEPENDENCY

// bool sufficientmainfiles()  {
// 	if ( nodecounts( &maintrie) > SUFFICIENTMAINSIZE * (recvcount + 1) ) return true ;  // <-----------------------DEPENDENCY ON TRIE
// 	else return false ;
// }

bool sufficientmainfiles()  {
	if ( avgnodecounts( &maintrie) > (TLS * 2 * (recvcount + 1)) ) return true ;  // <-----------------------DEPENDENCY ON TRIE
	else return false ;
}


void mergetomainfiles() {
	printf("[%d] MERGE TO MAIN FILES\n",world_rank);
	mergetrie(&maintrie,&tmptrie);  // <-----------------------DEPENDENCY ON TRIE
}

void querytrie(Trieptr *t, char *s){
	Trienodeptr tn = (*t)->start ; // get the first one 
	int r ; // the current alphabet index
	for (int i = 0 ; i < strlen(s) ; i ++) {
		if ( chartoint(s[i],&r) ) {
			if ( i == ( strlen(s)-1 ) ) {
				
				if ( tn->data[r] == NULL ) {
					printf("NOT FOUND [%s].\n", s);
					return ;
				}
				else {// Now show the list 
					// If found 
					printf("FOUND[%s]: \n",s);
					Nodeptr tmp = tn->data[r]->start ;
					int i = 0 ;
					while (tmp != NULL) {
						//printf("[%d]%d ", tmp->index, tmp->count);
						i ++ ;
						int rank = (tmp->index >> SHIFT_INDEX ); 
						int index = tmp->index - (rank << SHIFT_INDEX) ;
						printf("%d. [Rank: %d] %s (%d times)\n",i,rank, filenames[rank][index],tmp->count );
						tmp = tmp->next ;
					}
					printf("\n");
					return ;
				}
			}
			if (tn->next[r] == NULL)  { // if no trie existed before
				printf("NOT FOUND [%s].\n", s);
				return ;
			}
			tn = tn->next[r] ; // move to next trie
		}
		else {
			printf("WARNING: Not Valid Form [%s].\n",s);
		}
	}
}
