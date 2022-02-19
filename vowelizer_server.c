/* TCP-based server example of socket programming.    */
/* The server receives an input word (e.g., "dog")    */
/* and returns the length of the word (e.g., "3").    */
/*                                                    */
/* Usage: cc -o wordlen-TCPserver wordlen-TCPserver.c */
/*        ./wordlen-TCPserver                         */
/*                                                    */
/* Written by Carey Williamson       January 13, 2012 */

/* Include files for C socket programming and stuff */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>

/* Global manifest constants */
#define MAX_MESSAGE_LENGTH 100
#define MY_TCP_PORT 12346
#define MY_UDP_PORT 12347

/* Optional verbose debugging output */
#define DEBUG 1

/* Global variable */
int childsockfd;        // tcp socket we accept connection from client on 

void split(char *str);

void merge();

void advanced_split();

int check_vowel(char);

#define MAX_STRING_LENGTH 100

struct message {
    int command;
    char string[MAX_STRING_LENGTH];
};


char v[MAX_STRING_LENGTH];         // vowels
char nv[MAX_STRING_LENGTH];        // non-vowels
char mstr[MAX_STRING_LENGTH];      // new merged string

/* This is a signal handler to do graceful exit if needed */
void catcher(int sig) {
    close(childsockfd);
    exit(0);
}

/* Main program for server */
int main() {
    struct sockaddr_in server;
    static struct sigaction act;
    char messagein[MAX_MESSAGE_LENGTH];
    char messageout[MAX_MESSAGE_LENGTH];
    int parentsockfd;           // server socket 
    int i, j;
    int pid;
    char c;
    int sockfd_udp;

    struct sockaddr_in si_server, si_client;
    struct sockaddr *udp_server, *udp_client;

    /* Set up a signal handler to catch some weird termination conditions. */
    act.sa_handler = catcher;
    sigfillset(&(act.sa_mask));
    sigaction(SIGPIPE, &act, NULL);

    /* Initialize server sockaddr structure */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(MY_TCP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /* set up the transport-level end point to use TCP */
    if ((parentsockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "wordlen-TCPserver: socket() call failed!\n");
        exit(1);
    }

    /* bind a specific address and port to the end point */
    if (bind(parentsockfd, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "wordlen-TCPserver: bind() call failed!\n");
        exit(1);
    }

    /* start listening for incoming connections from clients */
    if (listen(parentsockfd, 5) == -1) {
        fprintf(stderr, "wordlen-TCPserver: listen() call failed!\n");
        exit(1);
    }


    /* set up the transport-level end point to use TCP */
    if ((sockfd_udp = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "wordlen-TCPserver: socket() call failed!\n");
        exit(1);
    }

    memset((char *) &si_server, 0, sizeof(si_server));
    si_server.sin_family = AF_INET;
    si_server.sin_port = htons(MY_UDP_PORT);
    si_server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd_udp, (const struct sockaddr *) &si_server, sizeof(si_server)) == -1)
    {
        printf("Could not bind to port %d!\n", MY_UDP_PORT);
        return 1;
    }

    /* initialize message strings just to be safe (null-terminated) */
    bzero(messagein, MAX_MESSAGE_LENGTH);
    bzero(messageout, MAX_MESSAGE_LENGTH);

    fprintf(stderr, "Welcome! I am the TCP version of the word length server!!\n");
    fprintf(stderr, "server listening on TCP port %d...\n\n", MY_TCP_PORT);

    /* Main loop: server loops forever listening for requests */
    for (;;) {
        /* accept a connection */
        if ((childsockfd = accept(parentsockfd, NULL, NULL)) == -1) {
            fprintf(stderr, "wordlen-TCPserver: accept() call failed!\n");
            exit(1);
        }

        /* try to create a child process to deal with this new client */
        pid = fork();

        /* use process id (pid) returned by fork to decide what to do next */
        if (pid < 0) {
            fprintf(stderr, "wordlen-TCPserver: fork() call failed!\n");
            exit(1);
        } else if (pid == 0) {
            /* the child process is the one doing the "then" part */

            /* don't need the parent listener socket that was inherited */
            close(parentsockfd);

            struct message incoming_message = {0,""};

            /* obtain the message from this client */
            // since we're receiving a messange on the tcp socket then we will devowel
            while (recv(childsockfd, &incoming_message, sizeof(struct message), 0) > 0)
            {
                /* print out the received message */
                printf("Child process received string : %s\n", incoming_message.string);
                if (incoming_message.command == 1)
                {
                    split(incoming_message.string);
                    send(childsockfd, v, strlen(v), 0);

                    struct sockaddr_in to_client;
                    to_client.sin_family = AF_INET;
                    to_client.sin_port = htons(12349);
                    to_client.sin_addr.s_addr = htonl(INADDR_ANY);
                    if (inet_pton(AF_INET, "127.0.0.1", &to_client.sin_addr.s_addr)==0)
                    {
                        printf("inet_pton() failed\n");
                        return 1;
                    }

                    unsigned int tolen = sizeof(struct sockaddr_in);
                    ssize_t bytes_sent = sendto(sockfd_udp, nv, strlen(nv), 0, (const struct sockaddr *) &to_client, tolen);
                    if (bytes_sent < 0)
                    {
                        fprintf(stderr, "wordlengthclient: sendto() call failed!\n");
                        exit(1);
                    }
                }
                else if (incoming_message.command == 2)
                {
                    merge();
                }
                else if (incoming_message.command == 3)
                {
                    //advanced_split();
                }
                else
                {
                    printf("Invalid command\n");
                }



                //printf("That word has %d characters!\n", strlen(messagein));

                /* create the outgoing message (as an ASCII string) */
                // sprintf(messageout, "%d\n", strlen(messagein));

                // #ifdef DEBUG
                // printf("Child about to send message: %s\n", messageout);
                // #endif

                /* send the result message back to the client */
                // send non vowels back to client using TCP
                // send(childsockfd, messageout, strlen(messageout), 0);

                /* clear out message strings again to be safe */
                bzero(messagein, MAX_MESSAGE_LENGTH);
                bzero(messageout, MAX_MESSAGE_LENGTH);
            }

            /* when client is no longer sending information to us, */
            /* the socket can be closed and the child process terminated */
            close(childsockfd);
            exit(0);
        } /* end of then part for child */
        else {
            /* the parent process is the one doing the "else" part */
            fprintf(stderr, "Created child process %d to handle that client\n", pid);
            fprintf(stderr, "Parent going back to job of listening...\n\n");

            /* parent doesn't need the childsockfd */
            close(childsockfd);
        }
    }
}

int check_vowel(char c) {
    if (c == 'a' || c == 'A' || c == 'e' || c == 'E' || c == 'i' || c == 'I' || c == 'o' || c == 'O' || c == 'u' ||
        c == 'U')
        return 1;

    return 0;
}


void split(char *str) {
    char word[100]; // user input with max characters of 100
    int i = 0;      // number of characters in user input

    // printf("Enter your message to devowel: ");
    // fgets(str, sizeof str, stdin); 
    while (str[i] != '\0') {
        if (check_vowel(str[i]))    // check for vowels in the input string
        {
            v[i] = str[i];          // input string will contain vowels only
            nv[i] = ' ';            // non vowels are denoted by a space
        } else {
            nv[i] = str[i];         // input string will contain non vowels only
            v[i] = ' ';             // vowels are denoted by a space
        }
        i++;
    }
    nv[i] = '\0';
    v[i] = '\0';

    printf("vowels: %s \n", v);
    printf("non-vowels: %s \n", nv);
}


void merge() {

    int i = 0;      // number of characters in the input string

    while (v[i] != '\0')          // loop until the end of the length of the input string containing non-vowels
    {
        if (v[i] != ' ')         // check if index of the string in v is not a space
        {
            mstr[i] = v[i];      // store vowel in the new merged string
        } else {
            mstr[i] = nv[i];     // store non vowels in the new merged string
        }
        i++;
    }
    mstr[i] = '\0';

    printf("merged string: %s \n", mstr);
}
