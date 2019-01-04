#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <codecvt>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <iconv.h>

#define SA struct sockaddr
#define LISTENQ 6
using namespace std;
std::mutex mtx; ;
bool lk = true;
string old_string, new_string;

string exec(const char* cmd)
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
    int sockfd = *(int*)arg;
    while(1)
    {
        sleep(1);
        while(!lk){}
        new_string = paste();
        
        if(new_string.compare(old_string) == 0 || !lk) continue;
        old_string = new_string;
        int size = old_string.size();
        if(write(sockfd, old_string.c_str(), size) != size)
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
    char mesg[5000];

again:
    while( (n = read(sockfd, mesg, 5000)) > 0)
    {
        lk = false;
        mesg[n] = 0;
        copy(mesg);
        old_string = paste();
        new_string = paste();
        lk = true;
    }

    if(n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
    {
        cout << "update ERROR!" << endl;
        return;
    }
}

int main(int argc, char** argv)
{
    int listenfd, connfd;
    pthread_t tid;
    const in_port_t SERV_PORT = 9090;
    struct sockaddr_in servaddr;

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "socket ERROR!" << endl;
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if( bind(listenfd, (const SA*)&servaddr, (socklen_t)sizeof(servaddr)) < 0)
    {
        cout << "bind ERROR!" << endl;
        return -1;
    }
    listen(listenfd, LISTENQ);
    connfd = accept(listenfd, NULL, NULL);
    pthread_create(&tid, NULL, &check, &connfd);
    update(connfd);
}
