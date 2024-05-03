#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <shmlib.h>

Shm_attribs1* shm_ptr;

void init_shm() {
    printf("initializing data struct...\n");
    int i;
    for (i = 0; i < SHM_ARR_SIZE; i++) {
        shm_ptr[i].is_valid = 0;
        shm_ptr[i].x = 0.0;
        shm_ptr[i].y = 0.0;
    }
}

void reinstall(){
    init_shm();
    printf("reinstalling data...\n");
    
    char line[1024];
    char* argv_ = get_file();
    FILE* file = fopen(argv_, "r");
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
                shm_ptr[idx].is_valid = 0;
                // printf("nothing installed, negative time increment\n");
            } else {
                sleep(delay);
                shm_ptr[idx].is_valid = 1;
                shm_ptr[idx].x = x;
                shm_ptr[idx].y = y;
                // printf("data installed on/written to shm - index: %d, x: %f, y: %f\n", idx, shm_ptr[idx].x, shm_ptr[idx].y);
            }
        }
    }

}

void sighup_handler(int signum){
    reinstall();
}

void sigterm_handler(){
    destroy_shm(KEY);
}


/* /home/unix_class/Homework4/test.in */
int main(int argc, char* argv[]){
    signal(SIGHUP, sighup_handler);
    signal(SIGTERM, sigterm_handler);

    FILE* file;
    char line[1024];
    
    if (argc < 2){
        printf("error: command line\n");
        exit(1);
    }
    file = fopen(argv[1], "r");
    if (file == NULL){
        printf("error: file\n");
        exit(1);
    }
    set_file(argv[1]);

    shm_ptr = (Shm_attribs1*) connect_shm(KEY, sizeof(Shm_attribs1) * SHM_ARR_SIZE);
    if (shm_ptr == NULL) {
        printf("error: connect_shm\n");
        fclose(file);
        exit(1);
    }

    init_shm();
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
                shm_ptr[idx].is_valid = 0;
                // printf("nothing installed, negative time increment\n");
            } else {
                sleep(delay);
                shm_ptr[idx].is_valid = 1;
                shm_ptr[idx].x = x;
                shm_ptr[idx].y = y;
                // printf("data installed on/written to shm - index: %d, x: %f, y: %f\n", idx, shm_ptr[idx].x, shm_ptr[idx].y);
            }
        }
    }

    fclose(file);
    if (destroy_shm(KEY) != OK){
        printf("error: destroy_shm\n");
        exit(1);
    }

    exit(0);
}


// ** TEST SIGNALS **


/*

install_data

The install_data program takes a file name as an argument. The file will contain lines of
text which describes the data to be written into shared memory and the time (relative to
the start time of the program) at which the data shall be placed in the shared memory
area. The program will perform the following:

(1) Verify that the argument file is provided on the command line, and that the file can be
opened for reading.
(2) Call connect_shm( ) which should return a pointer to the shared memory area.
(3) Process the data from the file, a line at a time. Write the data to the shared memory 
at the designated time. (Be sure to verify that the index given in the input file is valid.) 
After you have installed all the data according to the input file, don’t put any other data 
into shared memory.


(4) Call destroy_shm( ) to delete the shared memory segment from the system. (Does
this need to be coordinated with the use of the shared memory by monitor_shm? Why 
or why not?)


(5) Exit.

The file which install_data reads will be a text file. Each line of the file will follow the
following format:

<index> <x_value> <y_value> <time increment>

where index ranges from 0 to 19 and indicates which element of the shared memory
structure is to be written to; x_value and y_value are floating point numbers which are to
be installed in the x and y members of that structure. If the * time increment variable is
non-negative, then this value represents the integral number of seconds to delay until
the data on that line are installed in the shared memory. * If the time increment value is
* negative, the absolute value of this increment represents the integral number of seconds
to delay before making the corresponding index invalid. * (The x and y values are ignored
in this case.) There can be * any number of white space (tabs or spaces) between each
field on a line. *


* signals *

Additionally, install_data should handle errors in the input data file; the output of
install_data (that is, what gets installed into shared memory) is undefined (NULL) in this 
case, but the program should never "core dump" or get caught in an infinite loop due to errors
in the format of the input file. (For more details on the operation of install_data, see the
example below.)

Upon receipt of a * SIGHUP signal *, the install_data program should * clear 
the shared memory area *, and re-install the requested data as described above 
from the beginning of the input file.

Also, upon the receipt of a * SIGTERM signal, the install_data program 
shall detach and destroy the shared memory segment * and then exit.

//// //// //// //// //// //// //// //// //// //// //// //// //// //// //// //// 

The format of the shared memory
The two programs described immediately following this section (4.3.2 install_data and
4.3.3 monitor_shm) communicate using shared memory as an array of structures. Each
element of the array must be the following structure:

struct {
    int is_valid;
    float x;
    float y;
};

The array will contain 20 elements. The first member of the structure, is_valid, shall
contain 0 if the array element is not valid, and 1 if the array element is valid. Any valid
floating point number can be entered into structure members x and y. Upon creation of
shared memory, assure that all members of each array element are set to 0. (Make sure
your shared memory library does not make any presumptions about this particular
format; the shared memory format should only affect the following programs.)

*/