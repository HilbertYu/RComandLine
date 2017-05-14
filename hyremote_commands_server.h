#ifndef HYREMOTE_COMMANDS_SERVER_H
#define HYREMOTE_COMMANDS_SERVER_H

#include "hysocket.h"
#include "hyremote_commands.h"


class HyCmdFuncPutFileModeServer: public HyCmdFuncPutFileMode
{
public:
    HyCmdFuncPutFileModeServer(int hd):
        HyCmdFuncPutFileMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        std::cout << "[copy file mode]" << std::endl;
        char file_name[128] = {0};
        int file_size = 0;

        m_sock->recv(&file_size, 4);
        m_sock->recv(file_name, 128);

        const char * mode_msg = "[file-copy-mode]";
        printf("%s", mode_msg);
        printf("get file name: %s\n", file_name);
        printf("%s", mode_msg);
        printf("get file size: %d\n", file_size);


        char  * recv_buf = new char[file_size + 1];
        m_sock->recv_all(recv_buf, file_size);

        WriteBuffToFile(file_name, recv_buf, file_size);

        delete [] recv_buf;
        recv_buf = NULL;

        return 0;
    }

};

class HyCmdFuncGetFileModeServer: public HyCmdFuncGetFileMode
{
public:
    HyCmdFuncGetFileModeServer(int hd):
        HyCmdFuncGetFileMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        HySocketBase * cl = m_sock;
        using namespace std;
        cout << "[get file mode]" << endl;
        int len = 0;
        cl->recv(&len ,4);
        printf("file name len = %d\n", len);

        char dest_file_name[128] = {0};
        cl->recv(dest_file_name, len);
        printf("file name = %s\n", dest_file_name);

        vector<char> dest_file_buf;
        int ret = GetFileBufferVector(dest_file_buf, dest_file_name);

        if (ret < 0)
        {
            int dead = HY_KEY_DEAD;
            cl->send(&dead, 4);
            printf("[Server](file-get-mode) open file fail\n");
            return -1;
        }
        else
        {
            int ack = HY_KEY_BEEF;
            cl->send(&ack, 4);
        }

        int file_size = dest_file_buf.size();
        cl->send(&file_size, 4);

        char * file_buf = Vector2Buf(dest_file_buf);
        cl->send_all(file_buf, file_size);

        delete [] file_buf;
        file_buf = NULL;

        return 0;
    }

};

class HyCmdFuncFreeCmdModeServer: public HyCmdFuncFreeCmdMode
{
    void freeCommandModeRun(const char * fcmd)
    {
        using namespace std;
        int hd = m_hd;
        HySocketBase * cl = m_sock;

        if (hd == HY_KEY_FREE_CMD_LOCAL_INFO_MODE)
        {
            FILE* fp_cmd = popen(fcmd, "r");
            vector<char> info_buf;
            char ch = 0;
            int ret = 0;
            while (1)
            {
                ret = fread(&ch, 1, 1, fp_cmd);
                if (ret <= 0) break;
                info_buf.push_back(ch);
            }
            pclose(fp_cmd);

            int info_len = info_buf.size();
            char * back_buf = Vector2Buf(info_buf);

            printf("----\n");
            printf("%s", back_buf);
            printf("\n----");
            cout << endl;

            cl->send(&info_len, 4);
            cl->send(back_buf, info_len);
            cout << "[local info command done]" << endl;

            delete [] back_buf;
            return;
        }
        else if (hd == HY_KEY_FREE_CMD_MODE)
        {
            system(fcmd);
            return;
        }
        else
        {
            fprintf(stderr, "fatal error -flow error\n");
            getchar();
            exit(EXIT_FAILURE);

        }

    }

public:
    HyCmdFuncFreeCmdModeServer(int hd):
        HyCmdFuncFreeCmdMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        printf("[pass command to shell]: ");
        fflush(stdout);
        char fcmd[128] = {0};

        int len = 0;
        m_sock->recv_all(&len , 4);
        m_sock->recv_all(fcmd, len);
        printf("%s\n", fcmd);

        freeCommandModeRun(fcmd);

        int ack = HY_KEY_BEEF;
        m_sock->send(&ack, 4);

        return 0;
    }

};


class HyCmdFuncKillServerModeServer: public HyCmdFuncKillServerMode
{
public:
    HyCmdFuncKillServerModeServer(int hd):
        HyCmdFuncKillServerMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        extern int g_server_live;
        g_server_live = 0;
        printf("[close server from client]\n");
        return DO_CMD_ST_BREAK;
    }

};

class HyCmdFuncClientDisconnectModeServer: public HyCmdFuncClientDisconnectMode
{
public:
    HyCmdFuncClientDisconnectModeServer(int hd):
        HyCmdFuncClientDisconnectMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        printf("[client disconnect]\n");
        return DO_CMD_ST_BREAK;
    }

};

class HyCmdFuncServerFactory : public HyCmdFuncBaseFactory
{
    void initMap(void)
    {
        insert_map<HyCmdFuncPutFileModeServer>(HY_KEY_PUT_FILE_MODE);
        insert_map<HyCmdFuncGetFileModeServer>(HY_KEY_GET_FILE_MODE);
        insert_map<HyCmdFuncFreeCmdModeServer>(HY_KEY_FREE_CMD_MODE);
        insert_map<HyCmdFuncFreeCmdModeServer>(HY_KEY_FREE_CMD_LOCAL_INFO_MODE);

        insert_map<HyCmdFuncClientDisconnectModeServer>(HY_KEY_CLIENT_DISCONNECT);
        insert_map<HyCmdFuncKillServerModeServer>(0x1016baba);
    }

public:
    HyCmdFuncServerFactory(void):
        HyCmdFuncBaseFactory()
    {
        initMap();
    }

    HyCmdFuncServerFactory(HySocketBase * cs):
        HyCmdFuncBaseFactory()
    {
        initMap();
        setAllSockets(cs);

    }

};
#endif /* end of include guard: HYREMOTE_COMMANDS_SERVER_H */
