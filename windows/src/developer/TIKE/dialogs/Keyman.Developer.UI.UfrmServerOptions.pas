unit Keyman.Developer.UI.UfrmServerOptions;

interface

uses
  System.Classes,
  System.SysUtils,
  System.Variants,
  Vcl.Controls,
  Vcl.Dialogs,
  Vcl.Forms,
  Vcl.Graphics,
  Vcl.StdCtrls,
  Winapi.Messages,
  Winapi.Windows,

  UfrmDownloadProgress,
  UfrmTike;

type
  TfrmServerOptions = class(TTikeForm)
    cmdOK: TButton;
    cmdCancel: TButton;
    gbSetup: TGroupBox;
    editAuthToken: TEdit;
    lblAuthToken: TLabel;
    cbRegion: TComboBox;
    lblRegion: TLabel;
    cmdDownload: TButton;
    gbAdvanced: TGroupBox;
    chkServerShowConsoleWindow: TCheckBox;
    cmdCreateAccount: TButton;
    lblVersion: TLabel;
    Label1: TLabel;
    lblGetToken: TLabel;
    lblDefaultPort: TLabel;
    editDefaultPort: TEdit;
    chkUseNgrok: TCheckBox;
    chkLeaveServerRunning: TCheckBox;
    procedure FormCreate(Sender: TObject);
    procedure cmdOKClick(Sender: TObject);
    procedure cmdDownloadClick(Sender: TObject);
    procedure cmdCreateAccountClick(Sender: TObject);
    procedure lblGetTokenClick(Sender: TObject);
  private
    FDownloadProgress: TfrmDownloadProgress;
    procedure UpdateVersionLabel;
    function NgrokPath: string;
    procedure DownloadNgrok(AOwner: TfrmDownloadProgress; var Result: Boolean);
    procedure HttpReceiveData(const Sender: TObject; AContentLength,
      AReadCount: Int64; var Abort: Boolean);
    { Private declarations }
  public
    { Public declarations }
  end;

implementation

uses
  System.Net.HttpClient,
  System.Zip,
  Winapi.ShlObj,

  KeymanDeveloperOptions,
  Keyman.Developer.System.ServerAPI,
  KeymanPaths,
  RegistryKeys,
  UmodWebHttpServer,
  utilexecute;

{$R *.dfm}

const
  // These URLs come from ngrok documentation and ngrok source. They appear
  // to be stable, as they are used in other apps also.
  SUrlNgrokAuthToken = 'https://dashboard.ngrok.com/get-started/your-authtoken';
  SUrlNgrokSignup = 'https://dashboard.ngrok.com/signup';
  SUrlNgrokDownload = 'https://bin.equinox.io/c/4VmDzA7iaHb/ngrok-stable-windows-386.zip';

procedure TfrmServerOptions.cmdCreateAccountClick(Sender: TObject);
begin
  TUtilExecute.URL(SUrlNgrokSignup);
end;

procedure TfrmServerOptions.HttpReceiveData(const Sender: TObject; AContentLength: Int64; AReadCount: Int64; var Abort: Boolean);
begin
  Abort := False;
  if Assigned(FDownloadProgress) then
  begin
    FDownloadProgress.HTTPStatus(nil, 'Downloading ngrok', AReadCount, AContentLength);
    FDownloadProgress.HTTPCheckCancel(nil, Abort);
  end;
end;

procedure TfrmServerOptions.DownloadNgrok(AOwner: TfrmDownloadProgress; var Result: Boolean);
var
  http: THttpClient;
  res: IHTTPResponse;
  fs: TFileStream;
  zip: TZipFile;
  ms: TStream;
  zh: TZipHeader;
begin
  Result := False;

  FDownloadProgress := AOwner;
  http := THttpClient.Create;
  try
    http.OnReceiveData := HttpReceiveData;

    try
      res := http.Get(SUrlNgrokDownload);
    except
      on E:ENetHTTPClientException do
      begin
        ShowMessage(Format('Unable to download ngrok.exe: %s', [E.Message]));
        Exit;
      end;
    end;

    if res.StatusCode <> 200 then
    begin
      ShowMessage(Format('Unable to download ngrok.exe: %d %s',
        [res.StatusCode, res.StatusText]));
      Exit;
    end;

    // Download is a zip file with a single executable, so let's extract it
    ms := nil;
    try
      zip := TZipFile.Create;
      try
        try
          zip.Open(res.ContentStream, zmRead);
          zip.Read('ngrok.exe', ms, zh);
          ms.Position := 0;
        finally
          zip.Free;
        end;
      except
        on E:Exception do
        begin
          ShowMessage(Format('Unable to extract ngrok.exe from downloaded archive: %s',
            [E.Message]));
          Exit;
        end;
      end;
      try
        fs := TFileStream.Create(NgrokPath, fmCreate);
      except
        on E:Exception do
        begin
          ShowMessage(Format('Unable to save ngrok.exe: %s', [E.Message]));
          Exit;
        end;
      end;
      try
        fs.CopyFrom(ms, 0);
      finally
        fs.Free;
      end;
    finally
      ms.Free;
    end;
  finally
    http.Free;
    FDownloadProgress := nil;
  end;
end;

procedure TfrmServerOptions.cmdDownloadClick(Sender: TObject);
var
  DownloadProgress: TfrmDownloadProgress;
begin
  TServerDebugAPI.StopServer;

  try
    DownloadProgress := TfrmDownloadProgress.Create(Self);
    try
      DownloadProgress.Callback := DownloadNgrok;
      DownloadProgress.ShowModal;
    finally
      DownloadProgress.Free;
    end;
  finally
    UpdateVersionLabel;
    TServerDebugAPI.StartServer;
  end;
end;

procedure TfrmServerOptions.cmdOKClick(Sender: TObject);
var
  DefaultPort: Integer;
  KeepAlive, UseNgrok, ngrokKeepVisible: Boolean;
  ngrokToken, ngrokRegion: string;
  Changed: Boolean;
begin
  DefaultPort := StrToIntDef(editDefaultPort.Text, 8008);   // I4021
  KeepAlive := chkLeaveServerRunning.Checked;
  UseNgrok := chkUseNgrok.Checked;
  ngrokToken := editAuthToken.Text;
  ngrokRegion := Copy(cbRegion.Text, 1, 2);
  ngrokKeepVisible := chkServerShowConsoleWindow.Checked;

  Changed :=
    (FKeymanDeveloperOptions.ServerDefaultPort <> DefaultPort) or
    (FKeymanDeveloperOptions.ServerKeepAlive <> KeepAlive) or
    (FKeymanDeveloperOptions.ServerUseNgrok <> UseNgrok) or
    (FKeymanDeveloperOptions.ServerNgrokToken <> ngrokToken) or
    (FKeymanDeveloperOptions.ServerNgrokRegion <> ngrokRegion) or
    (FKeymanDeveloperOptions.ServerServerShowConsoleWindow <> ngrokKeepVisible);

  if Changed then
  begin
    FKeymanDeveloperOptions.ServerDefaultPort := DefaultPort;
    FKeymanDeveloperOptions.ServerKeepAlive := KeepAlive;
    FKeymanDeveloperOptions.ServerUseNgrok := UseNgrok;
    FKeymanDeveloperOptions.ServerNgrokToken := ngrokToken;
    FKeymanDeveloperOptions.ServerNgrokRegion := ngrokRegion;
    FKeymanDeveloperOptions.ServerServerShowConsoleWindow := ngrokKeepVisible;
    FKeymanDeveloperOptions.Write; // TODO: Cancel button in parent dialog is a problem
    modWebHttpServer.RestartServer;
  end;
  ModalResult := mrOk;
end;

procedure TfrmServerOptions.FormCreate(Sender: TObject);
var
  i: Integer;
begin
  editDefaultPort.Text := IntToStr(FKeymanDeveloperOptions.ServerDefaultPort);   // I4021
  chkUseNgrok.Checked := FKeymanDeveloperOptions.ServerUseNgrok;
  chkLeaveServerRunning.Checked := FKeymanDeveloperOptions.ServerKeepAlive;

  lblVersion.Caption := '';
  editAuthToken.Text := FKeymanDeveloperOptions.ServerNgrokToken;

  cbRegion.ItemIndex := 0;
  for i := 0 to cbRegion.Items.Count - 1 do
    if cbRegion.Items[i].StartsWith(FKeymanDeveloperOptions.ServerNgrokRegion) then
    begin
      cbRegion.ItemIndex := i;
      Break;
    end;

  chkServerShowConsoleWindow.Checked := FKeymanDeveloperOptions.ServerServerShowConsoleWindow;

  UpdateVersionLabel;
end;

procedure TfrmServerOptions.lblGetTokenClick(Sender: TObject);
begin
  TUtilExecute.URL(SUrlNgrokAuthToken);
end;

function TfrmServerOptions.NgrokPath: string;
begin
  Result := GetFolderPath(CSIDL_APPDATA) + SFolderKeymanDeveloper + '\ngrok.exe';
end;

procedure TfrmServerOptions.UpdateVersionLabel;
var
  logtext: string;
  ec: Integer;
begin
  if not FileExists(NgrokPath) then
    lblVersion.Caption := 'Ngrok not installed'
  else if TUtilExecute.Console(Format('"%s" --version', [NgrokPath]), ExtractFilePath(ParamStr(0)),
    logtext, ec) then
  begin
    if ec = 0
      then lblVersion.Caption := logtext.Trim
      else lblVersion.Caption := Format('Error %d running ngrok: %s', [ec, logtext.Trim])
  end
  else
    lblVersion.Caption := Format('Unable to run ngrok: %s', [SysErrorMessage(GetLastError)]);
end;

end.
