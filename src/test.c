#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main() {
    struct addrinfo hints, *info, *p;
    int gai_result;
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if ((gai_result = getaddrinfo("google.com", 0, &hints, &info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
        exit(1);
    }

    for(p = info; p != NULL; p = p->ai_next) {
        printf("hostname: %s\n", p->ai_canonname);
    }

    freeaddrinfo(info);
}
