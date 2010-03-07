; SIMINSTALL.nsi
;
; Installer written by Tobias Franz (noragen@gmx.net), 2002 - 2006 
;
; SetCompressing Options:
SetCompress auto
SetCompressor lzma

BGGradient topc

!include "MUI.nsh"
!include "LogicLib.nsh"
!include "WordFunc.nsh"

!macro BIMAGE IMAGE PARMS
	Push $0
	GetTempFileName $0
	File /oname=$0 "${IMAGE}"
	SetBrandingImage ${PARMS} $0
	Delete $0
	Pop $0
!macroend

!macro Print text
  DetailPrint "${text}"
!macroend

;Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Russian"
  ;!insertmacro MUI_LANGUAGE "French"
  ;!insertmacro MUI_LANGUAGE "French"
  ;!insertmacro MUI_LANGUAGE "Spanish"
  ;!insertmacro MUI_LANGUAGE "SimpChinese"
  ;!insertmacro MUI_LANGUAGE "TradChinese"
  ;!insertmacro MUI_LANGUAGE "Japanese"
  ;!insertmacro MUI_LANGUAGE "Korean"
  ;!insertmacro MUI_LANGUAGE "Italian"
  ;!insertmacro MUI_LANGUAGE "Dutch"
  ;!insertmacro MUI_LANGUAGE "Danish"
  ;!insertmacro MUI_LANGUAGE "Swedish"
  ;!insertmacro MUI_LANGUAGE "Norwegian"
  ;!insertmacro MUI_LANGUAGE "Finnish"
  ;!insertmacro MUI_LANGUAGE "Greek"
  ;!insertmacro MUI_LANGUAGE "Portuguese"
  ;!insertmacro MUI_LANGUAGE "PortugueseBR"
  ;!insertmacro MUI_LANGUAGE "Polish"
  ;!insertmacro MUI_LANGUAGE "Ukrainian"
  ;!insertmacro MUI_LANGUAGE "Czech"
  ;!insertmacro MUI_LANGUAGE "Slovak"
  ;!insertmacro MUI_LANGUAGE "Croatian"
  ;!insertmacro MUI_LANGUAGE "Bulgarian"
  ;!insertmacro MUI_LANGUAGE "Hungarian"
  ;!insertmacro MUI_LANGUAGE "Thai"
  ;!insertmacro MUI_LANGUAGE "Romanian"
  ;!insertmacro MUI_LANGUAGE "Latvian"
  ;!insertmacro MUI_LANGUAGE "Macedonian"
  ;!insertmacro MUI_LANGUAGE "Estonian"
  ;!insertmacro MUI_LANGUAGE "Turkish"
  ;!insertmacro MUI_LANGUAGE "Lithuanian"
  ;!insertmacro MUI_LANGUAGE "Catalan"
  ;!insertmacro MUI_LANGUAGE "Slovenian"
  ;!insertmacro MUI_LANGUAGE "Serbian"
  ;!insertmacro MUI_LANGUAGE "SerbianLatin"
  ;!insertmacro MUI_LANGUAGE "Arabic"
  ;!insertmacro MUI_LANGUAGE "Farsi"
  ;!insertmacro MUI_LANGUAGE "Hebrew"
  ;!insertmacro MUI_LANGUAGE "Indonesian"
  ;!insertmacro MUI_LANGUAGE "Mongolian"
  ;!insertmacro MUI_LANGUAGE "Luxembourgish"


; The name of the installer
Name "SIM CVS/SVN VC8 FULL-Version ${__DATE__}, ${__TIME__}"

 
Function .onInit
  
  SetSilent silent
  !insertmacro MUI_LANGDLL_DISPLAY
  SetSilent normal
  
FunctionEnd

; The file to write
OutFile "SIM-IM_SVN_${__DATE__}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\SIM

; The text to prompt the user to enter a directory
LangString welcome ${LANG_ENGLISH} "SIM-ICQ - Simple Instant Messenger, written by Vladimir Shutoff, continued by SIM-IM Development Team, is licensed under the Terms of the GPL.$\nThis SIM-IM - SVN Setup was created by Tobias Franz.$\nIt is a full-functional setup and it should contain all you need to run SIM-IM.$\n$\nPlease choose your SIM-IM - Rootdirectory now, where your sim.exe is stored:"
LangString welcome ${LANG_GERMAN}  "SIM-ICQ - Simple Instant Messenger, geschrieben von Vladimir Shutoff, weitergefьhrt vom SIM-IM Development Team, wird unter den Bedingungen der GPL lizensiert.$\nDieses SIM-IM - SVN Setup wurde von Tobias Franz erstellt.$\nEs handelt sich um ein vollstдndig funktionsfдhiges Setup und es sollte alles Nцtige enthalten, um SIM-IM auszufьhren$\n$\nBitte wдhlen Sie jetzt das SIM-IM-Installationsverzeichnis aus, wo die sim.exe liegen soll:"
LangString welcome ${LANG_RUSSIAN} "SIM-ICQ - Simple Instant Messenger, создан Владимиром Шутовым и развиваемый SIM-IM Development Team под лицензией GNU GPL.$\nДанная сборка SIM - SVN Setup выполнена Tobias Franz.$\nЭто версия содержит всё необходимое для использования SIM-IM.$\n$\nПожалуйста, выберите папку, куда следует установить SIM:"
DirText "$(welcome)"

; SetXP Style
XPStyle On
;BGGradient 000000 8000A0 FFFFFF
BGGradient 000000 B400E1 FFFFFF
InstallColors FF8080 000030
AddBrandingImage left 100
Page directory dirImage
Page instfiles 

LangString message ${LANG_ENGLISH} "I can run SIM for you now.$\nShould I do this for you?"
LangString message ${LANG_GERMAN} "Ich kann SIM fьr Sie ausfьhren.$\nSoll ich das fьr Sie ьbernehmen?"
LangString message ${LANG_RUSSIAN} "SIM-IM установлен и готов к работe.$\nЗапустить SIM-IM?"
; The stuff to install


Section "Install"
  ;Page instfiles instImage
 
  ;SetBrandingImage /RESIZETOFIT ..\Release\sim-window-small.bmp
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  SetOverwrite on
  
  File ..\Release\sim.exe
  File ..\Release\simctrl.exe
  File VC8_deploy\Microsoft.VC80.CRT.manifest
  File VC8_deploy\sim.exe.intermediate.manifest
  
  ;VC8DLL's
  File $%WINDIR%\system32\msvcm80.dll
  File $%WINDIR%\system32\msvcp80.dll
  File $%WINDIR%\system32\msvcr80.dll
  
  UnRegDLL  $INSTDIR\simremote.dll
  UnRegDLL  $INSTDIR\simext.dll
  
  File ..\Release\*.dll

  SetOutPath $INSTDIR\po
  File ..\Release\po\*.qm

  SetOutPath $INSTDIR\plugins

  File ..\Release\plugins\*.dll
  File VC8_deploy\plugins\Microsoft.VC80.CRT.manifest

  SetOutPath $INSTDIR\plugins\styles

  ;File ..\Release\plugins\styles\*.dll

  SetOutPath $INSTDIR\styles
  File ..\Release\styles\*.xsl

  SetOutPath $INSTDIR\icons
  File ..\Release\icons\*.jisp
  
  SetOutPath $INSTDIR\pict
  File ..\Release\pict\*.gif
  File ..\Release\pict\*.png
  
  SetOutPath $INSTDIR\copyright
  File ..\Release\copyright\COPYING
  
  SetOutPath $INSTDIR\sounds
  
  File ..\Release\sounds\*.wav
  
;Currently not working:
  Delete $INSTDIR\plugins\styles\wood.dll
  Delete $INSTDIR\plugins\styles\metal.dll
  ;Delete $INSTDIR\plugins\styles.dll
  Delete $INSTDIR\simremote.dll
  Delete $INSTDIR\simext.dll
  Delete $INSTDIR\sounds\*.dll
  ;RegDLL  ..\Release\simremote.dll
  ;RegDLL  ..\Release\simext.dll
  
;System:
SetOutPath $SYSDIR

File $%QTDIR%\lib\$%DLLQT%

File $%WINDIR%\system32\libeay32.dll
File $%WINDIR%\system32\ssleay32.dll


SectionEnd ; end the section

Section "Verknьpfungen"
CreateDirectory "$SMPROGRAMS\SIM"
  WriteIniStr "$INSTDIR\SIM.url" "InternetShortcut" "URL" "http://sim-im.org"
  WriteIniStr "$INSTDIR\Updates.url" "InternetShortcut" "URL" "http://www.sim-icq.de"
  CreateShortCut "$SMPROGRAMS\SIM\SIM-IM.lnk" "$INSTDIR\SIM.url" "" "$INSTDIR\SIM.url" 0
  CreateShortCut "$SMPROGRAMS\SIM\Updates.lnk" "$INSTDIR\Updates.url" "" "$INSTDIR\Updates.url" 0
  
  CreateShortCut "$SMPROGRAMS\SIM\SIM.lnk" "$INSTDIR\sim.exe" \
    "" "$INSTDIR\sim.exe" 0 SW_SHOWNORMAL
  ;CreateShortCut "$SMPROGRAMS\RERSVC\Readme.lnk" "$INSTDIR\Readme.txt" \
  ;  "" "%SystemRoot%\notepad.exe" 1 SW_SHOWMAXIMIZED 
  CreateShortCut "$SMPROGRAMS\SIM\Uninstaller.lnk" "$INSTDIR\uninst.exe" \
    "" "$INSTDIR\uninst.exe" 0 SW_SHOWNORMAL

WriteUninstaller "$INSTDIR\Uninst.exe"
SectionEnd ; end the section

 
Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) wurde erfolgreich deinstalliert.."
FunctionEnd
  
Function un.onInit 
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Mцchten Sie $(^Name) und alle seinen Komponenten deinstallieren?" IDYES +2
  Abort
FunctionEnd
 
Section "Uninstall" 

Delete $INSTDIR\Uninst.exe ; delete self (temporarily copying in temp directory ;))
Delete "$INSTDIR\sim.exe"
Delete "$INSTDIR\simctrl.exe"

UnRegDLL  "$INSTDIR\simremote.dll"
UnRegDLL  "$INSTDIR\simext.dll"
  
Delete "$INSTDIR\*.dll"
Delete "$INSTDIR\po\*.qm"
Delete "$INSTDIR\plugins\*.dll"
Delete "$INSTDIR\plugins\styles\*.dll"
Delete "$INSTDIR\styles\*.xsl"
Delete "$INSTDIR\icons\*.jisp"
Delete "$INSTDIR\copyright\COPYING"
Delete "$INSTDIR\SIM.url"
Delete "$INSTDIR\Updates.url"
Delete "$INSTDIR\pict\*.gif"
Delete "$INSTDIR\pict\*.png"
Delete "$INSTDIR\sounds\*.wav"

Delete "$SMPROGRAMS\SIM\SIM.lnk"
Delete "$SMPROGRAMS\SIM\SIM-IM.lnk"
Delete "$SMPROGRAMS\SIM\Updates.lnk"
Delete "$SMPROGRAMS\SIM\Uninstaller.lnk"
Delete "$SMPROGRAMS\SIM"

RMDir $INSTDIR\po
RMDir $INSTDIR\plugins\styles
RMDir $INSTDIR\plugins
RMDir $INSTDIR\styles
RMDir $INSTDIR\icons
RMDir $INSTDIR\copyright
RMDir $INSTDIR\sounds
RMDir $INSTDIR\pict
RMDir $INSTDIR

Quit
SectionEnd

Section Start
  MessageBox MB_YESNO "$(message)" IDNO done
	Exec '"$INSTDIR\sim.exe"'
	
	done:
  Quit
SectionEnd ; end the section
   
Function dirImage
	!insertmacro BIMAGE "sim-window-small.bmp" /RESIZETOFIT
FunctionEnd
; eof