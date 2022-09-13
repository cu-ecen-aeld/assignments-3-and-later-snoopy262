#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>

// Global Variables
const char* file_path = "/var/tmp/aesdsocketdata";

// This happens on SIGTERM and SIGINT
void termination_handler(int s)
{
    syslog(LOG_INFO, "Caught signal, exiting.");
    closelog();
    remove(file_path);
    exit(0);
}
    
    

int main(int argc, char **argv)
{
    int socket_fd;
    int connection_fd;
    int output_fd;
    int rc;

    const char* client_port = "9000";
    const int connection_backlog = 20;
    const int buffer_size = 10000;

    struct addrinfo hints;
    struct addrinfo* my_addrinfo;

    struct sockaddr client_adress;
    char client_adress_string[255];
    socklen_t client_adress_size;

    char data_buffer[buffer_size];
    ssize_t size_received;

    struct sigaction sa_int;
    struct sigaction sa_term;

    // prepare logging
    openlog("aesdsocket", LOG_CONS|LOG_NDELAY, LOG_USER);


    // open Socket
    
    printf("Opening Socket... ");
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_fd == -1)
    {
        printf("Open Socket failed.\n");
        syslog(LOG_ERR, "Open Socket failed.");
        return -1;
    } 
    
    printf("OK.\n");

    int enable = 1;
    rc = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); 
    if(rc != 0)
    {
        printf("setsockopt() failed.\n");
        syslog(LOG_ERR, "setsockopt() failed.\n");
        return -1;
    } 

    printf("Getting addrinfo... ");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(NULL, client_port, &hints, &my_addrinfo);

    if(rc != 0)
    {
        printf("getaddrinfo() failed.\n");
        syslog(LOG_ERR, "getaddrinfo() failed.");
        return -1;
    } 

    printf("OK.\n");

    // bind socket to port

    printf("Binding to Port... ");
    rc = bind(socket_fd, my_addrinfo->ai_addr, my_addrinfo->ai_addrlen);

    if(rc != 0)
    {
        printf("bind() failed with Error %d\n", errno);
        syslog(LOG_ERR, "bind() failed with Error %d", errno);
        freeaddrinfo(my_addrinfo);
        return -1;
    } 

    printf("OK.\n");

    printf("Listening for Connection... ");
    rc = listen(socket_fd, connection_backlog);

    if(rc != 0)
    {
        printf("listen() failed with Error %d\n", errno);
        syslog(LOG_ERR, "listen() failed with Error %d", errno);
        freeaddrinfo(my_addrinfo);
        return -1;
    }

    printf("OK.\n");
    
    printf("Opening file for saving data... ");
    output_fd = open(file_path, O_CREAT | O_APPEND | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    if(output_fd == -1)
    {
        printf("File %s cannot be opened", file_path);
        syslog(LOG_ERR, "File %s cannot be opened", file_path);
        freeaddrinfo(my_addrinfo);
        return -1;
    }

    if (argc > 1)
    {
        if (strcmp(argv[1], "-d") == 0)
        {
            rc = fork();
            if(rc == -1)
            {
                printf("fork() failed");
                syslog(LOG_ERR, "fork() failed");
            }
            else if (rc == 0)
            {
                // in the child. do nothing
            }
            else
            {
                exit(0);
            }

        }
    }

    // Setup Signals
    sa_int.sa_handler = termination_handler;
    sigemptyset(&(sa_int.sa_mask));
    sa_int.sa_flags = SA_RESTART;
    sa_term = sa_int;
    rc = sigaction(SIGINT, &sa_int, NULL);
    rc += sigaction(SIGTERM, &sa_int, NULL);
    if (rc != 0)
    {
        printf("sigaction failed");
        syslog(LOG_ERR, "sigaction failed");
        freeaddrinfo(my_addrinfo);
        return -1;
    }

    freeaddrinfo(my_addrinfo);

    printf("OK. starting infinite loop...\n");

    while(1)
    {

        printf("Waiting for Connection...\n");
        client_adress_size = sizeof(client_adress);
        connection_fd = accept(socket_fd, &client_adress, &client_adress_size);

        if(rc == -1)
        {
            printf("failed with Error %d\n", errno);
            syslog(LOG_ERR, "accept() failed with Error %d", errno);
            freeaddrinfo(my_addrinfo);
            return -1;
        }

        // convert Adress to human readable string
        inet_ntop(AF_INET, &client_adress, client_adress_string, client_adress_size);

        syslog(LOG_INFO, "Accepted connection from %s", client_adress_string );
        printf("Accepted connection from %s.\n", client_adress_string );

        do {
            memset(data_buffer,0,buffer_size);
            size_received = recv(connection_fd, data_buffer, buffer_size, 0);
            if (size_received < 0) {
              printf("Error: Receive failed: %s\n",strerror(errno));
              syslog(LOG_ERR,"Error: Receive failed");
              exit(1);
            }
            printf("received: [snip] (len = %d, sizeof(data_buffer) = %d)\n", (int)size_received, (int)sizeof(data_buffer));
            rc = write(output_fd, data_buffer, size_received);
            printf("wrote %d bytes\n", (int)size_received);

        }while (size_received == sizeof(data_buffer));

        // while(1){}

        int file_length = lseek(output_fd, 0, SEEK_END);

        char* output_buffer = malloc(sizeof(char)*file_length);

        lseek(output_fd, 0, SEEK_SET);
        int read_len = read(output_fd, output_buffer, file_length);
        printf("read %d bytes\n", (int)read_len);

        printf("going to send Message [snip], file_lenght=%d,  \n", file_length);
        int send_len = send(connection_fd,output_buffer,file_length,0);   
        
        if (send_len < 0)
        {
            printf("send() failed\n");
            syslog(LOG_ERR, "send() failed");
            freeaddrinfo(my_addrinfo);
            return -1;
        }        
        
        free(output_buffer);
        printf("sent Message %s \n", data_buffer);

    }

    printf("cleaning up... ");
    remove(file_path);    
    freeaddrinfo(my_addrinfo);
    closelog();
    printf("done.\n");

    return 0;
}
