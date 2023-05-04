#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

struct Node {
    char symbol = ' ';
    int frequency = -1;
    Node *next = nullptr;
    bool sum = false;
    bool tree = false;
    Node *left = nullptr;
    Node *right = nullptr;
    string encoding = "";
    vector<int> positions;
};

class PriorityQueue {
private:
    Node *front;
    int size;
public:
    PriorityQueue() {
        front = nullptr;
        size = 0;
    }
    Node *getFront() {
        return front;
    }
    int getSize() {
        return size;
    }
    void addNode(Node myNode) {
        Node *temp = new Node();
        temp->symbol = myNode.symbol;
        temp->frequency = myNode.frequency;
        if (front == nullptr) {
            front = temp;
        }
        else {
            Node *cur = front;
            while (cur->next != nullptr) {
                cur = cur->next;
            }
            cur->next = temp;
        }
    }
    void makeFront(Node *newFront) {
        if (front = nullptr) {
            return;
        }
        newFront->next = front;
        front = newFront;
    }
    void popFront() {
        if (front == nullptr) {
            return;
        }
        else if (front->next == nullptr) {
            front = nullptr;
        }
        else {
            front = front->next;
        }
        return;
    }
    void queueList(vector<Node> myList) {
        for (int i = myList.size()-1; i >= 0; i--) {
            addNode(myList[i]);
            size++;
        }
        return;
    }
    void printQueue() {
        if (front == nullptr) {
            return;
        }
        else {
            Node *cur = front;
            while (cur != nullptr) {
                if (cur->sum) {
                    cout << "[" << cur->frequency << "]" << cur->left->symbol << "|" << cur->right->symbol << " ";
                } 
                else {
                    cout << cur->symbol << ":" << cur->frequency << " ";
                }
                cur = cur->next;
            }
            cout << endl;
            return;
        }
    }
};