# TFTP Server (Single process)
## Description
TFTP stands for **Trivial File Transfer Protocol**. This is an aplication level protocol initially developed in the late 90s to be a simple file transfer protocol as suggested by the name. The basic task of TFTP is similar to any other File Transfer Protocol: transfer files from one device on a network to another. This file may be a _JPEG, PDF, EXE, ZIP, TXT_, or any other file; the work remains the same. 

Here are some features that make TFTP useful and noteworthy:    
  1. __Lockstep protocol__    - It is fault tolerant and can correct mistakes committed on the network   
  2. __Light weight__         - Hence, it does not require huge client or server computational power   
  3. __Uses UDP, hence fast__ - A unique exception in an otherwise TCP dominated protcol environment  

To read more on the details TFTP, visit *http://www.ietf.org/rfc/rfc1350.txt*. This documentation contains the details of the algorithm that is implemnted in all the latest TFTP clients and servers.

TFTP's importance today can be seen by the simple fact that port 69 is reserved for it by IANA. Only ports 1-1024 are reserved for the most common and useful applications like HTTP, SSH etc.

How does this project help TFTP?
In a world of exceptionally complex codes that can only be used directly and not really understood, there is a need for simplicity in code, implementation, and usage.

Keeping this in mind, this TFTP server has the following properties:    
  1. __Concurrent__     - This server can handle multiple clients at the same time without any hastle    
  2. __Single process__ - A single process that handles multiple clients   
  3. __Simplicity__     - Easily understandable code that uses variables, functions and loops in a justified manner     
  4. __Speed__          - Functions that currently take hundereds of lines have been optimised to do the same task in very few lines    
  5. __Intuitive__      - Variable & function names are made to be very intuitive so as to eliminate confusion   

### Concurrency
This is a unique feature in this TFTP server, not because this server is concurrent (as many others are) but because of how it handles concurrency.   

A traditional server would create a new child process or a new thread (in the case for multi-threaded concurrency) for each new client. 

![image](https://user-images.githubusercontent.com/76866159/106448570-f43f3b00-64a8-11eb-9c48-04cb430ed682.png)   

In the above picture, say if a new "Client C" were to connect, then another server child process (or thread) would be created.
But this implementation has a very major flaw: as the number of child processes (or threads) increase, the server becomes **exponentially slow.**  

![image](https://user-images.githubusercontent.com/76866159/106449420-f6ee6000-64a9-11eb-9cb8-44e45a52c106.png)

To handle this problem, a "single child" concurrent server would work extremeley well. It would only occupy the computional load of a single process while also parallely be able to serve many more clients with ease than a multi process server. 
To implement this, I/O multiplexing using select() has been used. This makes the system calls non-blocking. Thus when a server calls select(), it sees if client A has not sent anything, without wasting its time, the server would see if client B has sent anything. This will continue till atleast one of the clients have sent something, then that will be processed and the server will call select() again.


## Usage

### Server Side
On the machine where you want the server, create a new empty folder and download "tftp.c".
To do this you may either copy-paste the code or simple run the command: 

