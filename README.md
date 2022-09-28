# Message_Serialization_in_TCP
Message serialization between clients in TCP/IP

Firstly we should implement your code in the Terminal of Linux
gcc -pthread serverThread.c -o server

Then we need to compile our server with the port number.
./server 9999

And we can call the clients
gcc client.c -o client
./client 127.0.0.1 9999

"show" command shows online clients
"connect" command establishes connection
"disconnect" command disconnects
