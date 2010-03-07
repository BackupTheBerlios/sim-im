// SimControl.h : Declaration of the CSimControl

#ifndef __SIMCONTROL_H_
#define __SIMCONTROL_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSimControl
class ATL_NO_VTABLE CSimControl :
            public CComObjectRootEx<CComSingleThreadModel>,
            public CComCoClass<CSimControl, &CLSID_SimControl>,
            public IDispatchImpl<ISimControl, &IID_ISimControl, &LIBID_SIMREMOTELib>
{
public:
    CSimControl()
    {
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_SIMCONTROL)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSimControl)
    COM_INTERFACE_ENTRY(ISimControl)
    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    // ISimControl
public:
    STDMETHOD(Process)(/*[in]*/ BSTR in_str, /*[out, retval]*/ BSTR *out_str);
    STDMETHOD(get_Running)(/*[out, retval]*/ BOOL *pVal);
};

#endif //__SIMCONTROL_H_
