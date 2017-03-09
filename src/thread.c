#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <netdb.h>
#include <netinet/in.h>

#include "../include/thread.h"

//extern struct Thread *head_thread;
//extern volatile sig_atomic_t do_shutdown;

char output[MAX_SIZE];
FILE *fp;
const char current_dir[] = "Now you are in directory: ";
const char file_copied[] = "File copied\n";
const char space[] = " ";

static void send_message(int socket_fd, const char *msg) {

  int rc;
  int message_size = strlen(msg);

  do {
    rc = send(socket_fd, msg, message_size, 0);
    if(rc < 0) {
      fprintf(stderr, "send() failed: %s\n", strerror(errno));
      abort();
    }
  } while(rc < message_size && !do_shutdown);
}

void prepare_tokens(char* msg, MsgParts* msgp) {
  char* fix = strtok(msg,space);

  while(fix != NULL) {
    int partNum = 0;
    if(partNum == 0) {
      msgp->part1 = fix;
      fix = strtok(NULL,space);
    }
    if(partNum == 1) {
      msgp->part2 = fix;
      fix = strtok(NULL,space);
    }
    if(partNum == 2) {
      msgp->part3 = fix;
      fix = strtok(NULL,space);
    }
    if(partNum == 3) {
      msgp->part4 = fix;
      fix = strtok(NULL,space);
    }
    if(partNum == 4) {
      msgp->part5 = fix;
      fix = strtok(NULL,space);
    }
    partNum++;
  }
}

void handle_ls() {
  //TO-DO
}

void handle_cd() {
  //TO-DO
}

void handle_cp() {
  //TO-DO

}

void handle_mv() {
  //TO-DO
}

void handle_scp() {
  //TO-DO
}

void handle_wget() {
  //TO-DO
}


/* This handles new connections */
void* handle_connection(void* inc)
{
  int socket_fd = *(int *)inc;
  int message_size;
  int i = 0;
  int c;
  char* message;
  char input[MAX_SIZE];
  char output[MAX_SIZE];

  MsgParts userCommand = {NULL, NULL, NULL, NULL, NULL};
  MsgParts* userCommand_p = &userCommand;
 
  send_message(socket_fd, "You are now connected\n");
  send_message(socket_fd, "READY\n");

  while(!do_shutdown) {
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));

    message_size = recv(socket_fd, input, sizeof(input)/sizeof(input[0]), 0);
    if(message_size > 0) {

      /* Process incoming message */
      char* message = (char*) malloc(strlen(input)+1);
      strcpy(message, input);

      /* Lets get the tokens */
      prepare_tokens(message, userCommand_p);
      
      /* ls */
      if(strcmp(userCommand.part1, "ls") == 0) {
        fp = popen(input, "r");
        if(fp == NULL)
          fprintf(stderr, "popen() failed: %s\n", strerror(errno));
        else {
	// ls
	  while((c = getc(fp)) != EOF) {
	    output[i] = c;
	    i++;
	  }
	  i = 0;
	  if(strlen(output) > 0)
	    send_message(socket_fd, output);
	  else
	    send_message(socket_fd, "No files\n");
	}
	pclose(fp);
      }

      /* cd .. */
      else if(strcmp(userCommand.part1, "cd") == 0) {
        if(strcmp(userCommand.part2, "..") == 0) {
          chdir(userCommand.part2);
	  if(getcwd(cwd, sizeof(cwd)) != NULL) {
            /* Char array to char* */
	    char* temp = (char*)malloc(strlen(cwd)+1);
	    strcpy(temp, cwd);

      	    int temp_size = strlen(temp) + strlen(current_dir) + 1 + 1;
	    char* msg  = (char*) malloc(temp_size);
	    strcpy(msg, current_dir);
	    strcat(msg, temp);
	    strcat(msg, "\n");
	    send_message(socket_fd, msg);
	    free(temp);
	    free(msg);
	  }
	  else
    	    perror("getcwd() error");
        }
        /* anything else really - directory MUST exist */
        else {
	    chdir(userCommand.part2);
	    if(getcwd(cwd, sizeof(cwd)) != NULL) {
	      /* Char array to char* */
	      char* temp = (char*)malloc(strlen(cwd)+1);
	      strcpy(temp, cwd);

	      int temp_size = strlen(temp) + strlen(current_dir) + 1 + 1;
	      char* msg  = (char*) malloc(temp_size);
	      strcpy(msg, current_dir);
	      strcat(msg, temp);
	      strcat(msg, "\n");
	      send_message(socket_fd, msg);
	      free(temp);
	      free(msg);
	    } else
              perror("getcwd() error");
	}
      }

      /* copy - cp file.txt /home/lisek/file.txt */
      else if(strcmp(userCommand.part1, "cp") == 0) { 
        int src_fd, dst_fd, n, err;
	unsigned char buffer[4096];
	char* src_path;
	char* dst_path;
	src_path = userCommand.part2;
	dst_path = userCommand.part3;
	src_fd = open(src_path, O_RDONLY);
	dst_fd = open(dst_path, O_CREAT | O_WRONLY);

        while (1) {
          err = read(src_fd, buffer, 4096);
          if (err == -1) {
            printf("Error reading file.\n");
            exit(1);
          }
          n = err;

          if (n == 0) break;

          err = write(dst_fd, buffer, n);
          if (err == -1) {
            printf("Error writing to file.\n");
            exit(1);
          }
        }
        chmod(dst_path, S_IRUSR|S_IRGRP|S_IROTH);
        close(src_fd);
        close(dst_fd);
        send_message(socket_fd, file_copied);
      }

      /* move - mv file.xt /home/lisek/Workspace/file.txt */
      else if(strcmp(userCommand.part1, "mv") == 0) {
        int n;
        char* src_path;
        char* dst_path;
        src_path = userCommand.part2;
        dst_path = userCommand.part3;
        if(rename(src_path, dst_path)) {
          perror("error moving file");
    	  send_message(socket_fd, "Error while moving file\n");
        }
        else
          send_message(socket_fd, "File has been moved to target location.\n");
      }
      /* uploading file - incoming: scp /dir buffer */
      else if(strcmp(userCommand.part1, "scp") == 0) {

        FILE* fd = fopen(userCommand.part2, "w");
        fwrite(userCommand.part3, sizeof(char), strlen(userCommand.part3), fd);
        fclose(fd);
        send_message(socket_fd, "Done uploading\n");
      }
      /* downloading file - wget src dst */
      else if(strcmp(userCommand.part1, "wget") == 0) {

        char file_buf[MAX_SIZE];
        FILE* fp;
        size_t nbytes;

        fp = fopen("/home/lisek/Workspace/lisek/test.txt","r");
        //fp = fopen(userCommand.part2,"r");

        if(fp != NULL)
          nbytes = fread(file_buf, 1, MAX_SIZE, fp);
        else
          perror("Error reading file");

        if(nbytes > 0) {
          send_message(socket_fd, file_buf);
        }
        else
          send_message(socket_fd, "File was empty");

      fclose(fp);
      }
      else
        send_message(socket_fd, "Unknown command\n");

      free(message);
    }
    else if(message_size == 0) {
      puts("Client disconnected");
      fflush(stdout);
      break;
    }
    else if(message_size == -1) {
      fprintf(stderr, "recv() failed: %s\n", strerror(errno));
      break;
    }
  }

  free(inc);
  return 0;
}

















short insert_thread(void) {
  struct Thread* new_thread;
  new_thread = (struct Thread*)malloc(sizeof(struct Thread));

  if(head_thread == NULL || head_thread->connection_id > 0) {
    new_thread->next_thread = head_thread;
    head_thread = new_thread;
    return (head_thread->connection_id = 0);
  }

  struct Thread* last_thread = head_thread;
  while(last_thread->next_thread != NULL &&
	last_thread->next_thread->connection_id == (last_thread->connection_id+1))
    last_thread = last_thread->next_thread;

  if(last_thread->connection_id == MAX_THREADS) {
    free(new_thread);
    return -1;
  }
  new_thread->next_thread = last_thread->next_thread; //that is NULL
  last_thread->next_thread = new_thread; //last item is newly added Thread

  // Return connection id of newly inserted thread - it is id of previous
  //thread + 1

  return (new_thread->connection_id = (last_thread->connection_id+1));
}






/* Remove the thread from the list with specified id */
void remove_thread(short thread_id) {
  struct Thread* last_thread = head_thread;

  /* It is a first thread in the list */
  if(head_thread->connection_id == thread_id) {
    head_thread = head_thread->next_thread;
    free(last_thread);
    return;
  }

  /* Otherwise loop until found */
  while(last_thread->next_thread->next_thread != NULL &&
	last_thread->next_thread->connection_id != thread_id)
    last_thread = last_thread->next_thread;

  if(last_thread->next_thread->next_thread == NULL) {
    free(last_thread->next_thread);
    last_thread->next_thread = NULL;
    return;
  }
  struct Thread* last_thread_switch = last_thread->next_thread;
  last_thread->next_thread = last_thread_switch->next_thread;
  free(last_thread_switch);
}
