#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <ctype.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include <poll.h>

#define HOST "34.254.242.81"
#define PORT 8080
#define TYPE "application/json"
#define MAX_COMMAND 20 /* max strlen for a command read as input */


void logoutClient(int sockfd, char *cookies) {
    char *message;
    char *response;

    message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, &cookies, 1, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* check whether the request was a success or not */
    if (strstr(response, "200 OK") != NULL) {
        printf("User logged out successfully\n");
    } else {
        char *error_start = strchr(response, '{');
        if (error_start == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error_end = strchr(response, '}');
        if (error_end == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error = calloc(LINELEN, sizeof(char));
        memcpy(error, error_start, error_end - error_start + 1);
        printf("%s\n", error);
        free(error);
    }

    free(message);
    free(response);
}

void deleteBookFromLibrary(int sockfd, char *token) {
    char *message;
    char *response;
    char *id = calloc(LINELEN, sizeof(char));

    printf("id=");
    fgets(id, LINELEN, stdin);
    if (id[strlen(id) - 1] == '\n')
        id[strlen(id) - 1] = '\0';
    int idInt = atoi(id);
    if (idInt == 0) {
        printf("ERROR: Invalid id\n");
        free(id);
        return;
    }

    char *url = calloc(LINELEN, sizeof(char));
    sprintf(url, "/api/v1/tema/library/books/%d", idInt);

    message = compute_delete_request(HOST, url, NULL, NULL, 0, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* check whether the request was a success or not */
    if (strstr(response, "200 OK") != NULL) {
        printf("Book deleted successfully\n");
    } else {
        char *error_start = strchr(response, '{');
        if (error_start == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error_end = strchr(response, '}');
        if (error_end == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error = calloc(LINELEN, sizeof(char));
        memcpy(error, error_start, error_end - error_start + 1);
        printf("%s\n", error);
        free(error);
    }

    free(id);
    free(url);

    free(message);
    free(response);
}


void addBookToLibrary(int sockfd, char *token) {
    char *message;
    char *response;

    char *title = calloc(LINELEN, sizeof(char));
    char *author = calloc(LINELEN, sizeof(char));
    char *genre = calloc(LINELEN, sizeof(char));
    char *publisher = calloc(LINELEN, sizeof(char));
    char *page_count = calloc(LINELEN, sizeof(char));

    /* we read the info about the book we want to add from stdin */
    printf("title=");
    fgets(title, LINELEN, stdin);
    if (title[strlen(title) - 1] == '\n')
        title[strlen(title) - 1] = '\0';
    printf("author=");
    fgets(author, LINELEN, stdin);
    if (author[strlen(author) - 1] == '\n')
        author[strlen(author) - 1] = '\0';
    printf("genre=");
    fgets(genre, LINELEN, stdin);
    if (genre[strlen(genre) - 1] == '\n')
        genre[strlen(genre) - 1] = '\0';
    printf("publisher=");
    fgets(publisher, LINELEN, stdin);
    if (publisher[strlen(publisher) - 1] == '\n')
        publisher[strlen(publisher) - 1] = '\0';
    printf("page_count=");
    fgets(page_count, LINELEN, stdin);
    if (page_count[strlen(page_count) - 1] == '\n')
        page_count[strlen(page_count) - 1] = '\0';
    
    /* we check if the page_count is valid */
    int page_countInt = atoi(page_count);
    if (page_countInt == 0) {
        printf("ERROR: Invalid page_count\n");
        free(title);
        free(author);
        free(genre);
        free(publisher);
        free(page_count);
        return;
    }

    /* we create a json object with the info read */
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_number(root_object, "page_count", page_countInt);
    char *serialized_string = json_serialize_to_string_pretty(root_value);
    puts(serialized_string);

    char **body = calloc(1, sizeof(char*));
    body[0] = calloc(LINELEN, sizeof(char));
    memcpy(body[0], serialized_string, strlen(serialized_string));

    message = compute_post_request(HOST, "/api/v1/tema/library/books", TYPE, body, 1, NULL, 0, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* check whether the request was a success or not */
    if (strstr(response, "200 OK") != NULL) {
        printf("Book added successfully\n");
    } else {
        char *error_start = strchr(response, '{');
        if (error_start == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error_end = strchr(response, '}');
        if (error_end == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error = calloc(LINELEN, sizeof(char));
        memcpy(error, error_start, error_end - error_start + 1);
        printf("%s\n", error);
        free(error);
    }

    free(title);
    free(author);
    free(genre);
    free(publisher);
    free(page_count);

    free(body[0]);
    free(body);

    free(message);
    free(response);

    free(serialized_string);
    json_value_free(root_value);
}

void bookInfo(int sockfd, char *token) {
    char *message;
    char *response;
    char *id = calloc(LINELEN, sizeof(char));

    printf("id=");
    fgets(id, LINELEN, stdin);
    if (id[strlen(id) - 1] == '\n')
        id[strlen(id) - 1] = '\0';

    int idInt = atoi(id);
    if (idInt == 0) {
        printf("ERROR: Invalid id\n");
        free(id);
        return;
    }

    char *url = calloc(LINELEN, sizeof(char));
    sprintf(url, "/api/v1/tema/library/books/%d", idInt);

    message = compute_get_request(HOST, url, NULL, NULL, 0, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* check whether the request was a success or not */
    if (strstr(response, "200 OK") != NULL) {
        /* we print the info about the book in json format */
        char *bookStart = strstr(response, "{");
        if (bookStart != NULL) {
            JSON_Value *root_value = json_parse_string(bookStart);
            char *book = json_serialize_to_string_pretty(root_value);
            printf("%s\n", book);
            free(book);
            json_value_free(root_value);
        }
    } else {
        char *error_start = strchr(response, '{');
        if (error_start == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error_end = strchr(response, '}');
        if (error_end == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error = calloc(LINELEN, sizeof(char));
        memcpy(error, error_start, error_end - error_start + 1);
        printf("%s\n", error);
        free(error);
    }

    free(id);
    free(url);

    free(message);
    free(response);
}

void booksLibrary(int sockfd, char *token) {
    char *message;
    char *response;

    /* we create the get request to get info on the all the books */
    message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, NULL, 0, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* check whether the request was a success or not */
    if (strstr(response, "200 OK") != NULL) {
        char *booksLibraryStart = strstr(response, "[{");
        if (booksLibraryStart != NULL) {
            JSON_Value *root_value = json_parse_string(booksLibraryStart);
            JSON_Array *booksArray = json_value_get_array(root_value);
            int booksCount = json_array_get_count(booksArray);
            for (int i = 0; i < booksCount; i++) {
                JSON_Object *bookObject = json_array_get_object(booksArray, i);
                int id = json_object_get_number(bookObject, "id");
                char const *title = json_object_get_string(bookObject, "title");
                printf("index %d: id=%d, title=%s\n", i, id, title);
            }
            json_value_free(root_value);
        }
    } else {
        char *error_start = strchr(response, '{');
        if (error_start == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error_end = strchr(response, '}');
        if (error_end == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error = calloc(LINELEN, sizeof(char));
        memcpy(error, error_start, error_end - error_start + 1);
        printf("%s\n", error);
        free(error);
    }

    free(message);
    free(response);
}



char *accessLibrary(int sockfd, char *cookies) {
    char *message;
    char *response;

    /* we create the get request to gain access to the library, we use the cookies that we
    saved */
    message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, &cookies, 1, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* check whether the request was a success or not */
    if (strstr(response, "200 OK") != NULL) {
        printf("We have access to library\n");
    } else {
        char *error_start = strchr(response, '{');
        if (error_start == NULL) {
            printf("ERROR: Invalid response\n");
            return NULL;
        }
        char *error_end = strchr(response, '}');
        if (error_end == NULL) {
            printf("ERROR: Invalid response\n");
            return NULL;
        }
        char *error = calloc(LINELEN, sizeof(char));
        memcpy(error, error_start, error_end - error_start + 1);
        printf("%s\n", error);

        free(error);
        free(message);
        free(response);

        return NULL;
    }

    /* if the request was a success, we get the token from the response,
    and save it for future access to the library */
    char *token = calloc(LINELEN, sizeof(char));
    char *token_start = strstr(response, "token");
    if (token_start != NULL) {
        token_start = strchr(token_start, ':');
        token_start += 2;
        char *token_end = strchr(token_start, '}');
        token_end--;
        memcpy(token, token_start, token_end - token_start);
    }

    free(message);
    free(response);

    return token;
}

char *loginClient(int sockfd) {
    char *username = calloc(LINELEN, sizeof(char));
    char *password = calloc(LINELEN, sizeof(char));
    char *message;
    char *response;

    /* read credentials */
    printf("username=");
    fgets(username, LINELEN, stdin);
    if (username[strlen(username) - 1] == '\n')
        username[strlen(username) - 1] = '\0';
    printf("password=");
    fgets(password, LINELEN, stdin);
    if (password[strlen(password) - 1] == '\n')
        password[strlen(password) - 1] = '\0';

    /* create the json object containing the credentials */
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *serialized_string = json_serialize_to_string_pretty(root_value);

    /* create the post request to login */
    char **body = calloc(1, sizeof(char*));
    body[0] = calloc(LINELEN, sizeof(char));
    memcpy(body[0], serialized_string, strlen(serialized_string));

    message = compute_post_request(HOST, "/api/v1/tema/auth/login", TYPE, body, 1, NULL, 0, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* if the request was a success than we'll find the Cookies header in the
    response */
    char *cookies = calloc(LINELEN, sizeof(char));
    char *cookiesStart = strstr(response, "connect.sid");
    if (cookiesStart != NULL) {
        char *cookiesEnd = strchr(cookiesStart, ';');
        memcpy(cookies, cookiesStart, cookiesEnd - cookiesStart);
        printf("User %s logged in successfully\n", username);
    } else {
        /* the request wasn't a success, we print the error in the response
        to stdout */
        char *error_start = strchr(response, '{');
        if (error_start == NULL) {
            printf("ERROR: Invalid response\n");
            return NULL;
        }
        char *error_end = strchr(response, '}');
        if (error_end == NULL) {
            printf("ERROR: Invalid response\n");
            return NULL;
        }
        char *error = calloc(LINELEN, sizeof(char));
        memcpy(error, error_start, error_end - error_start + 1);
        printf("%s\n", error);
        free(error);

        free(username);
        free(password);

        free(body[0]);
        free(body);

        free(message);
        free(response);

        free(serialized_string);
        json_value_free(root_value);

        free(cookies);

        return NULL;
    }

    free(username);
    free(password);

    free(body[0]);
    free(body);

    free(message);
    free(response);

    free(serialized_string);
    json_value_free(root_value);

    return cookies;
}

void registerClient(int sockfd) {
    char *username = calloc(LINELEN, sizeof(char));
    char *password = calloc(LINELEN, sizeof(char));
    char *message;
    char *response;

    /* read credentials */
    printf("username=");
    fgets(username, LINELEN, stdin);
    if (username[strlen(username) - 1] == '\n')
        username[strlen(username) - 1] = '\0';
    printf("password=");
    fgets(password, LINELEN, stdin);
    if (password[strlen(password) - 1] == '\n')
        password[strlen(password) - 1] = '\0';

    /* create the json object containing the credentials using
    the parson library recommended */
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *serialized_string = json_serialize_to_string_pretty(root_value);

    /* create the post request */
    char **body = calloc(1, sizeof(char*));
    body[0] = calloc(LINELEN, sizeof(char));
    memcpy(body[0], serialized_string, strlen(serialized_string));

    message = compute_post_request(HOST, "/api/v1/tema/auth/register", TYPE, body, 1, NULL, 0, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    /* analyse the response sent by the server and print to
    stdout a relevant message */
    if (strstr(response, "201 Created") != NULL) {
        printf("User %s registered successfully\n", username);
    } else {
        /* in case the user couldn't be registered, I print
        to stdout the message included in the response from the 
        server, since I don't know all the instances each possible error
        could happen */
        char *error_start = strchr(response, '{');
        if (error_start == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error_end = strchr(response, '}');
        if (error_end == NULL) {
            printf("ERROR: Invalid response\n");
            return;
        }
        char *error = calloc(LINELEN, sizeof(char));
        memcpy(error, error_start, error_end - error_start + 1);
        printf("%s\n", error);
        free(error);
    }

    /* free memory */
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    free(body[0]);
    free(body);

    free(message);
    free(response);

    free(username);
    free(password);
}



int main(int argc, char *argv[])
{
    int sockfd;
    char line[MAX_COMMAND];
    char *cookies = NULL;
    char *token = NULL;

    while (1) {
        /* open connection with server */
        sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Cannot open connection");
            exit(EXIT_FAILURE);
        }

        /* read command from stdin */
        fgets(line, MAX_COMMAND, stdin);
        if (isspace(line[0])) {
            /* if we read an empty space */
            continue;
        } else if (strncmp(line, "register", 8) == 0) {
            registerClient(sockfd);
        } else if (strncmp(line, "exit", 4) == 0) {
            if (cookies != NULL)
                free(cookies);
            if (token != NULL)
                free(token);
            close_connection(sockfd);
            return 0;
        } else if (strncmp(line, "login", 5) == 0) {
            if (cookies != NULL) {
                printf("You are already logged in\n");
                continue;
            }
            cookies = loginClient(sockfd);
        } else if (strncmp(line, "enter_library", 13) == 0) {
            if (cookies == NULL) {
                printf("You must login first before accessing the library\n");
                close_connection(sockfd);
                continue;
            }
            token = accessLibrary(sockfd, cookies);
        } else if (strncmp(line, "get_books", 9) == 0) {
            if (cookies == NULL) {
                printf("You must login first before accessing the library\n");
                close_connection(sockfd);
                continue;
            } else if (token == NULL) {
                printf("You must enter the library first before accessing the books\n");
                close_connection(sockfd);
                continue;
            } 
            booksLibrary(sockfd, token);
        } else if (strncmp(line, "get_book", 8) == 0) {
            if (cookies == NULL) {
                printf("You must login first before accessing the library\n");
                close_connection(sockfd);
                continue;
            } else if (token == NULL) {
                printf("You must enter the library first before accessing the books\n");
                close_connection(sockfd);
                continue;
            } 
            bookInfo(sockfd, token);
        } else if (strncmp(line, "add_book", 8) == 0) {
            if (cookies == NULL) {
                printf("You must login first before accessing the library\n");
                close_connection(sockfd);
                continue;
            } else if (token == NULL) {
                printf("You must enter the library first before accessing the books\n");
                close_connection(sockfd);
                continue;
            } 
            addBookToLibrary(sockfd, token);
        } else if (strncmp(line, "delete_book", 11) == 0) {
            if (cookies == NULL) {
                printf("You must login first before accessing the library\n");
                close_connection(sockfd);
                continue;
            } else if (token == NULL) {
                printf("You must enter the library first before accessing the books\n");
                close_connection(sockfd);
                continue;
            } 
            deleteBookFromLibrary(sockfd, token);
        } else if (strncmp(line, "logout", 6) == 0) {
            if (cookies == NULL) {
                printf("You must login first before logging out\n");
                close_connection(sockfd);
                continue;
            } else {
                logoutClient(sockfd, cookies);
                free(cookies);
                cookies = NULL;
                if (token != NULL) {
                    /* daca deja accesasem libraria */
                    free(token);
                    token = NULL;
                }
            }
        } else {
            printf("Invalid command\n");
        }
        close_connection(sockfd);
    }
    return 0;
}
