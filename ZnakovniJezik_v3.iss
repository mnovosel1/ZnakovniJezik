; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Znakovni jezik"
#define MyAppVersion "0.3.95"
#define MyAppPublisher "Marijo.Novosel@FOI.hr"
#define MyAppExeName "ZnakovniJezik_v3.exe"
#define SetupIco "Release\ZnakovniJezik.ico"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{DE06907C-C8CE-4F59-BBF4-F730E28BBC3B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} v.{#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=ZnakovniJezik_v3_install
OutputDir=Installer
Compression=lzma
SolidCompression=yes
SetupIconFile={#SetupIco}
UninstallDisplayIcon={app}\{#MyAppExeName}
WizardImageFile=Release\ZnakovniJezik.bmp

[Languages]
Name: "croatian"; MessagesFile: "compiler:Languages/Croatian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Dirs]
Name: "{app}\img"

[Files]
Source: "Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "*.pdb, ZnakovniJezik.bmp, ZnakovniJezik.ico"
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\ZnakovniJezik.ico"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; IconFilename: "{app}\ZnakovniJezik.ico"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

