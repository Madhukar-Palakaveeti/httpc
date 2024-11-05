#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 6969
typedef struct {
  char *first_line;
  char *method;
  char *path;
} RequestParams;

RequestParams get_req_params(char *req_buf, size_t req_len) {
  RequestParams request_params;

  request_params.first_line = strtok(req_buf, "\n");
  request_params.method = strtok(request_params.first_line, " ");
  request_params.path = strtok(NULL, " ");

  return request_params;
}

void build_http_response(char *res_buf, RequestParams *request_params) {
  char *http_verb = request_params->method;
  char *path = request_params->path;

  if (strcmp(http_verb, "GET") == 0) {
    snprintf(res_buf, 1024,
             "HTTP/1.1 200 OK\r\n"
             "server: Http\r\n"
             "content-type: text/html\r\n"
             "connection: closed\r\n"
             "\r\n"
             "<h1>Hello Beautiful!</h1>\n");
  }

  else {
    snprintf(res_buf, 1024,
             "HTTP/1.1 201 Created\r\n"
             "Server: Http\r\n"
             "Content-Type: text/html\r\n"
             "Connection: Closed\r\n"
             "\r\n"
             "{'status' : 'Item Created'}\n");
  }
}
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

  printf("Listening on Port %d!\n", PORT);
  cli_len = sizeof(cli_addr);
  for (;;) {
    connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);
    if (connfd < 0) {
      printf("ERROR: Server not accepting!\n");
      exit(EXIT_FAILURE);
    }

    printf("Successfully connected to the server\n");

    char req[1024];
    size_t req_len = sizeof(req) - 1;
    ssize_t bytes_read = read(connfd, req, req_len);
    req[bytes_read] = '\0';

    RequestParams request_params = get_req_params(req, req_len);
    char *http_verb = request_params.method;
    char *path = request_params.path;

    char res[1024];
    if (strcmp(http_verb, "GET") == 0 && strcmp(path, "/quit") == 0) {
      snprintf(res, 1024,
               "HTTP/1.1 200 ok\r\n"
               "server: Http\r\n"
               "content-type: text/html\r\n"
               "connection: closed\r\n"
               "\r\n"
               "Connection closed from server side\n");
      write(connfd, res, strlen(res));
      close(connfd);
      printf("Connection closed!\n");
      break;
    }

    build_http_response(res, &request_params);
    int res_len = strlen(res);
    write(connfd, res, res_len);
    close(connfd);
  }
  close(sockfd);
  return 0;
}
