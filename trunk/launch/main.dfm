object dlgMain: TdlgMain
  Left = 655
  Top = 879
  BorderIcons = []
  BorderStyle = bsNone
  Caption = 'OLEX II'
  ClientHeight = 29
  ClientWidth = 156
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 13
  object tTimer: TTimer
    Enabled = False
    Interval = 50
    OnTimer = tTimerTimer
  end
end
