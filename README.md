# Distributed Huffman Decompressor

This program creates a client and server to decompress Huffman codes. This was originally an Operating Systems (COSC 3360 at the University of Houston with Professor Carlos Rincon) assignment.

The client program uses **POSIX threads and UNIX stream sockets**.

The server program uses multiple processes using **fork() and UNIX stream sockets**.

### Example Server Input File
```
A 3
C 3
B 1
D 2
```
### Example Client Input File
```
11 1 3 5
0 0 2 4
101 6 8
100 7
```
### Expected Output
```
Symbol: C, Frequency: 3, Code: 0
Symbol: B, Frequency: 1, Code: 100
Symbol: D, Frequency: 2, Code: 101
Symbol: A, Frequency: 3, Code: 11
Original message: CACACADBD
```
## Usage
To use these programs, you will need to use a **GNU C++ compiler**.

### Server
The user will execute this program using the following syntax:

```console
# Compile
g++ server.cpp -o server
# Run
./server port_no < inputfile.txt
```

The server program receives from STDIN the alphabet's information (using input redirection). The input file has multiple lines, where each line contains information (character and frequency) about a symbol from the alphabet. The input file format is as follows: A char representing the symbol, an integer representing the frequency of the symbol.

The Huffman tree is generated using the following steps:
- Reads the alphabet information from STDIN.
- Arranges the symbols based on their frequencies. If two or more symbols have the same frequency, they will be sorted based on their ASCII value.

The server receives multiple requests from the client program using sockets. It creates a child process per request. Each child process executes the following tasks:
1. First, receive the binary code from the client program.
2. Next, use the generated Huffman tree to decode the binary code.
3. Finally, return the character to the client program using sockets.

### Client
The user will execute this program using the following syntax:

```console
# Compile
g++ client.cpp -o client
# Run
./client hostname port_no < compressedfile.txt
```
The client program receives from STDIN (using input redirection) m lines (where m is the number of symbols in the alphabet). Each line of the compressed file has the following format: A string representing the binary code of the symbol, a list of n integers (where n is the frequency of the symbol) representing the positions where the symbol appears in the message.

After reading the information from STDIN, this program creates m child threads (where m is the size of the alphabet). Each child thread executes the following tasks:
1. Receives the information about the symbol to decompress (binary code and list of positions) from the main thread.
2. Create a socket to communicate with the server program.
3. Send the binary code to the server program using sockets. 
4. Wait for the decoded representation of the binary code (character) from the server program.
5. Write the received information into a memory location accessible by the main thread.
