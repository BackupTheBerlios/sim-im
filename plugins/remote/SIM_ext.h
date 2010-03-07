// SIM_ext.h : Declaration of the CSIM_ext

#ifndef __SIM_EXT_H_
#define __SIM_EXT_H_

#include "resource.h"       // main symbols
#include "comdef.h"
#include "shlobj.h"
#include "IShellExtInitImpl.h"			// IShellExtInit
#include "IContextMenuImpl.h"			// IContextMenu
#include <shlguid.h>
#include <shobjidl.h>
#include <shlobj.h>

#if _MSC_VER > 1020
#include <yvals.h>              
#pragma warning(disable: 4097)
#pragma warning(disable: 4244)  
#pragma warning(disable: 4275)
#pragma warning(disable: 4514)
#pragma warning(disable: 4710)  
#pragma warning(disable: 4786)  
#pragma warning(push)
#pragma warning(disable: 4018)  
#pragma warning(disable: 4100)  
#pragma warning(disable: 4146)  
#pragma warning(disable: 4511)  
#pragma warning(disable: 4512)  
#pragma warning(disable: 4530)  
#pragma warning(disable: 4663)  
#endif

#include <string>
#include <list>
#include <map>

struct ItemInfo
{
    std::string		text;
    HICON		icon;
    unsigned	id;
};

typedef std::map<std::string, HICON>		ICON_MAP;
typedef std::map<unsigned, ItemInfo>	ITEM_MAP;

struct __declspec(uuid("000214e4-0000-0000-c000-000000000047"))
IContextMenu;

_COM_SMARTPTR_TYPEDEF(IContextMenu, __uuidof(IContextMenu));

struct __declspec(uuid("000214e4-0000-0000-c000-000000000048"))
IContextMenu2;

_COM_SMARTPTR_TYPEDEF(IContextMenu2, __uuidof(IContextMenu2));

struct __declspec(uuid("000214e4-0000-0000-c000-000000000049"))
IContextMenu3;

_COM_SMARTPTR_TYPEDEF(IContextMenu3, __uuidof(IContextMenu3));

/////////////////////////////////////////////////////////////////////////////
// CSIM_ext
class ATL_NO_VTABLE CSIM_ext :
            public CComObjectRootEx<CComSingleThreadModel>,
            public CComCoClass<CSIM_ext, &CLSID_SIM_ext>,
            public ISIM_ext,
            public IObjectWithSiteImpl<CSIM_ext>,
            public IShellExtInitImpl,
            public IContextMenuImpl
{
public:
    CSIM_ext();
    ~CSIM_ext();
    LPDATAOBJECT lpData;
    static HINSTANCE hInstance;

    DECLARE_REGISTRY_RESOURCEID(IDR_SIM_EXT)
    DECLARE_NOT_AGGREGATABLE(CSIM_ext)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSIM_ext)
    COM_INTERFACE_ENTRY(ISIM_ext)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IContextMenu)
    COM_INTERFACE_ENTRY(IContextMenu2)
    COM_INTERFACE_ENTRY(IContextMenu3)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    END_COM_MAP()

    ICON_MAP	  m_icons;
    ITEM_MAP	  m_items;
    ItemInfo	  getItemInfo(unsigned id);
    HICON		  createIcon(const char *name);
    HBITMAP		  createBitmap(std::string &info);
    unsigned	  CmdBase;

    // ISIM_ext
public:
    STDMETHOD(QueryContextMenu)(THIS_
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags);

    STDMETHOD(InvokeCommand)(THIS_
                             LPCMINVOKECOMMANDINFO lpici);

    STDMETHOD(HandleMenuMsg)(UINT, WPARAM, LPARAM);

    STDMETHOD(HandleMenuMsg2)(UINT, WPARAM, LPARAM, LRESULT *);

    STDMETHOD(GetCommandString)(THIS_
                                UINT        idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax);

    STDMETHOD(Initialize)(THIS_ LPCITEMIDLIST pidlFolder,
                          LPDATAOBJECT lpdobj, HKEY hkeyProgID);
private:
    void MeasureItem(LPMEASUREITEMSTRUCT);
    void DrawMenuItem(LPDRAWITEMSTRUCT);
};

#endif //__SIM_EXT_H_
