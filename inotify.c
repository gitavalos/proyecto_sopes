#include  <stdio.h>
#include  <signal.h>
#include  <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))


void     INThandler(int);
FILE* fptr;

int  main(void)
{
     signal(SIGINT, INThandler);
	 
	 int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];
	fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
    }
    wd = inotify_add_watch(fd, ".",IN_MODIFY | IN_CREATE | IN_DELETE);
	fptr = fopen("./log.txt", "a");
     while (1){
		  i = 0;	  
		length = read(fd, buffer, BUF_LEN);
		if (length < 0) {
			perror("read");
		}
		while (i < length) {
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			if (event->len) {
				if (event->mask & IN_CREATE) {
					printf("The file %s was created.\n", event->name);
					
					fprintf(fptr,"The file %s was created.\n",event->name);
				} else if (event->mask & IN_DELETE) {
					printf("The file %s was deleted.\n", event->name);
					
					fprintf(fptr,"The file %s was deleted.\n", event->name);
				} else if (event->mask & IN_MODIFY) {
					printf("The file %s was modified.\n", event->name);
					
					fprintf(fptr,"The file %s was modified.\n", event->name);
				}
			}
			i += EVENT_SIZE + event->len;
		}
        pause();
	 }
	 (void) inotify_rm_watch(fd, wd);
    (void) close(fd);
	 printf ("Exiting inotify example...\n");
     return 0;
}

void  INThandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);
     printf("OUCH, did you hit Ctrl-C?\n"
            "Do you really want to quit? [y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y'){
		  fclose(fptr);
	 exit(0);}
     else{
	 signal(SIGINT, INThandler);}
     getchar(); // Get new line character
}