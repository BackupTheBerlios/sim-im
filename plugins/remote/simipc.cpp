/***************************************************************************
                          simipc.cpp  -  description
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

extern "C" bool ProcessStr(BSTR in_str, BSTR *out_str);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdLine, int)
{
    CComBSTR in("FILE ");
    in.Append(cmdLine);
    CComBSTR out;
    ProcessStr(in, &out);
    return 0;
}
