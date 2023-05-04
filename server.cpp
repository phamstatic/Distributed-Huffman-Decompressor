#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <pthread.h>
#include <iostream>
#include "huffmanTree.cpp"
using namespace std;

Node *HuffmanCode(PriorityQueue &myQueue) {
    bool complete = false;
    while (myQueue.getFront()->next->next != nullptr) {
        Node *temp = new Node();
        temp->sum = true;
        temp->tree = true;
        temp->left = myQueue.getFront();
        temp->right = myQueue.getFront()->next;
        temp->frequency = temp->left->frequency + temp->right->frequency;
        myQueue.popFront();
        myQueue.popFront();
        Node *cur = myQueue.getFront();
        Node *prev = new Node();
        if (myQueue.getFront()->frequency >= temp->frequency) {
            myQueue.makeFront(temp);
            temp->next = cur;
        }
        else {
            while (cur->frequency < temp->frequency) {
                prev = cur;
                cur = cur->next;
                if (cur == nullptr) {
                    break;
                }
            }
            prev->next = temp;
            temp->next = cur;
        }
    }
    Node *root = new Node();
    root->sum = true;
    root->tree = true;
    root->left = myQueue.getFront();
    root->right = myQueue.getFront()->next;
    root->frequency = root->left->frequency + root->right->frequency;
    myQueue.popFront();
    myQueue.popFront();
    return root;
}

void EncodeEdges(Node *node) {
    if (node == nullptr) {
        return;
    }
    if (node->left != nullptr) {
        node->left->encoding += node->encoding;
        node->left->encoding += "0";
    }
    if (node->right != nullptr) {
        node->right->encoding += node->encoding;
        node->right->encoding += "1";
    }
    EncodeEdges(node->left);
    EncodeEdges(node->right);
}

void TreeTraversal(Node *node, vector<Node> &myCodes) {
    if (node == nullptr) {
        return;
    }
    TreeTraversal(node->left, myCodes);
    if (!node->sum) {
        cout << "Symbol: " << node->symbol << ", Frequency: " << node->frequency << ", Code: " << node->encoding << endl;
        Node temp;
        temp.encoding = node->encoding;
        temp.symbol = node->symbol;
        temp.frequency = node->frequency;
        myCodes.push_back(temp);
    }
    TreeTraversal(node->right, myCodes);
}

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port or file provided\n");
        exit(1);
    }
    // Read the alphabet information from STDIN.
    vector<Node> myList;
    vector<Node> myCodes;
    string input = "";
    while (getline(cin, input)) {
        string temp = "";
        Node myNode;
        myNode.symbol = input[0];
        temp += input[2];
        myNode.frequency = stoi(temp);
        myList.push_back(myNode);
    }
    // Arrange the symbols based on their frequencies. If two or more symbols have the same frequency, they will be sorted based on their ASCII value.
    for (int i = 0; i < myList.size(); i++) {
        for (int j = 0; j < myList.size() - 1; j++) {
            if (myList[j].frequency < myList[j+1].frequency) {
                Node temp = myList[j+1];
                myList[j+1] = myList[j];
                myList[j] = temp;
            }
            else if (myList[j].frequency == myList[j+1].frequency) {
                if (int(myList[j].symbol) > int(myList[j+1].symbol)) {
                    continue;
                }
                else if (int(myList[j].symbol) < int(myList[j+1].symbol)) {
                    Node temp = myList[j+1];
                    myList[j+1] = myList[j];
                    myList[j] = temp;                
                }
            }
        }
    }
    // Execute the Huffman algorithm to generate the tree. To guarantee that your solution generates the expected output from the test cases, every time you generate an internal node of the tree, you must insert this node into the queue of nodes as the lowest node based on its frequency. Additionally, you must label the edge to the left child as 0 and the edge to the right child as 1.  
    PriorityQueue myQueue = PriorityQueue();
    myQueue.queueList(myList);
    Node *treeRoot = HuffmanCode(myQueue);
    EncodeEdges(treeRoot);
    TreeTraversal(treeRoot, myCodes);
    // Receive multiple requests from the client program using sockets. Therefore, the server program creates a child process per request to handle these requests simultaneously. For this reason, the parent process must handle zombie processes by implementing the fireman() function call (unless you can determine the number of requests the server program receives from the client program).
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }
    for (int i = 0; i < myList.size(); i++) {
        listen(sockfd, 50);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen); 
    //  Each child process executes the following tasks:
        if (fork() == 0) {
            char value[256];
            n = read(newsockfd, value, 255); // First, receive the binary code from the client program.
            for (int i = 0; i < myCodes.size(); i++) { // Next, use the generated Huffman tree to decode the binary code.
                if (string(myCodes[i].encoding) == string(value)) {
                    n = write(newsockfd, &myCodes[i].symbol, 255); // Finally, return the character to the client program using sockets.
                    break;
                }
            }
            close(newsockfd);
            _exit(0);
        }
        wait(0);
    }
    close(sockfd);
    return 0;
}
