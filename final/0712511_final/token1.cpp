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
std::string s;
bool flag_3 = false;

void str_cli(FILE *fp, int sockfd);
void str_cli_2(FILE *fp, int sockfd);
ssize_t Readline(int fd, void *ptr, size_t maxlen);
void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen);

int main(int argc, char **argv){

    int sockfd, udp_sockfd;
    struct sockaddr_in servaddr, udpserv;
    if (argc != 3) {
        std::cout << "usage: tcpcli <IPaddress>\n";
        exit(0);
    }
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

    if(connect(sockfd, (struct  sockaddr*) &servaddr, sizeof(servaddr)) < 0){
        std::cout << "connection error.\n";
        exit(0);
    }
    if((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        std::cout << "udp socket create error.\n";
        exit(0);
    }
    bzero(&udpserv, sizeof(udpserv));
	udpserv.sin_family      = AF_INET;
	udpserv.sin_addr.s_addr = htonl(INADDR_ANY);
	udpserv.sin_port        = htons(12345);
    bind(udp_sockfd, (struct sockaddr*) &udpserv, sizeof(udpserv));
    /*while(true){
        str_cli(stdin, sockfd);

        dg_cli(stdin, udp_sockfd, (struct sockaddr*) &udpserv, sizeof(udpserv));
        exit(0);
        //str_cli(stdin, sockfd);

        //break;
    }*/
    str_cli(stdin, sockfd); /* tcp*/

    dg_cli(stdin, udp_sockfd, (struct sockaddr*) &udpserv, sizeof(udpserv)); /*udp*/
    std::cout << s;

    str_cli_2(stdin, sockfd);
    exit(0);
    
}

void
dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen)
{
	int	n, counts = 0;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	while (1) {
        bzero(recvline, MAXLINE);
		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
        if(n < 0){
            std::cout << "recvline error.\n";
        }
        //std::cout << recvline;
        s = "";
        for(int i = 0; i < strlen(recvline); i++){
            s += recvline[i];
        }
        s += '\n';
        return;
        //sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
	}
}

void 
str_cli_2(FILE *fp, int sockfd){
    char	sendline[MAXLINE], recvline[MAXLINE];
    std::string tmp;
    bool flag = false, flag_2 = false;
    int counts = 0;
    write(sockfd, s.c_str(), s.size());
	while (1) {
        bzero(recvline, MAXLINE);
		if (Readline(sockfd, recvline, MAXLINE) == 0){
            exit(0);
        }
        std::cout << recvline;
    }
}

void
str_cli(FILE *fp, int sockfd)
{
	char	sendline[MAXLINE], recvline[MAXLINE];
    std::string tmp;
    bool flag = false, flag_2 = false;
    int counts = 0;

	while (1) {
        if(flag_2){
            if(counts == 3){
                flag_2 = false;
                flag_3 = false;
                return;
            }
            counts++;
        }
        bzero(recvline, MAXLINE);
		if (Readline(sockfd, recvline, MAXLINE) == 0){
			std::cout << "str_cli: server terminated prematurely";
            exit(0);
        }
        std::cout << recvline;
        if(recvline[0] == 'E'){
            tmp = "12345\n";
            std::cout << tmp;
            write(sockfd, tmp.c_str(), tmp.size());
            flag_2 = true;
            counts++;
        }
        /*if(flag){
            if(counts == 1){
                tmp += '\n';
                std::cout << tmp;
                write(sockfd, tmp.c_str(), tmp.size());
                flag = false;
                counts = 0;
            }
            else{
                counts++;
            }
        }
        if(recvline[0] == 'S'){
            tmp = "";
            for(int i = 25; i < strlen(recvline); i++){
                if((recvline[i] >= 65 && recvline[i] <= 90) || (recvline[i] >= 97 && recvline[i] <= 122)){
                    tmp += recvline[i];
                }
                else{
                    flag = true;
                    break;
                }
            }
        }*/
        //write(sockfd, sendline, strlen(sendline));
	}
}

static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];

static ssize_t
my_read(int fd, char *ptr)
{

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
    char	c, *ptr;
    //char c;
    //(char*) ptr;
	ptr = (char*)vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

ssize_t
readlinebuf(void **vptrptr)
{
	if (read_cnt)
		*vptrptr = read_ptr;
	return(read_cnt);
}
/* end readline */

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		std::cout << "readline error\n";
	return(n);
}

