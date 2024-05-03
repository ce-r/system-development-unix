
#define OK 0
#define ERROR -1

#define SHM_ARR_SIZE 20
#define KEY	0x6819

typedef struct {
    int key;
    int shmid;
    void* addrs[SHM_ARR_SIZE];
    int addr_count;
} Shm_attribs0;

typedef struct {
    int is_valid;
    float x;
    float y;
} Shm_attribs1;

void* connect_shm(int, int);
int detach_shm(void*);
int destroy_shm(int);
int detach_shm_all(int);
int get_valid_ct();
void set_file(char*);
char* get_file();
