#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef HAVE_STDLIB_H
    #include <stdlib.h>
#endif
#ifdef HAVE_STDDEF_H
    #include <stddef.h>
#endif
#ifdef HAVE_INTTYPES_H
    #include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
    #include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

#ifndef WIN32
    #include <sys/socket.h>
    #include <unistd.h>
    #include <sys/un.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <pwd.h>
    #include <signal.h>
#endif

#include <string>
#include <list>
#include <memory>

#ifdef WIN32
    #include <windows.h>
    #define socklen_t	int
    #define in_addr_t int
#endif

#ifndef INADDR_NONE
    #define INADDR_NONE     0xFFFFFFFF
#endif

using namespace std;

class Processor
{
public:
    Processor() { bInit = false; };
    virtual ~Processor() {};
    virtual bool process(const char *in_str, string &out_str) = 0;
    bool bInit;
};

class SocketProcessor : public Processor
{
public:
    SocketProcessor(int s);
    ~SocketProcessor();
    virtual bool process(const char *in_str, string &out_str);
protected:
    int m_s;
};

SocketProcessor::SocketProcessor(int s)
{
    m_s = s;
    if (s == -1)
        return;
    bool bCR = false;
    for (;;){
        char c;
        int rs = recv(m_s, &c, 1, 0);
        if (rs <= 0)
            return;
        if (bCR && (c == '>'))
            break;
        bCR = (c == '\n');
    }
    bInit = true;
}

SocketProcessor::~SocketProcessor()
{
    if (m_s != -1)
#ifdef WIN32
        closesocket(m_s);
#else
        close(m_s);
#endif
}

bool SocketProcessor::process(const char *in_str, string &out_str)
{
    if (m_s == -1)
        return false;
    string ss;
    ss = in_str;
    ss += "\n";
    unsigned size = ss.length();
    const char *p = ss.c_str();
    while (size){
        int ws = send(m_s, p, size, 0);
        if (ws <= 0)
            return false;
        size -= ws;
    }
    string out;
    bool bCR = false;
    for (;;){
        char c;
        int rs = recv(m_s, &c, 1, 0);
        if (rs <= 0)
            return false;
        if (c == '\r')
            continue;
        if (bCR && (c == '>'))
            break;
        out += c;
        bCR = (c == '\n');
    }
    out = out.substr(0, out.length() - 1);
    if ((out.length() >= 2) && (out.substr(0, 2) == "? ")){
        out_str = "?";
        out_str += out.substr(2);
    }else{
        out_str += ">";
        out_str += out;
    }
    return true;
}

#ifdef WIN32
#define WS_VERSION_REQD   0x0101
#endif

Processor *createTCPProcessor(const char *addr_str)
{
    string addr;
    if (addr_str)
        addr = addr_str;
    if (addr.length() <= 4)
        return NULL;
    if (addr.substr(0, 4) != "tcp:")
        return NULL;
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(WS_VERSION_REQD, &wsaData);
#endif
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s != -1){
        sockaddr_in addr;
        addr.sin_family		 = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port		 = 0;
        socklen_t addr_len = sizeof(addr);
        if (bind(s, (sockaddr*)&addr, addr_len) == -1){
#ifdef WIN32
            closesocket(s);
#else
            close(s);
#endif
            s = -1;
        }
    }
    if (s != -1){
        addr = addr.substr(4);
        string   host;
        unsigned port;
        int n = addr.find(':');
        if (n > 0){
            host = addr.substr(0, n);
            port = atol(addr.substr(n + 1).c_str());
        }else{
            port = atol(addr.c_str());
        }
        in_addr_t ip = INADDR_NONE;
        if (port){
            if (!host.empty()){
                ip = inet_addr(host.c_str());
                hostent *h = gethostbyname(host.c_str());
                if (h)
                    ip = *((int*)(*(h->h_addr_list)));
            }else{
                ip = inet_addr("127.0.0.1");
            }
        }
        if (ip == INADDR_NONE){
#ifdef WIN32
            closesocket(s);
#else
            close(s);
#endif
            s = -1;
        }else{
            sockaddr_in addr;
            addr.sin_family		 = AF_INET;
            addr.sin_addr.s_addr = ip;
            addr.sin_port        = htons((u_short)port);
            socklen_t addr_len = sizeof(addr);
            if (connect(s, (sockaddr*)&addr, addr_len) == -1){
#ifdef WIN32
                closesocket(s);
#else
                close(s);
#endif
                s = -1;
            }
        }
    }
    return new SocketProcessor(s);
}

#ifdef WIN32

#include <ole2.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlconv.h>

class AutoProcessor : public Processor
{
public:
    AutoProcessor();
    virtual bool process(const char *in_str, string &out_str);
protected:
    CComDispatchDriver disp;
};

AutoProcessor::AutoProcessor()
{
    CoInitialize(NULL);

    CLSID	clsid;
    HRESULT	hResult;
    hResult = CLSIDFromProgID(L"Simremote.SimControl", &clsid);
    if (FAILED(hResult))
        return;

    CComPtr<IUnknown> pUnk;
    hResult = pUnk.CoCreateInstance(clsid);
    if (FAILED(hResult))
        return;
    disp = pUnk;
    bInit = true;
}

bool AutoProcessor::process(const char *in_str, string &out_str)
{
    if (!bInit)
        return false;
    CComBSTR str(in_str);
    CComVariant vArg(str);
    CComVariant vRes;
    HRESULT hResult = disp.Invoke1(L"Process", &vArg, &vRes);
    if (FAILED(hResult))
        return false;
    CComBSTR out(vRes.bstrVal);
    if (out.Length()){
        size_t size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), 0, 0, NULL, NULL);
        char *res = new char[size + 1];
        size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), res, size, NULL, NULL);
        res[size] = 0;
        out_str = res;
        delete[] res;
    }else{
        return false;
    }
    return true;
}

Processor *createProcessor(const char *addr_str)
{
    Processor *processor = createTCPProcessor(addr_str);
    if (processor)
        return processor;
    string addr;
    if (addr_str)
        addr = addr_str;
    if (addr.empty())
        addr = "auto:";
    int n = addr.find(':');
    if (n < 0)
        return NULL;
    string proto = addr.substr(0, n);
    addr = addr.substr(n + 1);
    if (proto == "auto")
        return new AutoProcessor;
    return NULL;
}

#else

Processor *createProcessor(const char *addr_str)
{
    Processor *processor = createTCPProcessor(addr_str);
    if (processor != NULL)
        return processor;
    string addr = "/tmp/sim.%user%";
    if (addr_str)
        addr = addr_str;
    int n = addr.find("%user%");
    if (n >= 0){
        uid_t uid = getuid();
        struct passwd *pwd = getpwuid(uid);
        string user;
        if (pwd){
            user = pwd->pw_name;
        }else{
            char b[32];
            sprintf(b, "%u", uid);
            user = b;
        }
        addr = addr.substr(0, n) + user + addr.substr(n + 6);
    }
    int s = socket(PF_UNIX, SOCK_STREAM, 0);
    if (s < 0){
        fprintf(stderr, "Can't create socket: %s\n", strerror(errno));
        return NULL;
    }

    char local_name[256];
    int tmpfd;
    strcpy(local_name, "/tmp/sim.XXXXXX");
    if ((tmpfd = mkstemp(local_name)) == -1)
        return NULL;
    close(tmpfd);

    struct sockaddr_un sun_local;
    sun_local.sun_family = AF_UNIX;
    strcpy(sun_local.sun_path, local_name);
    unlink(local_name);
    if (bind(s, (struct sockaddr*)&sun_local, sizeof(sun_local)) < 0){
        fprintf(stderr, "Can't bind socket %s: %s\n", local_name, strerror(errno));
        return NULL;
    }

    struct sockaddr_un sun_remote;
    sun_remote.sun_family = AF_UNIX;
    strcpy(sun_remote.sun_path, addr.c_str());
    if (connect(s, (struct sockaddr*)&sun_remote, sizeof(sun_remote)) < 0){
        fprintf(stderr, "Can't connect to %s: %s\n", addr.c_str(), strerror(errno));
        unlink(local_name);
        return NULL;
    }
    unlink(local_name);
    return new SocketProcessor(s);
}

#endif

static void usage(char *s)
{
#ifndef WIN32
    uid_t uid = getuid();
    struct passwd *pwd = getpwuid(uid);
    char uid_buf[256];
    char *name = NULL;
    if (pwd){
        name = pwd->pw_name;
    }else{
        snprintf(uid_buf, sizeof(uid_buf), "%u", uid);
        name = uid_buf;
    }
#endif
    fprintf(stderr,
            "usage: %s [options] [uin files]\n"
            "Options are: [followed by default value]:\n"
#ifndef WIN32
            "      -s socket	[/tmp/sim.%s] Control socket\n"
#endif
            //            "      -d                         Debug mode\n"
            "      -c command                 Command\n"
            "      -h                         Show this help\n"
            "\n",
            s
#ifndef WIN32
            ,name
#endif
           );
}

int main(int, char **argv)
{
    list<string> uins;
    const char *cmd  = NULL;
    const char *addr = NULL;
    for (char **p = argv + 1; *p; p++){
        if (!strcmp(*p, "-h")){
            usage(argv[0]);
            return 0;
        }
        if (!strcmp(*p, "-s")){
            p++;
            if (*p == NULL){
                usage(argv[0]);
                return 1;
            }
            addr = *p;
            continue;
        }
        if (!strcmp(*p, "-c")){
            p++;
            if (*p == NULL){
                usage(argv[0]);
                return 1;
            }
            cmd = *p;
            continue;
        }
        uins.push_back(*p);
    }
    auto_ptr<Processor> processor(createProcessor(addr));
    if ((processor.get() == NULL) || !processor->bInit){
        fprintf(stderr, "Can't create processor\n");
        return 1;
    }
    if (uins.size()){
        for (list<string>::iterator it = uins.begin(); it != uins.end(); ++it){
            string in_str;
            in_str = "FILE \"";
            in_str += *it;
            in_str += "\"";
            string out_str;
            if (!processor->process(in_str.c_str(), out_str))
                break;
        }
        return 0;
    }
    if (cmd){
        string out_str;
        if (!processor->process(cmd, out_str)){
            fprintf(stderr, "Can't execute %s\n", cmd);
            exit(1);
        }
        if (out_str.empty()){
            fprintf(stderr, "No answer\n");
            exit(1);
        }
        if (out_str[0] != '>'){
            fprintf(stderr, "Execute %s fail\n", cmd);
            exit(1);
        }
        printf("%s\n", out_str.c_str() + 1);
        exit(0);
    }
    FILE *f = stdin;
    while (!feof(f) && !ferror(f)){
        printf(">");
        char buf[4096];
        char *line = fgets(buf, sizeof(buf), f);
        if (line == NULL)
            break;
        if (strlen(line) && (line[strlen(line) - 1] == '\n'))
            line[strlen(line) - 1] = 0;
        if (strlen(line) && (line[strlen(line) - 1] == '\r'))
            line[strlen(line) - 1] = 0;
        string out_str;
        if (!processor->process(line, out_str))
            break;
        if (out_str.empty())
            break;
        bool bBad = (out_str[0] == '?');
        out_str = out_str.substr(1);
        if (bBad)
            printf("? ");
        printf("%s\n", out_str.c_str());
    }
    return 0;
}
