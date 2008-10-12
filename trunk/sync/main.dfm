object fMain: TfMain
  Left = -905
  Top = 230
  Width = 639
  Height = 444
  Caption = 'fMain'
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
  object Label1: TLabel
    Left = 40
    Top = 24
    Width = 34
    Height = 13
    Caption = 'Source'
  end
  object Label2: TLabel
    Left = 40
    Top = 40
    Width = 53
    Height = 13
    Caption = 'Destination'
  end
  object sbDest: TSpeedButton
    Left = 432
    Top = 40
    Width = 23
    Height = 22
    Caption = '...'
    OnClick = sbSrcClick
  end
  object sbSrc: TSpeedButton
    Left = 432
    Top = 16
    Width = 23
    Height = 22
    Caption = '...'
    OnClick = sbSrcClick
  end
  object Label3: TLabel
    Left = 32
    Top = 88
    Width = 76
    Height = 13
    Caption = 'Overall progress'
  end
  object Label4: TLabel
    Left = 32
    Top = 112
    Width = 88
    Height = 13
    Caption = 'File copy  progress'
  end
  object Label5: TLabel
    Left = 32
    Top = 136
    Width = 81
    Height = 13
    Caption = 'Current file/folder'
  end
  object eDest: TEdit
    Left = 104
    Top = 40
    Width = 329
    Height = 21
    TabOrder = 0
    Text = 'F:\My Documents'
  end
  object eSrc: TEdit
    Left = 104
    Top = 16
    Width = 329
    Height = 21
    TabOrder = 1
    Text = 'X:\My Documents'
  end
  object bbRun: TBitBtn
    Left = 464
    Top = 40
    Width = 75
    Height = 25
    Caption = 'Run...'
    TabOrder = 2
    OnClick = bbRunClick
  end
  object reEdit: TRichEdit
    Left = 24
    Top = 160
    Width = 545
    Height = 201
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = '@Arial Unicode MS'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 3
  end
  object pbOverall: TProgressBar
    Left = 125
    Top = 88
    Width = 150
    Height = 17
    Min = 0
    Max = 100
    Smooth = True
    TabOrder = 4
  end
  object pbFC: TProgressBar
    Left = 125
    Top = 112
    Width = 150
    Height = 17
    Min = 0
    Max = 100
    Smooth = True
    TabOrder = 5
  end
  object stCurrent: TStaticText
    Left = 125
    Top = 136
    Width = 30
    Height = 17
    Caption = 'None'
    TabOrder = 6
  end
  object stTotal: TStaticText
    Left = 280
    Top = 88
    Width = 26
    Height = 17
    Caption = '0 Kb'
    TabOrder = 7
  end
  object stSpeed: TStaticText
    Left = 280
    Top = 112
    Width = 36
    Height = 17
    Caption = '0 Kb/s'
    TabOrder = 8
  end
  object sbStatus: TStatusBar
    Left = 0
    Top = 389
    Width = 623
    Height = 19
    Panels = <
      item
        Width = 50
      end>
    SimplePanel = False
  end
end
