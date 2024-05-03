#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shmlib.h"
#include "thread_mgr.h"


static Shm_attribs0 shm_attribs_arr0[SHM_ARR_SIZE];
static Shm_attribs1 shm_attribs_arr1[SHM_ARR_SIZE];

static volatile sig_atomic_t perish = 0;
int valid_count = 0;

//// //// //// //// //// //// //// //// //// //// //// //// //// //// //// ////

void init_struct_arr() {
    printf("initializing/clearing data struct...\n");

    int i;
    for (i = 0; i < SHM_ARR_SIZE; i++) {
        shm_attribs_arr1[i].is_valid = 0;
        shm_attribs_arr1[i].x = 0.0;
        shm_attribs_arr1[i].y = 0.0;
    }
}

void reinstall(){
    init_struct_arr();
    printf("reinstalling data...\n");
    
    char line[1024];
    char* argv = get_file0();
    FILE* file = fopen(argv, "r");
    if (file == NULL){
        printf("error: file\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), file)){
        int idx, delay;
        float x, y;

        if (sscanf(line, "%d %f %f %d", &idx, &x, &y, &delay) == 4){
            if (idx < 0 || idx >= SHM_ARR_SIZE) {
                fprintf(stderr, "error: index: %d\n", idx);
                continue;
            }
            // printf("data read from file - index: %d, x: %f, y: %f, time increment: %d\n", idx, x, y, delay);
            if (delay < 0){
                delay = abs(delay);
                sleep(delay);
                shm_attribs_arr1[idx].is_valid = 0;
                // printf("nothing installed, negative time increment\n");
            } else {
                sleep(delay);
                shm_attribs_arr1[idx].is_valid = 1;
                shm_attribs_arr1[idx].x = x;
                shm_attribs_arr1[idx].y = y;
                // printf("data installed on/written to shm - index: %d, x: %f, y: %f\n", idx, shm_attribs_arr1[idx].x, shm_attribs_arr1[idx].y);
            }
        }
    }
}

void sighup_handler(int signum){
    reinstall();
}

void sigterm_handler(){
    init_struct_arr();
}

//// //// //// //// //// //// //// //// //// //// //// //// //// //// //// ////

void alrm_handler(int sig) {
    perish = 1;
}

float valid_xavg(){
    int i;
    valid_count = 0;
    float x_sum = 0;
    for (i=0; i<SHM_ARR_SIZE; i++){
        if (shm_attribs_arr1[i].is_valid){
            x_sum += shm_attribs_arr1[i].x;
            valid_count++;
        }
    }
    if (valid_count == 0){
        return OK;
    }
    
    return x_sum/valid_count; 
}

float valid_yavg(){
    int i;
    float y_sum = 0;
    for (i=0; i<SHM_ARR_SIZE; i++){
        if (shm_attribs_arr1[i].is_valid){
            y_sum += shm_attribs_arr1[i].y;
        }
    }
    if (valid_count == 0){
        return OK;
    }
    
    return y_sum/valid_count; 
}

void* install_data(void* arg){
    Init_commandline* ips = (Init_commandline*) arg;

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGHUP);
    sigaddset(&sigset, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);

    signal(SIGHUP, sighup_handler);
    signal(SIGTERM, sigterm_handler);

    FILE* file = fopen(ips->file, "r");
    if (file == NULL){
        printf("error: file\n");
        exit(1);
    }
    set_file0(ips->file);

    //// //// //// //// //// //// //// //// //// //// //// //// //// //// //// //// 
    init_struct_arr();
    char line[1024];
    while (fgets(line, sizeof(line), file)){
        int idx;
        float x, y, delay;

        if (sscanf(line, "%d %f %f %f", &idx, &x, &y, &delay) == 4){
            if (idx < 0 || idx >= SHM_ARR_SIZE) {
                fprintf(stderr, "error: index: %d\n", idx);
                continue;
            }
            if (delay < 0){
                delay = abs(delay);
                sleep(delay);
                shm_attribs_arr1[idx].is_valid = 0;
            } else {
                sleep(delay);
                shm_attribs_arr1[idx].is_valid = 1;
                shm_attribs_arr1[idx].x = x;
                shm_attribs_arr1[idx].y = y;
            }
        }
    }

    fclose(file);
    exit(0);
    //// //// //// //// //// //// //// //// //// //// //// //// //// //// //// ////
}

void* monitor_shm(void* arg){
    Init_commandline* ips = (Init_commandline*) arg;
    FILE* file;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));    
    sa.sa_handler = alrm_handler;
    sigaction(SIGALRM, &sa, NULL);
    // sigprocmask(SIGBLOCK) module 6, block main and monitor_shm

    struct itimerval timer;
    timer.it_value.tv_sec = abs(ips->optional_wait);
    timer.it_value.tv_usec = (abs(ips->optional_wait) - timer.it_value.tv_sec) * 1000000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    int ct = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
    while (1){
        float xavg = valid_xavg();
        float yavg = valid_yavg();
        printf("at time: %d:%d are active: xavg = %f and yavg = %f\n", ct, valid_count, xavg, yavg);
        sleep(1);   
        ct++;     
    }
    
    exit(0);
}

int main(int argc, char* argv[]){
    if (argc < 2) {
        printf("usage: %s <file>\n", argv[0]);
        exit(1);
    }
    set_file0(argv[1]);

    if (argc == 3){
        set_optional_wait(atof(argv[2]));
    }

    set_nthreads(2);
    int nthreads = get_nthreads();
    if (nthreads != 2){
        perror("number of threads out of bounds, not in range\n");
        exit(1);
    }
    set_struct_flag();

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGHUP);
    sigaddset(&sigset, SIGTERM);
    sigaddset(&sigset, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    th_execute(install_data);
    th_execute(monitor_shm);
    
    // pid_t pid = getpid();
    // kill(pid, SIGHUP);
    // kill(pid, SIGTERM);
    
    th_wait_all();    
    th_exit();
}


/*

install_and_monitor

After you have successfully completed the above, you then should combine these two
programs into a multi-threaded program using your thread_mgr library from
homework 3. This install_and_monitor program shall work as described above, but ** only
in the context of a single process ** - the install_data program should be one thread 
within the program, and the monitor_shm shall be in another thread. This program should 
not use the shared memory library - in this program the "shared memory" should only be
** global memory ** available to both threads.

*/
