object dlgSplash: TdlgSplash
  Left = 669
  Top = 479
  BorderIcons = []
  BorderStyle = bsNone
  Caption = 'OLEX II'
  ClientHeight = 204
  ClientWidth = 299
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object iImage: TImage
    Left = 0
    Top = 0
    Width = 299
    Height = 163
    Align = alClient
    Center = True
  end
  object stVersion: TStaticText
    Left = 0
    Top = 163
    Width = 299
    Height = 17
    Align = alBottom
    Alignment = taCenter
    Caption = '1.0'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clGray
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 0
  end
  object pFile: TPanel
    Left = 0
    Top = 192
    Width = 299
    Height = 12
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object pbFProgress: TProgressBar
      Left = 77
      Top = 0
      Width = 46
      Height = 12
      Align = alLeft
      Min = 0
      Max = 100
      Smooth = True
      TabOrder = 0
    end
    object stFileName: TStaticText
      Left = 123
      Top = 0
      Width = 176
      Height = 12
      Align = alClient
      AutoSize = False
      Caption = ' Current file'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGray
      Font.Height = -9
      Font.Name = 'MS Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
    end
    object StaticText3: TStaticText
      Left = 0
      Top = 0
      Width = 77
      Height = 12
      Align = alLeft
      AutoSize = False
      Caption = ' File Progress'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGray
      Font.Height = -9
      Font.Name = 'MS Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
    end
  end
  object pOverall: TPanel
    Left = 0
    Top = 180
    Width = 299
    Height = 12
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 2
    object pbOProgress: TProgressBar
      Left = 77
      Top = 0
      Width = 222
      Height = 12
      Align = alClient
      Min = 0
      Max = 100
      Smooth = True
      TabOrder = 0
    end
    object StaticText2: TStaticText
      Left = 0
      Top = 0
      Width = 77
      Height = 12
      Align = alLeft
      AutoSize = False
      Caption = ' Overall Progress'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGray
      Font.Height = -9
      Font.Name = 'MS Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
    end
  end
end
