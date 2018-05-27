#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include "trie.h"

#ifndef _MAIN_
#define _MAIN_

extern Trieptr tmptrie ;
extern Trieptr maintrie ;
extern int world_rank ;
extern int world_size ;
extern char dirname[];
void mpi_initialize(); //
bool isgrandmaster(); // 
bool ismaster() ; //
void mpi_finalize(); //
bool isslave() ; // Not Used in Main

bool indexfile() ; //
bool sufficientwork(); //
bool nomorefiles(); //

void mergetomainfiles(); //
void mergercvdtomainfiles();//
bool sendpending();//
void waitonpendingsend();//
void sendfilenames();//
void sendfiles();//


bool receivedfilenames() ;//
bool receivedfiles() ;//
bool canrecvfiles();//
	
bool notyetindexedfilenames();//
void indexfilenames(); 	//
void indexrecvfilenames();//


void combinefilenames();//
void decrementfilerecvs(); //
		
bool sufficientmainfiles() ;//

void showfilenames(); // TEST FUNCTIOn

void mergeremainingfiles(); //

void querytrie(Trieptr *t, char *s);
#endif