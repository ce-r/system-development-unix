/*
--HEADER--
Carlos Rivas 
605.614: HW3

Description:
macros and function prototypes for our thread library. everything that validates what we have in the
thread library. 
*/


#ifndef THREAD_MGR_H
#define THREAD_MGR_H

#include <stdbool.h>
#include <pthread.h>
#include <td_common.h>
#include <sys/time.h>

#define THD_OK 0
#define THD_ERROR -1
#define THD_LIMIT 50
#define BUFFER_SIZE 256

#define DATA_SIZE 1000
#define ITERATIONS 10000
#define SIGMA 1.0 
#define PROPOSAL_SD 0.1  

typedef pthread_t ThreadID;
typedef int ThreadHandle;
typedef char* ThreadName;
typedef void* (*Funcptr) (void*);

typedef struct {
    ThreadHandle index;
    ThreadID tid;
    ThreadName name;
    ThreadState state;
} ThreadInfo;

typedef struct {
    char* file;
    float optional_wait;
} Init_commandline;

int th_execute(Funcptr);
int th_wait(ThreadHandle);
int th_wait_all();
int th_kill(ThreadHandle);
int th_kill_all();
int th_exit();

void custom_sig_handlers();
void th_info_init(ThreadHandle, ThreadID, ThreadName, ThreadState);
void th_info_purge(ThreadHandle);
void th_info_init0();
void th_info_str();

void set_nthreads(int);
int get_nthreads();
int get_creat_flag();
void set_creat_flag(int);
void set_struct_flag();
int get_struct_flag();

void set_delay(float);
float get_delay();


void init_programs_struct();
int get_optional_wait();
void set_optional_wait(int);
char* get_file0();
void set_file0(char*);

void sigIntHandler();
void sigQuitHandler();
void sigAlarmHandler();

#endif