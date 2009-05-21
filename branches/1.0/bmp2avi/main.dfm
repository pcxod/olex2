object dlgMain: TdlgMain
  Left = 478
  Top = 145
  Width = 995
  Height = 787
  Caption = 'Bmp2AVI'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  WindowState = wsMaximized
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object Splitter2: TSplitter
    Left = 185
    Top = 0
    Width = 3
    Height = 753
    Cursor = crHSplit
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 185
    Height = 753
    Align = alLeft
    TabOrder = 0
    object Panel3: TPanel
      Left = 1
      Top = 113
      Width = 183
      Height = 639
      Align = alClient
      Caption = 'Panel3'
      TabOrder = 0
      object Splitter1: TSplitter
        Left = 1
        Top = 1
        Width = 181
        Height = 9
        Cursor = crVSplit
        Align = alTop
      end
      object lvFiles: TListView
        Left = 1
        Top = 10
        Width = 181
        Height = 628
        Align = alClient
        Checkboxes = True
        Columns = <
          item
            AutoSize = True
            Caption = 'File Name'
          end>
        GridLines = True
        ReadOnly = True
        RowSelect = True
        TabOrder = 0
        ViewStyle = vsReport
        OnSelectItem = lvFilesSelectItem
      end
    end
    object Panel4: TPanel
      Left = 1
      Top = 1
      Width = 183
      Height = 112
      Align = alTop
      TabOrder = 1
      object bbLoadFileList: TBitBtn
        Left = 0
        Top = 0
        Width = 105
        Height = 25
        Caption = 'Load Image List'
        TabOrder = 0
        OnClick = bbLoadFileListClick
      end
      object bbClearFileList: TBitBtn
        Left = 0
        Top = 24
        Width = 105
        Height = 25
        Caption = 'Clear Image List'
        TabOrder = 1
        OnClick = bbClearFileListClick
      end
      object bbSaveAVI: TBitBtn
        Left = 0
        Top = 48
        Width = 105
        Height = 25
        Caption = 'Save AVI'
        Enabled = False
        TabOrder = 2
        OnClick = bbSaveAVIClick
      end
      object leFps: TLabeledEdit
        Left = 24
        Top = 80
        Width = 81
        Height = 21
        EditLabel.Width = 20
        EditLabel.Height = 13
        EditLabel.Caption = 'FPS'
        LabelPosition = lpLeft
        LabelSpacing = 3
        TabOrder = 3
        Text = '10'
      end
    end
  end
  object Panel2: TPanel
    Left = 188
    Top = 0
    Width = 799
    Height = 753
    Align = alClient
    TabOrder = 1
    object iImage: TImage
      Left = 1
      Top = 1
      Width = 797
      Height = 751
      Align = alClient
    end
  end
  object dlgBmpLoad: TOpenDialog
    Filter = 'Bitmap files (*.bmp)|*.bmp'
    Options = [ofHideReadOnly, ofAllowMultiSelect, ofFileMustExist, ofEnableSizing]
    Left = 480
    Top = 376
  end
  object dlgSaveAVI: TSaveDialog
    DefaultExt = 'avi'
    Filter = 'AVI files (*.avi)|*.avi'
    Left = 448
    Top = 320
  end
end
