object fMain: TfMain
  Left = 232
  Top = 131
  Width = 686
  Height = 554
  Caption = 'Sync'
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
  object pgTasks: TPageControl
    Left = 0
    Top = 0
    Width = 678
    Height = 520
    ActivePage = tsDublicates
    Align = alClient
    TabIndex = 1
    TabOrder = 0
    object tbSync: TTabSheet
      Caption = 'Synchronise'
      object Label5: TLabel
        Left = 5
        Top = 136
        Width = 50
        Height = 13
        Caption = 'Current file'
      end
      object Label4: TLabel
        Left = 5
        Top = 112
        Width = 88
        Height = 13
        Caption = 'File copy  progress'
      end
      object Label3: TLabel
        Left = 5
        Top = 88
        Width = 76
        Height = 13
        Caption = 'Overall progress'
      end
      object sbSrc: TSpeedButton
        Left = 395
        Top = 16
        Width = 23
        Height = 22
        Caption = '...'
        OnClick = sbSrcClick
      end
      object sbDest: TSpeedButton
        Left = 395
        Top = 40
        Width = 23
        Height = 22
        Caption = '...'
        OnClick = sbSrcClick
      end
      object Label2: TLabel
        Left = 5
        Top = 40
        Width = 53
        Height = 13
        Caption = 'Destination'
      end
      object Label1: TLabel
        Left = 5
        Top = 24
        Width = 34
        Height = 13
        Caption = 'Source'
      end
      object lLog: TLabel
        Left = 5
        Top = 168
        Width = 18
        Height = 13
        Caption = 'Log'
      end
      object Label6: TLabel
        Left = 5
        Top = 152
        Width = 72
        Height = 13
        Caption = 'Average spped'
      end
      object sbSyncStatus: TStatusBar
        Left = 0
        Top = 473
        Width = 670
        Height = 19
        Panels = <
          item
            Width = 50
          end>
        SimplePanel = False
      end
      object reEdit: TRichEdit
        Left = 5
        Top = 184
        Width = 658
        Height = 286
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = '@Arial Unicode MS'
        Font.Style = []
        ParentFont = False
        ScrollBars = ssVertical
        TabOrder = 1
      end
      object stCurrent: TStaticText
        Left = 96
        Top = 134
        Width = 30
        Height = 17
        Caption = 'None'
        TabOrder = 2
      end
      object pbOverall: TProgressBar
        Left = 96
        Top = 88
        Width = 175
        Height = 17
        Min = 0
        Max = 100
        Smooth = True
        TabOrder = 3
      end
      object pbFC: TProgressBar
        Left = 96
        Top = 112
        Width = 175
        Height = 17
        Min = 0
        Max = 100
        Smooth = True
        TabOrder = 4
      end
      object stTotal: TStaticText
        Left = 280
        Top = 88
        Width = 26
        Height = 17
        Caption = '0 Kb'
        TabOrder = 5
      end
      object stSpeed: TStaticText
        Left = 96
        Top = 152
        Width = 36
        Height = 17
        Caption = '0 Kb/s'
        TabOrder = 6
      end
      object eDest: TEdit
        Left = 67
        Top = 40
        Width = 329
        Height = 21
        TabOrder = 7
        Text = 'F:\My Documents'
      end
      object eSrc: TEdit
        Left = 67
        Top = 16
        Width = 329
        Height = 21
        TabOrder = 8
        Text = 'X:\My Documents'
      end
      object bbRun: TBitBtn
        Left = 427
        Top = 40
        Width = 75
        Height = 25
        Caption = 'Run...'
        TabOrder = 9
        OnClick = bbRunClick
      end
    end
    object tsDublicates: TTabSheet
      Caption = 'Dublicates'
      ImageIndex = 1
      object Label7: TLabel
        Left = 5
        Top = 12
        Width = 29
        Height = 13
        Caption = 'Folder'
      end
      object SpeedButton1: TSpeedButton
        Left = 395
        Top = 12
        Width = 23
        Height = 22
        Caption = '...'
        OnClick = sbSrcClick
      end
      object Label8: TLabel
        Left = 5
        Top = 42
        Width = 76
        Height = 13
        Caption = 'Overall progress'
      end
      object Label9: TLabel
        Left = 5
        Top = 62
        Width = 106
        Height = 13
        Caption = 'File compare  progress'
      end
      object Label10: TLabel
        Left = 5
        Top = 86
        Width = 50
        Height = 13
        Caption = 'Current file'
      end
      object Label11: TLabel
        Left = 5
        Top = 102
        Width = 72
        Height = 13
        Caption = 'Average speed'
      end
      object Label12: TLabel
        Left = 5
        Top = 118
        Width = 18
        Height = 13
        Caption = 'Log'
      end
      object eDupSource: TEdit
        Left = 67
        Top = 12
        Width = 329
        Height = 21
        TabOrder = 0
        Text = 'F:\My Documents'
      end
      object BitBtn1: TBitBtn
        Left = 427
        Top = 12
        Width = 75
        Height = 25
        Caption = 'Run...'
        TabOrder = 1
        OnClick = bbRunClick
      end
      object pbDupAverall: TProgressBar
        Left = 116
        Top = 42
        Width = 175
        Height = 17
        Min = 0
        Max = 100
        Smooth = True
        TabOrder = 2
      end
      object StaticText1: TStaticText
        Left = 300
        Top = 42
        Width = 26
        Height = 17
        Caption = '0 Kb'
        TabOrder = 3
      end
      object pbDupFC: TProgressBar
        Left = 116
        Top = 62
        Width = 175
        Height = 17
        Min = 0
        Max = 100
        Smooth = True
        TabOrder = 4
      end
      object stDupCurrentFile: TStaticText
        Left = 116
        Top = 84
        Width = 30
        Height = 17
        Caption = 'None'
        TabOrder = 5
      end
      object stDupAverageSpeed: TStaticText
        Left = 116
        Top = 102
        Width = 36
        Height = 17
        Caption = '0 Kb/s'
        TabOrder = 6
      end
      object RichEdit1: TRichEdit
        Left = 5
        Top = 328
        Width = 658
        Height = 142
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = '@Arial Unicode MS'
        Font.Style = []
        ParentFont = False
        ScrollBars = ssVertical
        TabOrder = 7
      end
      object lvDublicates: TListView
        Left = 8
        Top = 152
        Width = 649
        Height = 150
        Columns = <
          item
            Caption = 'Source'
          end
          item
            Caption = 'Timestamp'
          end>
        GridLines = True
        MultiSelect = True
        ReadOnly = True
        RowSelect = True
        TabOrder = 8
        ViewStyle = vsReport
      end
    end
  end
end
