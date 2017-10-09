/*
Estelle Bayer
CS 332

A program using semaphores (specifically MAC compatible methods sem_open, sem_wait,
and sem_post) to synchronize hydrogen, sulfur, and oxygen threads. Written for use
with H2SO4Test.c

Author: Estelle Bayer
Last modified: 10/9/2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "H2SO4.h"

// used to handle error that occurs when reuse semaphore name that was not previously closed correctly
int checkSem(sem_t*, char*);
//used to determine whether the atoms necessary for a molecule are present
int checkCounts();
// used to "spin" for some amount of time
void delay(int limit);

//semaphores for allowing threads to leave, allowing a molecule to be created,
//and tracking the status of the previously formed molecule.
sem_t* hleave;
sem_t* sleave;
sem_t* oleave;
sem_t* molecule;
sem_t* exited;

//count all present threads by type, and count how many hydrogens and oxygens have
//left at a given time
int count[3];
int hleaveCount;
int oleaveCount;

void* oxygen(void* args){
  delay(rand()%5000);
  printf("oxygen produced\n");
  fflush(stdout);

  //update the number of oxygen threads currently open
  count[2]++;

  //check to see whether a molecule can be produced
  checkCounts();

  //block leaving until sulfur has left
  sem_wait(oleave);
  printf("oxygen exited\n");
  fflush(stdout);
  //update how many oxygens have left
  oleaveCount++;
  //when the fourth oxygen leaves, post permission for next molecule to be created.
  if (oleaveCount % 4 == 0) {
    sem_post(exited);
  }
  return(void*) 0;
}

void* hydrogen(void* args) {
  delay(rand()%5000);
  printf("hydrogen produced\n");
  fflush(stdout);

  //update number of open hydrogen threads
  count[0]++;

  //check to see if molecule can now be produced
  checkCounts();

  //wait until molecule is produced to leave
  sem_wait(hleave);
  printf("hydrogen exited\n");
  fflush(stdout);
  //update how many hydrogens have left
  hleaveCount++;
  if (hleaveCount % 2 == 0) {
    //the second hydrogen to leave gives sulfur permission to leave
    sem_post(sleave);
  }
  return (void*) 0;

}
void* sulfur(void* args){
  delay(rand()%5000);
  printf("sulfur produced\n");
  fflush(stdout);

  //update number of open sulfur threads
  count[1]++;

  //check to see if molecule can now be produced
  checkCounts();

  //wait for molecule semaphore
  sem_wait(molecule);
  //wait for previous molecule to finish exiting
  sem_wait(exited);
  printf("*** H20 molecule produced ***\n");
  fflush(stdout);

  //dismiss both hydrogens
  sem_post(hleave);
  sem_post(hleave);

  //wait for hydrogen to return permission to leave
  sem_wait(sleave);
  printf("sulfur exited\n");
  fflush(stdout);
  //post enough oleaves for all 4 oxygens to leave
  int i;
  for(i = 0; i < 4; i++) {
    sem_post(oleave);
  }
  return(void*) 0;
}

void openSems() {
  molecule = sem_open("molecule", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(molecule, "molecule") == -1) {
    molecule = sem_open("molecule", O_CREAT|O_EXCL, 0466, 0);
  }
  hleave = sem_open("hleave", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(hleave, "hleave") == -1) {
    hleave = sem_open("hleave", O_CREAT|O_EXCL, 0466, 0);
  }
  sleave = sem_open("sleave", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(sleave, "sleave") == -1) {
    sleave = sem_open("sleave", O_CREAT|O_EXCL, 0466, 0);
  }
  oleave = sem_open("oleave", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(oleave, "oleave") == -1) {
    oleave = sem_open("oleave", O_CREAT|O_EXCL, 0466, 0);
  }
  exited = sem_open("exited", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(exited, "exited") == -1) {
    exited = sem_open("exited", O_CREAT|O_EXCL, 0466, 0);
  }
}
void closeSems() {
  // important to BOTH close the semaphore object AND unlink the semaphore file
  sem_close(molecule);
  sem_unlink("molecule");
  sem_close(hleave);
  sem_unlink("hleave");
  sem_close(sleave);
  sem_unlink("sleave");
  sem_close(oleave);
  sem_unlink("oleave");
  sem_close(exited);
  sem_unlink("exited");
}

/* next two functions from Sherri's code*/
void delay( int limit )
{
  int j, k;

  for( j=0; j < limit; j++ )
    {
      for( k=0; k < limit; k++ )
        {
        }
    }
}

int checkSem(sem_t* sema, char* filename) {
  if (sema==SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore %s already exists, unlinking and reopening\n", filename);
      fflush(stdout);
      sem_unlink(filename);
      return -1;
    }
    else {
      printf("semaphore %s could not be opened, error # %d\n", filename, errno);
      fflush(stdout);
      exit(1);
    }
  }
  return 0;
}

int checkCounts() {
  //if there are at least two hydrogens, at least one sulfur, and at least
  // four oxygens, produce a molecule
  if(count[0] >= 2 && count[1] >= 1 && count[2] >= 4) {
    sem_post(molecule);
    //these atoms will leave. update counts accordingly
    count[0] = count[0] - 2;
    count[1] = count[1] - 1;
    count[2] = count[2] - 4;
  }
  // the first molecule needs permission to post
  if(oleaveCount == 0) {
    sem_post(exited);
  }
  return 0;
}
