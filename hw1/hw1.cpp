#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include <algorithm>

#include <cstring>
#include <vector>
#include <map>

struct user
{
	std::string username;
	std::string password;
    bool        flag = false;
    std::vector<std::string> message;
    std::vector<std::string> message_author;
    int         message_count[1000] = {};
};

int user_count = 0;
user user_info[1000];

//the function to handle the register command
void register_handler(int connection, std::string username, std::string password){

    //check if the user has registered
    for(int i = 0; i < user_count; i++){
        if(!strcmp(user_info[i].username.c_str(), username.c_str())){
            std::string s = "Username is already used.\n";
            send(connection, s.c_str(), s.size(), 0);
            return;
        }
    }

    //add the information
    user_info[user_count].username = username;
    user_info[user_count++].password = password;
    std::string s = "Register successfully.\n";
    send(connection, s.c_str(), s.size(), 0);
}

void login_handler(int connection, std::string username, std::string password){
    for(int i = 0; i < user_count; i++){
        if(!strcmp(user_info[i].username.c_str(), username.c_str())){
            if(!strcmp(user_info[i].password.c_str(), password.c_str())){
                if(user_info[i].flag){
                    std::string s = "Please logout first.\n";
                    send(connection, s.c_str(), s.size(), 0);
                    return;
                }
                else{
                    std::string s = "Welcome, " + username + ".\n";
                    send(connection, s.c_str(), s.size(), 0);
                    user_info[i].flag = true;
                    return;
                }
            }
        }
    }
    std::string s = "Login failed.\n";
    send(connection, s.c_str(), s.size(), 0);
}

void logout_handler(int connection){
    for(int i = 0;i < user_count; i++){
        if(user_info[i].flag == true){
            user_info[i].flag = false;
            std::string s = "Bye, " + user_info[i].username + ".\n";
            send(connection, s.c_str(), s.size(), 0);
            return;
        }
    }
    std::string s  = "Please login first.\n";
    send(connection, s.c_str(), s.size(), 0);
    return;
}

void whoami_handler(int connection){
    for(int i = 0;i < user_count; i++){
        if(user_info[i].flag == true){
            std::string s = user_info[i].username + "\n";
            send(connection, s.c_str(), s.size(), 0);
            return;
        }
    }
    std::string s  = "Please login first.\n";
    send(connection, s.c_str(), s.size(), 0);
}

void listuser_handler(int connection){
    std::string listuser_sort[1000];
    for(int i = 0;i < user_count; i++){
        listuser_sort[i] = user_info[i].username; 
    }
    sort(listuser_sort, listuser_sort + user_count);
    std::string s;
    for(int i = 0; i < user_count; i++){
        s = listuser_sort[i] + "\n";
        send(connection, s.c_str(), s.size(), 0);
    }
    
}

void send_message_handler(int connection, std::string username, std::string message){
    std::string s;
    for(int i = 0; i < user_count; i++){
        if(!strcmp(user_info[i].username.c_str(), username.c_str())){
            int j, check = 0;
            for(j = 0; j < user_count; j++){
                if(user_info[j].flag){
                    check = 1;
                    break;
                }
            }
            if(!check){
                s = "Please login first.\n";
                send(connection, s.c_str(), s.size(), 0);
                return;
            }
            user_info[i].message.push_back(message);
            user_info[i].message_author.push_back(user_info[j].username);
            user_info[i].message_count[j]++;
            return;
        }
    }
    s = "User not existed.\n";
    send(connection, s.c_str(), s.size(), 0);
}

void list_message_handler(int connection){
    std::string s;
    std::vector<std::string> sort_user;
    int check = 0, counts = 0, flag, user_state = 0;
    for(int i = 0; i < user_count; i++){
        if(user_info[i].flag){
            for(int j = 0; j < user_count; j++){
                if(user_info[i].message_count[j]){
                    check = 1;
                    sort_user.push_back(user_info[j].username);
                    counts++;
                }
            }
            user_state = 1;
            flag = i;
            break;
        }
    }
    if(!check && user_state){
        s = "Your message box is empty.\n";
        send(connection, s.c_str(), s.size(), 0);
        return;
    }
    else if(!user_state){
        s = "Please login first.\n";
        send(connection, s.c_str(), s.size(), 0);
        return;
    }
    sort(sort_user.begin(), sort_user.end());
    for(int i = 0; i < counts; i++){
        for(int j = 0; j < user_count; j++){
            if(user_info[flag].message_count[j] && !strcmp(sort_user[i].c_str(), user_info[j].username.c_str())){
                s = std::to_string(user_info[flag].message_count[j]) + " message from " + user_info[j].username + ".\n";
                send(connection, s.c_str(), s.size(), 0);
                break;
            }
        }
    }
}

void receive_message_handler(int connection, std::string username){
    std::string s;
    for(int i = 0; i < user_count; i++){
        if(user_info[i].flag){
            for(int j = 0; j < user_info[i].message_author.size(); j++){
                if(!strcmp((user_info[i].message_author[j] + "\n").c_str(), username.c_str())){
                    s = user_info[i].message[j].substr(1, user_info[i].message[j].length() - 3) + "\n";
                    send(connection, s.c_str(), s.size(), 0);
                    user_info[i].message.erase(user_info[i].message.begin() + j);
                    user_info[i].message_author.erase(user_info[i].message_author.begin() + j);
                    user_info[i].message_count[j]--;
                    return;
                }
                else{
                    s = "User not existed.\n";
                    send(connection, s.c_str(), s.size(), 0);
                    return;
                }
            }
            s = "Not exist message.\n";
            send(connection, s.c_str(), s.size(), 0);
            return;
        }
        
    }
    s = "Please login first.\n";
    send(connection, s.c_str(), s.size(), 0);
}

/////////

int main(int argc, char *argv[]) {

    while(1){
        if(argc < 2){
            std::cout << "Please enter: ./hw1 <port number>" << std::endl;
            break;
        }

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            std::cout << "Failed to create socket." << std::endl;
            exit(EXIT_FAILURE);
        }
        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

        struct sockaddr_in sockaddr;
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_addr.s_addr = INADDR_ANY;
        sockaddr.sin_port = htons(atoi(argv[1])); 
        
        if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
            std::cout << "Failed to bind to port " << argv[1] << std::endl;
            exit(EXIT_FAILURE);
        }

        // Start listening.
        if (listen(sockfd, 5) < 0) {
            exit(EXIT_FAILURE);
        }

        //get connection
        auto addrlen = sizeof(sockaddr);
        int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
        if (connection < 0) {
            std::cout << "Failed to grab connection." << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string s = "********************************\n** Welcome to the BBS server. **\n********************************\n";
        send(connection, s.c_str(), s.size(), 0);

        //listen from the client
        while(1){
            char input[10000];
            bzero(input,10000);
            std::string command;
            std::string info_1 = "";
            std::string info_2 = "";
        
            //get the input
            int input_result = read(connection, input, 9999);
            int i, j = 0, counts = 0;
            for(i = 0;;i++){
                if(j == 0){
                    if(input[i] != ' ' && input[i] != '\0'){
                        command[counts++] = input[i];
                    }
                }
                else if(j == 1){
                    if(input[i] != ' ' && input[i] != '\0'){
                        info_1 += input[i];
                    }
                }
                else if(j == 2){
                    if(input[i] != '\0'){
                        info_2 += input[i];
                        continue;
                    }
                }

                if(input[i] == ' '){
                    counts = 0;
                    j++;
                }
                if(input[i] == '\0'){
                    break;
                }
            }

            //register
            if(!strncmp(command.c_str(), "register", 8)){
                if(j == 0){
                    s = "Usage: register <username> <password>\n";
                    send(connection, s.c_str(), s.size(), 0);
                }
                else if(j == 2){
                    register_handler(connection, info_1, info_2);
                }
                else{
                    s = "Usage: register <username> <password>\n";
                    send(connection, s.c_str(), s.size(), 0);
                }
            }
            //login
            else if(!strncmp(command.c_str(), "login", 5)){
                if(j == 0){
                    s = "Usage: login <username> <password>\n";
                    send(connection, s.c_str(), s.size(), 0);
                }
                else if(j == 2){
                    login_handler(connection, info_1, info_2);
                }
                else{
                    s = "Usage: login <username> <password>\n";
                    send(connection, s.c_str(), s.size(), 0);
                }
            }
            else if(!strncmp(command.c_str(), "logout", 6)){
                logout_handler(connection);
            }
            else if(!strncmp(command.c_str(), "whoami", 6)){
                whoami_handler(connection);
            }
            else if(!strncmp(command.c_str(), "list-user", 9)){
                listuser_handler(connection);
            }
            else if(!strncmp(command.c_str(), "send", 4)){
                if(j == 0){
                    s = "Usage: send <username> <message>\n";
                    send(connection, s.c_str(), s.size(), 0);
                }
                else if(j == 2){
                    send_message_handler(connection, info_1, info_2);
                }
                else{
                    s = "Usage: send <username> <message>\n";
                    send(connection, s.c_str(), s.size(), 0);
                }
            }
            else if(!strncmp(command.c_str(), "receive", 7)){
                if(j == 0){
                    s = "Usage: recevie <username>\n";
                    send(connection, s.c_str(), s.size(), 0);
                }
                else{
                    receive_message_handler(connection, info_1);
                }
            }
            else if(!strncmp(command.c_str(), "list-msg", 8)){
                list_message_handler(connection);
            }
            else if(!strncmp(command.c_str(), "exit", 4)){
                for(i = 0; i < user_count; i++){
                    if(user_info[i].flag){
                        s = "Bye, " + user_info[i].username + ".\n";
                        send(connection, s.c_str(), s.size(), 0);
                        user_info[i].flag = false;
                        break;
                    }
                }
                close(connection);
                close(sockfd);
                break;
            }
        }
    }
}