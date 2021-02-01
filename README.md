# TFTP Server (Single process)
## Description
TFTP stands for **Trivial File Transfer Protocol**. This was a protocol initially developed in the late 90s to be a simple file transfer protocol as suggested by the name.
Here are some features that make TFTP useful and noteworthy:    
  _1. Lockstep protocol_    - It is fault tolerant and can correct mistakes committed on the network   
  _2. Light weight_         - Hence, it does not require huge client or server computational power   
  _3. Uses UDP, hence fast_ - A unique exception in an otherwise TCP dominated protcol environment  

To read more on the details TFTP, visit *http://www.ietf.org/rfc/rfc1350.txt*. This documentation contains the details of the algorithm that is implemnted in all the latest TFTP clients and servers.

TFTP's importance today can be seen by the simple fact that port 69 is reserved for it by IANA. Only ports 1-1024 are reserved for the most common and useful applications like HTTP, SSH etc.

How does this project help TFTP?
In a world of exceptionally complex codes that can only be used directly and not really understood, there is a need for simplicity in code, implementation, and usage.
Keeping this in mind, this TFTP server has the following properties:    
  _1. Simplicity_ - Easily understandable code that uses variables, functions and loops in a justified manner    
  _2. Speed     _ - Functions that currently take hundereds of lines have been optimised to do the same task in very few lines    
  _3. Intuitive _ - Variable & function names are made to be very intuitive so as to eliminate confusion   
