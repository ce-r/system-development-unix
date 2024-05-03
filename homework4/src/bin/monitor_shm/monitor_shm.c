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
#include <shmlib.h>

static Shm_attribs1* shm_ptr;
static volatile sig_atomic_t perish = 0;
int valid_count = 0;

void alrm_handler(int sig) {
    perish = 1;
}

float valid_xavg(){
    int i;
    valid_count = 0;
    float x_sum = 0;
    for (i=0; i<SHM_ARR_SIZE; i++){
        if (shm_ptr[i].is_valid){
            x_sum += shm_ptr[i].x;
            valid_count++;
            // printf("x_sum: %f,  valid_count: %d\n", x_sum, valid_count);
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
        if (shm_ptr[i].is_valid){
            y_sum += shm_ptr[i].y;
            // printf("y_sum: %f\n", y_sum);
        }
    }
    if (valid_count == 0){
        return OK;
    }
    
    return y_sum/valid_count; 
}

int main(int argc, char* argv[]){
    FILE* file;

    if (argc < 1){
        printf("error: command line\n");
        exit(1);
    }

    float monitor_time = 30.0;
    if (argc == 2){
        monitor_time = atof(argv[1]);
    }

    shm_ptr = (Shm_attribs1*) connect_shm(KEY, sizeof(Shm_attribs1) * SHM_ARR_SIZE);
    if (shm_ptr == NULL) {
        printf("error: connect_shm\n");
        exit(1);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));    
    sa.sa_handler = alrm_handler;
    sigaction(SIGALRM, &sa, NULL);

    struct itimerval timer;
    timer.it_value.tv_sec = abs(monitor_time);
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);
    int ct = 0;
    while (1){
        if (perish){
            detach_shm_all(KEY);
            exit(1);
        }

        float xavg = valid_xavg();
        float yavg = valid_yavg();
        // printf("updated shm info - xavg: %f, yavg: %f, time increment: %d\n", xavg, yavg, monitor_time);
        printf("at time: %d:%d are active: xavg = %f and yavg = %f\n", ct, valid_count, xavg, yavg);
        sleep(1);
        ct++;    
        
        /* 
        At time 0:no elements are active
        At time 1:2 elements are active:x = 35.00 and y = 23.00
        At time 2:2 elements are active:x = 35.00 and y = 23.00
        At time 3:2 elements are active:x = 35.00 and y = 23.00
        At time 4:3 elements are active:x = 28.47 and y = 15.33
        At time 5:3 elements are active:x = 28.47 and y = 15.33
        At time 6:4 elements are active:x = 21.35 and y = 12.50
        At time 7:3 elements are active:x = 18.47 and y = 8.00
        At time 8:2 elements are active:x = 20.00 and y = 12.00
        At time 9:1 elements are active:x = 40.00 and y = 20.00
        At time 10:no elements are active  
        */ 
    }
    
    exit(0);
}


/*

monitor_shm

The monitor_shm program shall take one optional argument. This argument, if present,
would be an integer which represents the amount of time in seconds to monitor the
shared memory segment. If the argument is not present, 30 seconds will be the default
value.

Approximately each second, this program will print a line of information to the screen
about the contents of the shared memory. Information to be reported includes:

(1) Count of the active array elements (i.e. the number of elements which are valid)
(2) The average x value over the active array elements, and
(3) The average y value over the active array elements.
Before monitor_shm exits, it shall detach (but not destroy) the shared memory segment.

** start timer after data installation **

An example
Assume that the file input_data contains the following:
0 40.0 20.0 1
1 30.0 26.0 0
3 15.4 0.0 3
2 0.0 4.0 2
1 0.0 0.0 -1
3 0.0 0.0 -1
2 0.0 0.0 -1
0 0.0 0.0 -1
19 99.0 -10.0 2
18 -15.0 -2.5 1

If the following command was executed:
//bin> install_data input data & monitor_shm 10//
bin> install_data input.txt & monitor_shm 10

monitor_shm might report the following (your results may differ due to slight timing
differences):
At time 0:no elements are active
At time 1:2 elements are active:x = 35.00 and y = 23.00
At time 2:2 elements are active:x = 35.00 and y = 23.00
At time 3:2 elements are active:x = 35.00 and y = 23.00
At time 4:3 elements are active:x = 28.47 and y = 15.33
At time 5:3 elements are active:x = 28.47 and y = 15.33
At time 6:4 elements are active:x = 21.35 and y = 12.50
At time 7:3 elements are active:x = 18.47 and y = 8.00
At time 8:2 elements are active:x = 20.00 and y = 12.00
At time 9:1 elements are active:x = 40.00 and y = 20.00
At time 10:no elements are active

Note that the times provided in the input file are time increments; if the time increment is
0, then that entry is installed with no delay. Note that an *index cannot be deleted with 
no delay*; a delay of at least one second is required.

*/