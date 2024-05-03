/*
--HEADER--
Carlos Rivas 
605.614: HW3

Description:
contains log_mgr function prototypes and globals for file descriptors and an initialized logfile name,
a few macros for return values

*/



#ifndef LOG_MGR_H
#define LOG_MGR_H

#define OK 0
#define ERROR -1

static char* Logfile = "logfile";
static int Fd, Read_fd;

int set_logfile (const char *logfile_name);
int log_event (ThreadState state, const char* fmt, ...);
void close_logfile ();

#endif
