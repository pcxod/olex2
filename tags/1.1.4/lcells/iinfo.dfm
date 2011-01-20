object dlgIndexInfo: TdlgIndexInfo
  Left = 380
  Top = 234
  BorderStyle = bsDialog
  Caption = 'Index Info'
  ClientHeight = 271
  ClientWidth = 360
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 55
    Height = 13
    Caption = 'Files Count:'
  end
  object Label2: TLabel
    Left = 8
    Top = 43
    Width = 90
    Height = 13
    Caption = 'Last Time Updated'
  end
  object eCount: TEdit
    Left = 35
    Top = 21
    Width = 63
    Height = 21
    ReadOnly = True
    TabOrder = 0
  end
  object eDate: TEdit
    Left = 35
    Top = 67
    Width = 148
    Height = 21
    ReadOnly = True
    TabOrder = 1
  end
  object bbClose: TBitBtn
    Left = 282
    Top = 234
    Width = 75
    Height = 25
    TabOrder = 2
    Kind = bkClose
  end
  object GroupBox1: TGroupBox
    Left = 193
    Top = 8
    Width = 163
    Height = 82
    Caption = 'Index Cleanup'
    TabOrder = 3
    object bbClean: TBitBtn
      Left = 8
      Top = 51
      Width = 75
      Height = 25
      Caption = 'Clean'
      TabOrder = 0
      OnClick = bbCleanClick
    end
    object cbDead: TCheckBox
      Left = 8
      Top = 33
      Width = 97
      Height = 17
      Caption = 'Dead links'
      TabOrder = 1
    end
    object cbCell: TCheckBox
      Left = 8
      Top = 16
      Width = 150
      Height = 17
      Caption = 'Records with the same cell'
      Checked = True
      State = cbChecked
      TabOrder = 2
    end
  end
  object GroupBox2: TGroupBox
    Left = 8
    Top = 96
    Width = 349
    Height = 129
    Caption = 'Update/Create Index'
    TabOrder = 4
    object sbBrowse: TSpeedButton
      Left = 315
      Top = 67
      Width = 23
      Height = 22
      Caption = '...'
      OnClick = sbBrowseClick
    end
    object Label3: TLabel
      Left = 151
      Top = 24
      Width = 114
      Height = 13
      Caption = 'Files should not exceed:'
    end
    object Label4: TLabel
      Left = 326
      Top = 24
      Width = 15
      Height = 13
      Caption = 'Mb'
    end
    object rbAllDrives: TRadioButton
      Left = 4
      Top = 24
      Width = 113
      Height = 17
      Caption = 'All Drives'
      Checked = True
      TabOrder = 0
      TabStop = True
    end
    object rbFromFolder: TRadioButton
      Left = 4
      Top = 48
      Width = 113
      Height = 17
      Caption = 'From Folder'
      TabOrder = 1
    end
    object bbUpdate: TBitBtn
      Left = 263
      Top = 94
      Width = 75
      Height = 25
      Caption = 'Update'
      TabOrder = 2
      OnClick = bbUpdateClick
    end
    object eLimit: TEdit
      Left = 273
      Top = 21
      Width = 45
      Height = 21
      TabOrder = 3
      Text = '50'
    end
    object rbList: TRadioButton
      Left = 6
      Top = 98
      Width = 113
      Height = 17
      Caption = 'Using the List'
      TabOrder = 4
    end
  end
  object eDir: TEdit
    Left = 24
    Top = 164
    Width = 276
    Height = 21
    TabOrder = 5
  end
  object bbClear: TBitBtn
    Left = 106
    Top = 18
    Width = 75
    Height = 25
    Caption = 'Clear'
    TabOrder = 6
    OnClick = bbClearClick
  end
end
