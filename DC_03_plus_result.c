#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <libgen.h>
#include <ctype.h>
#define MAX_SIZE 300
#define IP_ADDRESS "127.0.0.1"

enum my_type {
    NORMAL = 0,
    UPPER,
    LOWER,
    ECHO_SENT,
    ECHO_RECV,
};

struct L {
    int type;
    int length;
    char data[MAX_SIZE];
};

void L_send(int type, char *input);
char *L_receive(int *);
void data_send(char *data, int length);
char *data_receive(int *);

void *do_thread(void *);
void check_is_server(char *const *argv);
void init_socket();

int sndsock, rcvsock, clen;
struct sockaddr_in s_addr, r_addr;
int is_server = 0;

int main(int argc, char *argv[]) {
    char input[MAX_SIZE];
    int type;
    pthread_t t_id;
    check_is_server(argv);
    init_socket();
    int status = pthread_create(&t_id, NULL, do_thread, NULL);
    if (status != 0) {
        printf("Thread Error!\n");
        exit(1);
    }
	while (1) {
        scanf("%s", input);
        scanf("%d", &type);
        L_send(type, input);
	}    
}

void L_send(int type, char *input) {
    struct L layer;
    char message[350];
    int size = 0;

    layer.type = type;
    layer.length = strlen(input);
    printf("Sent Length : %d\n", layer.length);
    memset(layer.data, 0x00, MAX_SIZE);
    memcpy(layer.data, (void *)input, layer.length);

    size = sizeof(struct L) - sizeof(layer.data) + layer.length;

    memset(message, 0x00, 350);
    memcpy(message, (void *)&layer, size);
    data_send(message, size);
}

char *L_receive(int *length) {
    struct L *layer;

    layer = (struct L *)data_receive(length);
    *length = *length - sizeof(layer->length) - sizeof(layer->type);
    layer->data[layer->length] = '\0';
    if(layer->type == NORMAL) {
    }
    else if(layer->type == UPPER) {
        for(int i = 0; i < layer->length; i++) {
            layer->data[i] = toupper(layer->data[i]);
        }
    }
    else if(layer->type == LOWER) {
        for(int i = 0; i < layer->length; i++) {
            layer->data[i] = tolower(layer->data[i]);
        }
    }
    else if(layer->type == ECHO_SENT) {
        printf("Echo Sent\n");
        printf("Sent Data : %s\n", layer->data);
        printf("Sent Type : %d\n", ECHO_RECV);
        L_send(ECHO_RECV, layer->data);
    }
    else if(layer->type == ECHO_RECV) {
        printf("Echo Received\n");
    }
    else {
        layer->data[0] = '\0';
        printf("Wrong Type\n");
        printf("####################################################\n");
        return (char *)layer->data;
    }
    printf("Received Length : %d\n", layer->length);
    printf("Received Type : %d\n", layer->type);
    return (char *)layer->data;
}

///////////////////////////////////////////////////////////////////////////////////////
void data_send(char *data, int length) {
    sendto(sndsock, data, length, 0, (struct sockaddr *)&s_addr, sizeof(s_addr));
    printf("####################################################\n");
}
char *data_receive(int *length) {
    static char data[MAX_SIZE];
    *length = recvfrom(rcvsock, data, MAX_SIZE, 0, (struct sockaddr *)&r_addr, &clen);
    return data;
}
void *do_thread(void *arg) {
    char output[MAX_SIZE];
    int length;
    while (1) {
        strcpy(output, L_receive(&length));
        if (strlen(output) > 1) {
            printf("Received Data : %s\n", output);
            printf("####################################################\n");
        }
    }
    return NULL;
}
void check_is_server(char *const *argv) {
    if (argv[1] == NULL) {
        printf("Usage: %s [server|client]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if (strcmp(argv[1], "server") == 0) {
        is_server = 1;
    } else if (strcmp(argv[1], "client") == 0) {
        is_server = 0;
    } else {
        printf("Usage: %s [server|client]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}
void init_socket() {
    int send_port;
    int receive_port; 	
    if (is_server == 0) {
        printf("Session 2 Start\n");
        send_port = 8811;
        receive_port = 8810;
    }
    else {
        printf("Session 1 Start\n");
        send_port = 8810;
        receive_port = 8811;
    }
    if ((sndsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket error : ");
        exit(1);
    }
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    s_addr.sin_port = htons(send_port);
    if (connect(sndsock, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0) {
        perror("connect error : ");
        exit(1);
    }
    if ((rcvsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket error : ");
        exit(1);
    }
    clen = sizeof(r_addr);
    memset(&r_addr, 0, sizeof(r_addr));
    r_addr.sin_family = AF_INET;
    r_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    r_addr.sin_port = htons(receive_port);
    if (bind(rcvsock, (struct sockaddr *)&r_addr, sizeof(r_addr)) < 0) {
        perror("bind error : ");
        exit(1);
    }
    int optvalue = 1;
    int optlen = sizeof(optvalue);
    setsockopt(sndsock, SOL_SOCKET, SO_REUSEADDR, &optvalue, optlen);
    setsockopt(rcvsock, SOL_SOCKET, SO_REUSEADDR, &optvalue, optlen);
    printf("####################################################\n");
}