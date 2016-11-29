#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#include <netdb.h>
#include <netinet/in.h>

#define MAX_THREADS 512

char cwd[1024];

struct Thread {
   int connection_id;
   struct Thread *next_thread;
};

void* handle_connection(void*);
short insert_thread();
void remove_thread(short);
static void sigterm_handler(int);