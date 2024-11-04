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
  char req[1024];
  size_t req_len = sizeof(req) - 1;
  ssize_t bytes_read = read(connfd, req, req_len);
  req[bytes_read] = '\0';
  printf("%s\n", req);

  char buff[128];
  snprintf(buff, sizeof(buff),
           "HTTP/1.1 200 OK\r\n"
           "Server: Http\r\n"
           "Content-Type: text/html\r\n"
           "Connection: Closed\r\n"
           "\r\n"
           "<h1>Hello Beautiful!</h1>\n");

  int bufflen = strlen(buff);
  write(connfd, buff, bufflen);
  close(connfd);
  close(sockfd);
  return 0;
}
