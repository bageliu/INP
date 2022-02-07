#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <vector>
#include <ctime>
#include <string>
#include <arpa/inet.h>

typedef class talkerInfo
{
    public:
        int port;
        int version;
        std::string username;
        struct sockaddr_in cliaddr;
        bool ban_flag;
        talkerInfo(){}
        talkerInfo(int a, int b, std::string c, bool d):port(a), version(b), username(c), ban_flag(d){}
} talkerInfo;


struct user{
    std::string username;
    std::string password;
    bool flag = false;
    bool ban_flag = false;
    int times = 0;
    int port;
};

struct state{
    std::string username;
    std::string board_name;
    bool flag = false;
};

state client_state[10];
std::vector<user> user_info;
std::string s;
std::vector<std::string> chat_message;
talkerInfo VtalkerInfo[10];
int room_port, room_version;
bool chat_flag = false;
std::string message_filter[9] = {"how", "you", "or","pek0", "tea", "ha", "kon", "pain", "Starburst Stream"};
std::string base64_info = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz"
                        "0123456789+/";

void register_handler(std::string username, std::string passwd, int sockfd){
    for(int i = 0; i < user_info.size(); i++){
        if(!strcmp(user_info[i].username.c_str(), username.c_str())){
            s = "Username is already used.\n";
            send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
    }
    int user_count = user_info.size();
    user info;
    info.username = username;
    info.password = passwd;
    user_info.push_back(info);
    s = "Register successfully.\n";
    send(sockfd, s.c_str(), s.size(), 0);

}

void login_handler(std::string username, std::string passwd, int sockfd, int j){
    for(int i = 0; i < user_info.size(); i++){
        if(!strcmp(user_info[i].username.c_str(), username.c_str())){
            //std::cout << client_state[j].flag << " " << user_info[i].flag << " " << username << std::endl;
            if(client_state[j].flag || user_info[i].flag){
                s = "Please logout first.\n";
                send(sockfd, s.c_str(), s.size(), 0);
                return;
            }
            else{
                if(!strcmp(user_info[i].password.c_str(), passwd.c_str())){
                    user_info[i].flag = true;
                    client_state[j].username = username;
                    client_state[j].flag = true;
                    if(user_info[i].times >= 3){
                        //user_info[i].flag = false;
                        client_state[j].username = "";
                        client_state[j].flag = false;
                        s = "We don't welcome "+ username + "!\n";
                        send(sockfd, s.c_str(), s.size(), 0);
                        return;
                    }
                    s = "Welcome, " + username + ".\n";
                    send(sockfd, s.c_str(), s.size(), 0);
                    return;
                }
            }
            break;
        }
    }

    s = "Login failed.\n";
    send(sockfd, s.c_str(), s.size(), 0);
}

void logout_handler(int sockfd, int j){
    int index = -1;
    for(int i = 0; i < user_info.size(); i++){
        if(!strcmp(user_info[i].username.c_str(), client_state[j].username.c_str())){
            index = i;
            break;
        }
    }
    //std::cout << index << std::endl;
    if(client_state[j].flag){
        s = "Bye, " + client_state[j].username + ".\n";
        send(sockfd, s.c_str(), s.size(), 0);
        client_state[j].username = "";
        client_state[j].flag = false;
        user_info[index].flag = false;
    }
    else{
        s = "Please login first.\n";
        send(sockfd, s.c_str(), s.size(), 0);
    }
}

//check whether the input is number
bool isNumber(const std::string& s){
    for (char const &ch : s) {
        if (std::isdigit(ch) == 0) 
            return false;
    }
    return true;
}

int findClientIdx(struct sockaddr_in target)
{
    for (int i = 0 ; i < 10 ; i++)
    {
        // if (htons(VtalkerInfo[i].cliaddr.sin_port) == target)
        if (target.sin_port == VtalkerInfo[i].cliaddr.sin_port)
            return i;
    }
    return -1;
}

void storePort(int sockfd, std::string username, int port, int version, int clientIdx)
{
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    int tmpForPort;
    talkerInfo tmp(port, version, username, true);
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    cliaddr.sin_port = htons(port);
    tmp.cliaddr = cliaddr;
    //tmp.username = username;
    VtalkerInfo[clientIdx] = tmp;
}

void enter_chat_room_handler(std::string command[100], int counts, int sockfd, int j){
    std::string port_input = command[1];
    std::string version_input = command[2];
    version_input = version_input.substr(0, version_input.length() - 1);
    int port, version;
    //std::cout << port_input << " " << version_input << std::endl;
    //handle the wrong input format
    if(isNumber(port_input)){
        port = stoi(port_input);
        if(port < 1 || port > 65535){
            s = "Port " + port_input + " is not valid.\n";
            send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
    }
    else{
        s = "Port " + port_input + " is not valid.\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }
    if(isNumber(version_input)){
        version = stoi(version_input);
        if(version < 1 || version > 65535){
            s = "Version " + version_input + " is not supported.\n";
            send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
    }
    else{
        s = "Version " + version_input + " is not supported.\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }
    if(!client_state[j].flag){
        s = "Please login first.\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }

    //handle chat room
    printf("%d", room_port);
    room_port = port, room_version = version;
    s = "Welcome to public chat room.\nPort:" + port_input + "\nVersion:" + version_input + "\n";
    storePort(sockfd, client_state[j].username, port, version, j);
    //send(sockfd, s.c_str(), s.size(), 0);
    chat_flag = true;

    //do not add history already
    for(int i = 0; i < chat_message.size(); i++){
        s += chat_message[i];
    }
    send(sockfd, s.c_str(), s.size(), 0);
    return;
}

//tcp chat message
void chat_message_handler(std::string command[100], int counts, int sockfd, int j){
    s = client_state[j].username + ":";
    std::string message = "";
    for(int i = 1; i < counts; i++){
        if(i == counts - 1)
            message += command[i];
        else
            message += command[i] + " ";
    }

    //string filter
    for(int i = 0; i < 9; i++){
        int found = message.find(message_filter[i]);
        //std::cout << found << std::endl;
        if (found != -1) {
            for(int k = found; k < found + message_filter[i].size(); k++){
                message[k] = '*';
            }
        }
    }

    //record the content filter times and handle somone who offend the rule
    for(int i = 0; i < user_info.size(); i++){
        if(!strcmp(user_info[i].username.c_str(), client_state[j].username.c_str())){
            user_info[i].times++;
            if(user_info[i].times == 3){
                s = "Bye," + user_info[i].username + ".\n";
                send(sockfd, s.c_str(), s.size(), 0);
                client_state[j].username = "";
                client_state[j].flag = false;
                return;
            }
            break;
        }
    }

    //retrun and save the message
    s += message;
    chat_message.push_back(s);
    s = send(sockfd, s.c_str(), s.size(), 0);
    return;
}

bool msg_filter(std::string &msg, int clientidx){
    bool flag = false;
    for(int i = 0; i < 9; i++){
        int found = msg.find(message_filter[i]);
        //std::cout << found << std::endl;
        if (found != -1) {
            for(int k = found; k < found + message_filter[i].size(); k++){
                msg[k] = '*';
                flag = true;
            }
        }
    }
    return flag;
}

std::string from_base64(std::string const &data) 
{
    int counter = 0;
    uint32_t bit_stream = 0;
    std::string decoded = "";
    int offset = 0;
    for (unsigned char c : data) {
        auto num_val = base64_info.find(c);
        if (num_val != std::string::npos) {
            offset = 18 - counter % 4 * 6;
            bit_stream += num_val << offset;
            if (offset == 12) {
                decoded += static_cast<char>(bit_stream >> 16 & 0xff);
            }
            if (offset == 6) {
                decoded += static_cast<char>(bit_stream >> 8 & 0xff);
            }
            if (offset == 0 && counter != 4) {
                decoded += static_cast<char>(bit_stream & 0xff);
                bit_stream = 0;
            }
        } 
        else if (c != '=') {
            return std::string();
        }
        counter++;
    }
    return decoded;
}

std::string to_base64(std::string const &data) 
{
    int counter = 0;
    uint32_t bit_stream = 0;
    std::string encoded = "";
    int offset = 0;
    for (unsigned char c : data) {
        auto num_val = static_cast<unsigned int>(c);
        offset = 16 - counter % 3 * 8;
        bit_stream += num_val << offset;
        if (offset == 16) {
        encoded += base64_info.at(bit_stream >> 18 & 0x3f);
        }
        if (offset == 8) {
        encoded += base64_info.at(bit_stream >> 12 & 0x3f);
        }
        if (offset == 0 && counter != 3) {
        encoded += base64_info.at(bit_stream >> 6 & 0x3f);
        encoded += base64_info.at(bit_stream & 0x3f);
        bit_stream = 0;
        }
        counter++;
    }
    if (offset == 16) {
        encoded += base64_info.at(bit_stream >> 12 & 0x3f);
        encoded += "==";
    }
    if (offset == 8) {
        encoded += base64_info.at(bit_stream >> 6 & 0x3f);
        encoded += '=';
    }
    return encoded;
}

void msg_v1(unsigned char* buffer, std::string &name, std::string &msg){
    uint16_t name_len, msg_len;
    int i;
    name = "";
    msg = "";

    name_len = ((uint16_t)buffer[2] << 8 | buffer[3]);
    for (i = 4; i < 4 + name_len; i++)
    {
        name += buffer[i];
    }
    msg_len = ((uint16_t)buffer[i] << 8 | buffer[i+1]);
    i += 2;
    msg = "";
    for (; i < 6+name_len+msg_len ; i ++)
    {
        msg += buffer[i];
    }
}

void msg_v2(unsigned char* buffer, std::string &name, std::string &msg){
    int i;
    std::string name_tmp = "", msg_tmp = "";
    bool flag = true;

    for (i = 2 ; i < strlen((char*)buffer) ; i++)
    {
        if (flag)
        {
            if (buffer[i] == '\n')
                flag = false;
            else
                name_tmp += buffer[i];
        }
        else
        {
            if (buffer[i] == '\n')
                break;
            msg_tmp += buffer[i];
        }
    }  
    name = from_base64(name_tmp);
    msg = from_base64(msg_tmp);
}

struct a 
{
    unsigned char flag;
    unsigned char version;
    unsigned char payload[0];
} __attribute__((packed));

struct b 
{
    unsigned short len;
    unsigned char data[0];
} __attribute__((packed));

void packetVer1(std::string &name, std::string &chatMsg, unsigned char *buf)
{
    uint16_t name_len    = (uint16_t)name.length();
    uint16_t chatMsg_len = (uint16_t)chatMsg.length();

    struct a *pa  = (struct a*) buf;
    struct b *pb1 = (struct b*) (buf + sizeof(struct a));
    struct b *pb2 = (struct b*) (buf + sizeof(struct a) + sizeof(struct b) + name_len);
    pa->flag = 0x01;
    pa->version = 0x01;
    pb1->len = htons(name_len);
    memcpy(pb1->data, name.c_str(), name_len);
    pb2->len = htons(chatMsg_len);
    memcpy(pb2->data, chatMsg.c_str(), chatMsg_len);
}

void packetVer2(std::string name, std::string chatMsg, char *buf)
{
    name = to_base64(name);
    chatMsg = to_base64(chatMsg);

    sprintf(buf, "\x01\x02%s\n%s\n", name.c_str(), chatMsg.c_str());
}

void sendtoall(int sockfd_udp, std::string name, std::string chatMsg, int clientIdx, bool flag)
{
    int port, version, idx = -1;
    std::string cliname;
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    unsigned char buf_1[4096];
    char buf_2[4096], buf[10];

    /*for(int j = 0; j < user_info.size(); j++){
        if(flag && !strcmp(user_info[j].username.c_str(), name.c_str())){
            if(user_info[j].times >= 3){
                return;
            }
       }
    }*/

    for(int i = 0 ; i < 10; i++)
    {
        version = VtalkerInfo[i].version;
        cliaddr = VtalkerInfo[i].cliaddr;
        cliname = VtalkerInfo[i].username;
        if(flag && !strcmp(cliname.c_str(), name.c_str())){
            for(int j = 0; j < user_info.size(); j++){
                if(!strcmp(user_info[j].username.c_str(), name.c_str())){
                    user_info[j].times++;
                    idx = j;
                    break;
                }
            }
            if(user_info[idx].times >= 3){

                continue;
            }
        }

        if(version == 1) {
            packetVer1(name, chatMsg, buf_1);
            sendto(sockfd_udp, buf_1, 6+name.length()+chatMsg.length(), 0, (struct sockaddr*)&cliaddr, len);
        }
        else if(version == 2) {
            packetVer2(name, chatMsg, buf_2);
            sendto(sockfd_udp, buf_2, strlen(buf_2), 0, (struct sockaddr*)&cliaddr, len);
        }
    }
}

int main(int argc, char *argv[]){
    int i, max_client = 10, client_socket[10], maxi, maxfd, nready, connfd, sockfd, servfd, input_result;  
    char input[1000]; 
    if(argc < 2){
        std::cout << "server usage: ./hw3 [port number]\n";
        exit(EXIT_FAILURE);
    }

    servfd = socket(AF_INET, SOCK_STREAM, 0);
    if(servfd == 0){
        std::cout << "Failed to create socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    //let the program can reuse the port
    int opt = 1;
    setsockopt(servfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in sockaddr, cliaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(atoi(argv[1]));

    if(bind(servfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0){
        std::cout << "Failed to bind to the port " << argv[1] << std::endl;
        exit(EXIT_FAILURE);
    }

    if(listen(servfd, 10) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //initialize udp socket
    int udp_sockfd;
    //struct sockaddr_in udp_servaddr, udp_cliaddr;
    if ( (udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("udp socket creation failed");
        exit(EXIT_FAILURE);
    }

    bind(udp_sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));


    //set socket descriptor
    fd_set rset, allset;

    //initialize the state
    maxfd = servfd;
    maxi = -1;
    for(i = 0; i < max_client; i++){
        client_socket[i] = -1;
    }

    FD_ZERO(&allset);
	FD_SET(servfd, &allset);
    FD_SET(udp_sockfd, &allset);
    /*
    auto addrlen = sizeof(sockaddr);
    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)addrlen);
    if(connection < 0){
        std::cout << "Failed to grab the connection." << std::endl;
        exit(EXIT_FAILURE);
    }*/

    while(true){
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        //handle udp chat room meg packet
        if(FD_ISSET(udp_sockfd, &rset)){
            int clientidx, version;
            if (chat_flag){ 
                struct sockaddr_in udp_cliaddr;
                socklen_t len = sizeof(udp_cliaddr);    
                unsigned char buffer[1024];              
                std::string name, msg, new_msg;  
                bool flag_check = true;       
                bzero(buffer, sizeof(buffer));
                recvfrom(udp_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&udp_cliaddr, &len);
                clientidx = findClientIdx(udp_cliaddr);                      
                version = VtalkerInfo[clientidx].version;              

                //version 1            
                if(version == 1){
                    msg_v1(buffer, name, msg);
                }
                //version 2
                else if(version == 2){
                    msg_v2(buffer, name, msg);
                }
                bool filter_flag = msg_filter(msg, clientidx);
                //handle message to each chatroom
                for(int i = 0; i < user_info.size(); i++){
                    if(!strcmp(user_info[i].username.c_str(), name.c_str())){
                        if(user_info[i].times >= 3){
                            flag_check = false;
                            break;
                        }
                    }
                }
                if(flag_check){
                    sendtoall(udp_sockfd, name, msg, clientidx, filter_flag);

                    new_msg = name + ":" + msg + '\n';
                    chat_message.push_back(new_msg);
                } 
            }
        }



        if(FD_ISSET(servfd, &rset)){
            socklen_t client_len = sizeof(cliaddr);
            connfd = accept(servfd, (struct sockaddr*)&cliaddr, &client_len);   
            
            //save the descriptor
            for (i = 0; i < max_client; i++){
				if (client_socket[i] < 0) {
					client_socket[i] = connfd;
                    std::string s = "********************************\n** Welcome to the BBS server. **\n********************************\n% ";
                    send(connfd, s.c_str(), s.size(), 0);
					break;
				}
            }
			if (i == max_client){
                //err_quit("too many clients");
                continue;
            }
			//add new descriptor
            FD_SET(connfd, &allset);
			if (connfd > maxfd)
				maxfd = connfd;			
			if (i > maxi)
				maxi = i;				
			if (--nready <= 0)
				continue;				
        }

        //listen the client
        bool tcp_flag = true;
        for (i = 0; i <= maxi; i++) {
			if ( (sockfd = client_socket[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
                if(!user_info.empty()){
                    if(user_info[i].times >= 3){
                        if(!user_info[i].ban_flag){
                            logout_handler(sockfd, i);
                            s = "% ";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        user_info[i].ban_flag = true;
                    }
                }

                bzero(input, 1000);
                input_result = read(sockfd, input, 1000);

				if (input_result == 0) {
                    close(sockfd);
					FD_CLR(sockfd, &allset);
					client_socket[i] = -1;
				}
                //handle the command from the client
                else{
                    int y, z, counts = 0, flag = 1;
                    std::string command[100] = {}, command_temp = "";
                    tcp_flag = false;
                    for(y = 0;; y++){
                        if(input[y] != ' ' && input[y] != '\0'){
                            command_temp += input[y];
                            flag = 1;
                        }
                        else{
                            if(flag){
                                command[counts++] = command_temp;
                                if(!strcmp(command_temp.c_str(), "\n")){
                                    counts--;
                                    command_temp = "";
                                }
                                command_temp = "";
                                flag = 0;
                            }
                        }
                        
                        if(input[y] == '\0')
                            break;
                    }
                    if(!strcmp("register\n", command[0].c_str()) || !strcmp("register", command[0].c_str())){
                        if(counts == 3){
                            register_handler(command[1], command[2], sockfd);
                        }
                        else{
                            s = "Usage: register <username> <password>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }
                    else if(!strcmp("login\n", command[0].c_str()) || !strcmp("login", command[0].c_str())){
                        if(counts == 3){
                            login_handler(command[1], command[2], sockfd, i);
                        }
                        else{
                            s = "Usage: login <username> <password>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }
                    
                    else if(!strcmp("logout\n", command[0].c_str()) || !strcmp("logout", command[0].c_str())){
                        if(counts != 1){
                            s = "Usage: logout\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        else if(client_state[i].flag){
                            logout_handler(sockfd, i);
                        }
                        else{
                            s = "Please login first.\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }

                    else if(!strcmp("exit\n", command[0].c_str()) || !strcmp("exit", command[0].c_str())){
                        if(counts != 1){
                            s = "Usage: exit\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        else if(client_state[i].flag){
                            s = "Bye, " + client_state[i].username + ".\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                            client_state[i].username = "";
                            client_state[i].flag = false;
                            close(sockfd);
					        FD_CLR(sockfd, &allset);
					        client_socket[i] = -1;
                        }
                        else{
                            close(sockfd);
					        FD_CLR(sockfd, &allset);
					        client_socket[i] = -1;
                        }
                    }

                    else if(!strcmp("enter-chat-room\n", command[0].c_str()) || !strcmp("enter-chat-room", command[0].c_str())){
                        if(counts != 3){
                            s = "Usage: enter-chat-room <port> <version>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        else{
                            enter_chat_room_handler(command, counts, sockfd, i);
                        }
                    }
                    
                    else if(!strcmp("chat\n", command[0].c_str()) || !strcmp("chat", command[0].c_str())){
                        chat_message_handler(command, counts, sockfd, i);
                    }

                    s = "% ";
                    send(sockfd, s.c_str(), s.size(), 0);
                }

				if (--nready <= 0)
					break;
			}
            
        }
	}

}