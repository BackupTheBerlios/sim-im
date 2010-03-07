/******************************************************************
***
***
***				FREE WINDOWLESS FLASH CONTROL
***
***					   by Makarov Igor
***
***		for questions and remarks mailto: mak_july@list.ru
***
***
*******************************************************************/
// FlashWnd.cpp: implementation of the COleContainerWnd class.
//
//////////////////////////////////////////////////////////////////////

//for help and reference see: http://www.codeproject.com/KB/COM/flashcontrol.aspx


#include "stdafx.h"
#include "FlashWnd.h"

CFlashWnd::CFlashWnd()
{
	m_lVersion = 0;
}
CFlashWnd::~CFlashWnd()
{
}

//DShockwaveFlashEvents
HRESULT STDMETHODCALLTYPE CFlashWnd::OnReadyStateChange(long newState)
{
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CFlashWnd::OnProgress(long percentDone)
{
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CFlashWnd::FSCommand(_bstr_t command, _bstr_t args)
{
	return S_OK;
}

void CFlashWnd::OnErrorClassNotReg()
{
	//some notification code here
}
BOOL CFlashWnd::OnBeforeShowingContent()
{
	m_lVersion = m_lpControl->FlashVersion();
	if ((m_lVersion & 0x00FF0000) == 0x00080000)
		m_bFixTransparency = TRUE;
	else
		m_bFixTransparency = FALSE;
	HRESULT hr;
	hr = m_lpControl->QueryInterface(IID_IConnectionPointContainer, (void**)&m_lpConCont);
	if (FAILED(hr))
		return FALSE;
	hr = m_lpConCont->FindConnectionPoint(ShockwaveFlashObjects::DIID__IShockwaveFlashEvents, &m_lpConPoint);
	if (FAILED(hr))
		return FALSE;
	hr = m_lpConPoint->Advise((ShockwaveFlashObjects::_IShockwaveFlashEvents *)this, &m_dwConPointID);
	if (FAILED(hr))
		return FALSE;

	if (m_bTransparent)
		m_lpControl->PutWMode(L"transparent");
	m_lpControl->PutScale(L"showAll");
	m_lpControl->PutBackgroundColor(0x00000000);
	m_lpControl->PutEmbedMovie(FALSE);
	return TRUE;
}
BOOL CFlashWnd::OnAfterShowingContent()
{
	HRESULT hr;
	m_lpControl->PutEmbedMovie(TRUE);
	hr = m_lpControl->LoadMovie(0, L"http://www.djdean.de/dean_bot.swf");
//	hr = m_lpControl->LoadMovie(0, L"c:\\mailru.swf");
	if (FAILED(hr))
		return FALSE;
	hr = m_lpControl->Play();
	if (FAILED(hr))
		return FALSE;
	return TRUE;
}

//IUnknown

HRESULT STDMETHODCALLTYPE CFlashWnd::QueryInterface(REFIID riid, void ** ppvObject)
{
	HRESULT hr = COleContainerWnd<ShockwaveFlashObjects::IShockwaveFlash>::QueryInterface(riid, ppvObject);
	if (hr != E_NOINTERFACE)
		return hr;
	if (IsEqualGUID(riid, ShockwaveFlashObjects::DIID__IShockwaveFlashEvents))
		*ppvObject = (void*)dynamic_cast<ShockwaveFlashObjects::_IShockwaveFlashEvents *>(this);
	else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
	if (!(*ppvObject))
		return E_NOINTERFACE; //if dynamic_cast returned 0
	m_iRef++;
	return S_OK;
}

ULONG STDMETHODCALLTYPE CFlashWnd::AddRef()
{
	m_iRef++;
	return m_iRef;
}

ULONG STDMETHODCALLTYPE CFlashWnd::Release()
{
	m_iRef--;
	return m_iRef;
}

//IDispatch
HRESULT STDMETHODCALLTYPE CFlashWnd::GetTypeInfoCount(UINT __RPC_FAR *pctinfo)
{
	NOTIMPLEMENTED;
}
HRESULT STDMETHODCALLTYPE CFlashWnd::GetTypeInfo( 
    /* [in] */ UINT iTInfo,
    /* [in] */ LCID lcid,
    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
	NOTIMPLEMENTED;
}
HRESULT STDMETHODCALLTYPE CFlashWnd::GetIDsOfNames( 
    /* [in] */ REFIID riid,
    /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
    /* [in] */ UINT cNames,
    /* [in] */ LCID lcid,
    /* [size_is][out] */ DISPID __RPC_FAR *rgDispId)
{
	NOTIMPLEMENTED;
}
HRESULT STDMETHODCALLTYPE CFlashWnd::Invoke( 
    /* [in] */ DISPID dispIdMember,
    /* [in] */ REFIID riid,
    /* [in] */ LCID lcid,
    /* [in] */ WORD wFlags,
    /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
    /* [out] */ VARIANT __RPC_FAR *pVarResult,
    /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
    /* [out] */ UINT __RPC_FAR *puArgErr)
{
/*	switch (dispIdMember)
	{
	default:
		return S_OK;
	}*/
	return S_OK;
}
