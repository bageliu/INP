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
int map[11][11];

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
	int	n, counts = 0, start_x, start_y, end_x, end_y, new_x, new_y;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];
    bool flag = true, flag_2 = false, flag_3 = false;
    std::string tmp = "\n";
	while (1) {
        if(flag){
            sendto(sockfd, tmp.c_str(), tmp.size(), 0, pservaddr, servlen);
            flag = false;
            flag_2 = true;
        }

        if(flag_3){
            std::cout << tmp;
            sendto(sockfd, tmp.c_str(), tmp.size(), 0, pservaddr, servlen);
            exit(0);
        }

        bzero(recvline, MAXLINE);
		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
        if(n < 0){
            std::cout << "recvline error.\n";
            exit(0);
        }
        std::cout << recvline;
        if(flag_2){
            tmp = "M\n";
            std::cout << tmp;
            sendto(sockfd, tmp.c_str(), tmp.size(), 0, pservaddr, servlen);
            flag_2 = false;
        }
        
        if(recvline[0] == '#'){
            //std::cout << recvline[11] << recvline[12] << recvline[13] << std::endl;
            for(int i = 12; i < strlen(recvline); i++){
                if(recvline[i] == '*'){
                    start_x = i - counts * 12 - 12;
                    start_y = counts;
                    //std::cout << "start: " << start_x << " " << start_y << std::endl;
                    //std::cout << strlen(recvline) << std::endl;
                }
                else if(recvline[i] == 'E'){
                    end_x = i - counts * 12 - 12;
                    end_y = counts;
                    //std::cout << "End: " << end_x << " " << end_y << std::endl;
                }
                if(recvline[i] == '#' && recvline[i + 1] == '\n'){
                    //std::cout << counts << std::endl;
                    counts++;
                }
            }

            tmp = "";
            new_x = end_x - start_x;
            new_y = end_y - start_y;
            if(new_x > 0){
                for(int i = 0; i < new_x; i++){
                    tmp += 'D';
                }
            }
            if(new_x < 0){
                for(int i = 0; i < (new_x * (-1)); i++){
                    tmp += 'A';
                }
            }

            if(new_y > 0){
                for(int i = 0; i < new_y; i++){
                    tmp += 'S';
                }
            }
            if(new_y < 0){
                for(int i = 0; i < (new_y * (-1)); i++){
                    tmp += 'W';
                }
            }
            tmp += '\n';
            //std::cout << tmp;
            flag_3 = true;
            sleep(1);
            //sendto(sockfd, tmp.c_str(), tmp.size(), 0, pservaddr, servlen);
        }
        //sendto(sockfd, recvline, strlen(recvline), 0, pservaddr, servlen);
	}
}


