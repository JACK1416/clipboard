#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SA struct sockaddr
using namespace std;

string old_string, new_string;

std::string exec(const char* cmd)
{
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    string result = "";
    while(!feof(pipe))
    {
        if(fgets(buffer, 128, pipe) != NULL)
        {
        result += buffer;
        }
    }
    pclose(pipe);
    return result;
}

string paste()
{
    return exec("pbpaste");
}

string copy(const char * new_clipboard)
{
    stringstream cmd;
    cmd << "echo \"" << new_clipboard << "\" | pbcopy";
    return exec(cmd.str().c_str());
}

void *
check(void *arg)
{
    old_string = paste();
    while(1)
    {
        sleep(1);
        new_string = paste();
        if(new_string.compare(old_string) == 0) continue;
        old_string = new_string;
        int size = old_string.size();
        if(write(*(int*)arg, old_string.c_str(), size) != size)
        {
            cout << "write ERROR!" << endl;
            return NULL;
        }
    }
}

void
update(int sockfd)
{
    ssize_t n;
    char mesg[1000];

again:
    while( (n = read(sockfd, mesg, 1000)) > 0)
    {
        copy(mesg);
        old_string = mesg;
    }

    if(n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
    {
        cout << "update ERROR!" << endl;
        return;
    }
}

int main(int argc, char * argv[])
{
    int sockfd;
    pthread_t tid;
    const in_port_t SERV_PORT = 9090;
    struct sockaddr_in servaddr;

    if(argc != 2)
    {
        cout << "usage: openCB <IPaddress>" << endl;
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1)
    {
        cout << "IPaddress ERROR!" << endl;
        return -1;
    }
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "socket ERROR!" << endl;
        return -1;
    }
    if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
    {
        cout << "connect ERROR!" << endl;
        return -1;
    }
    pthread_create(&tid, NULL, &check, &sockfd);
    update(sockfd);
    return 0;
}
