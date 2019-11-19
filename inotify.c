#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

void add_watches(int fd, char *root)
{
  int wd;
  char *abs_dir;
  struct dirent *entry;
  DIR *dp;
 
  dp = opendir(root);
  if (dp == NULL)
    {
      perror("Error opening the starting directory");
      exit(0);
    }
 
  /* add watch to starting directory */
  wd = inotify_add_watch(fd, root, IN_CREATE | IN_MODIFY | IN_DELETE); 
  if (wd == -1)
    {
      fprintf(fp_log,"Couldn't add watch to %s\n",root);
    }
  else
    {
      printf("Watching:: %s\n",root);
    }
 
  /* Add watches to the Level 1 sub-dirs*/
  abs_dir = (char *)malloc(MAX_LEN);
  while((entry = readdir(dp)))
    { 
      /* if its a directory, add a watch*/
      if (entry->d_type == DT_DIR)
        {
          strcpy(abs_dir,root);
          strcat(abs_dir,entry->d_name);
           
          wd = inotify_add_watch(fd, abs_dir, IN_CREATE | IN_MODIFY | IN_DELETE); 
          if (wd == -1)
              printf("Couldn't add watch to the directory %s\n",abs_dir);
          else
            printf("Watching:: %s\n",abs_dir);
        }
    }
   
  closedir(dp);
  free(abs_dir);
}

int main(int argc, char **argv) {   
    int length, i = 0;
    int fd;
    //int wd;
    char buffer[BUF_LEN];
	FILE* fptr;
    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
    }
    //wd = inotify_add_watch(fd, "/home",IN_MODIFY | IN_CREATE | IN_DELETE);	
	add_watches(fd,"/home");
	//fptr = fopen("/log.txt", "w+");	
		//fprintf(fptr,"HELLO WORLD");
		//fclose(fptr);
	
	while(1)
    {
		
		i = 0;
	  
		length = read(fd, buffer, BUF_LEN);

		if (length < 0) {
			perror("read");
		}

		if (i < length) {
			fptr = fopen("/log.txt", "a");
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
			fclose(fptr);
		}
		
	}
	
    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);
	printf ("Exiting inotify example...\n");
	return 1;
}