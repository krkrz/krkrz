object ProjectSettingForm: TProjectSettingForm
  Left = 0
  Top = 0
  Caption = #12503#12525#12472#12455#12463#12488#35373#23450
  ClientHeight = 247
  ClientWidth = 521
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object ExeLabel: TLabel
    Left = 16
    Top = 16
    Width = 77
    Height = 13
    Caption = #23455#34892#12501#12449#12452#12523#12497#12473
  end
  object ArgLabel: TLabel
    Left = 16
    Top = 52
    Width = 24
    Height = 13
    Caption = #24341#25968
  end
  object ProjectFolderLabel: TLabel
    Left = 16
    Top = 91
    Width = 86
    Height = 13
    Caption = #12503#12525#12472#12455#12463#12488#12501#12457#12523#12480
  end
  object WorkingFolderLabel: TLabel
    Left = 16
    Top = 131
    Width = 59
    Height = 13
    Caption = #20316#26989#12501#12457#12523#12480
  end
  object ScriptExtLabel: TLabel
    Left = 16
    Top = 171
    Width = 77
    Height = 13
    Caption = #12473#12463#12522#12503#12488#25313#24373#23376
  end
  object ExeEdit: TEdit
    Left = 108
    Top = 13
    Width = 297
    Height = 21
    TabOrder = 0
  end
  object BrowseExeButton: TButton
    Left = 416
    Top = 11
    Width = 73
    Height = 25
    Caption = #21442#29031'...'
    TabOrder = 1
    OnClick = BrowseExeButtonClick
  end
  object ArgEdit: TEdit
    Left = 108
    Top = 49
    Width = 297
    Height = 21
    TabOrder = 2
  end
  object ProjectFolderEdit: TEdit
    Left = 108
    Top = 88
    Width = 297
    Height = 21
    TabOrder = 3
  end
  object BrowseProjectFolderButton: TButton
    Left = 416
    Top = 86
    Width = 75
    Height = 25
    Caption = #21442#29031'...'
    TabOrder = 4
    OnClick = BrowseProjectFolderButtonClick
  end
  object WorkingFolderEdit: TEdit
    Left = 108
    Top = 128
    Width = 297
    Height = 21
    TabOrder = 5
  end
  object BrowseWorkingFolderButton: TButton
    Left = 416
    Top = 126
    Width = 75
    Height = 25
    Caption = #21442#29031'...'
    TabOrder = 6
    OnClick = BrowseWorkingFolderButtonClick
  end
  object OKButton: TButton
    Left = 324
    Top = 204
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 7
  end
  object CancelButton: TButton
    Left = 416
    Top = 204
    Width = 75
    Height = 25
    Cancel = True
    Caption = #12461#12515#12531#12475#12523
    ModalResult = 2
    TabOrder = 8
  end
  object ScriptrExtEdit: TEdit
    Left = 108
    Top = 168
    Width = 297
    Height = 21
    TabOrder = 9
  end
  object ExeOpenDialog: TOpenDialog
    DefaultExt = '*.exe'
    Filter = '*.exe|*.exe'
    Title = #23455#34892#12501#12449#12452#12523#12434#36984#25246#12375#12390#12367#12384#12373#12356
    Left = 52
    Top = 200
  end
end
