#include "main.h"
#include <string.h>
#include <limits.h>
void slave ();
void master();
void grandmaster();

char dirname[PATH_MAX];
int main(int argc, char *argv[]) {
	if ( argc != 2 ) {
		printf("<executable> path\n");
		exit(EXIT_FAILURE);
	}
	strncpy(dirname , argv[1], PATH_MAX);
	mpi_initialize();	
	if ( isgrandmaster() ) {
		clock_t start = clock();
		grandmaster();
		double timetake = ( clock() - start ) / ( (double) CLOCKS_PER_SEC );
		printf("\n\n #### Total time take:%lf seconds #### \n\n", timetake);
		char str[100] ;
		while (true) {
			printf("ENTER WORD: >>\n");
			scanf("%99s", str);
			if (str[0] == '.' || str[0] == '\0') {
				printf("DONE.\n");
				break;
			}
			else if (str[0] == ',') {
				showfilenames();
			}
			else if (str[0] == ';') {// USED FOR DEBUGGING ONLY
				displaytrie(&maintrie);
			}
			else querytrie(&maintrie,str);
		}
	}
	else if ( ismaster() ) {
		master();
	}
	else { // then it is slave
		slave();
	}
	mpi_finalize();
	return 0;
}

// NOTE slave has single trie ? HOW DO YOU IMPLEMENT
void slave () {
	printf("slave created with rank %d.\n", world_rank);
	while ( true ) {
		if ( indexfile() ) { // index files
			if ( sufficientwork() && ! sendpending() ) { // if sufficient work is done and no pending send

				printf("slave [%d] has done sufficient work (%d) and sending.\n",world_rank, triesize(&tmptrie));
				sendfiles(); // send indexed files
			}
		}
		else { // if no more file to index
			waitonpendingsend(); // wait till pending send completes
			mergeremainingfiles(); // Note : DIFFERENT FUNCTIONALITY OF SLAVE just push current file to lists
			printf("slave [%d] finished work and sending remaining files of trie size %d.\n", world_rank,triesize(&tmptrie));
			sendfiles(); // send remaining files
			waitonpendingsend(); // wait till all files send
			printf(" -->slave [%d] now sending filenames.\n", world_rank);
			sendfilenames(); // now finally send all files
			waitonpendingsend(); // wait till sending files names completes
			return ; // return and task is complete
		}
	}
}
void grandmaster() {
	printf("grandmaster created with rank %d.\n", world_rank);
	while ( true ) {
		if ( indexfile() ) {
			if ( sufficientwork() ) {
				printf("grandmaster has done sufficient work (%d) node count (%d) and merging to (%d).\n",triesize(&tmptrie),nodecounts(&tmptrie),triesize(&maintrie));
				mergetomainfiles() ;
			}
		}
		else { // no more files to index ...
			if ( notyetindexedfilenames() ) // if file names not indexed 
				{
					printf("grandmaster has finished indexing files and now indexing its filenames.\n");
					mergeremainingfiles();
					indexfilenames(); // index them
					printf("grandmaster finished indexing its own files.\n");
				}
		}
		if ( canrecvfiles() ) {
			if ( receivedfiles() ) {
				printf("grandmaster has received files and merging to main trie (%d).\n", triesize(&maintrie));
				mergercvdtomainfiles();
			}
		 	if ( receivedfilenames() ) {
				printf("grandmaster has received filesnames.\n");
				indexrecvfilenames();
				decrementfilerecvs();
			}
		}
		if ( !canrecvfiles() && nomorefiles() ) {
			mergeremainingfiles();
			printf("grandmaster completes its job no more files and can't receive files.\n -->Total trie sizes TMP(%d) MAIN(%d).\n", triesize(&tmptrie), triesize(&maintrie));
			//displaytrie(&maintrie);
			//showfilenames();
			return ;
		}
	}
}
void master() {

	printf("master created with rank:%d.\n", world_rank);
	while ( true ) {
		if ( indexfile() ) {
			if ( sufficientwork() ) {
				printf("master [%d] has done sufficient work (%d) and merging to main (%d).\n",world_rank,triesize(&tmptrie),triesize(&maintrie));
				mergetomainfiles();
			}
		}
		if ( canrecvfiles() ){
			if ( receivedfilenames() ) {
				printf("master [%d] has received filenames combining them with existing if any.\n",world_rank);
				combinefilenames();
				decrementfilerecvs();
			}
			if ( receivedfiles() ) {
				mergercvdtomainfiles();
				printf("master [%d] has receive files and merging them to main (%d).\n", world_rank, triesize(&maintrie));
			}
		}
		
		if ( sufficientmainfiles() && !sendpending() ) {
			printf("master [%d] has sufficient main (%d) and forwarding the index.\n",world_rank,triesize(&maintrie));
			sendfiles();
		}
		if ( !canrecvfiles() && nomorefiles() ){
			printf("master [%d] has no more files and can't receive files TMP(%d) MAIN(%d).\n",world_rank,triesize(&tmptrie),triesize(&maintrie));
			mergeremainingfiles();
			waitonpendingsend(); // wait till pending send completes
			sendfiles(); // send remaining files
			waitonpendingsend(); // wait till all files send
			printf("--> master [%d] now sending filenames.\n", world_rank );
			sendfilenames(); // now finally send all files
			waitonpendingsend(); // wait till sending files names completes
			return ;
		}
	}
}
