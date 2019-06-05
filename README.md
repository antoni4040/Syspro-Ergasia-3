Syspro Project 3

AM: 1115201600060
Created by Antonis Karvelas

To compile, simply run make and it will create both the server and client executables.
For the server, run for example:
./server -p 8000

For the client, run for example:
./client -d ./dir1 -p 8001 -w 4 -b 128 -sp 8000 -sip 192.168.1.1
Replace accordingly for server ip and server/client ports of choise.

There's not much to say here, most of the documentation lies in the code comments.
The server simply starts listening to a given port and answers accordingly.
The client greets the server and then creates a threadpool with a given number of threads to deal with the round buffer and also starts listening to a given port for replies from the server and the other clients.
I have two mutex conditions, one for when the round buffer is empty and one for when it's full, so that the threads are not simply busy-waiting for a chance to alter the round buffer and instead wait for the signal to do so.