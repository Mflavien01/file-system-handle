#include "../uinclude/communication.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <dirent.h>

#define MAX_PACKET_SIZE 2048
#define HEADER_SIZE 7
#define MAX_DATA_SIZE (MAX_PACKET_SIZE - HEADER_SIZE)
#define MAX_PARAM_SIZE 32

bool file_exists(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file)
    {
        fclose(file);
        return true;
    }
    return false;
}

char *int_to_string(unsigned int n)
{
    char *str = malloc(32);
    sprintf(str, "%u", n);
    return str;
}

unsigned int string_to_int(char *str)
{
    return (unsigned int)strtol(str, NULL, 10);
}

char char_of_int(int n)
{
    return (char)n;
}

int int_of_char(char c)
{
    return (int)c;
}

// FLAVIEN
int put(char *recvbuf, int channel)
{
    char recvbuf_f[MAX_PACKET_SIZE]={0};
    memcpy(recvbuf_f, recvbuf, MAX_PACKET_SIZE);

    printf("\n\n------start put function : -------\n");
    char filename[33];
    memcpy(filename, recvbuf + 6, 32);

    filename[33] = '\0';
    char size_file_array[32];
    memcpy(size_file_array, recvbuf + 37, 32);

    printf("Received file name: %s\n", filename);

    unsigned int size_file = string_to_int(size_file_array);
    if (file_exists(filename))
    {
        printf("File already exists\n");
        char *pkt2 = malloc(sizeof(char) * 2048);
        if (pkt2 == NULL)
        {
            printf("Memory allocation error for pkt2\n");
            return 1;
        }
        pkt2[0] = 'E';
        pkt2[1] = 'D';
        pkt2[2] = 'r';
        memset(pkt2+3, 0, 2);
        memset(pkt2 + 5, -2, 1);
        memset(pkt2 + 6, 0, 32);
        memset(pkt2 + 38, 0, 32);
        send_pkt(pkt2, channel);
        if (pkt2 != NULL)
        {
            free(pkt2);
            pkt2 = NULL;
        }
        return 1;
    }
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        printf("Error creating the file\n");
        char *pkt2 = malloc(sizeof(char) * 2048);
        if (pkt2 == NULL)
        {
            printf("Memory allocation error for pkt2\n");
            return 1;
        }
        pkt2[0] = 'E';
        pkt2[1] = 'D';
        pkt2[2] = 'r';
        memset(pkt2+3, 0, 2);
        memset(pkt2 + 5, -3, 1);
        memset(pkt2 + 6, 0, 32);
        memset(pkt2 + 38, 0, 32);
        send_pkt(pkt2, channel);
        if (pkt2 != NULL)
        {
            free(pkt2);
            pkt2 = NULL;
        }
        fclose(file);
        return 1;
    }
    

    int nb_packets = size_file / MAX_DATA_SIZE;
    int i = 0;

    char pkt_accept[8] = {'E', 'D', 'r', 0, 0, '\0', '\0'};
    send_pkt(pkt_accept, channel);

    while (i < nb_packets)
    {
        recv_pkt(recvbuf_f, channel);
        unsigned int packet_size = *(unsigned int *)(recvbuf_f + 2);
        if (packet_size > MAX_DATA_SIZE)
        {
            printf("Invalid packet size\n");
            char *pkt2 = malloc(sizeof(char) * 2048);
            if (pkt2 == NULL)
            {
                printf("Memory allocation error for pkt2\n");
                return 1;
            }
            pkt2[0] = 'E';
            pkt2[1] = 'D';
            pkt2[2] = 'r';
            memset(pkt2+3, 0, 2);
            memset(pkt2 + 5, -4, 1);
            memset(pkt2 + 6, 0, 32);
            memset(pkt2 + 38, 0, 32);
            send_pkt(pkt2, channel);
            if (pkt2 != NULL)
            {
                free(pkt2);
                pkt2 = NULL;
            }
            fclose(file);
            return 1;

        }
        char add[packet_size+1];
        memcpy(add, recvbuf_f + 70, packet_size);
        add[packet_size] = '\0';
        file = fopen(filename, "a");
        if (!file)
        {
            printf("Error opening the file\n");
            char *pkt2 = malloc(sizeof(char) * 2048);
            if (pkt2 == NULL)
            {
                printf("Memory allocation error for pkt2\n");
                return 1;
            }
            pkt2[0] = 'E';
            pkt2[1] = 'D';
            pkt2[2] = 'r';
            memset(pkt2+3, 0, 2);
            memset(pkt2 + 5, -3, 1);
            memset(pkt2 + 6, 0, 32);
            memset(pkt2 + 38, 0, 32);
            send_pkt(pkt2, channel);
            if (pkt2 != NULL)
            {
                free(pkt2);
                pkt2 = NULL;
            }
            return 1;
        }
        i++;
    }
    fclose(file);
    return 0;
}

// PAUL
int rm(char *recvbuf, int channel)
{
    char file_name[33] = {0};
    memcpy(file_name, recvbuf + 6, 32);
    file_name[32] = '\0';

    printf("rm' command received for the file: %s\n", file_name);

    unlink(file_name);

    if (!file_exists(file_name))
    {
        char *pkt2 = malloc(sizeof(char) * 2048);
            if (pkt2 == NULL)
            {
                printf("Memory allocation error for pkt2\n");
                return 1;
            }
            pkt2[0] = 'E';
            pkt2[1] = 'D';
            pkt2[2] = 'r';
            memset(pkt2+3, 0, 2);
            memset(pkt2 + 5, 0, 1);
            memset(pkt2 + 6, 0, 32);
            memset(pkt2 + 38, 0, 32);
            send_pkt(pkt2, channel);
            if (pkt2 != NULL)
            {
                free(pkt2);
                pkt2 = NULL;
            }
            return 1;
    }
    return 0;
}

// PAUL
int get(char *recvbuf, int channel)
{
    char recvbuf_f[MAX_PACKET_SIZE] = {0};
    memcpy(recvbuf_f, recvbuf, MAX_PACKET_SIZE);

    printf("\n\n------start get function : -------\n");

    char filename[33];
    memcpy(filename, recvbuf + 6, 32);
    filename[32] = '\0';

    printf("Name of file requested : %s\n", filename);

    if (!file_exists(filename))
    {
        printf("The file doesn't exist.\n");
        char *pkt2 = malloc(sizeof(char) * MAX_PACKET_SIZE);
        if (pkt2 == NULL)
        {
            printf("Memory allocation error for pkt2.\n");
            return 1;
        }
        pkt2[0] = 'E';
        pkt2[1] = 'D';
        pkt2[2] = 'r';
        memset(pkt2 + 3, 0, 2);
        memset(pkt2 + 5, -2, 1);
        memset(pkt2 + 6, 0, 32);
        memset(pkt2 + 38, 0, 32);
        send_pkt(pkt2, channel);
        free(pkt2);
        return 1;
    }

    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf("Error opening file.\n");
        char *pkt2 = malloc(sizeof(char) * MAX_PACKET_SIZE);
        if (pkt2 == NULL)
        {
            printf("Memory allocation error for pkt2.\n");
            return 1;
        }
        pkt2[0] = 'E';
        pkt2[1] = 'D';
        pkt2[2] = 'r';
        memset(pkt2 + 3, 0, 2);
        memset(pkt2 + 5, -3, 1);
        memset(pkt2 + 6, 0, 32);
        memset(pkt2 + 38, 0, 32);
        send_pkt(pkt2, channel);
        free(pkt2);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    unsigned int size_file = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("File size : %u octets\n", size_file);

    int nb_packets = size_file / MAX_DATA_SIZE;
    int size_last_packet = size_file % MAX_DATA_SIZE;

    char pkt_accept[8] = {'E', 'D', 'r', 0, 0, '\0', '\0'};
    send_pkt(pkt_accept, channel);

    char buffer[MAX_DATA_SIZE];
    int i = 0;

    while (i < nb_packets)
    {
        printf("Sending the package %d\n", i + 1);
        fread(buffer, 1, MAX_DATA_SIZE, file);

        char pkt[MAX_PACKET_SIZE] = {'E', 'D', 'r'};
        pkt[3] = (char)(MAX_DATA_SIZE / 256);
        pkt[4] = (char)(MAX_DATA_SIZE % 256);
        memcpy(pkt + 5, filename, 32);

        char *size_str = int_to_string(size_file);
        memcpy(pkt + 37, size_str, 32);
        free(size_str);

        memcpy(pkt + 69, buffer, MAX_DATA_SIZE);

        send_pkt(pkt, channel);
        free(pkt);
        i++;
    }

    if (size_last_packet > 0)
    {
        printf("Sending the last package (%d octets)\n", size_last_packet);
        fread(buffer, 1, size_last_packet, file);

        char pkt[MAX_PACKET_SIZE] = {'E', 'D', 'r'};
        pkt[3] = (char)(size_last_packet / 256);
        pkt[4] = (char)(size_last_packet % 256);
        memcpy(pkt + 5, filename, 32);

        char *size_str = int_to_string(size_file);
        memcpy(pkt + 37, size_str, 32);
        free(size_str);

        memcpy(pkt + 69, buffer, size_last_packet);

        send_pkt(pkt, channel);
        free(pkt);
    }
    fclose(file);
    return 0;
}


// FLAVIEN
int ls(char *recvbuf, int channel)
{
    struct dirent *dir;
    DIR *d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
            char *pkt = malloc(sizeof(char) * 2048);
            if (pkt == NULL)
            {
                printf("Memory allocation error for pkt\n");
                return 1;
            }
            pkt[0] = 'E';
            pkt[1] = 'D';
            pkt[2] = 'r';
            memset(pkt + 3, 1978, 2);
            memset(pkt + 5, 6, 32);
            memset(pkt + 6, 1, 32);
            memcpy(pkt + 38, dir->d_name, strlen(dir->d_name));
            send_pkt(pkt, channel);
            if (pkt != NULL)
            {
                free(pkt);
                pkt = NULL;
            }
        }
        closedir(d);
    }
    else
    {
        printf("Error opening the directory\n");
        char *pkt2 = malloc(sizeof(char) * 2048);
        if (pkt2 == NULL)
        {
            printf("Memory allocation error for pkt2\n");
            return 1;
        }
        pkt2[0] = 'E';
        pkt2[1] = 'D';
        pkt2[2] = 'r';
        memset(pkt2 + 3, 0, 2);
        memset(pkt2 + 5, -3, 1);
        memset(pkt2 + 6, 0, 32);
        memset(pkt2 + 38, 0, 32);
        send_pkt(pkt2, channel);
        if (pkt2 != NULL)
        {
            free(pkt2);
            pkt2 = NULL;
        }
        return 1;
    }
    return 0;
    
}

// FLAVIEN
int cat(char *recvbuf, int channel)
{
    char filename[33] = {0};
    uint32_t n_lines = 0;

    memcpy(filename, recvbuf + 6, 32);
    filename[32] = '\0';
    memcpy(&n_lines, recvbuf + 72, sizeof(uint32_t));
    printf("'cat' command for the file: %s, number of lines: %u\n", filename, n_lines);

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error opening the file\n");
        char *pkt2 = malloc(sizeof(char) * 2048);
        if (pkt2 == NULL)
        {
            printf("Memory allocation error for pkt2\n");
            return 1;
        }
        pkt2[0] = 'E';
        pkt2[1] = 'D';
        pkt2[2] = 'r';
        memset(pkt2 + 3, 0, 2);
        memset(pkt2 + 5, -3, 1);
        memset(pkt2 + 6, 0, 32);
        memset(pkt2 + 38, 0, 32);
        send_pkt(pkt2, channel);
        if (pkt2 != NULL)
        {
            free(pkt2);
            pkt2 = NULL;
        }
        fclose(file);
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int i = 0;

    while ((read = getline(&line, &len, file)) != -1 && i < n_lines)
    {
        char *pkt = malloc(sizeof(char) * 2048);
        if (pkt == NULL)
        {
            printf("Memory allocation error for pkt\n");
            return 1;
        }
        pkt[0] = 'E';
        pkt[1] = 'D';
        pkt[2] = 'r';
        pkt[3] = 0;
        pkt[4] = 1;
        memset(pkt + 5, 0, 1);
        memset(pkt + 6, 0, 32);
        memset(pkt + 38, 0, 32);
        memcpy(pkt + 70, line, read);
        send_pkt(pkt, channel);
        if (pkt != NULL)
        {
            free(pkt);
            pkt = NULL;
        }
        printf("Line sent: %s\n", line);
        i++;
    }

    if (line)
    {
        free(line);
    }

    fclose(file);
    return 0;

}

// PAUL
int mv(char *recvbuf, int channel)
{
    char filename[33] = {0}, newfile[33] = {0};
    memcpy(filename, recvbuf + 6, 32);
    memcpy(newfile, recvbuf + 38, 32);
    filename[32] = '\0';
    newfile[32] = '\0';

    printf("mv' command for files : %s and %s\n", filename, newfile);
  
    int rename_file = rename(filename, newfile);

    if (rename_file != 0)
    {
        printf("Error renaming the file\n");
        char *pkt2 = malloc(sizeof(char) * 2048);
            if (pkt2 == NULL)
            {
                printf("Memory allocation error for pkt2\n");
                return 1;
            }
            pkt2[0] = 'E';
            pkt2[1] = 'D';
            pkt2[2] = 'r';
            memset(pkt2+3, 0, 2);
            memset(pkt2 + 5, 0, 1);
            memset(pkt2 + 6, 0, 32);
            memset(pkt2 + 38, 0, 32);
            send_pkt(pkt2, channel);
            if (pkt2 != NULL)
            {
                free(pkt2);
                pkt2 = NULL;
            }
            return 1;
    }
    return 0;
}

const char *const server_help_options = "\
 server options are :\n\
\n\
 -quotasize int :\n\
	maximum number of bytes that the remote directory can store.\n\
\n\
 -quotanumber int :\n\
	maximum number of files that the remote directory can store.\n\
\n\
 -directory string :\n\
	specifies the directory to be used to store files. If this\n\
	directory is non empty when the server starts, then existing\n\
	files are assumed to be part of the remote drive.\n\
\n\
";

void student_server(int channel, int argc, char *argv[])
{
    // bool quotasize = false;
    // bool quotanumber = false;
    // long quotasize_limit = 0;
    // int quotanumber_limit = 0;

    signal(SIGPIPE, SIG_IGN);

    char directory[MAX_PARAM_SIZE] = "./tmp";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-directory") == 0 && i + 1 < argc)
        {
            strncpy(directory, argv[i + 1], MAX_PARAM_SIZE - 1);
            i++;
        }
        // else if (strcmp(argv[i], "-quotasize") == 0 && i + 1 < argc)
        // {
        //     quotasize = true;
        //     quotasize_limit = atol(argv[i + 1]);
        //     i++;
        // }
        // else if (strcmp(argv[i], "-quotanumber") == 0 && i + 1 < argc)
        // {
        //     quotanumber = true;
        //     quotanumber_limit = atoi(argv[i + 1]);
        //     i++;
        // }
        else
        {
            printf("Unknown option: %s\n", argv[i]);
            printf("%s", server_help_options);
            return;
        }
    }

    struct stat st = {0};
    if (stat(directory, &st) == -1)
    {
        if (mkdir(directory, 0700) != 0)
        {
            perror("Error creating the directory");
            return;
        }
    }

    if (chdir(directory) != 0)
    {
        perror("Unable to change directory");
        return;
    }

    char recvbuf[81];
    while (1)
    {
        printf(" -- wait a packet (^C to exit) --\n");
        int res = recv_pkt(recvbuf, channel);
        if (!res)
        {
            printf("Reception error\n");
            return;
        }

        // printf("Received packet content: ");
        // for (int i = 0; i < 70; i++)
        // {
        //     printf("%02X ", (unsigned char)recvbuf[i]);
        // }
        // printf("\n");

        if (recvbuf[0] != 'E' || recvbuf[1] != 'D' || recvbuf[2] != 'r')
        {
            printf("Invalid packet header. Received: %c%c%c\n", recvbuf[0], recvbuf[1], recvbuf[2]);
            continue;
        }

        uint8_t cmd = recvbuf[5];
        uint16_t data_size = (recvbuf[3] << 8) | recvbuf[4];

        // printf("Announced data size: %u\n", data_size);

        if (data_size > MAX_DATA_SIZE)
        {
            printf("Invalid data size: %u\n", data_size);
            continue;
        }
        else
        {
            printf("Valid data size: %u\n", data_size);
        }

        char option1[33] = {0}, option2[33] = {0}, data[MAX_DATA_SIZE + 1] = {0};
        memcpy(option1, recvbuf + 6, 32);
        option1[32] = '\0';
        memcpy(option2, recvbuf + 38, 32);
        option2[32] = '\0';
        memcpy(data, recvbuf + 72, data_size);
        data[data_size] = '\0';
        // printf("Command: %02X ", cmd);
        // printf("Option 1: %s Option 2: %s Data: %.*s\n", option1, option2, data_size, data);

        switch (cmd)
        {
        case 0x01:
            cat(recvbuf, channel);
            break;
        case 0x02:
            put(recvbuf, channel);
            break;
        case 0x03:
            mv(recvbuf, channel);
            break;
        case 0x04:
            rm(recvbuf, channel);
            break;
        case 0x05:
            get(recvbuf, channel);
            break;
        case 0x06:
            ls(recvbuf, channel);
            break;
        case 0x07:
            return;
        default:
            printf("Unknown command: %c\n", cmd);
        }
    }
}