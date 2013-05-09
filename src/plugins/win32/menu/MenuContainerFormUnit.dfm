object TVPMenuContainerForm: TTVPMenuContainerForm
  Left = 16
  Top = 152
  BorderStyle = bsNone
  Caption = 'TVPMenuContainerForm'
  ClientHeight = 32
  ClientWidth = 486
  Color = clBtnFace
  DefaultMonitor = dmDesktop
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = '‚l‚r ‚oƒSƒVƒbƒN'
  Font.Style = []
  OldCreateOrder = False
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 12
  object Timer: TTimer
    Interval = 500
    OnTimer = TimerTimer
  end
end
