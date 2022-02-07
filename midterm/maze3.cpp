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
#include <bits/stdc++.h>

#include<cstdio>
#include<cstring>
#include<algorithm>
#include<queue>
#include<stack>
#include<iostream>

#define	SA	struct sockaddr
#define MAXLINE 1500
using namespace std;

void str_cli(FILE *fp, int sockfd);
ssize_t	 Readline(int, void *, size_t);

int main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 3){
		cout << ("usage: tcpcli <IPaddress> <IPport>\n");
        return 0;
    }

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));

    char *ptr, **pptr;
    char str[INET_ADDRSTRLEN];
    struct hostent *hptr;


    hptr = gethostbyname(argv[1]);
    pptr = hptr->h_addr_list;
    inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));

	inet_pton(AF_INET, str, &servaddr.sin_addr);

	connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	str_cli(stdin, sockfd);		/* do it all */

	exit(0);
}

int graph[101][101];
int visited[101][101], dist[101][101];
int dr[] = {-1, 1, 0, 0};
int dc[] = {0, 0, -1, 1};
struct Node{
    int x, y;
    Node(int x, int y):x(x),y(y) {}
    Node(){}
}pre[101][101];
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
            if(nx >= 0 && nx < 101 && ny >= 0 && ny < 101 && visited[nx][ny] == 0 && graph[nx][ny] == 0){
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

void str_cli(FILE *fp, int sockfd)
{
	char	sendline[MAXLINE], recvline[MAXLINE];
    int start_x, start_y, end_x, end_y, now_x, now_y, count_x = 0;
    string tmp, tmp2;
    bool flag = true, flag_2 = true, flag_3 = true, flag_4 = true, flag_5 = true, flag_6 = true;
    bzero(sendline, MAXLINE);
	while (1) {
        bzero(recvline, MAXLINE);
        if (Readline(sockfd, recvline, MAXLINE) == 0)
        {
            cout << ("str_cli: server terminated prematurely\n");
            return;
        }
			
        size_t k = strlen(recvline);
        cout << recvline;
        //cout << tmp2;
        if(recvline[3] == '-' && flag){
            tmp = "IIIIIIIIIIII";
            if(recvline[7] == ' '){
                tmp += "JJJJJ";
            }
            tmp += "\n";
            cout << tmp;
            strcpy(sendline, tmp.c_str());
            write(sockfd, sendline, strlen(sendline));
            continue;
        }
        if((recvline[7] == '#' || recvline[7] == '.') && flag){
            tmp2 = recvline;
            tmp = "";
            for(size_t i = 0; i < k; i++){
                if(isdigit(recvline[i])){
                    tmp += recvline[i];
                }
            }
            now_x = stoi(tmp);
            flag = false;
            continue;
        }
        if(recvline[0] == 'E' && flag_2){
            tmp = "";
            for(int i = 0; i < now_x; i++)
                tmp += 'I';
            tmp += '\n';
            cout << tmp;
            strcpy(sendline, tmp.c_str());
            write(sockfd, sendline, strlen(sendline));
            flag_2 = false;
        }
        if(recvline[8] == '#' || recvline[8] == '.' || recvline[8] == 'E' || recvline[8] == '*'){
            if(recvline[7] == ' ')
                flag_3 = false;
        }
        if(!flag_2 && flag_3 && recvline[0] == 'E'){
            tmp = "J\n";
            cout << "J";
            strcpy(sendline, tmp.c_str());
            write(sockfd, sendline, strlen(sendline));
        }
        if(!flag_2 && !flag_3 && flag_4 && recvline[0] == 'E'){
            tmp = "LL\n";
            cout << "LL";
            flag_4 = false;
            now_x = 0, now_y = 0;
            strcpy(sendline, tmp.c_str());
            write(sockfd, sendline, strlen(sendline));
            continue;
        }
        else if(!flag_4 && flag_5 && recvline[7] != ' '){
            int col = now_y;
            for(int i = 7; i < 18; i++){
                if(recvline[i] == '#')
                    graph[now_x][now_y++] = 1;
                else if(recvline[i] == '.')
                    graph[now_x][now_y++] = 0;
                else if(recvline[i] == '*'){
                    start_x = now_x;
                    start_y = now_y;
                    graph[now_x][now_y++] = 0;
                }
                else if(recvline[i] == 'E'){
                    end_x = now_x;
                    end_y = now_y;
                    graph[now_x][now_y++] = 0;
                }
            }
            if(recvline[7] == '#' || recvline[7] == '.' || recvline[7] == 'E' || recvline[7] == '*'){
                now_y = col;
                if(now_x++ == (6 + count_x)) flag_5 = false;
            }
        }
        else if(!flag_5 && flag_6 && recvline[k - 2] == '>'){
            tmp = "LLLLLLLLLLL\n";
            now_x = count_x, now_y += 11;
            if(now_y > 100){
                count_x += 7;
                now_x = count_x;
                now_y = 0;
                tmp = "";
                for(int i = 0; i < 9; i++)
                    tmp += "JJJJJJJJJJJ";
                if(now_x == 98){
                    tmp += "KKK\n";
                    count_x -= 4;
                }
                else{
                    tmp += "KKKKKKK\n";
                }
                if(count_x >= 101){
                    flag_6 = false;
                }
            }
            if(flag_6){
                flag_5 = true;
                cout << tmp;
                strcpy(sendline, tmp.c_str());
                write(sockfd, sendline, strlen(sendline));
            }
            else{
                tmp = "R\n";
                cout << tmp;
                strcpy(sendline, tmp.c_str());
                write(sockfd, sendline, strlen(sendline));
            }
        }
        else if(!flag_6 && (recvline[k - 2] == '>')){
            BFS(start_x, start_y, end_x, end_y);
            stack<Node> S;
            int cur_x = end_x, cur_y = end_y;
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
            printf("%s", tmp.c_str());
            strcpy(sendline, tmp.c_str());
            write(sockfd, sendline, strlen(sendline));
            //break;
        }
	
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
	size_t	n, rc;
	char	c, *ptr;

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

	ptr = 0;	/* null terminate like fgets() */
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
		cout << ("readline error");
	return(n);
}


