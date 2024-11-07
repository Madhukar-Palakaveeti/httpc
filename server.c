#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cjson/cJSON.h>

#define PORT 6969
typedef struct {
  int id;
  char *name;
  double price;
} Data;

Data data[10];
int row;

typedef struct {
  char *first_line;
  char *method;
  char *path;
  char *body;
} RequestParams;

RequestParams get_req_params(char *req_buf, size_t req_len) {
  RequestParams request_params;

  request_params.first_line = strtok(req_buf, "\n");
  for (int i = 0; i < 6; i++) {
    strtok(NULL, "\n");
  }
  request_params.body = strtok(NULL, "\n");

  request_params.method = strtok(request_params.first_line, " ");
  request_params.path = strtok(NULL, " ");
  return request_params;
}

int insert_into_db(const char *req_body) {
  cJSON *json = cJSON_Parse(req_body);
  if (json == NULL) {
    return -1;
  }

  cJSON *id = cJSON_GetObjectItem(json, "id");
  cJSON *name = cJSON_GetObjectItem(json, "name");
  cJSON *price = cJSON_GetObjectItem(json, "price");

  int db_id = id->valueint;

  data[db_id].id = id->valueint;
  data[db_id].name = strdup(name->valuestring);

  data[db_id].price = price->valuedouble;

  cJSON_Delete(json);
  return (db_id);
}

void build_http_response(char *res_buf, RequestParams *request_params,
                         int res_id) {
  char *http_verb = request_params->method;
  char *path = request_params->path;
  char *body = request_params->body;

  if (strcmp(http_verb, "GET") == 0) {
    if (strcmp(path, "/") == 0) {
      snprintf(res_buf, 1024,
               "HTTP/1.1 200 OK\r\n"
               "server: Http\r\n"
               "content-type: text/html\r\n"
               "connection: closed\r\n"
               "\r\n"
               "<h1>Hello Beautiful!</h1>\n");
    } else {
      int idpath = atoi(path + 1);
      if (idpath < 0) {
        snprintf(res_buf, 1024,
                 "HTTP/1.1 404 OK\r\n"
                 "server: Http\r\n"
                 "content-type: text/html\r\n"
                 "connection: closed\r\n"
                 "\r\n"
                 "{'Error' : 'Data Not Found'}");
      } else {
        snprintf(res_buf, 1024,
                 "HTTP/1.1 200 OK\r\n"
                 "server: Http\r\n"
                 "content-type: text/html\r\n"
                 "connection: closed\r\n"
                 "\r\n"
                 "{'id' : %d, 'name' : %s, 'price' : '%lf'}",
                 data[idpath].id, data[idpath].name, data[idpath].price);
      }
    }
  }

  else {
    snprintf(res_buf, 1024,
             "HTTP/1.1 201 Created\r\n"
             "Server: Http\r\n"
             "Content-Type: text/json\r\n"
             "Connection: Closed\r\n"
             "\r\n"
             "{'status' : 'Item Created', 'id' : %d}\n",
             res_id);
  }
}

int main() {
  int sockfd, connfd, cli_len;
  struct sockaddr_in my_addr, cli_addr;
  // initializing the db
  for (row = 0; row < 10; row++) {
    data[row].id = 0;
    data[row].name = NULL;
    data[row].price = 0;
  }
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

    // printf("%s\n", req);
    RequestParams request_params = get_req_params(req, req_len);
    char *http_verb = request_params.method;
    char *path = request_params.path;
    const char *body = request_params.body;

    int res_id = 0;
    char res[1024];

    // Graceful exit
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

    if (strcmp(http_verb, "POST") == 0 && body[0] != '\0') {
      res_id = insert_into_db(body);
    }

    build_http_response(res, &request_params, res_id);
    int res_len = strlen(res);
    write(connfd, res, res_len);

    close(connfd);
  }
  for (int i = 0; i < 10; i++) {
    if (data[i].name != NULL && data[i].name[0] != '\0') {
      free(data[i].name);
    }
  }
  close(sockfd);
  return 0;
}
