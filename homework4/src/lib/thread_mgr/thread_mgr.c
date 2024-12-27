/*
--HEADER--
Carlos Rivas 
605.614
Project: HW3

Description:
all of the requested thread functions are included and should work as expected from testing. from thread
creation to joining, cancelling to exiting. and the setup of signal handlers was also written into th_execute. 
there are global variables which are accessed using getters and setters, facilitating accessibility in main()
for initialization of crucial values such as number threads or delay time to thread creation. 

*/


#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <td_common.h>
#include <thread_mgr.h>
#include <log_mgr.h>

volatile sig_atomic_t sigint_flag = 0;
volatile sig_atomic_t sigquit_flag = 0;

static bool Sighandlerflag = false;
static bool ThreadInfoInitFlag = false;
static int Nthreads = 0;
static ThreadHandle Th_handle = -1;
static ThreadHandle Th_count = 0;
static ThreadInfo Th_info[THD_LIMIT];
char Th_info_str[THD_LIMIT][BUFFER_SIZE];

static Init_commandline ips;

static pthread_mutex_t Th_info_mutex = PTHREAD_MUTEX_INITIALIZER;
static float delay = 0;
static int creation_flag = 0;
static int struct_flag = 0;

static char* cl_file = "";
static int cl_optional_wait = 30;

void set_struct_flag(){
    struct_flag = 1;
}

int get_struct_flag(){
    return struct_flag;
}

void set_file0(char* file){
    cl_file = file;
}

char* get_file0(){
    return cl_file;
}

void set_optional_wait(int opt_wait){
    cl_optional_wait = opt_wait;
}

int get_optional_wait(){
    return cl_optional_wait;
}

void init_programs_struct(){
    ips.file = get_file0();
    ips.optional_wait = get_optional_wait();
}

void set_creat_flag(int flag){
    creation_flag = flag;
}

int get_creat_flag(){
    return creation_flag;
}

void set_delay(float delay0){
    delay = delay0;
}

float get_delay(){
    return delay;
}

void set_nthreads(int nthreads){
    Nthreads = nthreads;
}

int get_nthreads(){
    return Nthreads;
}

void sigIntHandler(){// OK
    sigint_flag = 1;
    int i;
    int num_threads = get_nthreads();
    for (i=0; i<num_threads; i++){
        if (Th_info[i].state != TH_INACTIVE){
            write(STDOUT_FILENO, Th_info_str[i], strlen(Th_info_str[i]));
        }
    }
    // SIGINT receipt causes print out of th_info
}

void sigQuitHandler(){// OK
    sigquit_flag = 1;
    th_kill_all();
    // SIGQUIT receipt causes all threads to terminate
}

void sigAlarmHandler(){
    printf("sigalrm handler activated...\n");
    // sleep(delay);
}

void custom_sig_handlers() {
    struct sigaction sa_int, sa_quit;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_handler = sigIntHandler;
    sa_int.sa_flags = 0; 
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("error installing SIGINT handler\n");
    }

    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_handler = sigQuitHandler;
    sa_quit.sa_flags = 0; 
    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1) {
        perror("error installing SIGQUIT handler\n");
    }
}

void th_info_init0(){// OK
    int i;
    int num_threads = get_nthreads();
    for (i=0; i<num_threads; i++){
        Th_info[i].index = -1;
        Th_info[i].tid = -1;
        Th_info[i].name = "";
        Th_info[i].state = TH_INACTIVE;
    }
}

void th_struct_init(ThreadHandle th, ThreadID th_id, ThreadName th_name, ThreadState th_state){// OK
    Th_info[th].index = th;
    Th_info[th].tid = th_id;
    Th_info[th].name = strdup(th_name);
    Th_info[th].state = th_state;
}

void th_info_purge(ThreadHandle th){// OK
    Th_info[th].index = -1;
    Th_info[th].tid = -1;
    free(Th_info[th].name);
    Th_info[th].name = NULL;
    Th_info[th].state = TH_INACTIVE;
}

ThreadHandle get_thread_slot(){
    ThreadHandle th;
    int num_threads = get_nthreads();
    int i;
    for (i=0; i<num_threads; i++){
        if (Th_info[i].state == TH_INACTIVE){
            th = i;
            return th;
        }
    }
    return THD_ERROR;
}

void th_info_str(){// OK
    int i;
    int num_threads = get_nthreads();
    for (i=0; i<num_threads; i++){
        if (Th_info[i].state != TH_INACTIVE){
            snprintf(Th_info_str[i], BUFFER_SIZE, "handle: %d, tid: %p, name: %s, state: %d\n", Th_info[i].index, Th_info[i].tid, Th_info[i].name, Th_info[i].state);
        }
    }
}

int th_execute(Funcptr fptr){// OK
    pthread_mutex_lock(&Th_info_mutex);
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    if (ThreadInfoInitFlag != true){
        ThreadInfoInitFlag = true;
        th_info_init0();
    }
    pthread_mutex_unlock(&Th_info_mutex);
    
    pthread_t tid;
    int rtn_val;
    int* arg;

    if (get_struct_flag()){
        init_programs_struct();
        if ((rtn_val = pthread_create(&tid, NULL, fptr, (void*) &ips) != 0)){
            perror("error creating pthread\n");
            return THD_ERROR;
        }
    }

    else {
        if ((rtn_val = pthread_create(&tid, NULL, fptr, (void*) arg) != 0)){
            perror("error creating pthread\n");
            return THD_ERROR;
        }
    }

    pthread_mutex_lock(&Th_info_mutex);
    if ((Th_handle = get_thread_slot()) != THD_ERROR){
        Th_count++;
    } else {
        perror("no slots for thread\n");
        return THD_ERROR;
    }

    char thread_name[1024];
    snprintf(thread_name, sizeof(thread_name), "thread%d", Th_handle);
    th_struct_init(Th_handle, tid, thread_name, TH_CREATE);
    custom_sig_handlers();
    
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    if (get_creat_flag()){
        log_event(Th_info[Th_handle].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s, creation time: %.6f", Th_info[Th_handle].index, Th_info[Th_handle].tid, Th_info[Th_handle].name, elapsed_time+delay);
        printf("Th_info[].state: %d, Th_info[].index: %d, Th_info[].tid: %p, Th_info[].name: %s, creation time: %.6f\n", Th_info[Th_handle].state, Th_info[Th_handle].index, Th_info[Th_handle].tid, Th_info[Th_handle].name, elapsed_time+delay);
    } else {
        log_event(Th_info[Th_handle].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s, creation time: %.6f", Th_info[Th_handle].index, Th_info[Th_handle].tid, Th_info[Th_handle].name, elapsed_time);
        printf("Th_info[].state: %d, Th_info[].index: %d, Th_info[].tid: %p, Th_info[].name: %s, creation time: %.6f\n", Th_info[Th_handle].state, Th_info[Th_handle].index, Th_info[Th_handle].tid, Th_info[Th_handle].name, elapsed_time);
    }
    pthread_mutex_unlock(&Th_info_mutex);

    return Th_handle;
}

int th_wait(ThreadHandle th){// OK
    void* status;
    int num_threads = get_nthreads();
    if (num_threads == 0)
        return THD_ERROR;

    pthread_mutex_lock(&Th_info_mutex);
    if (Th_info[th].state == TH_INACTIVE){
            perror("error inactive thread\n");
    } else {
        if (pthread_join(Th_info[th].tid, &status) == 0) {
            Th_info[th].state = TH_TERM;
            log_event(Th_info[th].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[th].index, Th_info[th].tid, Th_info[th].name);
            printf("Th_info[].state: %d, Th_info[].index: %d, Th_info[].tid: %p, Th_info[].name: %s\n", Th_info[th].state, Th_info[th].index, Th_info[th].tid, Th_info[th].name);
            th_info_purge(th);
            Th_count--;
        } else {
            Th_info[th].state = TH_ERROR;
            log_event(Th_info[th].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[th].index, Th_info[th].tid, Th_info[th].name);
            pthread_mutex_unlock(&Th_info_mutex);
            perror("error joining thread\n");
            return THD_ERROR;
        }
    }
    pthread_mutex_unlock(&Th_info_mutex);
    
    return THD_OK;
}

int th_wait_all() {
    void* status;
    int i;
    int num_threads = get_nthreads();
    if (num_threads == 0)
        return THD_ERROR;

    pthread_mutex_lock(&Th_info_mutex);
    for (i=0; i<num_threads; i++) {
        if (Th_info[i].state == TH_INACTIVE){
            perror("error inactive thread\n");
        } else {
            if (pthread_join(Th_info[i].tid, &status) == 0) {
                Th_info[i].state = TH_TERM;
                log_event(Th_info[i].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[i].index, Th_info[i].tid, Th_info[i].name);
                printf("Th_info[].state: %d, Th_info[].index: %d, Th_info[].tid: %p, Th_info[].name: %s\n", Th_info[i].state, Th_info[i].index, Th_info[i].tid, Th_info[i].name);
                th_info_purge(i);
                Th_count--;
            } else {
                Th_info[i].state = TH_ERROR;
                log_event(Th_info[i].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[i].index, Th_info[i].tid, Th_info[i].name);
                pthread_mutex_unlock(&Th_info_mutex);
                perror("error joining thread\n");
                return THD_ERROR;
            }
        }
    }
    pthread_mutex_unlock(&Th_info_mutex);

    return THD_OK;
}


int th_kill(ThreadHandle th){// OK
    void* status;
    int num_threads = get_nthreads();
    if (num_threads == 0)
        return THD_ERROR;

    pthread_mutex_lock(&Th_info_mutex);
    if (Th_info[th].state == TH_INACTIVE){
            perror("error inactive thread\n");
            return THD_ERROR;
    } else {
        if (pthread_cancel(Th_info[th].tid) == 0) {
            Th_info[th].state = TH_CANCEL;
            log_event(Th_info[th].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[th].index, Th_info[th].tid, Th_info[th].name);
            printf("Th_info[].state: %d, Th_info[].index: %d, Th_info[].tid: %p, Th_info[].name: %s\n", Th_info[th].state, Th_info[th].index, Th_info[th].tid, Th_info[th].name);
        } else {
            Th_info[th].state = TH_ERROR;
            log_event(Th_info[th].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[th].index, Th_info[th].tid, Th_info[th].name);
            perror("error cancelling pthread\n");
            return THD_ERROR;
        }
    }
    pthread_mutex_unlock(&Th_info_mutex);

    return THD_OK;
}

int th_kill_all(){// OK
    int i;
    int num_threads = get_nthreads();
    if (num_threads == 0)
        return THD_ERROR;

    pthread_mutex_lock(&Th_info_mutex);
    for (i=0; i<num_threads; i++){
        if (Th_info[i].state == TH_INACTIVE){
            perror("error inactive thread\n");
        } else {
            if (pthread_cancel(Th_info[i].tid) == 0) {
                Th_info[i].state = TH_CANCEL;
                log_event(Th_info[i].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[i].index, Th_info[i].tid, Th_info[i].name);
                printf("Th_info[].state: %d, Th_info[].index: %d, Th_info[].tid: %p, Th_info[].name: %s\n", Th_info[i].state, Th_info[i].index, Th_info[i].tid, Th_info[i].name);
            } else {
                Th_info[i].state = TH_ERROR;
                log_event(Th_info[i].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[i].index, Th_info[i].tid, Th_info[i].name);
                pthread_mutex_unlock(&Th_info_mutex);
                
                perror("error cancelling pthread\n");
                return THD_ERROR;
            }
        }
    }
    pthread_mutex_unlock(&Th_info_mutex);
        
    return THD_OK;
}

int th_exit(){// OK
    pthread_mutex_lock(&Th_info_mutex);
    pthread_t exit_tid = pthread_self();
    int num_threads = get_nthreads();
    if (num_threads == 0)
        return THD_ERROR;

    int i;
    for (i=0; i<num_threads; i++){
        if (Th_info[i].state == TH_INACTIVE){
            perror("error inactive thread\n");
        } else {
            if (pthread_equal(Th_info[i].tid, exit_tid)) {
                Th_info[i].state = TH_EXIT;
                log_event(Th_info[i].state, ":thread_info[].index: %d, thread_info[].tid: %p, thread_info[].name: %s", Th_info[i].index, Th_info[i].tid, Th_info[i].name);
                printf("Th_info[].state: %d, Th_info[].index: %d, Th_info[].tid: %p, Th_info[].name: %s\n", Th_info[i].state, Th_info[i].index, Th_info[i].tid, Th_info[i].name);
                break;
            }
        }
    }
    pthread_mutex_unlock(&Th_info_mutex);
    
    pthread_exit(NULL);
}
