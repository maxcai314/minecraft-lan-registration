#include "tun_open.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int add_address_to_interface(char *interface_name, char *address);
int delete_route(char *address, char *interface_name);
char* allocate_payload(char* motd, char* port);
void advertise_server(char * host_address,char *advert);

#define MAGIC_ADDRESS "224.0.2.60" // multicast address
#define PORT 4445
#define SLEEP_TIME_MICROS 1500000 // 1.5 seconds

#define PAYLOAD(MOTD, PORT) "[MOTD]" MOTD "[/MOTD]" "[AD]" PORT "[/AD]"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <host_address> <port> <motd>\n", argv[0]);
        return 1;
    }

    char *host_address = argv[1];
    char *port = argv[2];
    char *motd = argv[3];
    char *advert = allocate_payload(motd, port);
    // char *advert = PAYLOAD("A server", "25565");

    printf("Server address: %s\n", host_address);
    printf("Server port: %s\n", port);
    printf("Display motd: %s\n", motd);
    printf("\n");

    advertise_server(host_address, advert);

    free(advert);
    return 0;
}

#define PAYLOAD_FORMAT "[MOTD]%s[/MOTD][AD]%s[/AD]"
#define PAYLOAD_FORMAT_LEN 22

char* allocate_payload(char* motd, char* port) {
    int len = PAYLOAD_FORMAT_LEN + strlen(motd) + strlen(port) + 1;
    char* payload = malloc(len);
    sprintf(payload, "[MOTD]%s[/MOTD][AD]%s[/AD]", motd, port);
    return payload;
}

void advertise_server(char * host_address, char *advert) {
    int res;

    struct sockaddr_in source_addr;
    memset(&source_addr, 0, sizeof(source_addr));
    source_addr.sin_family = AF_INET;
    source_addr.sin_port = htons(0); // arbitrary
    // source_addr.sin_addr.s_addr = inet_addr(host_address);
    res = inet_pton(AF_INET, host_address, &(source_addr.sin_addr));
    if (res != 1) {
        perror("input address is not a valid network address");
        exit(1);
    }

    printf("Creating tun device. This requires root permissions.\n");

    TunOpenName tunName;
    memset(&tunName.name, 0, sizeof(tunName.name));
    int fd = tunOpen(NULL, &tunName);
    if (fd < 0 || strlen(tunName.name) == 0) {
        perror("Failed to create tun device");
        exit(1);
    }

    printf("Successfully created tun device %s\n", tunName.name);
    
    res = add_address_to_interface(tunName.name, host_address);
    if (res != 0) {
        perror("Failed to add address to network interface");
        exit(1);
    }

    res = delete_route(host_address, tunName.name);
    if (res != 0) {
        perror("Failed to delete ip table route");
        exit(1);
    }

    printf("Starting multicast on UDP\n");
    printf("\n");

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create UDP socket");
        exit(1);
    }

    res = bind(sockfd, (struct sockaddr *) &source_addr, sizeof(source_addr));

    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(PORT);
    target_addr.sin_addr.s_addr = inet_addr(MAGIC_ADDRESS);
    
    int i = 0;
    while (1) {
        if (++i % 10 == 0) {
            printf("still alive: %d\n", i);
        }

        int bytes_sent = sendto(sockfd, advert, strlen(advert), 0, (struct sockaddr *) &target_addr, sizeof(target_addr));
    
        if (bytes_sent < 0) {
            perror("Sending multicast failed");
            break;
        }
        
        res = usleep(SLEEP_TIME_MICROS);
        if (res != 0) {
            perror("Sleep failed");
            break;
        }
    }

    printf("Closing socket\n");
    close(sockfd);

    printf("Closing tun device\n");
    close(fd);
}

#define IFCONFIG_FORMAT "ifconfig %s add %s %s"
#define IFCONFIG_FORMAT_LEN 15

int add_address_to_interface(char *interface_name, char *address) {
    int len = IFCONFIG_FORMAT_LEN + strlen(interface_name) + 2 * strlen(address) + 1;
    char *command = malloc(len);
    memset(command, 0, len);
    sprintf(command, IFCONFIG_FORMAT, interface_name, address, address);
    
    printf("%s\n", command);

    int ret = system(command);
    free(command);
    return ret;
}

#define ROUTE_FORMAT "route delete %s -interface %s"
#define ROUTE_FORMAT_LEN 25

int delete_route(char *address, char *interface_name) {
    int len = ROUTE_FORMAT_LEN + strlen(address) + strlen(interface_name) + 1;
    char *command = malloc(len);
    memset(command, 0, len);
    sprintf(command, ROUTE_FORMAT, address, interface_name);

    printf("%s\n", command);

    int ret = system(command);
    free(command);
    return ret;
}