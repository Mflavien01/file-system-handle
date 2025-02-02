**CLEVER Paul**  
**MATHIEU Flavien**
**CHAOUI KOURAICHI Ahmed**
**BULLIER Gauthier**



# **EURECOM-Drive: a file repository**

# **I. Introduction**

## **1. Context**
The project aims to design a system for remotely storing files. The system consists of two main components:  a server that handles file requests from clients and stores files in a configurable directory, and a client that based on the user inputs sends commands to the server.

## **2. Main objectives**
            

1. File Storage System:    
Develop a system that allows clients to store, retrieve, and manage files on the server.

2. Command Implementation:    
Implement various commands to allow user to interact with the server, such as listing files, removing a file displaying the content of a file, and so on.
                     
3. Error Handling:   
Implement a robust error-handling mechanism, at the client and at the server sides.
           

## **3. Problematic**
            
1. Efficient File Transfer:   
Implement a mechanism for transferring files between the client and server, especially for large files that may require multiple packets.
            
2. Command Consistency:   
Ensure that commands are correctly interpreted on both sides, and vice versa, adhering to the specified packet format.
            
3. Error Prevention:   
Implement measures to prevent errors such as overwriting files, incorrect command syntax, and other critical issues.

## **4. Implementation**
            
1. Server Implementation:
* Accepts command-line arguments specifying quotas, and directory.
* Manages file storage, processes client commands, and sends appropriate responses.

2. Client Implementation:
* Accepts command-line options for analysis, interactive mode, and directory settings.
* Processes user commands, sends requests to the server, and displays responses.

3. Packet Format:
* Ensures adherence to the specified packet format for communication.
* Implements reliable packet transmission between the client and server.

4. Error Handling:
* Implements error codes and messages for various scenarios, both on the client and server sides.
* Ensures proper termination in case of critical errors.

5. Testing:
* Ensures the overall functionality and reliability of the system.


## **6. Group organization**

1. Work on client commands
* CHAOUI KOURAICHI Ahmed and BULLIER Gauthier 

2. Work on server commands
* CLEVER Paul and MATHIEU Flavien

# **II.    Background**

## **1. EURECOM - Academic Institution**
The project was undertaken at EURECOM, a leading graduate school and research center located in Sophia Antipolis, France. EURECOM specializes in information and communication technologies (ICT), providing a collaborative and innovative environment for students and researchers.

## **2. Teacher's Support**
The teacher and his team played a crucial role in providing guidance, clarifications, and support throughout the project's development. Their expertise and insights contributed to the project', ensuring that the implemented solutions met the required standards and best practices.

# **III.    Unique Problem**
While various solutions provide remote file storage and access, the project's unique requirements (including the specified packet format, client-server architecture, and custom commands) make it distinct from existing solutions.

# **IV. Contributions and Solutions**

## **0. Communication specification**  

1. Server Setup:  
* The server opens a listening socket at a specific IP address and port. This socket acts as a gateway for incoming connection requests.
* The server then patiently waits for clients to connect to this address.

2. Client Connection Request:  
* A client initiates a connection by sending a request to the server’s address.

A communictaion.c file is included in both client and server. The send_pkt function initializes a counter of the maximum packet size (2048) and writes to the channel until 2048 bytes have been written. The recv_pkt function initializes a buffer of 2048 bytes and writes the channel content to the buffer until 2048 bytes have been written. 
## **1. Adding a remote file (put)**


1.  Client (BULLIER Gauthier)  
It retrieves the file size from the command parameter and divides the file into packets of maximum size (2048 bytes). It then opens the file in read mode, reads its content into a buffer, and sequentially transmits the data packets to the channel. If the size of the last packet is greater than zero, it also sends this last packet. The function returns 0 on success.

2.  Server (MATHIEU Flavien)  
It extracts the file name and size from the buffer, checks the file's existence, and returns an error packet to the channel if the file already exists, returning 1. If the file doesn't exist, it attempts to open the file in write mode. If unsuccessful, it sends an error packet and returns 1. Next, the function divides the file into packets of maximum size (2048 bytes) and sequentially writes them to the file. This process is repeated until all complete packets are written. If the size of the last packet is greater than zero, it is also written to the file. In case of a write failure, the function sends an error packet and returns 1. Upon successful completion of all steps, a confirmation packet is sent to indicate success, and the function returns 0.
This function work only if the file is empty. If the file is not empty, an error occurs due to the size of the file.

## **2. Removing a remote file (rm)**

1.  Client (CHAOUI KOURAICHI Ahmed )  
It dynamically allocates memory to create a deletion packet (pkt), fills it with the necessary information, sends it to the channel, and then frees the allocated memory. Next, it waits for confirmation from the channel and checks for any reported errors. In case of an error, the function returns the error code. If the operation is successful, the function returns 0.

2.  Server (CLEVER Paul)  
It extracts the file name from the buffer, attempts to remove the corresponding file, and then sends a confirmation packet to the channel based on the result of the deletion. The function then frees the allocated memory for the packet and returns 0 on success or 1 on failure.
## **3. Getting a remote file (get)**

1.  Client (BULLIER Gauthier)  
It extracts the destination file name from the command, sends an initialization packet to the channel, and then waits for the reception of the first packet from the server. If the first packet indicates an error, the function returns the error code. Otherwise, it creates an empty file with the specified name and proceeds to receive data packets from the server. The received data is sequentially written to the file.  This operation is repeated until all complete packets are received. If the size of the last packet is greater than zero, it is also written to the file. In case of an error during data reception, the function returns the error code. Ultimately, the function returns 0 on success.

2.  Server (CLEVER Paul)  
It extracts the file name from the buffer, retrieves the file size, and then opens the file in read mode. If the opening fails, it sends an error packet to the channel and returns 1. Next, it reads the file content into a data buffer, divides the data into packets of maximum size (2048 bytes), and sends them sequentially to the channel. This operation is repeated until all complete packets are sent. If the size of the last packet is greater than zero, it also sends it. Ultimately, the function returns 0.


## **4. Listing remote files (ls)**

1.  Client (BULLIER Gauthier)  
It begins by dynamically allocating a packet. Then, it fills the packet with command parameters, such as the name of the remote directory. The packet is sent to the server. Upon receiving the server's response, the function extracts the data size from the packet, retrieves the list of files, prints it, and concludes its execution.


2.  Server (MATHIEU Flavien)  
It opens the current directory using opendir() and checks if it can access the directory. If opening the directory fails, it sends an error packet to the server, indicating an error code, and returns 1. Otherwise, it iterates through the files in the directory, builds a formatted list with the name and size of each file, and sends this list to the client. It then frees the dynamically allocated memory before concluding its execution.


## **5. Printing n lines of a file (cat)**

1.  Client (CHAOUI KOURAICHI Ahmed)  
It dynamically allocates memory to create a packet, initializes it with the necessary information, sends it to the channel, and then frees the allocated memory. Next, it sequentially receives and displays the content of the file from the received packets on the channel. This operation is repeated until all packets are received. If an error is detected in the received packets, the function returns the corresponding error code. Ultimately, the function returns 0.


2.  Server (MATHIEU Flavien)  
It extracts the file name and additional information from the buffer, opens the file in read mode, and sequentially sends data packets to the channel. It reads the file until a certain number of newline characters (\n) are reached, determined by the additional information. The function then divides the data into packets of maximum size (2048 bytes) and sends them sequentially to the channel. This operation is repeated until all complete packets are sent. If the size of the last packet is greater than zero, it is also sent. In case of an error during file opening, the function sends an error packet and returns 1. Ultimately, the function returns 0. The fucntion is not working because of an issue whhen reading the number of lines in the received buffer which is not the one send by the client. 


## **6. Renaming a file (mv)**

1.  Client (CHAOUI KOURAICHI Ahmed)  
It dynamically allocates memory to create packet, initializes it with the necessary information, sends it to the channel, and then frees the allocated memory. Next, it waits for confirmation from the channel and checks for any reported errors. In case of an error, the function returns the corresponding error code. Ultimately, the function returns 0.

2.  Server (CLEVER Paul)  
It extracts the names of the source and destination files from the buffer, checks if the destination file exists, and sends an error packet if it does. Next, it opens the source file in read mode, gets the file size, reads its content into a buffer, and then closes the source file. It then opens the destination file in append mode. If the opening fails, it sends an error packet. Otherwise, it remove the source file and it sends a confirmation packet to the channel and frees the allocated memory. Ultimately, the function returns 0.

## **7. Quit the client (quit)** 

1. Client (BULLIER Gauthier)    
The quit function is responsible for the clean termination of the session. It dynamically allocates an exit packet with a specific code (7) to signal its intention to quit. This packet is then sent to the server through the communication channel. After sending the packet, the function uses the exit(0) function to ensure an orderly exit and release allocated resources before exiting the program.

## **8. Client reinitialization (restart)** 

1. Only client (CHAOUI KOURAICHI Ahmed)    
The restart function is checked each time the user enters a command. As quit the client sends a packet telling the server to stop listening. It then executes itself with execvp, which returns its input to stdout after stopping the current processor.

## **9. Describes the available commands (help)** 

1. Client (BULLIER Gauthier)   
It uses the printf function to display sections related to server and client options, as well as available commands.

# **V. Conclusion**

## **1. Reached objectives**

| Objective|
| ------ |
|File Storage System        |
|Communication between server and client|
|Command Implementation (put, rm, get, ls, cat, mv, quit, restart, help)|
|Format        |
|Security Measures        |
| -analyse -directory option on the client side|
|-directory option on the server side |


## **2. Non reached objectives**

* We didn't manage to implement the -interactive option on the client side due to a lack of time that made us prioritize other implementations.
* We didn't manage to implement to implement the quotas option an the server side.
