unit Keyman.Developer.System.KeymanDeveloperPaths;

interface

uses
  System.SysUtils,

  KeymanPaths;

type
  TKeymanDeveloperPaths = class sealed
  private
  public
    class function NodePath: string; static;

    const S_LexicalModelCompiler = 'kmlmc.cmd';
    class function LexicalModelCompilerPath: string; static;

    const S_ServerConfigJson = 'config.json';
    class function ServerDataPath: string; static;
    class function ServerPath: string; static;
  end;

implementation

uses
  Winapi.ShlObj,

  RegistryKeys,
  utilsystem;

{ TKeymanDeveloperPaths }

class function TKeymanDeveloperPaths.NodePath: string;
var
  KeymanRoot: string;
begin
  if TKeymanPaths.RunningFromSource(KeymanRoot)
    then Result := KeymanRoot + 'developer\inst\dist\node.exe'
    else Result := ExtractFilePath(ParamStr(0)) + 'node.js\node.exe';
end;

class function TKeymanDeveloperPaths.ServerDataPath: string;
begin
  Result := GetFolderPath(CSIDL_APPDATA) + SFolderKeymanDeveloper + '\server\';
end;

class function TKeymanDeveloperPaths.ServerPath: string;
var
  KeymanRoot: string;
begin
  if TKeymanPaths.RunningFromSource(KeymanRoot)
    then Result := KeymanRoot + 'developer\server\'
    else Result := ExtractFilePath(ParamStr(0)) + 'server\';
end;

class function TKeymanDeveloperPaths.LexicalModelCompilerPath: string;
var
  KeymanRoot: string;
begin
  if TKeymanPaths.RunningFromSource(KeymanRoot)
    then Result := KeymanRoot + 'windows\src\developer\tike\'
    else Result := ExtractFilePath(ParamStr(0));

  Result := Result + S_LexicalModelCompiler;
end;

end.
