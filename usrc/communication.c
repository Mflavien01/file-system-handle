/* to print messages associated to detected errors */
#include <errno.h>
/* for fprintf, stderr,...*/
#include <stdio.h>
/* for read/write,...*/
#include <unistd.h>
/* for memcpy,...*/
#include <string.h>

/* a function to receive a packet (in pkt) on a socket channel 
 * it assumes that the received packet respects the format.
 * (in a real program, this should be checked... )
 * Returns 1 in case of success, 0 in case of failure
 */
int recv_pkt(char *pkt, int channel) {
    // printf("Attente du premier octet...\n");
    int amount_to_receive = 2048;
    int amount_received = read(channel, pkt, 1);
    

    // amount_received = read(channel, buf, 1);
    if (amount_received == -1) { //error case
        fprintf(stderr, "Connexion fermée\n");
        perror("cannot write"); 
        return 0;
    }
    
    if (amount_received == 0) { // connection closed
        fprintf(stderr, "connection closed\n"); 
        return 0;
    }
    else { // set amount_to_receive
        if (pkt[4] == 0) amount_to_receive = 70;
        else if (pkt[4] == 1 || pkt[4] == 5 || pkt[4] == 6) amount_to_receive = 70 + pkt[3];
        else amount_to_receive = 70;
    }
    // printf("Premier octet (E): %02X\n", (unsigned char)pkt[0]);
    amount_to_receive = 69;
    char *buf = pkt + 1;
    while(amount_to_receive > 0) { // data remains to send
        amount_received = read(channel, buf, amount_to_receive);
        if (amount_received == -1) { //error case
            // print error-associated message on stderr. cf errno.h
            perror("cannot write"); 
            return 0;
        }
        if (amount_received == 0) { // connection closed
            // print relevant error message on stderr
            fprintf(stderr, "connection closed\n"); 
            return 0;
        }
        // amount_received data has been read
        // update amount of data to receive
        amount_to_receive = amount_to_receive - amount_received;
        // points to relevant data location
        buf = buf + amount_received;
    }
    printf("Paquet reçu avec succès\n");
    return 1; // if this line is reached, no error occurred.
}



/* A function to send a packet pkt in a socket channel. 
 * The "packet" parameter must respect the specified format (see before).
 * Returns 1 for success and 0 for failure
 */
int send_pkt(char *pkt, int channel) {
    int amount_to_send =70; // value when the field nb contains 0, 
    int amount_sent;
    char *buf = pkt; // pointer to the data to send, begins at pkt

    // compute the amount of data to be sent by looking at the nb field
    if(pkt[4] == 1) amount_to_send = 70;   //cmd contains 1
    else if (pkt[4] == 2) amount_to_send = 70 + pkt[3];   //cmd contain 2
    else if (pkt[4] == 3) amount_to_send = 70;             //cmd contain 3 
    else if (pkt[4] == 4) amount_to_send = 70;             //cmd contain 4 
    else if (pkt[4] == 5) amount_to_send = 70;             //cmd contain 5
    else if (pkt[4] == 6) amount_to_send =  70;            //cmd contain 6

    // send the packet which may require several writes
    while(amount_to_send > 0) { // data remains to send
        amount_sent = write(channel, buf, amount_to_send);
        if (amount_sent == -1) { //error case
            // print error message for closed connection (cf errno.h)
            if(errno == EPIPE)
                fprintf(stderr, "connection closed\n"); 
            else
            // print error-associated message to stderr cf (errno.h)
                perror("cannot write"); 
            return 0;
        }
        if (amount_sent == 0) { // problem: loop risk
            // print relevant error message on stderr
            fprintf(stderr, "write problem\n"); 
            return 0;
        }
        else { // amount_sent data has been sent
            // update amount of data to be sent
            amount_to_send = amount_to_send - amount_sent;
            // points to remaining data
            buf = buf + amount_sent;
        }
    }
    return 1; // if this line is reached, no error occurred.
}
