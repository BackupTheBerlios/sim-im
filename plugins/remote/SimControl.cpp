/***************************************************************************
                          SimControl.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "Simremote.h"
#include "SimControl.h"

#include "remote.h"

using namespace std;

class IPC
{
public:
    IPC();
    ~IPC();
    string prefix();
    bool isInit();
    bool process(const BSTR &in_str, BSTR *out_str);
protected:
    unsigned *s;
    HANDLE hMem;
    HANDLE hMutex;
    HANDLE hEventIn;
    HANDLE hEventOut;
    friend class IPCLock;
};

class IPCLock
{
public:
    IPCLock(IPC *ipc);
    ~IPCLock();
protected:
    IPC *m_ipc;
};

IPC::IPC()
{
    s = NULL;
    string name = prefix() + "mem";
    hMem = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    if (hMem)
        s = (unsigned*)MapViewOfFile(hMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (s)
        memset(s, 0, N_SLOTS * sizeof(unsigned));
    name = prefix() + "mutex";
    hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, name.c_str());
    name = prefix() + "in";
    hEventIn = OpenEventA(EVENT_ALL_ACCESS, FALSE, name.c_str());
    name = prefix() + "out";
    hEventOut = OpenEventA(EVENT_ALL_ACCESS, FALSE, name.c_str());
}

IPC::~IPC()
{
    if (s)
        UnmapViewOfFile(s);
    if (hMem)
        CloseHandle(hMem);
    if (hMutex)
        CloseHandle(hMutex);
    if (hEventIn)
        CloseHandle(hEventIn);
    if (hEventOut)
        CloseHandle(hEventOut);
}

#ifndef SM_REMOTECONTROL
#define SM_REMOTECONTROL	0x2001
#endif
#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION	0x1000
#endif

string IPC::prefix()
{
    string res;
    if (GetSystemMetrics(SM_REMOTECONTROL) || GetSystemMetrics(SM_REMOTESESSION))
        res = "Global/";
    res += SIM_SHARED;
    return res;
}

bool IPC::isInit()
{
    return hMem && hMutex && hEventIn && hEventOut;
}

bool IPC::process(const BSTR &in_str, BSTR *out_str)
{
    unsigned i = 0;
    HANDLE hMem = NULL;
    unsigned short *mem;
    {
        IPCLock lock(this);
        for (i = 0; i < N_SLOTS; i++){
            if (s[i] != SLOT_NONE)
                continue;
            string name = prefix();
            char b[5];
            sprintf(b, "%u", i);
            name += b;
            hMem = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 0x8000, name.c_str());
            if (hMem == NULL)
                return false;
            mem = (unsigned short*)MapViewOfFile(hMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if (mem == NULL)
                return false;
            memcpy(mem, in_str, (wcslen(in_str) + 1) * 2);
            s[i] = SLOT_IN;
            break;
        }
        if (i >= N_SLOTS)
            return false;
    }
    SetEvent(hEventIn);
    for (;;){
        WaitForSingleObject(hEventOut, INFINITE);
        IPCLock lock(this);
        if (s[i] == SLOT_IN)
            continue;
        if (s[i] == SLOT_OUT){
            CComBSTR res((int)mem);
            *out_str = res.Copy();
        }
        UnmapViewOfFile(mem);
        CloseHandle(hMem);
        break;
    }
    return true;
}

IPCLock::IPCLock(IPC *ipc)
{
    m_ipc = ipc;
    WaitForSingleObject(m_ipc->hMutex, INFINITE);
}

IPCLock::~IPCLock()
{
    ReleaseMutex(m_ipc->hMutex);
}

extern "C" __declspec(dllexport) bool ProcessStr(BSTR in_str, BSTR *out_str)
{
    IPC ipc;
    if (!ipc.isInit())
        return false;
    if (!ipc.process(in_str, out_str))
        return false;
    return true;
}

STDMETHODIMP CSimControl::get_Running(BOOL *pVal)
{
    IPC ipc;
    *pVal = ipc.isInit();
    return S_OK;
}

STDMETHODIMP CSimControl::Process(BSTR in_str, BSTR *out_str)
{
    if (ProcessStr(in_str, out_str))
        return S_OK;
    return S_FALSE;
}

