# TFTP Server (Single process)
### Description
TFTP stands for **Trivial File Transfer Protocol**. This is an aplication level protocol initially developed in the late 90s to be a simple file transfer protocol as suggested by the name. The basic task of TFTP is similar to any other File Transfer Protocol: transfer files from one device on a network to another. This file may be a _JPEG, PDF, EXE, ZIP, TXT_, or any other file.

Here are some features that make TFTP useful and noteworthy:    
  1. __Lockstep protocol__    - It is fault tolerant and can correct mistakes committed on the network   
  2. __Light weight__         - Hence, it does not require huge client or server computational power   
  3. __Uses UDP (fast)__ - A unique exception in an otherwise TCP dominated protcol environment  

To read more on the details TFTP, visit *http://www.ietf.org/rfc/rfc1350.txt*. This documentation contains the details of the algorithm that is implemnted in all the latest TFTP clients and servers.

How does this project help TFTP?        
In a world of exceptionally complex codes that cannot be understood, there is a need for simplicity.

Keeping this in mind, this TFTP server has the following properties:    
  1. __Concurrent__     - This server can handle multiple clients at the same time without any hastle.       
  2. __Muti process__   - A multi process server that can handle multiple clients while maintaining good speed.    
  3. __Simple API__     - Easily understandable code that uses variables, functions and loops in a justified manner.     
  4. __Fast__          - Functions that currently take hundereds of lines have been optimised to do the same task in very few lines.    
  5. __Intuitive__      - Variable & function names are very intuitive so as to eliminate confusion.    
  6. __Optional Verbose__ - The code has an option whether to print all the details of transfer or not.     
  7. __Client Timeout__ - Server disconnects with client if there is no response for 15 seconds.   

### Concurrency
This is a unique feature in this TFTP server, not because this server is concurrent (as many others are) but because of how it handles concurrency.   

A traditional server would create a new child process or a new thread (in the case for multi-threaded concurrency) for each new client. 

In the below picture, say if a new "Client C" were to connect, then another server child process (or thread) would be created.
[Note: Go see my single process server at *https://github.com/yuvrajmalhi/TFTP-Multi-Process-server* ]

![1](https://user-images.githubusercontent.com/76866159/107152058-e21f3a00-698b-11eb-9cd9-61489dbdca16.png)

But this implementation has a very major flaw: as the number of child processes (or threads) increase, the server becomes **exponentially slow.**  


To handle this problem, a "single child" concurrent server would works very well. It would only occupy the computional load of a single process while also parallely be able to serve many more clients with ease than a multi process server. 
To implement this, I/O multiplexing using non-blocking select() has been used. Thus when a server calls select(), it sees if client A has not sent anything, without wasting its time, the server would see if client B has sent anything. This will continue till atleast one of the clients have sent something, then that will be processed and the server will call select() again.



## Usage
### Initialize Server
Run the following commands to get started: 

```sudo ifconfig``` &emsp;&emsp;&emsp;&emsp;&emsp;&emsp; -  &emsp; *Check ther server IP. Mine is ```10.0.2.12```*        
``` sudo git clone https://github.com/yuvrajmalhi/TFTP-Single-Process-server.git```           
```cd TFTP-Single-Process-server/```           
```cp <file location> ./``` &emsp;&emsp;&nbsp; -  &emsp; *Copy the file to server location. I have chosen 'TT.deb' for demonstration*          
```gcc tftp.c -o server```           
```ls```  &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&nbsp; - &emsp;  *Confirm that ther 'server' executable and file are present*            
```sudo ./server 69```         

[Note: if you are a networking geek, you may change the 69 to any other port number, but keep in mind that all TFTP clients connect to port 69 by default]

On the prompt, the program asks if you want to print everything that goes on in the file transfer or just the basics.
I will recomment choosing no or 'n' as this information will be irrelevant to all other than networking folks.

Here's how:
![Sing_proc_Server_Setup](https://user-images.githubusercontent.com/76866159/134986646-b250a48b-d7ad-4227-84a5-f0869aa5f369.gif)

Voila! Server is all setup.



## Client
### Usage
Run the following commands to get started: 

``` sudo apt-get install tftp``` &emsp;&emsp;&emsp;&emsp;&emsp;&emsp; -  &emsp; *Install tftp client*        
``` sudo tftp```           
``` connect 10.0.2.12``` &emsp;&emsp;&emsp;&emsp;&emsp;&emsp; -  &emsp; *Connect to server's IP*        
``` get TT.deb ``` &emsp;&emsp;&emsp;&emsp;&emsp;&emsp; -  &emsp; *Get the file 'TT.deb' from server*  

Follow here:
![Sing_proc_client_setup](https://user-images.githubusercontent.com/76866159/134986667-06431a4a-6d7d-4378-b28a-5f42d857cbac.gif)


As soon as the transfer ends, a prompt is shown telling you the size and time of file transfer.
Morover, on the **Server Side** the speed is also shown in mbps.

In this case, 500.4 Mb was transferred in 39.5 seconds. Transfer speed = **12.67 Mb/s**.

### Concurrent Transfer
To demonstrate concurrency, **3** clients will receive file "TT.deb" top 3 clients parallely.
![Sing_proc_concurrency](https://user-images.githubusercontent.com/76866159/134986619-ded05aa1-f1e6-4ea7-943a-439a1de740e2.gif)

As the number of clients triple (from 1 to 3), the speed does not change proportionally (12.6 Mb/s to 9 Mb/s).


[Note: Go see my multi process server at *https://github.com/yuvrajmalhi/TFTP-Multi-Process-server* ]

_**Important conclusion, in single-process servers, the speed performance for numerous clients is BETTER than mult-process servers.
Traditionally as the number of clients double, the speed halves, but here the speed does not go down.**_

Do share if you liked my work. Thanks!

:smile:
