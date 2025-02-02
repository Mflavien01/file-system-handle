// INCLUDES
/* to use the provided parse_commandline function. */
#include "../include/utilities.h"
/* user defined library for sending and receiving packets */
#include "../uinclude/communication.h"
/* for stdin,...*/
#include <stdio.h>
/* for memcpy,...*/
/* to modify SIGPIPE signal handling, as default behaviour makes
 * the program exits when trying to write to a closed socket. 
 * We don't want this.
 */
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

/**  help message for commandline options */
const char * const client_help_options = "\
 client options are:\n\
\n\
 -interactive :\n\
	interactive mode ('better' interface messages)\n\
\n\
 -analyze filename :\n\
	reads commands from filename.\n\
\n\
 -directory string :\n\
	   specifies the directory to be used to store files. If this\n\
	   directory is non empty when the client starts, then existing\n\
	   files are assumed to be part of the local drive.\n\
\n\
";

// The following function is the one you must implement for your project.
// Here, we provide an implementation related to the example given above, which is
// not the exactly the project you have to implement
// It returns 0 to exit or another value to restart 
// the client.

//Convert a string to an integer (+ handling positive and negative signs)
int string_to_int(const char *str) {
    if (str == NULL) {
        return 0;
    }
    int result;
    int sign = 1;

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str) {
        if (!isdigit(*str)) {
            break;
        }
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}


// Print information about available options and commands
int help(){
  printf("%s\n\n","Help :");
 
 
  printf("%s\n","Options :");
 
  printf("\t%s\n","Server :");
  printf("\t\t%s\n","-directory [Remote file name]: specifies the directory to be used to store files. If this directory is non empty when the server starts, then existing files are assumed to be part of the remote drive. ");
 
  printf("\t%s\n","Client :");
  printf("\t\t%s\n","-analyze [Local file name]: the client executes the command of the file given as input. ");
  printf("\t\t%s\n","-interactive: the client waits for commands on the keyboard. If the '-analyze' option is also provided, then the client first executes of the commands of the referenced files before waiting for keyboard-entered commands. ");
  printf("\t\t%s\n\n","-directory [Local directory name]: specifies the directory to be used for client side files. If this directory is non empty when the client starts, then existing files are assumed to be part of the handled files, as long as they respect the requirements on file names. ");
 
  printf("%s\n","Commands :");
  printf("\t%s\n","put [Local file name] : This command is expected to copy a local file to the Eurecom Drive.");
  printf("\t%s\n","rm [Remote file name] : This command shall remove a file from the Eurecom Drive. ");
  printf("\t%s\n","get [Remote file name, Local new file name] : This command is used to copy a remote file to the local file system. ");
  printf("\t%s\n","ls : This command lists all the files remotely stored in the Eurecom Drive. ");
  printf("\t%s\n","cat [Remote file name, Number of line] : This command returns the n first lines of a file. ");
  printf("\t%s\n","mv [Remote file name, Remote new file name]: This command rename a remote file. ");
return 0;
}

// Handle errors concerning the server and print corresponding messages
int serv_error(int code) {
    if (code == -1) {
        printf("Bad packet format\n");
    } else if (code == -2) {
        printf("File not found\n");
    } else if (code == -3) {
        printf("File already exists\n");
    } else if (code == -4) {
        printf("Command fails\n");
    } else if (code == -5) {
        printf("Quota exceeded\n"); 
    }
    help();
    return 0;
}



// List files on the server
int ls(usercommand parsed_cmd, int channel) {
    char* sendbuf = malloc(sizeof(char)*70);
    sendbuf[0] = 'E';
    sendbuf[1] = 'D';
    sendbuf[2] = 'r';
    memset(sendbuf+3, 0, 2);
    memset(sendbuf+5, 6, 1);
    memset(sendbuf+6, '\0', 32);
    memset(sendbuf+6, '\0', 32);
    send_pkt(sendbuf,channel);
    
    char recvbuf[2048];
    recv_pkt(recvbuf, channel);
    if (recvbuf[5] !=6) {
        serv_error(recvbuf[5]);
    };

    int data_size = *recvbuf+3;
    if (data_size == 0) printf("Empty directory. \n");
    else {
        char file_list[data_size+1];
        memcpy(file_list, recvbuf+70, data_size);

        file_list[data_size] = '\0';

        printf("%s\n", file_list);

    };
    return 0;
}

// Retrieve and print the content of a file from the server
int cat(usercommand parsed_cmd, int channel) {
    char* sendbuf = malloc(sizeof(char)*2048);
    sendbuf[0] = 'E';
    sendbuf[1] = 'D';
    sendbuf[2] = 'r';
    memset(sendbuf + 3, 0, 2);
    memset(sendbuf + 5, 1, 1); 
    memcpy(sendbuf + 6, parsed_cmd.param1, 32);
    memcpy(sendbuf + 38, parsed_cmd.param2 , 32);
    send_pkt(sendbuf, channel);
    free(sendbuf);

    char content[2048];
    int i = 0;
    int size_pkt = 0;
    int nb_pkt = 1;
    while (i < nb_pkt){
        char recvbuf[2048];
        if (recv_pkt(recvbuf, channel) != 0) {
            perror("Failed to receive packet");
            return -1;
        }

        if (recvbuf[5] != 1) {
            serv_error(recvbuf[5]);
            return -1;
        }

        nb_pkt = string_to_int(recvbuf + 6);
        size_pkt = recvbuf[3];
        memcpy(content, recvbuf + 70, size_pkt);
        content[size_pkt] = '\0';
        printf("%s", content);

        i++;
    }

    return 0;
}

// Remove a file from the server
int rm (usercommand parsed_cmd, int channel){
    char* sendbuf = malloc(sizeof(char)*2048);
    sendbuf[0] = 'E';
    sendbuf[1] = 'D';
    sendbuf[2] = 'r';
    memset(sendbuf+3, 0, 2);
    memset(sendbuf+5, 4, 1);
    memcpy(sendbuf+6, parsed_cmd.param1, 32);
    memcpy(sendbuf+38, parsed_cmd.param2, 32);
    send_pkt(sendbuf,channel);
    free(sendbuf);
    char recvbuf[2048];
    recv_pkt(recvbuf, channel);
    if (recvbuf[5] !=0) {
        serv_error(recvbuf[5]);
    };

    return 0;
}


// Send a file to the server 
int put (usercommand parsed_cmd, int channel){
    struct stat filestat;
    char *file = parsed_cmd.param1;
    unsigned int size_file;
    if (stat(file, &filestat)==0) {
        size_file = filestat.st_size;
        } else {
            printf("Error while executing stat");
    };

    int nb_pkt = size_file / 1978;
    int size_last_pkt = size_file - (nb_pkt*1978);

    FILE* f = NULL;
    f = fopen(file,"r");
    if( f == NULL ) {
        return -2;
    }
    char* data = malloc(sizeof(char)*size_file);
    fread(data,size_file, 1, f);
    
    int i = 0;
    while (i<nb_pkt){
        char* sendbuf = malloc(sizeof(char)*2048);
        sendbuf[0] = 'E';
        sendbuf[1] = 'D';
        sendbuf[2] = 'r';
        if (nb_pkt == 1) {
            memset(sendbuf+3, size_file, 2);
            memset(sendbuf+5, 2, 1);
            memcpy(sendbuf+6, parsed_cmd.param1, 32);
            memset(sendbuf+38, size_file, 32);
            memcpy(sendbuf+70, data, size_file);
        } else { 
            memset(sendbuf+3, 1978, 2);
            memset(sendbuf+5, 2, 1);
            memcpy(sendbuf+6, parsed_cmd.param1, 32);
            memset(sendbuf+38, size_file, 32);
            memcpy(sendbuf+70, data+(i*1978), 1978);
        };
        i++;
        send_pkt(sendbuf,channel);
        free(sendbuf);
        char recvbuf[2048];
        recv_pkt(recvbuf, channel);
        if (recvbuf[5] !=0) {
            serv_error(recvbuf[5]);
        };
    }

    if (size_last_pkt > 0){
        char* sendbuf = malloc(sizeof(char)*2048);
        sendbuf[0] = 'E';
        sendbuf[1] = 'D';
        sendbuf[2] = 'r';
        memset(sendbuf+3, size_last_pkt, 2);
        memset(sendbuf+5, 2, 1);
        memcpy(sendbuf+6, parsed_cmd.param1, 32);
        memset(sendbuf+38, size_file, 32);
        memcpy(sendbuf+70, data+(i*1978), size_last_pkt);
        send_pkt(sendbuf,channel);
        free(sendbuf);
        char recvbuf[2048];
        recv_pkt(recvbuf, channel);
        if (recvbuf[5] !=0) {
            serv_error(recvbuf[5]);
        };
    }
    return 0;

}

// Retrieve a file from the server
int get(usercommand parsed_cmd,int channel){
    struct stat filestat;
    char name[strlen(parsed_cmd.param1)+1];
    memcpy(name,parsed_cmd.param1,strlen(parsed_cmd.param1));
    name[strlen(parsed_cmd.param1)]='\0';
    if(stat(name, &filestat)==0){
        return -3;
    }
   
    char* sendbuf = malloc(sizeof(char)*2048);
    sendbuf[0] = 'E';
    sendbuf[1] = 'D';
    sendbuf[2] = 'r';
    memset(sendbuf+3, 0, 2);
    memset(sendbuf+5, 5, 1);
    memcpy(sendbuf+6, parsed_cmd.param1, 32);
    memcpy(sendbuf+38, parsed_cmd.param2, 32);
    send_pkt(sendbuf,channel);
    free(sendbuf);
   

    char recvbuf_f[2048];
    recv_pkt(recvbuf_f,channel);
    if (recvbuf_f[5] !=5) {
        serv_error(recvbuf_f[5]);
    };
    int size_file = string_to_int(recvbuf_f+38);
    int pkt_size = string_to_int(recvbuf_f+3);
    if(stat(name, &filestat)==0){
        return -3;
    }
    FILE* f = NULL;
    f = fopen(name,"w+");
    fclose(f);

    int nb_pkt = size_file/1978;
    int size_last_pkt = size_file - (nb_pkt*1978);
    int i = 0;

    while (i<nb_pkt){
        char add[pkt_size+1];
        memcpy(add,recvbuf_f+70,pkt_size);
        add[pkt_size]='\0';
        f = fopen(name,"a");
        fputs(add, f);
        fclose(f);

        recv_pkt(recvbuf_f, channel);
        if (recvbuf_f[5] !=5) {
            serv_error(recvbuf_f[5]);
        };
        i++;

    }

    if (size_last_pkt>0){

        char add[size_last_pkt+1];
        memcpy(add,recvbuf_f+70,size_last_pkt);
        add[size_last_pkt]='\0';
        f = fopen(name,"a");
        fputs(add, f);
        fclose(f);
    }
    return 0;

}

// Rename a file on the server 
int mv(usercommand parsed_cmd, int channel) {
    char* sendbuf = malloc(sizeof(char)*2048);
    sendbuf[0] = 'E';
    sendbuf[1] = 'D';
    sendbuf[2] = 'r';
    memset(sendbuf+3, 0, 2);
    memset(sendbuf+5, 3, 1);
    memcpy(sendbuf+6, parsed_cmd.param1, 32);
    memcpy(sendbuf+38, parsed_cmd.param2, 32);
    send_pkt(sendbuf,channel);
    free(sendbuf);

    char recvbuf_f[2048];
    recv_pkt(recvbuf_f,channel);
    if (recvbuf_f[5] !=0) {
        serv_error(recvbuf_f[5]);
    };

    return 0;
}   

// Restart the client
int restart(char* argv, int channel) {
    char* sendbuf = malloc(sizeof(char)*70);
    sendbuf[0] = 'E';
    sendbuf[1] = 'D';
    sendbuf[2] = 'r';
    memset(sendbuf+3, 0, 2);
    memset(sendbuf+5, 8, 1);
    memset(sendbuf+6, '\0', 32);
    memset(sendbuf+6, '\0', 32);
    send_pkt(sendbuf,channel);
    free(sendbuf);

    char *const arguments[] = {"./client",argv[1],argv[2],NULL};
    execvp("./client",arguments);
}

// Quit the client
int quit(int channel){
    char* sendbuf = malloc(sizeof(char)*70);
    sendbuf[0] = 'E';
    sendbuf[1] = 'D';
    sendbuf[2] = 'r';
    memset(sendbuf+3, 0, 2);
    memset(sendbuf+5, 7, 1);
    memset(sendbuf+6, '\0', 32);
    memset(sendbuf+6, '\0', 32);
    send_pkt(sendbuf,channel);
    free(sendbuf);
    exit(0);
    return 0 ;
}

// Find and execute the appropriate command
int find_command(usercommand parsed_cmd, int channel, char* argv) {
   
    if (strcmp(parsed_cmd.cmd, "put") == 0) {return put(parsed_cmd,channel);}
    else if (strcmp(parsed_cmd.cmd, "rm") == 0) {return rm(parsed_cmd,channel);}
    else if (strcmp(parsed_cmd.cmd, "get") == 0) {return get(parsed_cmd,channel);}
    else if (strcmp(parsed_cmd.cmd, "mv") == 0) {return mv(parsed_cmd,channel);}
    else if (strcmp(parsed_cmd.cmd, "cat") == 0) {return cat(parsed_cmd,channel);}
    else if (strcmp(parsed_cmd.cmd, "ls") == 0) {return ls(parsed_cmd,channel);}
    else if (strcmp(parsed_cmd.cmd, "help") == 0) {return help(parsed_cmd,channel);}
    else if (strcmp(parsed_cmd.cmd, "quit") == 0 || strcmp(parsed_cmd.cmd, "exit") == 0) {return quit(channel);}
    else if (strcmp(parsed_cmd.cmd, "restart") == 0) {return restart(argv, channel);}
    return 1;

}

// Main client function
int student_client(int channel, int argc, char *argv[]) {
    // Writing to a closed socket causes a SIGPIPE signal, which makes 
    // the program exits. The following line inhibits this default behaviour.
    // Thus, writing to a cloned socket will simply return -1 with the EPIPE
    // error in errno and will therefore not cause an exit. 
    // (see the line with "EPIPE" in send_pkt in usrc/communication.c).
    signal(SIGPIPE, SIG_IGN);
    usercommand parsed_cmd;
    char cmdline[128];

    //option handling
    int interactive = 0;
    char* dir = NULL ;
    char* inputfile = NULL;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-directory") == 0 && i + 1 < argc) {
            dir = argv[i+1];
            i++;
        } 
        else if (strcmp(argv[i], "-analyze") == 0 && i+1 < argc) {
            inputfile = argv[i+1];
            i++;
        } 
        else if (strcmp(argv[i], "-interactive") == 0) {
            interactive++;
        } else {
            printf("Option inconnue : %s\n", argv[i]);
            help();
        }
    }

    //checking if the directory exist, otherwise creating it
    if (dir != NULL) {
        chdir(dir);
    }

    if (inputfile !=NULL) { 
        FILE *f = fopen(inputfile, "r");
        int b = 0;
        if (f == NULL) {
            perror("Error opening file");
            printf("Error code opening file: %d\n", errno);
            printf("Error opening file: %s\n", strerror(errno));
            b = 1;
        }
        if (b == 0) {
            struct stat filestat;
            unsigned int size_file;
            if (stat(inputfile, &filestat)==0) {
                size_file = filestat.st_size;
            } else {
                printf("Error while executing stat");
            }
            char str[size_file+1];
            str[size_file]='\0';
            fread(str,size_file, 1, f);
            char *p = strtok(str, "\n");
            int test = 0;
            while(p != NULL) {
                test = 1; 
                test = parse_commandline(&parsed_cmd, p);
                if (test ==0) { 
                    printf("%s%d\n", "KO", -6);
                } else if (strcmp(parsed_cmd.cmd, "restart") == 0) {
                    restart(*argv, channel);
                    char *const arguments[] = {"./client",argv[1],argv[2],NULL};
                    execvp("./client",arguments);
                } else {
                    int error = find_command(parsed_cmd,channel,*argv);
                    printf("%s\n", p );

                    if (error != 0){
                        printf("%s%d\n","KO ",error);
                    } else {
                        printf("%s%d\n", "KO ", error);
                    }
                }
                p = strtok(NULL, "\n");

            }
        }
    }

    while (1){
        if(! fgets(cmdline, 128, stdin)) {
            printf("cannot read command line\n");
            return 0;
        }
        int test = parse_commandline(&parsed_cmd, cmdline);
        if (test == 0){
            printf("%s%d\n","KO ",-6);
        } else if (strcmp(parsed_cmd.cmd, "restart") == 0){
            restart(*argv, channel);
            char *const arguments[] = {"./client",argv[1],argv[2],NULL};
            execvp("./client",arguments);
        } else {
            int error = find_command(parsed_cmd,channel, *argv);
            if (error != 0){
            printf("%s%d\n","KO ",error);
            } else {
                printf("%s\n","OK ");
            }
        }
                
    }
    return 0;
}