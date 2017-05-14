#ifndef HYREMOTE_COMMANDS_CLIENT_H
#define HYREMOTE_COMMANDS_CLIENT_H

class HyCmdFuncPutFileModeClient: public HyCmdFuncPutFileMode
{
public:
    HyCmdFuncPutFileModeClient(int hd):
        HyCmdFuncPutFileMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        char local_file_name[128] = {0};

        printf("[file-copy-mode]local file name:");
        scanf("%s", local_file_name);
        printf("\n");

        std::vector<char> file_buf;
        int ret = GetFileBufferVector(file_buf, local_file_name);
        if (ret < 0)
        {
            return -1;
        }

        char file_name[128] = {0};
        printf("[file-copy-mode]dest file name:");
        scanf("%s", file_name);
        printf("\n");

        int file_size = file_buf.size();
        printf("[file-copy-mode]lf = %s, df = %s\n", local_file_name, file_name);
        printf("[file-copy-mode]size = %d\n", file_size);

        m_sock->send(&m_hd, 4);
        m_sock->send(&file_size, 4);
        m_sock->send(file_name, 128);

        char  * sendbuf = Vector2Buf(file_buf);
        m_sock->send_all(sendbuf, file_size);

        return 0;
    }

};

class HyCmdFuncGetFileModeClient: public HyCmdFuncGetFileMode
{
public:
    HyCmdFuncGetFileModeClient(int hd):
        HyCmdFuncGetFileMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        int hd = m_hd;
        HySocketBase * csock = m_sock;
        csock->send(&hd, 4);

        char dst_file_name[128] = {0};

        printf("[file-get-mode]dest file name:");
        scanf("%s", dst_file_name);
        printf("\n");

        int len = strlen(dst_file_name);
        csock->send(&len, 4);
        csock->send(dst_file_name, len);

        int ack = 0;
        csock->recv(&ack, 4);

        if (ack == HY_KEY_DEAD)
        {
            fprintf(stderr, "Server open file error\n");
            return -1;
        }

        printf("[file-get-mode]server open file ok\n");


        int file_size = 0;
        csock->recv(&file_size, 4);
        printf("[file-get-mode]file size = %d\n", file_size );


        char * file_buf = new char[file_size + 1];
        memset(file_buf, 0, file_size + 1);

        //TODO error handle
        csock->recv_all(file_buf, file_size);


        char local_file_name[128] = {0};
        printf("[file-get-mode]local file name:");
        scanf("%s", local_file_name);
        printf("\n");
        WriteBuffToFile(local_file_name, file_buf, file_size);

        delete [] file_buf;
        file_buf = NULL;
        return 0;
    }

};



class HyCmdFuncFreeCmdModeClient: public HyCmdFuncFreeCmdMode
{
    void showLocalInfo(void)
    {
        int info_len = 0;
        m_sock->recv(&info_len, 4);

        char * info_buf =  new char[info_len+1];
        memset(info_buf, 0, info_len +1);
        m_sock->recv_all(info_buf, info_len);

        printf("-----------\n");
        printf("%s\n", info_buf);
        printf("-----------\n");
        delete [] info_buf;
    }

public:
    HyCmdFuncFreeCmdModeClient(int hd):
        HyCmdFuncFreeCmdMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        HySocketBase * csock = m_sock;
        int hd = m_hd;

        csock->send(&hd, 4);

        char buf[64];
        printf("fcmd> ");
        scanf("%[^\n]s", buf);

        int len = strlen(buf);
        buf[len] = 0;
        len += 1;

        csock->send(&len, 4);
        csock->send(buf, len);
        printf("[Free command] Waiting for %s ...\n", buf);

        //************************
        if (hd == HY_KEY_FREE_CMD_LOCAL_INFO_MODE)
        {
            showLocalInfo();
        }
        //************************

        int ack = 0;
        csock->recv(&ack, 4);
        if (ack == HY_KEY_BEEF)
        {
            printf("[Free command] %s Done\n", buf);
        }
        else
        {
            printf("[Free command] Ack error\n");
        }
        return 0;
    }

};

class HyCmdFuncKillServerModeClient: public HyCmdFuncKillServerMode
{
public:
    HyCmdFuncKillServerModeClient(int hd):
        HyCmdFuncKillServerMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        m_sock->send(&m_hd, 4);
        printf("close server\n");
        return DO_CMD_ST_BREAK;
    }

};

class HyCmdFuncClientDisconnectModeClient: public HyCmdFuncClientDisconnectMode
{
public:
    HyCmdFuncClientDisconnectModeClient(int hd):
        HyCmdFuncClientDisconnectMode(hd)
    {
    }

    virtual int doCommand(void)
    {
        m_sock->send(&m_hd, 4);
        printf("client disconnect\n");
        return DO_CMD_ST_BREAK;
    }

};


class HyCmdFuncClientFactory : public HyCmdFuncBaseFactory
{
    void initMap(void)
    {
        insert_map<HyCmdFuncPutFileModeClient>(HY_KEY_PUT_FILE_MODE);
        insert_map<HyCmdFuncGetFileModeClient>(HY_KEY_GET_FILE_MODE);
        insert_map<HyCmdFuncFreeCmdModeClient>(HY_KEY_FREE_CMD_MODE);
        insert_map<HyCmdFuncFreeCmdModeClient>(HY_KEY_FREE_CMD_LOCAL_INFO_MODE);
        insert_map<HyCmdFuncKillServerModeClient>(0x1016baba);
        insert_map<HyCmdFuncClientDisconnectModeClient>(HY_KEY_CLIENT_DISCONNECT);
        //help

    }

public:
    HyCmdFuncClientFactory(void):
        HyCmdFuncBaseFactory()
    {
        initMap();
    }

    HyCmdFuncClientFactory(HySocketBase * cs):
        HyCmdFuncBaseFactory()
    {
        initMap();
        setAllSockets(cs);

    }

};
#endif /* end of include guard: HYREMOTE_COMMANDS_CLIENT_H */
