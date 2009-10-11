object dlgProgress: TdlgProgress
  Left = 797
  Top = 163
  BorderIcons = []
  BorderStyle = bsToolWindow
  Caption = 'Progress Window'
  ClientHeight = 65
  ClientWidth = 395
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poScreenCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object pbBar: TProgressBar
    Left = 0
    Top = 0
    Width = 395
    Height = 16
    Align = alTop
    Min = 0
    Max = 100
    Smooth = True
    TabOrder = 0
  end
  object stAction: TStaticText
    Left = 0
    Top = 16
    Width = 395
    Height = 17
    Align = alTop
    AutoSize = False
    BorderStyle = sbsSingle
    Caption = 'Action'
    TabOrder = 1
  end
  object bbCancel: TBitBtn
    Left = 152
    Top = 40
    Width = 75
    Height = 25
    TabOrder = 2
    OnClick = bbCancelClick
    Kind = bkCancel
  end
end
