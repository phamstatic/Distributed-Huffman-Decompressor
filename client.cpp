#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <string>
#include <iostream>
#include "huffmanTree.cpp"
using namespace std;

struct arguments {
    char symbol;
    string frequency;
    vector<int> positions;
    char *output;
    int length;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
}; 

void error(char *msg) {
    perror(msg);
    exit(0);
}

void *threadFunction(void *void_ptr) {
    arguments *arg_ptr = (struct arguments *)void_ptr;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = arg_ptr->sockfd;
    portno = arg_ptr->portno;
    n = arg_ptr->n;
    serv_addr = arg_ptr->serv_addr;
    server = arg_ptr->server;
    char buffer[256];
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {  // Create a socket to communicate with the server program.
        error("ERROR connecting");
    }
    char frequency[arg_ptr->frequency.length() + 1]; 
	strcpy(frequency, arg_ptr->frequency.c_str()); 
    n = write(sockfd, frequency, 255); // Send the binary code to the server program using sockets. 
    //cout << arg_ptr->frequency << ":" << frequency<< endl;
    n = read(sockfd,buffer,255); // Wait for the decoded representation of the binary code (character) from the server program.
    arg_ptr->symbol = *buffer;
    close(sockfd);
    for (int i = 0; i < arg_ptr->length; i++) {
        for (int j = 0; j < arg_ptr->positions.size(); j++) {
            arg_ptr->output[arg_ptr->positions[j]] = arg_ptr->symbol; // Write the received information into a memory location accessible by the main thread.
        }
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    // The client program receives from STDIN (using input redirection) m lines (where m is the number of symbols in the alphabet). Each line of the compressed file has the following format:
    string input = "";
    vector<arguments> lines;
    int symbols; // Number of symbols in the alphabet
    int length = 0; // Length of the output
    while (getline(cin, input)) {
        arguments line;
        string code = "";
        int stop;
        for (int i = 0; i < input.length(); i++) {
            if (input[i] == ' ') {
                stop = i;
                break;
            }
            code += input[i];
        }
        line.frequency = code;
        string position = "";
        for (int i = stop + 1; i < input.length() + 1; i++) {
            if (input[i] == ' ' || i == input.length()) {
                line.positions.push_back(stoi(position));
                if (stoi(position) > length) { // Updates the length variable
                    length = stoi(position) + 1;
                }
                position = "";
                continue;
            }
            position += input[i];
        }
        lines.push_back(line);
    }
    // After reading the information from STDIN, this program creates m child threads (where m is the size of the alphabet). 
    char *output = new char[length];
    const int threads = lines.size(); // Number of threads
    pthread_t *tid = new pthread_t[threads];
    arguments *arg = new arguments[threads];
    for (int i = 0; i < threads; i++) {
        arg[i].frequency = lines[i].frequency; // Receives the information about the symbol to decompress (binary code and list of positions) from the main thread.
        arg[i].positions = lines[i].positions;
        arg[i].output = output;
        arg[i].length = length;
        arg[i].portno = atoi(argv[2]);
        arg[i].sockfd = socket(AF_INET, SOCK_STREAM, 0);
        arg[i].server = gethostbyname(argv[1]);
        if (pthread_create(&tid[i], NULL, threadFunction, &arg[i])) {
            fprintf(stderr, "Error creating thread \n");
            return 1;
        }
    }
    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }

    cout << "Original message: " << output; // Finally, after receiving the binary codes from the child threads, the main thread prints the original message.

    delete [] tid;
    delete [] arg;

    return 0;
}
