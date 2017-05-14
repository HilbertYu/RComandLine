#ifndef HYREMOTE_COMMANDS_H
#define HYREMOTE_COMMANDS_H

#include "hysocket.h"


class HyCmdFuncBase
{
protected:
    int m_hd;
    HySocketBase * m_sock;

public:
    HyCmdFuncBase(int hd):
        m_hd(hd),
        m_sock(NULL)
    {

    }

    enum DoCmdSt
    {
        DO_CMD_ST_OK = 0,
        DO_CMD_ST_ERROR = -1,
        DO_CMD_ST_BREAK = 1

    };

    virtual ~HyCmdFuncBase(void){};


    HyCmdFuncBase& setSocket(HySocketBase * cs)
    {
        m_sock = cs;
        return *this;
    }

    virtual int doCommand(void) = 0;

};

class HyCmdFuncPutFileMode: public HyCmdFuncBase
{
public:
    HyCmdFuncPutFileMode(int hd): HyCmdFuncBase(hd) { }
    virtual int doCommand(void) = 0;
    virtual ~HyCmdFuncPutFileMode(void){};

};

class HyCmdFuncGetFileMode: public HyCmdFuncBase
{
public:
    HyCmdFuncGetFileMode(int hd): HyCmdFuncBase(hd) { }
    virtual int doCommand(void) = 0;
    virtual ~HyCmdFuncGetFileMode(void){};

};

class HyCmdFuncFreeCmdMode: public HyCmdFuncBase
{
public:
    HyCmdFuncFreeCmdMode(int hd): HyCmdFuncBase(hd) { }
    virtual int doCommand(void) = 0;
    virtual ~HyCmdFuncFreeCmdMode(void){};

};

class HyCmdFuncKillServerMode: public HyCmdFuncBase
{
public:
    HyCmdFuncKillServerMode(int hd): HyCmdFuncBase(hd) { }
    virtual int doCommand(void) = 0;
    virtual ~HyCmdFuncKillServerMode(void){};

};

class HyCmdFuncClientDisconnectMode: public HyCmdFuncBase
{
public:
    HyCmdFuncClientDisconnectMode(int hd): HyCmdFuncBase(hd) { }
    virtual int doCommand(void) = 0;
    virtual ~HyCmdFuncClientDisconnectMode(void){};

};

class HyCmdFuncBaseFactory
{
protected:
    std::map<int, HyCmdFuncBase*> raw_map;

    template <typename T>
    void insert_map(int hd)
    {
        typedef std::pair<int, T*> pair_t;
        raw_map.insert(pair_t(hd, new T(hd)));

    }

    void setAllSockets(HySocketBase * sc)
    {
        std::map<int, HyCmdFuncBase*>::const_iterator itr = raw_map.begin();
        while (itr != raw_map.end())
        {
            itr->second->setSocket(sc);
            ++itr;
        }

    }

    virtual void initMap(void) = 0;

public:
    HyCmdFuncBase*  getFunc(int hd)
    {
        std::map<int, HyCmdFuncBase*>::const_iterator itr = raw_map.find(hd);
        if (itr == raw_map.end())
            return NULL;

        return itr->second;

    }

    virtual ~HyCmdFuncBaseFactory(void) { }
        
    
};

#endif /* end of include guard: HYREMOTE_COMMANDS_H */
