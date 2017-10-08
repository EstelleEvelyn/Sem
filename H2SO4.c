#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "H2SO4.h"

int checkSem(sem_t*, char*);
void delay(int limit);

sem_t* hleave;
sem_t* sleave;
sem_t* mutex;
sem_t* count[2];


void* oxygen(void*){
  delay(rand()%5000);
  printf("oxygen produced\n");
  fflush(stdout);

  count[1]++;

  sem_wait(mutex);

  sem_wait(sleave);
  printf("oxygen exited\n");
  fflush(stdout);
  return(void*) 0;
}

void* hydrogen(void*) {
  delay(rand()%5000);
  printf("hydrogen produced\n");
  fflush(stdout);

  count[0]++;

  sem_wait(mutex);

  printf("hydrogen exited\n");
  fflush(stdout);
  sem_post(hleave);
  return (void*) 0;

}
void* sulfur(void*){
  delay(rand()%5000);
  printf("sulfur produced\n");
  fflush(stdout);

  if(count[0] >= 2 && count[1] >= 1 && count[2] >= 4) {
    sem_post(mutex);
    printf("*** H20 molecule produced ***\n");
    fflush(stdout);
    count[0] = count[0] - 2;
    count[1] = count[1] - 4;
  }

  sem_wait(hleave);
  printf("sulfur exited\n");
  fflush(stdout);
  sem_post(sleave);
  return(void*) 0;
}

void openSems() {
  mutex = sem_open("mutex", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(mutex, "mutex") == -1) {
    mutex = sem_open("mutex", O_CREAT|O_EXCL, 0466, 0);
  }
  hleave = sem_open("hleave", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(hleave, "hleave") == -1) {
    hleave = sem_open("hleave", O_CREAT|O_EXCL, 0466, 0);
  }
  sleave = sem_open("sleave", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(sleave, "sleave") == -1) {
    sleave = sem_open("sleave", O_CREAT|O_EXCL, 0466, 0);
  }
}
void closeSems() {
  // important to BOTH close the semaphore object AND unlink the semaphore file
  sem_close(mutex);
  sem_unlink("mutex");
}

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
