#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include <apue.h>
#include <time.h>


void toUpperCase(int fd){
	char buf[MAXLINE];
    ssize_t nread;
    int i;
    while ((nread = read(fd, buf, MAXLINE)) > 0) {
        for (i=0; i < nread; i++) {
            buf[i] = toupper((unsigned char) buf[i]);
        }
        if (write(STDOUT_FILENO, buf, nread) != nread) {
            perror("write error to stdout");
        }
    }
    if (nread == -1) {
        perror("read error");
    }
    fflush(stdout);
}

int main(){
	int maxfd;
	fd_set readset;
	FILE* fpin;
	char line[MAXLINE];
	
	char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) < 0) {
        perror("gethostname error");
        exit(1);
    }
    printf("host: %s\n", hostname);

	int first = 0;
	while (1){
		if (strcmp(hostname, "absaroka.apl.jhu.edu") == 0){
			if ((fpin = popen("/home/jcn/unix_class/fortune_absaroka/fortune", "r")) == NULL){
				perror("popen error");
				exit(1);
			}
		} else {
			if ((fpin = popen("/home/jcn/unix_class/fortune/fortune", "r")) == NULL){
				perror("popen error");
				exit(1);
			}
		}

		int fortune_fd = fileno(fpin);// convert FILE* to fd
		srand(time(NULL));
		int rand_num = (rand() % 8) + 1;
		if (!first){
			toUpperCase(fortune_fd);
			first = 1;
		} else {
			sleep(rand_num);
			toUpperCase(fortune_fd);
		}

		if (pclose(fpin) == -1) {
            perror("pclose error");
            exit(1);
        }

		printf("\ntype something that begins with 'q' to quit or something else for another fortune: ");
        fflush(stdout);// flush stdout to display prompt

		FD_ZERO(&readset);
        FD_SET(STDIN_FILENO, &readset);// init fd set and add STDIN_FILENO to it

		if (select(maxfd+1, &readset, NULL, NULL, NULL) == -1){// select() checks fd readiness and specify fd range w/ maxfd+1 
			perror("select error");
			exit(1);
		}

		if (FD_ISSET(STDIN_FILENO, &readset)){// checks if fd contained in set, if marked by select as "ready"
			if (fgets(line, MAXLINE, stdin) == NULL){// read from stdin fd
				clearerr(stdin);// clean error
				continue;
			}
			if (tolower(line[0]) == 'q') {
                break;
            }
		}
	}

	exit(0);
}
