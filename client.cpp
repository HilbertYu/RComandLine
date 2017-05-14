#include "hysocket.h"
#include "hyremote_commands.h"
#include "hyremote_commands_client.h"

using namespace std;

class HyCommandHeader
{
    map<string, int> raw_map;
public:
    HyCommandHeader(void):
        raw_map()
    {
        typedef pair<string, int> pair_t;
        raw_map.insert(pair_t("cp",             HY_KEY_PUT_FILE_MODE));
        raw_map.insert(pair_t("getfile",        HY_KEY_GET_FILE_MODE));
        raw_map.insert(pair_t("cmd",            HY_KEY_FREE_CMD_MODE));
        raw_map.insert(pair_t("licmd",          HY_KEY_FREE_CMD_LOCAL_INFO_MODE));
        raw_map.insert(pair_t("bye",            HY_KEY_CLIENT_DISCONNECT));
        raw_map.insert(pair_t("kill-server",    0x1016baba));
        //help

    }

    int getHeader(const string & cmd)
    {
        map<string, int>::const_iterator itr = raw_map.find(cmd);
        if (itr == raw_map.end())
            return HY_KEY_NONE;

        return itr->second;

    }
};

int getCommandHeader(std::string cmd)
{
    static HyCommandHeader s_hd;
    return s_hd.getHeader(cmd);
}




void FirstSync(HySocketBase * csock)
{
    int check = 0;
    csock->recv(&check, 4);
    if (check == HY_KEY_BEEF)
    {
        printf("Connection ok\n");
        return;
    }

    if (check == HY_KEY_FULL)
    {
        fprintf(stderr, "client full\n");
    }

    csock->close();
    exit(EXIT_FAILURE);


}

int main(int argc, const char * argv[])
{

    HySocketClient * csock = new HySocketClient;

    int port = 20000;
    const char * ip = "127.0.0.1";

    if (argc >= 3)
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }

    printf("ip = %s, port = %d\n", ip, port);
    csock->connect(ip, port);
    printf("api-connect ok\n");


    FirstSync(csock);
    HyCmdFuncClientFactory * cmdfuncs = new HyCmdFuncClientFactory(csock);

    while(1)
    {
        printf("$ ");
        char cmd[128];
        int input = scanf("%s", cmd);
        getchar();

        if (input <= 0) break;

        printf("cmd: %s\n", cmd);

        int hd = getCommandHeader(cmd);

        if (hd == HY_KEY_NONE)
        {
            //fprintf(stderr, "no such command\n");
            fprintf(stderr, "$ %s: command not found\n", cmd);
            continue;
        }

        HyCmdFuncBase * func = cmdfuncs->getFunc(hd);

        if (!func)
        {
            printf("!!!!!!!!!!NIY\n");
            continue;
        }

        int st = func->doCommand();
        if (st == HyCmdFuncBase::DO_CMD_ST_BREAK) break;
    }

    csock->close();

    return 0;
}
