#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <shmlib.h>

#define FTOK_PATH "/home/jcn"
#define ANY_SHM_ADDR 0x00000000

static Shm_attribs0 shm_attribs_arr0[SHM_ARR_SIZE];
static Shm_attribs1 shm_attribs_arr1[SHM_ARR_SIZE];

static int shm_key_counter = 0;
static char* file;

void* connect_shm(int key, int size){
    int shmid;
    void* shm_addr_ptr;

    shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        return NULL;  
    }

    shm_addr_ptr = shmat(shmid, ANY_SHM_ADDR, 0);
    if (shm_addr_ptr == (void*) -1) {
        perror("shmat failed");
        return NULL; 
    }

    shm_key_counter++;
    shm_attribs_arr0[shm_key_counter-1].key = key;
    shm_attribs_arr0[shm_key_counter-1].shmid = shmid;
    shm_attribs_arr0[shm_key_counter-1].addr_count = shm_attribs_arr0[shm_key_counter-1].addr_count++;

    int addr_ct = shm_attribs_arr0[shm_key_counter-1].addr_count;
    shm_attribs_arr0[shm_key_counter-1].addrs[addr_ct] = shm_addr_ptr;

    return shm_addr_ptr;


    /*
    void* connect_shm(int key, int size)
    This function has two arguments. The first argument serves as the key for the shared
    memory segment. The second argument contains the size (in bytes) of the shared
    memory segment to be allocated. The return value for this function is a pointer to the
    shared memory area which has been attached (and possibly created) by this function. If,
    for some reason, this function cannot connect to the shared memory area as requested,
    it shall return a NULL pointer. *** A program using this library function must be able to use it
    to attach the maximum number of shared memory segments to the calling process. ***
    (Note that Solaris 10 does not have a limit to the number of attachments, so you can use
    the limit that Linux supports – use the limit you find on our Linux server
    absaroka.apl.jhu.edu).
    */
}

int detach_shm(void* addr){
    if (shmdt(addr) < 0) {
        perror("shmdt failed");
        return ERROR;
    }

    return OK;


    /*
    int detach_shm(void *addr)
    This function detaches the shared memory segment attached to the process via the
    argument addr. The associated shared memory segment is not deleted from the system.
    This function will return OK (0) on success, and ERROR (-1) otherwise.
    */
}

int detach_shm_all(int key){
    int i, j, key_found = 0;
    for (i=0; i<SHM_ARR_SIZE; i++){
        if (shm_attribs_arr0[i].key == key){
            key_found = 1;
            for (j=0; j<shm_attribs_arr0[i].addr_count; j++){
                detach_shm(shm_attribs_arr0[i].addrs[j]);
            }
            break;
        } else if((i == SHM_ARR_SIZE-1) && (key_found == 0)){
            return ERROR;
        }
    }

    return i;
}

int destroy_shm(int key){
    int i = detach_shm_all(key);
    if (shmctl(shm_attribs_arr0[i].shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl failed");
        return ERROR;
    }

    return OK;


    /*
    int destroy_shm(int key)
    This function detaches all shared memory segments (attached to the calling process by
    connect_shm( )) associated with the argument key from the calling process. The
    shared memory segment is then subsequently deleted from the system. This function
    will return OK (0) on success, and ERROR (-1) otherwise.
    */
}

int get_valid_ct(){
    int i, count = 0;
    for (i=0; i<SHM_ARR_SIZE; i++){
        if (shm_attribs_arr1[SHM_ARR_SIZE].is_valid){
            count++;
        }
    }
    return count;
}

void set_file(char* filename){
    file = filename;
}

char* get_file(){
    return file;
}
