#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 6969

int main() {
  int sockfd, connfd, cli_len;
  struct sockaddr_in my_addr, cli_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    printf("ERROR: Can't open a socket!\n");
    exit(sockfd);
  }

  printf("Successfully opened a socket!\n");

  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY;
  my_addr.sin_port = htons(PORT);

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
    printf("ERROR : Couldnt bind the socket to the address!\n");
    exit(EXIT_FAILURE);
  }

  printf("Successfully bound the socket to the address\n");

  if (listen(sockfd, 5) < 0) {
    printf("ERROR : Cannot listen!\n");
    exit(EXIT_FAILURE);
  }

  printf("Listening on Port 6969!\n");
  cli_len = sizeof(cli_addr);

  connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
  if (connfd < 0) {
    printf("ERROR: Server not accepting!\n");
    exit(EXIT_FAILURE);
  }
  // char hello[] = "Hello World!\n";

  printf("Successfully connected to the server\n");

  // write(connfd, "HTTP/1.1 200 OK\r\n", 18);
  // write(connfd, "Server: Http\r\n", 15);
  // write(connfd, "Content-Type: text/html\r\n", 26);
  // write(connfd, "Connection: Closed\r\n", 21);
  // write(connfd, "\r\n", 3);
  // write(connfd, "<h1>Hello Beautiful!</h1>\n", 26);
  char buff[128];
  strcat(buff, "HTTP/1.1 200 OK\r\n");
  strcat(buff, "Server: Http\r\n");
  strcat(buff, "Content-Type: text/html\r\n");
  strcat(buff, "Connection: Closed\r\n");
  strcat(buff, "\r\n");
  strcat(buff, "<h1>Hello Beautiful!</h1>\n");
  int bufflen = strlen(buff);

  write(connfd, buff, bufflen);
  close(connfd);
  close(sockfd);
  return 0;
}
