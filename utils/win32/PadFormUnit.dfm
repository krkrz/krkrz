object TVPPadForm: TTVPPadForm
  Left = 74
  Top = 278
  Width = 538
  Height = 352
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Pad'
  Color = clBtnFace
  Constraints.MinHeight = 100
  Constraints.MinWidth = 70
  DefaultMonitor = dmDesktop
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'ＭＳ Ｐゴシック'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefaultPosOnly
  OnClose = FormClose
  OnDestroy = FormDestroy
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 12
  object StatusBar: TStatusBar
    Left = 32
    Top = 306
    Width = 498
    Height = 19
    Align = alNone
    Anchors = [akLeft, akRight, akBottom]
    Panels = <
      item
        Width = 80
      end
      item
        Width = 50
      end>
    SimplePanel = False
  end
  object ToolBar: TToolBar
    Left = 0
    Top = 307
    Width = 25
    Height = 18
    Align = alNone
    Anchors = [akLeft, akBottom]
    ButtonHeight = 18
    ButtonWidth = 19
    Caption = 'ToolBar'
    EdgeBorders = []
    Flat = True
    Images = TVPMainForm.VerySmallIconImageList
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
    object ExecuteButton: TToolButton
      Left = 0
      Top = 0
      Hint = '実行'
      Caption = 'Execute'
      ImageIndex = 0
      OnClick = ExecuteButtonClick
    end
  end
  object Memo: TRichEdit
    Left = 0
    Top = 0
    Width = 530
    Height = 305
    Anchors = [akLeft, akTop, akRight, akBottom]
    Color = clNavy
    Font.Charset = SHIFTJIS_CHARSET
    Font.Color = clWhite
    Font.Height = -12
    Font.Name = 'ＭＳ 明朝'
    Font.Style = []
    HideScrollBars = False
    MaxLength = 10000000
    ParentFont = False
    PlainText = True
    PopupMenu = MemoPopupMenu
    ScrollBars = ssBoth
    TabOrder = 2
    WantTabs = True
    WordWrap = False
    OnKeyDown = MemoKeyDown
    OnMouseDown = MemoMouseDown
    OnMouseUp = MemoMouseUp
    OnSelectionChange = MemoSelectionChange
  end
  object MemoPopupMenu: TPopupMenu
    Images = TVPMainForm.SmallIconImageList
    OnPopup = MemoPopupMenuPopup
    Left = 24
    Top = 24
    object CutMenuItem: TMenuItem
      Caption = '切り取り(&T)'
      Hint = '切り取り'
      ImageIndex = 5
      ShortCut = 16472
      OnClick = CutMenuItemClick
    end
    object CopyMenuItem: TMenuItem
      Caption = 'コピー(&C)'
      Hint = 'コピー'
      ImageIndex = 6
      ShortCut = 16451
      OnClick = CopyMenuItemClick
    end
    object PasteMenuItem: TMenuItem
      Caption = '貼り付け(&P)'
      Hint = '貼り付け'
      ImageIndex = 7
      ShortCut = 16470
      OnClick = PasteMenuItemClick
    end
    object UndoMenuItem: TMenuItem
      Caption = '元に戻す(&U)'
      ImageIndex = 3
      ShortCut = 16474
      OnClick = UndoMenuItemClick
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object ExecuteMenuItem: TMenuItem
      Caption = '実行(&G)'
      Hint = 'スクリプトの実行'
      ImageIndex = 1
      ShortCut = 16397
      OnClick = ExecuteButtonClick
    end
    object N3: TMenuItem
      Caption = '-'
    end
    object SaveMenuItem: TMenuItem
      Caption = '保存(&S) ...'
      ImageIndex = 9
      ShortCut = 16467
      OnClick = SaveMenuItemClick
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object ShowControllerMenuItem: TMenuItem
      Caption = 'コントローラ(&Z)'
      Hint = 'コントローラを表示'
      ImageIndex = 28
      ShortCut = 8304
      OnClick = ShowControllerMenuItemClick
    end
    object ShowScriptEditorMenuItem: TMenuItem
      Caption = 'スクリプトエディタ(&E)'
      ShortCut = 8305
      Visible = False
      OnClick = ShowScriptEditorMenuItemClick
    end
    object ShowWatchMenuItem: TMenuItem
      Caption = '監視式(&W)'
      Hint = '監視式を表示'
      ImageIndex = 14
      ShortCut = 8306
      OnClick = ShowWatchMenuItemClick
    end
    object ShowConsoleMenuItem: TMenuItem
      Caption = 'コンソール(&D)'
      Hint = 'コンソールを表示'
      ImageIndex = 13
      ShortCut = 8307
      OnClick = ShowConsoleMenuItemClick
    end
    object N4: TMenuItem
      Caption = '-'
    end
    object ShowOnTopMenuItem: TMenuItem
      Caption = '常に手前に表示(&F)'
      ImageIndex = 10
      OnClick = ShowOnTopMenuItemClick
    end
    object ShowAboutMenuItem: TMenuItem
      Caption = 'about ...'
      ShortCut = 16507
      Visible = False
      OnClick = ShowAboutMenuItemClick
    end
    object CopyImportantLogMenuItem: TMenuItem
      Caption = 'copy important log'
      ShortCut = 16506
      Visible = False
      OnClick = CopyImportantLogMenuItemClick
    end
  end
  object SaveDialog: TSaveDialog
    DefaultExt = '.tjs'
    Filter = 'TJS2 スクリプト(*.tjs)|*.tjs|すべてのファイル (*.*)|*.*'
    Options = [ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Left = 56
    Top = 24
  end
end
