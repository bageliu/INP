#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#if TIME_WITH_SYS_TIME
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#else
#if HAVE_SYS_TIME_H
#include	<sys/time.h>	/* includes <time.h> unsafely */
#else
#include	<time.h>		/* old system? */
#endif
#endif
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */
#include    <string>
#include    <iostream>

# define MAXLINE 1024

void str_cli(FILE *fp, int sockfd);
ssize_t Readline(int fd, void *ptr, size_t maxlen);
void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv){

    int sockfd, udp_sockfd;
    struct sockaddr_in servaddr;
    if (argc != 3) {
        std::cout << "usage: tcpcli <IPaddress>\n";
        exit(0);
    }
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        std::cout << "socket error\n";
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    char *ptr, **pptr;
    char str[INET_ADDRSTRLEN];
    struct hostent *hptr;

    if((hptr = gethostbyname(argv[1])) == NULL){
        std::cout << "gethostbyname error\n";
        exit(0);
    }
    pptr = hptr->h_addr_list;
    inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
    //inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    inet_pton(AF_INET, inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)), &servaddr.sin_addr);


    dg_cli(stdin, sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)); /*udp*/

    exit(0);
    
}

void
dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen)
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];
    bool flag = true;
    std::string tmp = "1\n";
	while (1) {
        if(flag){
            sendto(sockfd, tmp.c_str(), tmp.size(), 0, pservaddr, servlen);
            flag = false;
        }
        bzero(recvline, MAXLINE);
		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
        if(n < 0){
            std::cout << "recvline error.\n";
            exit(0);
        }
        if(strlen(recvline) == 0)
            exit(0);
        std::cout << recvline;
        sendto(sockfd, recvline, strlen(recvline), 0, pservaddr, servlen);
	}
}


