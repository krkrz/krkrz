object FlashContainerForm: TFlashContainerForm
  Left = 157
  Top = 159
  Width = 541
  Height = 350
  Caption = 'FlashContainerForm'
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = '‚l‚r ‚oƒSƒVƒbƒN'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 12
  object Flash: TShockwaveFlash
    Left = 0
    Top = 0
    Width = 533
    Height = 323
    TabStop = True
    Align = alClient
    TabOrder = 0
    OnFSCommand = FlashFSCommand
    ControlData = {
      6755665500030000163700006221000008000200000000000800020000000000
      08000E000000570069006E0064006F00770000000B0000000B00000008000A00
      00004800690067006800000008000200000000000B0000000800020000000000
      080010000000530068006F00770041006C006C0000000B0000000B0000000800
      020000000000080002000000000008000C000000620065006C006F0077000000}
  end
  object Timer: TTimer
    OnTimer = TimerTimer
    Left = 72
    Top = 248
  end
end
