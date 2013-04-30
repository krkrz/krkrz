object TVPFontSelectForm: TTVPFontSelectForm
  Left = 232
  Top = 654
  BorderStyle = bsDialog
  ClientHeight = 343
  ClientWidth = 323
  Color = clBtnFace
  DefaultMonitor = dmPrimary
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'ＭＳ Ｐゴシック'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnDestroy = FormDestroy
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 12
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 4
    Height = 12
  end
  object Label3: TLabel
    Left = 8
    Top = 24
    Width = 59
    Height = 12
    Caption = 'フォント(&F) :'
    FocusControl = ListBox
  end
  object Label4: TLabel
    Left = 8
    Top = 224
    Width = 66
    Height = 12
    Caption = 'サンプル(&S) :'
  end
  object ListBox: TListBox
    Left = 8
    Top = 40
    Width = 305
    Height = 177
    ItemHeight = 12
    TabOrder = 0
    OnClick = ListBoxClick
    OnDblClick = ListBoxDblClick
    OnDrawItem = ListBoxDrawItem
  end
  object Memo: TMemo
    Left = 8
    Top = 240
    Width = 305
    Height = 65
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 1
  end
  object OKButton: TButton
    Left = 80
    Top = 312
    Width = 75
    Height = 25
    Caption = '&OK'
    Default = True
    TabOrder = 2
    OnClick = OKButtonClick
  end
  object CancelButton: TButton
    Left = 168
    Top = 312
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'キャンセル'
    ModalResult = 2
    TabOrder = 3
  end
end
