object TVPWindowForm: TTVPWindowForm
  Left = 245
  Top = 137
  Width = 323
  Height = 227
  HorzScrollBar.Tracking = True
  HorzScrollBar.Visible = False
  VertScrollBar.Tracking = True
  VertScrollBar.Visible = False
  Color = clBtnFace
  DefaultMonitor = dmDesktop
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'ＭＳ Ｐゴシック'
  Font.Style = []
  Menu = MainMenu
  OldCreateOrder = False
  PopupMenu = PopupMenu
  Position = poDefaultPosOnly
  Scaled = False
  OnActivate = FormActivate
  OnClose = FormClose
  OnCloseQuery = FormCloseQuery
  OnDestroy = FormDestroy
  OnDeactivate = FormDeactivate
  OnKeyDown = FormKeyDown
  OnKeyPress = FormKeyPress
  OnKeyUp = FormKeyUp
  OnMouseMove = FormMouseMove
  OnMouseWheel = FormMouseWheel
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 12
  object ScrollBox: TScrollBox
    Left = 0
    Top = 0
    Width = 315
    Height = 200
    HorzScrollBar.Tracking = True
    VertScrollBar.Tracking = True
    Align = alClient
    TabOrder = 0
    OnMouseMove = ScrollBoxMouseMove
  end
  object MainMenu: TMainMenu
    AutoHotkeys = maManual
    Left = 256
    Top = 8
  end
  object PopupMenu: TPopupMenu
    AutoPopup = False
    Left = 218
    Top = 10
    object ShowControllerMenuItem: TMenuItem
      Caption = 'コントローラ(&Z)'
      ShortCut = 8304
      Visible = False
      OnClick = ShowControllerMenuItemClick
    end
    object ShowScriptEditorMenuItem: TMenuItem
      Caption = 'スクリプトエディタ(&E)'
      ShortCut = 8305
      OnClick = ShowScriptEditorMenuItemClick
    end
    object ShowWatchMenuItem: TMenuItem
      Caption = '監視式(&W)'
      ShortCut = 8306
      OnClick = ShowWatchMenuItemClick
    end
    object ShowConsoleMenuItem: TMenuItem
      Caption = 'コンソール(&S)'
      ShortCut = 8307
      OnClick = ShowConsoleMenuItemClick
    end
    object UpdateRectDebugMenuItem: TMenuItem
      Caption = 'update rect debug'
      ShortCut = 8314
      OnClick = UpdateRectDebugMenuItemClick
    end
    object DumpLayerStructorMenuItem: TMenuItem
      Caption = 'dump layer structure'
      ShortCut = 8315
      OnClick = DumpLayerStructorMenuItemClick
    end
    object ShowAboutMenuItem: TMenuItem
      Caption = 'about ...'
      ShortCut = 16507
      OnClick = ShowAboutMenuItemClick
    end
    object CopyImportantLogMenuItem: TMenuItem
      Caption = 'copy important log'
      ShortCut = 16506
      Visible = False
      OnClick = CopyImportantLogMenuItemClick
    end
    object TabMenuItem: TMenuItem
      Caption = 'tab'
      ShortCut = 9
      Visible = False
      OnClick = TabMenuItemClick
    end
    object ShitTabMenuItem: TMenuItem
      Caption = 'shift+tab'
      ShortCut = 8201
      OnClick = ShitTabMenuItemClick
    end
    object AltEnterMenuItem: TMenuItem
      Caption = 'alt+enter'
      Visible = False
      OnClick = AltEnterMenuItemClick
    end
  end
end
