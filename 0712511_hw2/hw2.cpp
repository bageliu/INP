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

struct user{
    std::string username;
    std::string password;
    bool flag = false;
};

struct state{
    std::string username;
    std::string board_name;
    bool flag = false;
};

struct post{
    std::string title;
    std::string author;
    std::string date;
    std::string content;
    std::vector<std::string> comment;
    bool flag = true;
};
struct board{
    std::string board_name;
    std::string moderator;
    std::vector<post> post_list;
};

state client_state[10];
std::vector<user> user_info;
std::vector<board> board_list;
std::string s;

bool isNumber(const std::string& s)
{
    for (char const &ch : s) {
        if (std::isdigit(ch) == 0) 
            return false;
    }
    return true;
}

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

void create_board_handler(std::string name_temp, int counts, int sockfd, int j){
    int i;
    name_temp = name_temp.substr(0, name_temp.length() - 1);
    for(i = 0; i < board_list.size(); i++){
        int j = strcmp(board_list[i].board_name.c_str(), name_temp.c_str());
        if(!strcmp(board_list[i].board_name.c_str(), name_temp.c_str())){
            s = "Board already exists.\n";
            send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
    }

    board info;
    info.board_name = name_temp;
    info.moderator = client_state[j].username;
    board_list.push_back(info);
    s = "Create board successfully.\n";
    send(sockfd, s.c_str(), s.size(), 0);
}

void create_post_handler(std::string command[100], int counts, int sockfd, int j){
    std::string name_temp = "", title_temp = "", content_temp = "", content_temp_new = "", input, output;
    int i, count_temp, board_number;
    bool flag = true, board_name_flag = false, content_flag = false, title_flag = false;
    name_temp = command[1];
    for(i = 0; i < board_list.size(); i++){
        if(!strcmp(board_list[i].board_name.c_str(), name_temp.c_str())){
            board_number = i;
            board_name_flag = true;
            flag = false;
        }
    }
    count_temp = 2;
    if(!strcmp(command[count_temp].c_str(), "--title")){
        for(i = count_temp + 1; i < counts; i++){
            if(strcmp(command[i].c_str(), "--content")){
                title_temp += command[i] + " ";
            }
            else{
                count_temp = i;
                break; 
            }
        }
    }
    else if(!strcmp(command[count_temp].c_str(), "--content")){
        for(int i = count_temp + 1; i < counts; i++){
            if(strcmp(command[i].c_str(), "--title")){
                    content_temp += command[i] + " ";
            }
            else{
                count_temp = i;
                content_temp += "\n";
                break; 
            }
        }
    }
    if(!strcmp(command[count_temp].c_str(), "--title")){
        for(int i = count_temp + 1; i < counts; i++){
            if(strcmp(command[i].c_str(), "--content")){
                if(i != (counts - 1))
                    title_temp += command[i] + " ";
                else{
                    title_temp += command[i].substr(0, command[i].length() - 1);
                }
            }
            else{
                count_temp = i;
                break; 
            }
        }
    }
    else if(!strcmp(command[count_temp].c_str(), "--content")){
        for(int i = count_temp + 1; i < counts; i++){
            if(strcmp(command[i].c_str(), "--title")){
                if(i != (count_temp - 1))
                    content_temp += command[i] + " ";
                else{
                    content_temp += command[i];
                }
            }
            else{
                count_temp = i;
                break; 
            }
        }
    }

    for(int i = 0; i < content_temp.length(); i++){
        if(content_temp[i] == '<' && content_temp[i + 1] == 'b' && content_temp[i + 2] == 'r' && content_temp[i + 3] == '>'){
            content_temp_new += '\n';
            i += 3;
        }
        else{
            content_temp_new += content_temp[i];
        }
    }
    if(flag && (content_temp_new.length() > 0) && (title_temp.length() > 0)){
        s = "Board does not exist.\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }
    if(board_name_flag && (content_temp_new.length() > 0) && (title_temp.length() > 0)){
        //handle date
        time_t now = time(0);
        tm *ltm = localtime(&now);
        int mon = ltm->tm_mon + 1, day = ltm->tm_mday;

        board board_temp;
        post post_temp;
        post_temp.author = client_state[j].username;
        post_temp.title = title_temp;
        post_temp.content = content_temp_new;
        post_temp.date = std::to_string(mon) + "/" + std::to_string(day);
        board_list[board_number].post_list.push_back(post_temp);
        s = "Create post successfully.\n";
        send(sockfd, s.c_str(), s.size(), 0);
    }
    else{
        s = "Usage: create-post <board-name> --title <title> --content <content>\n";
        send(sockfd, s.c_str(), s.size(), 0);
    }
}

void list_board_handler(int sockfd){
    s = "Index Name Moderator\n";
    int i, j, k;
    send(sockfd, s.c_str(), s.size(), 0);
    for(i = 0; i < board_list.size(); i++){
        s = std::to_string(i + 1);
        s += " " + board_list[i].board_name;
        s += " " + board_list[i].moderator + "\n";
        send(sockfd, s.c_str(), s.size(), 0);
    }
}

void list_post_handler(std::string board_name, int counts, int sockfd, int z){
    int title_length = 0, author_length = 0, S_N_length = 3, index;
    bool flag = false;
    board_name = board_name.substr(0, board_name.length() - 1);

    for(int i = 0; i < board_list.size(); i++){
        if(!strcmp(board_list[i].board_name.c_str(), board_name.c_str())){
            flag = true;
            index = i;
            client_state[z].board_name = board_name;
            break;
        }
    }
    if(flag){
        s = "S/N Title Author Date\n";
        for(int i = 0 ;i < board_list[index].post_list.size(); i++){
            if(board_list[index].post_list[i].flag)
                s += std::to_string(i + 1) + " " + board_list[index].post_list[i].title + " " + board_list[index].post_list[i].author + " "  + board_list[index].post_list[i].date + "\n";
        }
        send(sockfd, s.c_str(), s.size(), 0);
    }
    else{
        s = "Board does not exist.\n";
        send(sockfd, s.c_str(), s.size(), 0);
    }
}

void read_post_handler(std::string command, int sockfd, int j){
    std::string name = client_state[j].board_name;
    command = command.substr(0, command.length() - 1);

    int index, flag = 1, post_idx;
    if(isNumber(command)){
        post_idx = stoi(command) - 1;
    }
    else{
        s = "Usage: read <post-S/N>\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }

    for(int i = 0; i < board_list.size(); i++){
        if(!strcmp(board_list[i].board_name.c_str(), name.c_str())){
            index = i;
            flag = 0;
            break;
        }
    }
    if(post_idx + 1 > board_list[index].post_list.size()){
        s = "Post does not exist.\n";
        send(sockfd, s.c_str(), s.size(), 0);
    }
    else{
        //error
        if(board_list[index].post_list[post_idx].flag){
            s = "Author: " + board_list[index].post_list[post_idx].author + "\n" + "Title: " + board_list[index].post_list[post_idx].title + "\n";
            s += "Date: " + board_list[index].post_list[post_idx].date + "\n--\n";
            s += board_list[index].post_list[post_idx].content + "--\n";
            for(int i = 0; i < board_list[index].post_list[post_idx].comment.size(); i++){
                s += board_list[index].post_list[post_idx].comment[i] + "\n";
            }
            send(sockfd, s.c_str(), s.size(), 0);
        }
        else{
            s = "Post does not exist.\n";
            send(sockfd, s.c_str(), s.size(), 0);
        }
    }
}

void update_post_handler(std::string command[100], int sockfd, int j, int counts){
    std::string name = client_state[j].username;
    std::string board_name = client_state[j].board_name;
    std::string temp = command[1];
    std::string input = "";

    int post_idx = stoi(temp) - 1, index;
    if(isNumber(temp)){
        post_idx = stoi(temp) - 1;
    }
    else{
        s = "Usage: update-post <post-S/N> --title/content <new>\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }
    //printf("%d\n", post_idx);
    for(int i = 0; i < board_list.size(); i++){
        if(!strcmp(board_list[i].board_name.c_str(), board_name.c_str())){
            index = i;
            break;
        }
    }

    //handle input
    std::string temp2;
    if(!strcmp("--title", command[2].c_str())){
        for(int i = 3; i < counts; i++){
            if(i != (counts - 1)){
                input += command[i] + " ";
            }
            else{
                temp2 = command[i].substr(0, command[i].length() - 1);
                input += temp2;
            }
        }
        if(input.empty()){
            s = "Usage: update-post <post-S/N> --title/content <new>\n";
            send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
        if(post_idx > board_list[index].post_list.size()){
            s = "Post does not exist.\n";
            s = send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
        std::string author = board_list[index].post_list[post_idx].author;
        if(strcmp(name.c_str(), author.c_str())){
            s = "Not the post owner.\n";
            send(sockfd, s.c_str(), s.size(), 0);
            return;
        }

        board_list[index].post_list[post_idx].title = input;
        s = "Update successfully.\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }
    else if(!(strcmp("--content", command[2].c_str()))){
        for(int i = 3; i < counts; i++){
            if(i != (counts - 1)){
                input += command[i] + " ";
            }
            else{
                input += command[i];
            }
        }
        std::string content_temp_new = "";
        for(int i = 0; i < input.length(); i++){
            if(input[i] == '<' && input[i + 1] == 'b' && input[i + 2] == 'r' && input[i + 3] == '>'){
                content_temp_new += '\n';
                i += 3;
            }
            else{
                content_temp_new += input[i];
            }
        }
        if(post_idx > board_list[index].post_list.size()){
            s = "Post does not exist.\n";
            s = send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
        std::string author = board_list[index].post_list[post_idx].author;
        if(strcmp(name.c_str(), author.c_str())){
            s = "Not the post owner.\n";
            send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
        if(content_temp_new.empty()){
            s = "Usage: update-post <post-S/N> --title/content <new>\n";
            send(sockfd, s.c_str(), s.size(), 0);
            return;
        }
        board_list[index].post_list[post_idx].content = content_temp_new;
        s = "Update successfully.\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }
    else{
        s = "Usage: update-post <post-S/N> --title/content <new>\n";
        send(sockfd, s.c_str(), s.size(), 0);
    }
}

void delete_post_handler(std::string command, int sockfd, int j){
    std::string name = client_state[j].username;
    std::string board_name = client_state[j].board_name;
    command = command.substr(0, command.length() - 1);
    int post_idx, index;
    if(isNumber(command)){
        post_idx = stoi(command) - 1;
    }
    else{
        s = "Usage: comment <post-S/N> <comment>";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }

    for(int i = 0; i < board_list.size(); i++){
        if(!strcmp(board_list[i].board_name.c_str(), board_name.c_str())){
            index = i;
            break;
        }
    }

    if(post_idx > board_list[index].post_list.size()){
        s = "Post does not exist.\n";
        s = send(sockfd, s.c_str(), s.size(), 0);
        return;
    }
    std::string author = board_list[index].post_list[post_idx].author;
    if(strcmp(name.c_str(), author.c_str())){
        s = "Not the post owner.\n";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }

    board_list[index].post_list[post_idx].flag = false;
    s = "Delete successfully.\n";
    send(sockfd, s.c_str(), s.size(), 0);
}

void comment_handler(std::string command[100], int counts, int sockfd, int j){
    std::string name = client_state[j].username;
    std::string board_name = client_state[j].board_name;
    std::string comment_temp = name + ": ", post_num = command[1];
    int post_idx, index;
    if(isNumber(post_num)){
        post_idx = stoi(post_num) - 1;
    }
    else{
        s = "Usage: comment <post-S/N> <comment>";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }

    for(int i = 0; i < board_list.size(); i++){
        if(!strcmp(board_list[i].board_name.c_str(), board_name.c_str())){
            index = i;
            break;
        }
    }

    for(int i = 2; i < counts; i++){
        comment_temp += command[i] + " ";
    }
    if(comment_temp.empty()){
        s = "Usage: comment <post-S/N> <comment>";
        send(sockfd, s.c_str(), s.size(), 0);
        return;
    }

    if(post_idx > board_list[index].post_list.size()){
        s = "Post does not exist.\n";
        s = send(sockfd, s.c_str(), s.size(), 0);
        return;
    }

    board_list[index].post_list[post_idx].comment.push_back(comment_temp);
    s = "Comment successfully.\n";
    send(sockfd, s.c_str(), s.size(), 0);
}

int main(int argc, char *argv[]){
    int i, max_client = 10, client_socket[10], maxi, maxfd, nready, connfd, sockfd, servfd, input_result;  
    char input[1000]; 
    if(argc < 2){
        std::cout << "server usage: ./hw2 [port number]\n";
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

    while(true){
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

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
                break;
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
        for (i = 0; i <= maxi; i++) {
			if ( (sockfd = client_socket[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
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
                    //std::cout << command[2] << counts << std::endl;
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
                        if(client_state[i].flag){
                            logout_handler(sockfd, i);
                        }
                        else{
                            s = "Please login first.\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }

                    else if(!strcmp("exit\n", command[0].c_str()) || !strcmp("exit", command[0].c_str())){
                        if(client_state[i].flag){
                            s = "Bye, " + client_state[i].username + ".\n";
                            client_state[i].username = "";
                            client_state[i].flag = false;
                        }
                        else{
                            close(sockfd);
					        FD_CLR(sockfd, &allset);
					        client_socket[i] = -1;
                        }
                    }
                    else if(!strcmp("create-board\n", command[0].c_str()) || !strcmp("create-board", command[0].c_str())){
                        if(!client_state[i].flag){
                            s = "Please login first.\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        else if(counts == 2){
                            create_board_handler(command[1], counts, sockfd, i);
                        }
                        else{
                            s = "Usage: create-board <name>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }
                    else if(!strcmp("create-post\n", command[0].c_str()) || !strcmp("create-post", command[0].c_str())){
                        if(!client_state[i].flag){
                            s = "Please login first.\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        else if(counts >= 6){
                            create_post_handler(command, counts, sockfd, i);
                        }
                        else{
                            s = "Usage: create-post <board-name> --title <title> --content <content>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }
                    else if(!strcmp("list-board\n", command[0].c_str()) || !strcmp("list-board", command[0].c_str())){
                        list_board_handler(sockfd);
                    }
                    else if(!strcmp("list-post\n", command[0].c_str()) || !strcmp("list-post", command[0].c_str())){
                        if(counts == 2)
                            list_post_handler(command[1], counts, sockfd, i);
                        else{
                            s = "Usage: list-post <board-name>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }
                    else if(!strcmp("read", command[0].c_str())){
                        if(counts == 2){
                            read_post_handler(command[1], sockfd, i);
                        }
                        else{
                            s = "Usage: read <post-S/N>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }
                    else if(!strcmp("update-post\n", command[0].c_str()) || !strcmp("update-post", command[0].c_str())){
                        if(!client_state[i].flag){
                            s = "Please login first.\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        else{
                            update_post_handler(command, sockfd, i, counts);
                        }
                    }
                    else if(!strcmp("delete-post\n", command[0].c_str()) || !strcmp("delete-post", command[0].c_str())){
                        if(!client_state[i].flag){
                            s = "Please login first.\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        else if(counts == 2){
                            delete_post_handler(command[1], sockfd, i);
                        }
                        else{
                            s = "Usage: delete-post <post-S/N>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }
                    else if(!strcmp("comment\n", command[0].c_str()) || !strcmp("comment", command[0].c_str())){
                        if(!client_state[i].flag){
                            s = "Please login first.\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                        else if(counts >= 3){
                            comment_handler(command, counts, sockfd, i);
                        }
                        else{
                            s = "Usage: comment <post-S/N> <comment>\n";
                            send(sockfd, s.c_str(), s.size(), 0);
                        }
                    }
                }
                s = "% ";
                send(sockfd, s.c_str(), s.size(), 0);

				if (--nready <= 0)
					break;
			}
		}
    }

}