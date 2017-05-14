#include "hysocket.h"
#include "hyremote_commands.h"
#include "hyremote_commands_server.h"


using namespace std;

int g_thread_cnt = 0;
int g_server_live = 1;


void * clientThread(void* arg)
{
    ++g_thread_cnt;
    HySocketClient * cl = (HySocketClient*)arg;
    int ret = 0;

    int check = HY_KEY_BEEF;
    cl->send(&check, 4);


    HyCmdFuncServerFactory serverFactor(cl);

    while(1)
    {
        printf("[%s]Server idle ...\n", cl->getIPv4().c_str());
        fflush(stdout);
        int hd = 0;
        ret = cl->recv_all(&hd, 4);
        if (ret < 4)
        {
            printf("[Server]Get header timeout\n");
            fflush(stdout);
            break;
        }

        HyCmdFuncBase * func = serverFactor.getFunc(hd);

        if (func)
        {
            int st = func->doCommand();
            if (st == HyCmdFuncBase::DO_CMD_ST_BREAK)
                break;

            continue;
        }

    }
    cl->close();
    --g_thread_cnt;

    return NULL;
}

int WaitToAccept(HySocketServer * s)
{
    int sfd = s->getFd();
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sfd, &rfds);
    struct timeval timeout;
    timeout.tv_usec = 0;
    timeout.tv_sec = 1;

    int sel = select(sfd+1, &rfds, NULL, NULL,&timeout);

    if (sel > 0 && FD_ISSET(sfd, &rfds))
    {
        printf("[Server] select accept\n");
        return 1;
    }

    if (sel < 0)
    {
        perror("[Server]select :");
        exit(EXIT_FAILURE);
    }

    return 0;


}

int main(int argc, const char * argv[])
{

    HySocketBase::initalize();
    HySocketBase::ShowHostInfo();

    int port = 20000;
    const char * ip = "0.0.0.0";

    if (argc >= 3)
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }

    printf("[Server]ip = %s, port = %d\n", ip, port);

    HySocketServer * s = new HySocketServer;
    s->setup(ip, port);


    while(g_server_live > 0 || g_thread_cnt > 0)
    {

        if (!WaitToAccept(s))
            continue;

        HySocketClient *cl = s->accept(new HySocketClient);

        if (g_thread_cnt > 0)
        {
            int k = HY_KEY_FULL;
            cl->send(&k, 4);
            printf("[Server] Reject connection\n");
            cl->close();
            delete cl;
            cl = NULL;
            continue;
        }

        pthread_t thd_id = 0;
        pthread_create(&thd_id, NULL, clientThread, cl);

        printf("[Server] client count is %d\n", g_thread_cnt);

    }

    printf("[Server]Close server\n");
    s->close();

    HySocketBase::cleanup();
    return 0;
}
