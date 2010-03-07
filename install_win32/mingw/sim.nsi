;Multilingular NSIS installer script for SIM Instant Messenger with Modern UI
;Written by Serhiy Kachanuk <serzholino@gmail.com> (c) 2006

;--------------------------------
;Includes

  !include "MUI.nsh"
  !include "Sections.nsh"

;--------------------------------
;Set compression settings
  SetCompress auto
  SetCompressor /SOLID /FINAL lzma

;--------------------------------
;General

  ;Name and file
  Name "SIM Instant Messenger"
  OutFile "sim-install.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SIM"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\SIM" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Variables

  Var STARTMENU_FOLDER

;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\SIM" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE ..\..\COPYING
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\SIM" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

  !insertmacro MUI_PAGE_INSTFILES
  !define MUI_FINISHPAGE_RUN
  !define MUI_FINISHPAGE_RUN_TEXT "Run SIM on Windows startup"
  !define MUI_FINISHPAGE_RUN_FUNCTION RegStartOnBoot
  !define MUI_FINISHPAGE_BUTTON "Next >"
  !insertmacro MUI_PAGE_FINISH
  !define MUI_FINISHPAGE_RUN "$INSTDIR\sim.exe"
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" # first language is the default language
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Korean"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
  !insertmacro MUI_LANGUAGE "Finnish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Croatian"
  !insertmacro MUI_LANGUAGE "Bulgarian"
  !insertmacro MUI_LANGUAGE "Hungarian"
  !insertmacro MUI_LANGUAGE "Thai"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Latvian"
  !insertmacro MUI_LANGUAGE "Macedonian"
  !insertmacro MUI_LANGUAGE "Estonian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Lithuanian"
  !insertmacro MUI_LANGUAGE "Catalan"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Serbian"
  !insertmacro MUI_LANGUAGE "SerbianLatin"
  !insertmacro MUI_LANGUAGE "Arabic"
  !insertmacro MUI_LANGUAGE "Farsi"
  !insertmacro MUI_LANGUAGE "Hebrew"
  !insertmacro MUI_LANGUAGE "Indonesian"
  !insertmacro MUI_LANGUAGE "Mongolian"
  !insertmacro MUI_LANGUAGE "Luxembourgish"
  !insertmacro MUI_LANGUAGE "Albanian"
  !insertmacro MUI_LANGUAGE "Breton"
  !insertmacro MUI_LANGUAGE "Belarusian"
  !insertmacro MUI_LANGUAGE "Icelandic"
  !insertmacro MUI_LANGUAGE "Malay"
  !insertmacro MUI_LANGUAGE "Bosnian"
  !insertmacro MUI_LANGUAGE "Kurdish"

;--------------------------------
;Reserve Files
  
  ;These files should be inserted before other files in the data block
  ;Keep these lines before any File command
  ;Only for solid compression (by default, solid compression is enabled for BZIP2 and LZMA)
  
  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;Installer Sections

Section "!SIM program files" SecSIM
  SectionIn RO
	
  ;SIM program
  SetOutPath "$INSTDIR"
  File ..\..\sim\.libs\sim.exe
  File ..\..\sim\.libs\libsim-0.dll

  ;Plugins
  SetOutPath "$INSTDIR\plugins"  
  File ..\..\plugins\__homedir\.libs\__homedir.dll
  File ..\..\plugins\__migrate\.libs\__migrate.dll
  File ..\..\plugins\_core\.libs\_core.dll
  File ..\..\plugins\about\.libs\about.dll
  File ..\..\plugins\action\.libs\action.dll
  File ..\..\plugins\autoaway\.libs\autoaway.dll
  File ..\..\plugins\background\.libs\background.dll
  File ..\..\plugins\dock\.libs\dock.dll
  File ..\..\plugins\filter\.libs\filter.dll
  File ..\..\plugins\floaty\.libs\floaty.dll
  File ..\..\plugins\forward\.libs\forward.dll
  File ..\..\plugins\gpg\.libs\gpg.dll
  File ..\..\plugins\icons\.libs\icons.dll
  File ..\..\plugins\icq\.libs\icq.dll
  File ..\..\plugins\jabber\.libs\jabber.dll
  File ..\..\plugins\livejournal\.libs\livejournal.dll
  File ..\..\plugins\logger\.libs\logger.dll
  File ..\..\plugins\msn\.libs\msn.dll
  File ..\..\plugins\navigate\.libs\navigate.dll
  File ..\..\plugins\netmonitor\.libs\netmonitor.dll
  File ..\..\plugins\ontop\.libs\ontop.dll
  File ..\..\plugins\osd\.libs\osd.dll
  File ..\..\plugins\proxy\.libs\proxy.dll
  File ..\..\plugins\replace\.libs\replace.dll
  File ..\..\plugins\shortcuts\.libs\shortcuts.dll
  File ..\..\plugins\sms\.libs\sms.dll
  File ..\..\plugins\sound\.libs\sound.dll
  File ..\..\plugins\spell\.libs\spell.dll
  File ..\..\plugins\splash\.libs\splash.dll
  File ..\..\plugins\styles\.libs\styles.dll
  File ..\..\plugins\transparent\.libs\transparent.dll
  File ..\..\plugins\weather\.libs\weather.dll
  File ..\..\plugins\windock\.libs\windock.dll
  File ..\..\plugins\yahoo\.libs\yahoo.dll
  File ..\..\plugins\zodiak\.libs\zodiak.dll

  ;Icons
  SetOutPath "$INSTDIR\icons"  
  File ..\..\plugins\_core\additional\additional.jisp
  File ..\..\plugins\_core\icq5\icq5.jisp
  File ..\..\plugins\_core\icq5.1\icq5.1.jisp
  File ..\..\plugins\_core\icqlite\icqlite.jisp
  File ..\..\plugins\_core\jisp\sim.jisp
  File ..\..\plugins\_core\smiles\smiles.jisp
  File ..\..\plugins\weather\jisp\weather.jisp

  ;Pictures
  SetOutPath "$INSTDIR\pict"  
  File ..\..\plugins\_core\pict\*.gif

  ;Locales
  SetOutPath "$INSTDIR\po" 
  File /oname=bg.qm ..\..\po\bg.gmo
  File /oname=ca.qm ..\..\po\ca.gmo
  File /oname=cs.qm ..\..\po\cs.gmo
  File /oname=de.qm ..\..\po\de.gmo
  File /oname=el.qm ..\..\po\el.gmo
  File /oname=es.qm ..\..\po\es.gmo
  File /oname=fr.qm ..\..\po\fr.gmo
  File /oname=he.qm ..\..\po\he.gmo
  File /oname=hu.qm ..\..\po\hu.gmo
  File /oname=it.qm ..\..\po\it.gmo
  File /oname=nl.qm ..\..\po\nl.gmo
  File /oname=pl.qm ..\..\po\pl.gmo
  File /oname=pt_BR.qm ..\..\po\pt_BR.gmo
  File /oname=ru.qm ..\..\po\ru.gmo
  File /oname=sk.qm ..\..\po\sk.gmo
  File /oname=sw.qm ..\..\po\sw.gmo
  File /oname=th.qm ..\..\po\th.gmo
  File /oname=tr.qm ..\..\po\tr.gmo
  File /oname=uk.qm ..\..\po\uk.gmo
  File /oname=zh_TW.qm ..\..\po\zh_TW.gmo

  ;Sounds
  SetOutPath "$INSTDIR\sounds"  
  File ..\..\plugins\sound\sounds\*.wav
  File ..\..\plugins\jabber\sounds\*.wav
  File ..\..\plugins\icq\sounds\*.wav
  File ..\..\plugins\sms\sounds\*.wav

  ;Styles
  SetOutPath "$INSTDIR\styles"  
  File ..\..\plugins\_core\styles\*.xsl

  ;Store installation folder
  WriteRegStr HKCU "Software\SIM" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; write uninstall strings
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SIM" "DisplayName" "SIM Instant messenger"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SIM" "UninstallString" '"$INSTDIR\Uninstall.exe"'

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\SIM.lnk" "$INSTDIR\sim.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

SectionGroup /e "Runtime libraries"

  Section "Qt library files" SecQt

    SetOutPath "$INSTDIR"    
    File "$%QTDIR%\bin\qt-mt3.dll"

  SectionEnd

  Section "Xml2 library files" SecXml

    SetOutPath "$INSTDIR"    
    File "$%MINGW%\bin\libxml2-2.dll"

  SectionEnd

  Section "Xslt library files" SecXslt

    SetOutPath "$INSTDIR"    
    File "$%MINGW%\bin\libxslt-1.dll"

  SectionEnd

  Section "OpenSSL library files" SecSSL

    SetOutPath "$INSTDIR"    
    File "$%MINGW%\bin\libeay32.dll"
    File "$%MINGW%\bin\libssl32.dll"

  SectionEnd

  Section "Mingw runtime" SecMingw

    SetOutPath "$INSTDIR"    
    File "$%MINGW%\bin\mingwm10.dll"

  SectionEnd

SectionGroupEnd

;--------------------------------
;Installer Functions

Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

;--------------------------------
;Descriptions
  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSIM} "Main program, plugins and needed data files."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecQt} "Qt library. You can disable its installation if you have already installed it somewhere in yor system."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecXml} "Xml processing library. You can disable its installation if you have already installed it somewhere in yor system."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecXslt} "Xslt transfornation library. You can disable its installation if you have already installed it somewhere in yor system."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSSL} "OpenSSL library. You can disable its installation if you have already installed it somewhere in yor system."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMingw} "Mingw runtime. You can disable its installation if you have already installed Mingw somewhere in yor system."
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\plugins\*"
  RMDir "$INSTDIR\plugins"

  Delete "$INSTDIR\icons\*"
  RMDir "$INSTDIR\icons"

  Delete "$INSTDIR\pict\*"
  RMDir "$INSTDIR\pict"
  Delete "$INSTDIR\po\*"
  RMDir "$INSTDIR\po"

  Delete "$INSTDIR\sounds\*"
  RMDir "$INSTDIR\sounds"

  Delete "$INSTDIR\styles\*"
  RMDir "$INSTDIR\styles"

  Delete "$INSTDIR\*.exe"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
     Delete "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk"
     Delete "$SMPROGRAMS\$STARTMENU_FOLDER\SIM.lnk"
     RMDir "$SMPROGRAMS\$STARTMENU_FOLDER"      
  !insertmacro MUI_STARTMENU_WRITE_END

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SIM"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\SIM"
  DeleteRegKey /ifempty HKCU "Software\SIM"

SectionEnd

;--------------------------------
;Uninstaller Functions
Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd

;Function to register startup on boot
Function RegStartOnBoot
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "SIM" '"$INSTDIR\sim.exe"'
FunctionEnd
