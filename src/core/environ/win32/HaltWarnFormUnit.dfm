object TVPHaltWarnForm: TTVPHaltWarnForm
  Left = 53
  Top = 251
  BorderStyle = bsDialog
  ClientHeight = 125
  ClientWidth = 476
  Color = clBtnFace
  DefaultMonitor = dmDesktop
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'ＭＳ Ｐゴシック'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 12
  object Label1: TLabel
    Left = 72
    Top = 16
    Width = 383
    Height = 12
    Caption = 
      'このプログラムは、現在イベント配信が停止しているか、待機状態にあ' +
      'るため'
  end
  object Label2: TLabel
    Left = 72
    Top = 32
    Width = 76
    Height = 12
    Caption = '応答しません。'
  end
  object Label3: TLabel
    Left = 72
    Top = 48
    Width = 384
    Height = 12
    Caption = 
      'プログラムを終了するには [強制終了] ボタンを、続行するには [続行' +
      '] ボタン'
  end
  object Label4: TLabel
    Left = 72
    Top = 64
    Width = 100
    Height = 12
    Caption = 'を選択してください。'
  end
  object IconPaintBox: TPaintBox
    Left = 16
    Top = 16
    Width = 49
    Height = 49
    OnPaint = IconPaintBoxPaint
  end
  object ExitButton: TButton
    Left = 141
    Top = 88
    Width = 91
    Height = 25
    Caption = '強制終了(&E)'
    TabOrder = 0
    OnClick = ExitButtonClick
  end
  object ContinueButton: TButton
    Left = 245
    Top = 88
    Width = 89
    Height = 25
    Cancel = True
    Caption = '続行(&C)'
    Default = True
    TabOrder = 1
    OnClick = ContinueButtonClick
  end
end
