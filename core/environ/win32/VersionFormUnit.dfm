object TVPVersionForm: TTVPVersionForm
  Left = 40
  Top = 237
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  BorderStyle = bsDialog
  BorderWidth = 8
  Caption = 'バージョン・著作権・環境情報'
  ClientHeight = 292
  ClientWidth = 557
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'ＭＳ Ｐゴシック'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 12
  object OKButton: TButton
    Left = 455
    Top = 263
    Width = 75
    Height = 25
    Anchors = [akBottom]
    Cancel = True
    Caption = '閉じる(&C)'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
  object Memo: TRichEdit
    Left = 9
    Top = 8
    Width = 540
    Height = 241
    Anchors = [akLeft, akTop, akRight, akBottom]
    Color = clBtnFace
    Font.Charset = SHIFTJIS_CHARSET
    Font.Color = clBtnText
    Font.Height = -12
    Font.Name = 'ＭＳ Ｐゴシック'
    Font.Style = []
    HideScrollBars = False
    Lines.Strings = (
      '')
    MaxLength = 1000000
    ParentFont = False
    PlainText = True
    PopupMenu = PopupMenu
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 1
    WantReturns = False
  end
  object CopyEnvInfoButton: TButton
    Left = 21
    Top = 263
    Width = 253
    Height = 25
    Anchors = [akBottom]
    Caption = '環境情報をクリップボードにコピー'
    TabOrder = 2
    OnClick = CopyEnvInfoButtonClick
  end
  object PopupMenu: TPopupMenu
    Images = TVPMainForm.SmallIconImageList
    OnPopup = PopupMenuPopup
    Top = 8
    object CopyMenuItem: TMenuItem
      Caption = 'コピー(&C)'
      ImageIndex = 6
      OnClick = CopyMenuItemClick
    end
  end
end
