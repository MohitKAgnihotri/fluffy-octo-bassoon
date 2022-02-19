/* TCP-based client example of socket programming.    */
/* This client interacts with the word length server, */
/* which needs to be running first.                   */
/*                                                    */
/* Usage: cc -o wordlen-TCPclient wordlen-TCPclient.c */
/*        ./wordlen-TCPclient                         */
/*                                                    */
/* Written by Carey Williamson       January 13, 2012 */

/* Include files for C socket programming and stuff */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Some generic error handling stuff */
extern int errno;

void perror(const char *s);

// functions we will make use of 
int check_vowel(char);

void split(char *str);

void merge();

void advanced_split();

#define MAX_STRING_LENGTH 100

char v[MAX_STRING_LENGTH];         // vowels
char nv[MAX_STRING_LENGTH];        // non-vowels
char mstr[MAX_STRING_LENGTH];      // new merged string


struct message
{
    int command;
    char string[MAX_STRING_LENGTH];
};


/* Manifest constants used by client program */
#define MAX_HOSTNAME_LENGTH 64
#define MAX_WORD_LENGTH 100
//#define BYNAME 0
#define MY_TCP_PORT 12346   /* must match the server's port! */
#define SERVER_UDP_PORT 12347
#define MY_UDP_PORT 12349   /* must match the server's port! */
#define SERVER_IP "127.0.0.1"

/* Menu selections */
#define ALLDONE 0
#define ENTER 1

/* Prompt the user to enter a word */
void printmenu() {
    printf("\n");
    printf("Please choose from the following selections:\n");
    printf("  1 - Devowel a message\n");
    printf("  2 - Envowel a message\n");
    printf("  0 - Exit program\n");

    printf("Your desired menu selection? ");
}

/* Main program of client */
int main() {
    int sockfd_tcp, sockfd_udp;
    char c;
    struct sockaddr_in tcp_server, udp_server;
    struct hostent *hp;
    char hostname[MAX_HOSTNAME_LENGTH];
    int choice, len, bytes;

    /* Initialization of tcp_server sockaddr data structure */
    memset(&tcp_server, 0, sizeof(tcp_server));
    tcp_server.sin_family = AF_INET;
    tcp_server.sin_port = htons(MY_TCP_PORT);
    tcp_server.sin_addr.s_addr = htonl(INADDR_ANY);

#ifdef BYNAME
    /* use a resolver to get the IP address for a domain name */
    /* I did my testing using csx3 (136.159.5.27)    Carey */
    strcpy(hostname, "csx3.cpsc.ucalgary.ca");
    hp = gethostbyname(hostname);
    if (hp == NULL) {
        fprintf(stderr, "%s: unknown host\n", hostname);
        exit(1);
    }
    /* copy the IP address into the sockaddr structure */
    bcopy(hp->h_addr, &tcp_server.sin_addr, hp->h_length);
#else
    /* hard code the IP address so you don't need hostname resolver */
   // tcp_server.sin_addr.s_addr = inet_addr("136.159.5.27");
#endif

    /* create the client socket for its transport-level end point */
    if ((sockfd_tcp = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "wordlengthclient: socket() call failed!\n");
        exit(1);
    }

    /* connect the socket to the tcp_server's address using TCP */
    if (connect(sockfd_tcp, (struct sockaddr *) &tcp_server, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "wordlengthclient: connect() call failed!\n");
        perror(NULL);
        exit(1);
    }

    /* create the client socket for its transport-level end point */
    if ((sockfd_udp = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "wordlengthclient: socket() call failed!\n");
        exit(1);
    }

    udp_server.sin_family = AF_INET;
    udp_server.sin_port = htons(MY_UDP_PORT);
    udp_server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd_udp, (const struct sockaddr *) &udp_server, sizeof(udp_server)) == -1)
    {
        printf("Could not bind to port %d!\n", MY_UDP_PORT);
        return 1;
    }

    /* Print welcome banner */
    printf("Welcome! I am the TCP version of the word length client!!\n");
    printmenu();
    scanf("%d", &choice);

    /* main loop: read a word, send to tcp_server, and print answer received */
    while (choice != 0)
    {
        if (choice == 1)   // if we're gonna devowel
        {
            /* Initialize a empty string */
            struct message out_going_msg = {1, ""};

            /* get rid of newline after the (integer) menu choice given */
            c = getchar();

            /* prompt user for the input */
            printf("Enter your message to devowel: ");
            len = 0;
            while ((c = getchar()) != '\n') {
                out_going_msg.string[len] = c;
                len++;
            }

            /* make sure the message is null-terminated in C */
            out_going_msg.string[len] = '\0';

            /* send it to the tcp_server via the socket */
            send(sockfd_tcp, &out_going_msg, sizeof(struct message), 0);

            struct message in_coming_msg_vowels = {0, ""};

            /* Get the vowels from the tcp_server */
            if ((bytes = recv(sockfd_tcp, in_coming_msg_vowels.string, len, 0)) > 0) {
                /* make sure the message is null-terminated in C */
                in_coming_msg_vowels.string[bytes] = '\0';
                printf("Answer received from tcp_server: ");
                printf("`%s'\n", in_coming_msg_vowels.string);
            } else {
                /* an error condition if the tcp_server dies unexpectedly */
                printf("Sorry, dude. Server failed!\n");
                close(sockfd_tcp);
                exit(1);
            }


            struct message in_coming_msg_consonants = {0, ""};

            struct sockaddr_in from_addr;
            unsigned int from_len = sizeof(struct sockaddr_in);

            /* Get the vowels from the tcp_server */
            if ((bytes = recvfrom(sockfd_udp, in_coming_msg_consonants.string, len, 0, (struct sockaddr *)&from_addr, &from_len )) > 0) {
                /* make sure the message is null-terminated in C */
                in_coming_msg_consonants.string[bytes] = '\0';
                printf("Answer received from tcp_server: ");
                printf("`%s'\n", in_coming_msg_consonants.string);
            } else {
                /* an error condition if the tcp_server dies unexpectedly */
                printf("Sorry, dude. Server failed!\n");
                close(sockfd_tcp);
                exit(1);
            }

        } else if (choice == 2)        // envoweling
        {

            char temp_string[MAX_STRING_LENGTH >> 1] = {0};

            /* Initialize a empty string */
            struct message out_going_msg = {2, ""};

            /* get rid of newline after the (integer) menu choice given */
            c = getchar();

            /* prompt user for the input */
            printf("Enter non-vowel part of message to envowel: ");
            len = 0;
            while ((c = getchar()) != '\n') {
                temp_string[len] = c;
                len++;
            }

            memcpy(out_going_msg.string, temp_string, MAX_STRING_LENGTH >> 1);

            /* prompt user for the input */
            printf("Enter vowel part of message to envowel: ");
            len = 0;
            while ((c = getchar()) != '\n') {
                temp_string[len] = c;
                len++;
            }
            memcpy(&out_going_msg.string[MAX_STRING_LENGTH >> 1], temp_string, MAX_STRING_LENGTH >> 1);

            /* make sure the message is null-terminated in C */
            out_going_msg.string[len] = '\0';

            /* send it to the tcp_server via the socket */
            send(sockfd_tcp, &out_going_msg, sizeof(struct message), 0);

            struct message in_coming_msg_vowels = {0, ""};

            /* Get the vowels from the tcp_server */
            if ((bytes = recv(sockfd_tcp, in_coming_msg_vowels.string, len, 0)) > 0) {
                /* make sure the message is null-terminated in C */
                in_coming_msg_vowels.string[bytes] = '\0';
                printf("Answer received from tcp_server: ");
                printf("`%s'\n", in_coming_msg_vowels.string);
            } else {
                /* an error condition if the tcp_server dies unexpectedly */
                printf("Sorry, dude. Server failed!\n");
                close(sockfd_tcp);
                exit(1);
            }


        }

        printf("Invalid menu selection. Please try again.\n");

        printmenu();
        scanf("%d", &choice);
    }

    /* Program all done, so clean up and exit the client */
    close(sockfd_tcp);
    exit(0);
}

// int check_vowel(char c)
// {
//   if (c == 'a' || c == 'A' || c == 'e' || c == 'E' || c == 'i' || c == 'I' || c =='o' || c =='O' || c == 'u' || c == 'U')
//     return 1;

//   return 0;
// }

// void split(char *str)
// {
//     char str[10000]; // user input with max characters of 100
//     int i= 0;      // number of characters in user input

//     // printf("Enter your message to devowel: ");
//     // fgets(str, sizeof str, stdin); 
//     while(str[i]!='\0')
//     {
//        if (check_vowel(str[i]))    // check for vowels in the input string
//        {
//            v[i] = str[i];          // input string will contain vowels only
//            nv[i] = ' ';            // non vowels are denoted by a space 
//        }
//        else                     
//        {
//            nv[i] = str[i];         // input string will contain non vowels only
//            v[i] = ' ';             // vowels are denoted by a space 
//        }
//        i++;
//     }
//     nv[i] = '\0';
//     v[i] = '\0';

//     printf("vowel: %s \n", v);
//     printf("non-vowel: %s \n", nv);
// }


// void merge()
// {

//     int i= 0;      // number of characters in the input string 

//     while(v[i]!='\0')          // loop until the end of the length of the input string containing non-vowels 
//     {
//        if (v[i] != ' ')         // check if index of the string in v is not a space
//        {
//            mstr[i] = v[i];      // store vowel in the new merged string 
//        }
//        else
//        {
//            mstr[i] = nv[i];     // store non vowels in the new merged string
//        }
//        i++;
//     }
//     mstr[i] = '\0';

//     printf("merged string: %s \n", mstr);
// }

// void advanced_split()
// {
//     char str[100]; // user input with max characters of 100
//     int i= 0;      // number of characters in user input
//     int v_count = 0; 
//     int nv_count = 0;
//     int space_count;
//     char text_num[20];

//     printf("Enter your message to devowel: ");
//     fgets(str, sizeof str, stdin); 
//     while(str[i]!='\0')
//     {
//        if (check_vowel(str[i]))    // check for vowels in the input string
//        {
//            sprintf(text_num, "%d", space_count);    // convert space count to string so we can concat it to string v
//            v[v_count] = '\0';                       // we need string v to be a complete string so we can apply concat operation on it 
//            strcat(v,text_num);                      // concat space number that we need to skip to v string 
//            int num_len = strlen(text_num);          // length of the number text
//            v_count = v_count + num_len;             
//            space_count = 0;                         // reset space count for next iteration 

//            v[v_count] = str[i];                     // adding the vowel in the end of string v 
//            v_count++;
//        }
//        else                     
//        {
//            nv[nv_count] = str[i];                   // add the non-vowels to string nv 
//            nv_count++;                              
//            space_count++;
//            //v[i] = ' ';             // vowels are denoted by a space 
//        }
//        i++;
//     }
//     nv[nv_count] = '\0';
//     v[v_count] = '\0';

//     printf("vowel: %s \n", v);
//     printf("non-vowel: %s \n", nv);
// }



