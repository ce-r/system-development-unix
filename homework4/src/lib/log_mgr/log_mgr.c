/*
--HEADER--
Carlos Rivas 
605.614: HW3

Description:
made some corrections on this library from the feedback.
Functions to close fd, change logfile name, write logs to a file with specific formatting. 

*/



#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <td_common.h>
#include <log_mgr.h>


int Fd_tmp = -1;

int set_logfile (const char* Logfile_name){
	if (Fd_tmp == -1){
		if ((Fd_tmp = open (Logfile_name, O_CREAT | O_WRONLY | O_APPEND, 0600)) < 0){
			perror("error with sys call to open()");
			close(Fd_tmp);
			return ERROR;
		}
		Fd = Fd_tmp;
	}

	else { 
		if ((Fd = open(Logfile_name, O_CREAT | O_WRONLY | O_APPEND, 0600)) < 0){
			perror("error opening Logfile");
			close(Fd);
			Fd = Fd_tmp;
			return ERROR;
		}
	}

	if ((Logfile = strdup(Logfile_name)) == NULL){
		perror("malloc error for Logfile");
		close(Fd);
		return ERROR;
	}

    return OK;
}

int log_event (ThreadState state, const char *fmt, ...){
	char buffer[1024];
	time_t curr_time;
	va_list args;
	va_start(args, fmt);

	if (set_logfile(Logfile) == ERROR){
		return ERROR;
	}

	time(&curr_time);
	strftime(buffer, sizeof(buffer), "%b %d %H:%M:%S %Z %Y:", localtime(&curr_time));
	
	switch (state){
		case TH_INACTIVE:
			strcat(buffer, "TH_INACTIVE:");
			break;
		case TH_CREATE:
			strcat(buffer, "TH_CREATE:");
			break;
		case TH_CANCEL_PT:
			strcat(buffer, "TH_CANCEL_PT:");
			break;
		case TH_CANCEL:
			strcat(buffer, "TH_CANCEL:");
			break;
		case TH_CANCELED:
			strcat(buffer, "TH_CANCELED:");
			break;
		case TH_EXIT:
			strcat(buffer, "TH_EXIT:");
			break;
		case TH_TERM:
			strcat(buffer, "TH_TERM:");
			break;
		case TH_ERROR:
			strcat(buffer, "TH_ERROR:");
			break;
	}

	vsprintf(buffer + strlen(buffer), fmt, args);
    strcat(buffer, "\n");

	if (Fd != -1){
		if (write(Fd, buffer, strlen(buffer)) < 0){
			perror("error writing to Logfile");
			va_end(args);
			return ERROR;
		}
	}

	va_end(args);
    return OK;
}

void close_logfile (){
	if (Fd != -1){
		close(Fd);
	}
}
