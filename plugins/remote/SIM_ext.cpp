// SIM_ext.cpp : Implementation of CSIM_ext
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "Simext.h"
#include "SIM_ext.h"
#include <stdio.h>

using namespace std;

static bool (*ProcessStr)(BSTR in_str, BSTR *out_str) = NULL;

static string getToken(string &from, char c)
{
    string res;
    unsigned i;
    for (i = 0; i < from.length(); i++){
        if (from[i] == c)
            break;
        if (from[i] == '\\'){
            i++;
            if (i >= from.length())
                break;
            res += from[i];
            continue;
        }
        res += from[i];
    }
    if (i < from.length()){
        from = from.substr(i + 1);
    }else{
        from = "";
    }
    return res;
}

HINSTANCE CSIM_ext::hInstance;

/////////////////////////////////////////////////////////////////////////////
// CSIM_ext

CSIM_ext::CSIM_ext()
{
    lpData = NULL;
    if (ProcessStr == NULL){
        WCHAR name[512];
        GetModuleFileName(hInstance, name, sizeof(name));
		
		char* namestr=(char *)malloc( 512 );
		size_t   i;
		wcstombs_s(&i, namestr, (size_t)512, name, (size_t)512 );

        char *r = strrchr(namestr, '\\');
        if (r){
            r++;
        }else{
            r = namestr;
        }
        strcpy(r, "simremote.dll");
        HINSTANCE hLib = LoadLibrary(name);
        (DWORD&)ProcessStr = (DWORD)GetProcAddress(hLib, "ProcessStr");
    }
}

CSIM_ext::~CSIM_ext()
{
    for (ICON_MAP::iterator it = m_icons.begin(); it != m_icons.end(); ++it)
        DestroyIcon((*it).second);
    if (lpData)
        lpData->Release();
}

HRESULT CSIM_ext::QueryContextMenu(HMENU hmenu,
                                   UINT indexMenu,
                                   UINT idCmdFirst,
                                   UINT idCmdLast,
                                   UINT uFlags)
{
    if ((lpData == NULL) || (uFlags & CMF_DEFAULTONLY))
        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

    CmdBase = idCmdFirst;

    if (((uFlags & 0x0000000F) == CMF_NORMAL) || (uFlags & CMF_EXPLORE)){
        STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
        FORMATETC formatetc = { CF_HDROP,
                                NULL,
                                DVASPECT_CONTENT,
                                -1,
                                TYMED_HGLOBAL
                              };
        HRESULT hr = lpData->GetData(&formatetc, &stgmedium);
        if (!SUCCEEDED(hr))
            return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
        //char *drop_files = (char*)GlobalLock(stgmedium.hGlobal);
        GlobalUnlock(stgmedium.hGlobal);

        CComBSTR in("CONTACTS 3");
        CComBSTR out;
        unsigned cmd_id = idCmdFirst;
        if (ProcessStr && ProcessStr(in, &out)){
            HMENU hMain = NULL;
            HMENU hSub  = NULL;
            size_t size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), 0, 0, NULL, NULL);
            char *res = new char[size + 1];
            size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), res, size, NULL, NULL);
            res[size] = 0;
            if (res[0] == '>'){
                string r = res + 1;
                string line = getToken(r, '\n');
                unsigned nContacts = atol(getToken(line, ' ').c_str());
                unsigned nGroups   = atol(line.c_str());
                bool bSubMenu = false;
                if ((nContacts > 20) && (nGroups > 1)){
                    hMain = CreatePopupMenu();
                    bSubMenu = true;
                }
                unsigned old_grp = (unsigned)(-1);
                while (!r.empty()){
                    line = getToken(r, '\n');
                    if (line.empty())
                        continue;
                    unsigned id  = atol(getToken(line, ' ').c_str());
                    unsigned grp = atol(getToken(line, ' ').c_str());
                    string icon  = getToken(line, ' ');
                    if (!line.empty()){
                        if (hMain){
                            if (grp != old_grp){
                                old_grp = grp;
                                if (bSubMenu){
                                    char *res = NULL;
                                    hSub = CreatePopupMenu();
                                    char *grp = "Group";
                                    char cmd[64];
                                    sprintf(cmd, "GROUP %u", old_grp);
                                    CComBSTR in(cmd);
                                    CComBSTR out;
                                    if (ProcessStr && ProcessStr(in, &out)){
                                        size_t size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), 0, 0, NULL, NULL);
                                        char *res = new char[size + 1];
                                        size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), res, size, NULL, NULL);
                                        res[size] = 0;
                                        if (res[0] == '>')
                                            grp = res + 1;
                                    }

									wchar_t *grpLPCWSTR      = (wchar_t *)malloc( sizeof( wchar_t ));
									wcstombs( grp, grpLPCWSTR,  size + 1 );
                                    AppendMenu(hMain, MF_POPUP | MF_STRING, (unsigned)hSub, grpLPCWSTR);
                                    if (res)
                                        delete[] res;
                                }else{
                                    AppendMenu(hSub, MF_SEPARATOR, 0, NULL);
                                }
                            }
                        }else{
                            hMain = CreatePopupMenu();
                            hSub  = hMain;
                        }
                        ItemInfo info;
                        info.text  = line.c_str();
                        info.icon  = createIcon(icon.c_str());
                        info.id	   = id;
                        m_items.insert(ITEM_MAP::value_type(cmd_id, info));

						const char* linestr = line.c_str();

                        AppendMenuA(hSub, MF_STRING | MF_OWNERDRAW, cmd_id, linestr);
                        cmd_id++;
                    }
                }
            }
            delete[] res;
            if (hMain != NULL)
                InsertMenu(hmenu, indexMenu++, MF_POPUP|MF_BYPOSITION,
                           (UINT)hMain, L"Send to SIM contact");
        }
        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, cmd_id - idCmdFirst);
    }
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
}

static char fromHex(char c)
{
    if ((c >= '0') && (c <= '9')) return (char)(c - '0');
    if ((c >= 'A') && (c <= 'F')) return (char)(c + 10 - 'A');
    if ((c >= 'a') && (c <= 'f')) return (char)(c + 10 - 'a');
    return (char)0;
}

static void *unpackData(const char *p)
{
    char *res = (char*)malloc(strlen(p) / 2);
    char *t = res;
    for (; p[0] && p[1]; p += 2)
        *(t++) = (fromHex(p[0]) << 4) + fromHex(p[1]);
    return res;
}

HBITMAP CSIM_ext::createBitmap(string &info)
{
    BITMAPINFO *bmp = (BITMAPINFO*)unpackData(getToken(info, '\n').c_str());
    void *bits = unpackData(getToken(info, '\n').c_str());
    HDC hDC = CreateCompatibleDC(NULL);
    HBITMAP hRes = CreateBitmap(bmp->bmiHeader.biWidth, bmp->bmiHeader.biHeight, bmp->bmiHeader.biPlanes, bmp->bmiHeader.biBitCount, NULL);
    SetDIBits(hDC, hRes, 0, bmp->bmiHeader.biHeight, bits, bmp, DIB_RGB_COLORS);
    DeleteDC(hDC);
    free(bmp);
    free(bits);
    return hRes;
}

HICON CSIM_ext::createIcon(const char *name)
{
    ICON_MAP::iterator it = m_icons.find(name);
    if (it != m_icons.end())
        return (*it).second;
    string cmd = "ICON ";
    cmd += name;
    CComBSTR in(cmd.c_str());
    CComBSTR out;
    if (!ProcessStr || !ProcessStr(in, &out))
        return NULL;
    size_t size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), 0, 0, NULL, NULL);
    char *res = new char[size + 1];
    size = WideCharToMultiByte(CP_ACP, 0, out, wcslen(out), res, size, NULL, NULL);
    res[size] = 0;
    if (res[0] != '>')
        return NULL;
    string r = res + 1;
    ICONINFO info;
    info.fIcon    = TRUE;
    info.xHotspot = 8;
    info.yHotspot = 8;
    info.hbmMask  = createBitmap(r);
    info.hbmColor = createBitmap(r);
    HICON hIcon = CreateIconIndirect(&info);
    m_icons.insert(ICON_MAP::value_type(name, hIcon));
    DeleteObject(info.hbmMask);
    DeleteObject(info.hbmColor);
    return hIcon;
}

#ifndef CF_HDROP
#define CF_HDROP            15
#endif

HRESULT CSIM_ext::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    if (lpData == NULL)
        return S_OK;
    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { CF_HDROP,
                            NULL,
                            DVASPECT_CONTENT,
                            -1,
                            TYMED_HGLOBAL
                          };
    HRESULT hr = lpData->GetData(&formatetc, &stgmedium);
    if (SUCCEEDED(hr)){
        char *drop_files = (char*)GlobalLock(stgmedium.hGlobal);
        DROPFILES *files = (DROPFILES*)drop_files;
        drop_files += files->pFiles;
        CComBSTR in("SENDFILE \"");
        if (files->fWide){
            in += CComBSTR(drop_files);
        }else{
            in += drop_files;
        }
        in += "\" ";
        ItemInfo info = getItemInfo(LOWORD(lpici->lpVerb) + CmdBase);
        char b[12];
        sprintf(b, "%u", info.id);
        in += b;
        GlobalUnlock(stgmedium.hGlobal);
        CComBSTR out;
        if (ProcessStr)
            ProcessStr(in, &out);
    }
    GlobalFree(stgmedium.hGlobal);
    if (lpData){
        lpData->Release();
        lpData = NULL;
    }
    return S_OK;
}

HRESULT CSIM_ext::HandleMenuMsg(UINT uMsg,WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

HRESULT CSIM_ext::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    switch(uMsg)
    {
    case WM_DRAWITEM:
        DrawMenuItem((LPDRAWITEMSTRUCT) lParam);
        break;

    case WM_MEASUREITEM:
        MeasureItem((LPMEASUREITEMSTRUCT) lParam);
        break;
    }

    return S_OK;
}


HRESULT CSIM_ext::GetCommandString(UINT        idCmd,
                                   UINT        uType,
                                   UINT      * pwReserved,
                                   LPSTR       pszName,
                                   UINT        cchMax)
{
    return S_OK;
}

HRESULT CSIM_ext::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID)
{
    if (lpData)
        lpData->Release();
    lpData = lpdobj;
    lpData->AddRef();
    return S_OK;
}

ItemInfo CSIM_ext::getItemInfo(unsigned id)
{
    ITEM_MAP::iterator it = m_items.find(id);
    if (it != m_items.end())
        return (*it).second;
    ItemInfo info;
    info.icon = NULL;
    return info;
}

void CSIM_ext::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
    if (lpmis->CtlType != ODT_MENU)
        return;
    ItemInfo info = getItemInfo(lpmis->itemID);
    if (info.icon == NULL)
        return;
    HDC hDC = CreateCompatibleDC(NULL);
    SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
    SIZE s;
    GetTextExtentPoint32A(hDC, info.text.c_str(), info.text.length(), &s);
    lpmis->itemWidth  = s.cx + GetSystemMetrics(SM_CXMENUCHECK) + GetSystemMetrics(SM_CXFRAME) * 2;
    lpmis->itemHeight = GetSystemMetrics(SM_CYMENU);
    DeleteDC(hDC);
}

void CSIM_ext::DrawMenuItem(LPDRAWITEMSTRUCT lpdis)
{
    if (lpdis->CtlType != ODT_MENU)
        return;
    ItemInfo info = getItemInfo(lpdis->itemID);
    if (info.icon == NULL)
        return;
    if (lpdis->itemAction & (ODA_DRAWENTIRE|ODA_SELECT)){
        COLORREF crText = 0, crBack = 0;
        int bgColor;
        if (lpdis->itemState & ODS_SELECTED)
        {
            bgColor = COLOR_HIGHLIGHT;
            crText = SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            crBack = SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
        }else{
            bgColor = COLOR_MENU;
        }
        FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(bgColor));
        ICONINFO icon_info;
        GetIconInfo(info.icon, &icon_info);
        BITMAP bmp;
        GetObject(icon_info.hbmColor, sizeof(BITMAP), (LPSTR)&bmp);
        DrawIconEx(lpdis->hDC,
                   lpdis->rcItem.left + (GetSystemMetrics(SM_CXMENUCHECK) - bmp.bmWidth) / 2,
                   lpdis->rcItem.top  + (lpdis->rcItem.bottom - lpdis->rcItem.top - bmp.bmHeight) / 2,
                   info.icon, bmp.bmWidth, bmp.bmHeight, 0, 0, DI_NORMAL);
        RECT rt = lpdis->rcItem;
        rt.left += GetSystemMetrics(SM_CXMENUCHECK) + GetSystemMetrics(SM_CXFRAME);
        DrawTextA(lpdis->hDC, info.text.c_str(), info.text.length(), &rt, DT_LEFT | DT_EXPANDTABS | DT_VCENTER | DT_SINGLELINE);
        if (lpdis->itemState & ODS_SELECTED)
        {
            SetTextColor(lpdis->hDC, crText);
            SetBkColor(lpdis->hDC, crBack);
        }
    }
}

