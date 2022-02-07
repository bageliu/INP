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
#include    <bits/stdc++.h>
#include    <string>
#include    <iostream>
#include    <stack>
#include    <cstdio>
#include    <cstring>
#include    <algorithm>
#include    <queue>

using namespace std;


# define MAXLINE 1500
//int map[15][76];

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

int graph[15][76];
int visited[15][76], dist[15][76];
int dr[] = {-1, 1, 0, 0};
int dc[] = {0, 0, -1, 1};
struct Node{
    int x, y;
    Node(int x, int y):x(x),y(y) {}
    Node(){}
}pre[15][76];
queue<Node> Q;
void BFS(int start_x, int start_y, int end_x, int end_y){
    while(!Q.empty()) Q.pop();
    memset(visited, 0, sizeof(visited));
    dist[start_x][start_y] = 0;
    visited[start_x][start_y] = 1;
    Q.push(Node(start_x, start_y));
    while(!Q.empty()){
        Node node = Q.front();
        Q.pop();
        int x = node.x; 
        int y = node.y;
        for(int d = 0; d < 4; d++){
            int nx = x + dr[d];
            int ny = y + dc[d];
            if(nx >= 0 && nx < 15 && ny >= 0 && ny < 75 && visited[nx][ny] == 0 && graph[nx][ny] == 0){
                visited[nx][ny] = 1;
                dist[nx][ny] = 1 + dist[x][y];
                Q.push(Node(nx, ny));
                pre[nx][ny] = Node(x, y);
                if(nx == end_x && ny == end_y) {
                    //cout<< nx << " " << ny << endl;
                    return;
                }
            }
        }
    }
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
            //exit(0);
            for(int i = 0; i < strlen(recvline); i++){
                if(recvline[i] == '#'){
                    graph[counts][i - counts * 76] = 1;
                }
                else if(recvline[i] == '.'){
                    graph[counts][i - counts * 76] = 0;
                }
                else if(recvline[i] == '*'){
                    graph[counts][i - counts * 76] = 0;
                    start_y = i - counts * 76;
                    start_x = counts;
                    //std::cout << start_x << " " << start_y << std::endl;
                }
                else if(recvline[i] == 'E'){
                    graph[counts][i - counts * 76] = 0;
                    end_y = i - counts * 76;
                    end_x = counts;
                    //std::cout << end_x << " " << end_y << std::endl;
                }
                else if(recvline[i] == '\n'){
                    //std::cout << i;
                    counts++;
                }
            }
            BFS(start_x, start_y, end_x, end_y);
            std::stack<Node> S;
            int cur_x = end_x, cur_y = end_y;
            //cout << cur_x << cur_y << endl;
            while(true)
            {
                S.push(Node(cur_x,cur_y));
                if(cur_x==start_x&&cur_y==start_y) break;
                int x = cur_x, y = cur_y;
                cur_x=pre[x][y].x;
                cur_y=pre[x][y].y;
            }
            
            int tmp_x = start_x, tmp_y = start_y;
            tmp = "";
            while(!S.empty())
            {
                Node node=S.top(); S.pop();
                //printf("(%d %d) ", node.x, node.y);
                tmp_x = node.x - tmp_x, tmp_y = node.y - tmp_y;
                //printf("(%d %d) ", tmp_x, tmp_y);
                if(tmp_x > 0)
                    tmp += 'S';
                else if(tmp_x < 0)
                    tmp += 'W';
                else if(tmp_y > 0)
                    tmp += 'D';
                else if(tmp_y < 0)
                    tmp += 'A';
                tmp_x = node.x, tmp_y = node.y;
            }
            tmp += '\n';
            flag_3 = true;
            
            sleep(1);
        }
        //sendto(sockfd, recvline, strlen(recvline), 0, pservaddr, servlen);
	}
}


