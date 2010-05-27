object dlgMain: TdlgMain
  Left = 508
  Top = 531
  Width = 709
  Height = 433
  ActiveControl = eA
  Caption = 'LCELLS'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = mMenu
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Splitter1: TSplitter
    Left = 0
    Top = 137
    Width = 693
    Height = 3
    Cursor = crVSplit
    Align = alTop
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 693
    Height = 137
    Align = alTop
    TabOrder = 0
    object lvList: TListView
      Left = 321
      Top = 1
      Width = 371
      Height = 135
      Align = alClient
      Columns = <
        item
          Caption = '#'
          Width = 30
        end
        item
          AutoSize = True
          Caption = 'File Name'
        end>
      GridLines = True
      HideSelection = False
      ReadOnly = True
      RowSelect = True
      TabOrder = 0
      ViewStyle = vsReport
      OnDblClick = lvListDblClick
      OnSelectItem = lvListSelectItem
    end
    object Panel3: TPanel
      Left = 1
      Top = 1
      Width = 320
      Height = 135
      Align = alLeft
      TabOrder = 1
      object Label1: TLabel
        Left = 0
        Top = 10
        Width = 26
        Height = 13
        Caption = 'Cell &a'
        FocusControl = eA
      end
      object Label2: TLabel
        Left = 0
        Top = 32
        Width = 26
        Height = 13
        Caption = 'Cell &b'
        FocusControl = eB
      end
      object Label3: TLabel
        Left = 0
        Top = 54
        Width = 26
        Height = 13
        Caption = 'Cell &c'
        FocusControl = eC
      end
      object Label4: TLabel
        Left = 141
        Top = 7
        Width = 46
        Height = 13
        Caption = 'Cell al&pha'
        FocusControl = eAA
      end
      object Label5: TLabel
        Left = 141
        Top = 31
        Width = 41
        Height = 13
        Caption = 'Cell be&ta'
        FocusControl = eAB
      end
      object Label6: TLabel
        Left = 141
        Top = 57
        Width = 54
        Height = 13
        Caption = 'Cell &gamma'
        FocusControl = eAC
      end
      object Label7: TLabel
        Left = 141
        Top = 80
        Width = 48
        Height = 13
        Caption = '&Deviation '
        FocusControl = eDev
      end
      object Label8: TLabel
        Left = 4
        Top = 81
        Width = 32
        Height = 13
        Caption = 'Lattice'
      end
      object eC: TEdit
        Left = 39
        Top = 52
        Width = 90
        Height = 21
        TabOrder = 2
      end
      object eB: TEdit
        Left = 39
        Top = 30
        Width = 90
        Height = 21
        TabOrder = 1
      end
      object eA: TEdit
        Left = 39
        Top = 8
        Width = 90
        Height = 21
        TabOrder = 0
      end
      object eDev: TEdit
        Left = 205
        Top = 80
        Width = 90
        Height = 21
        TabOrder = 7
        Text = '0.01'
      end
      object eAC: TEdit
        Left = 205
        Top = 54
        Width = 90
        Height = 21
        TabOrder = 5
        Text = '90'
      end
      object eAB: TEdit
        Left = 205
        Top = 29
        Width = 90
        Height = 21
        TabOrder = 4
        Text = '90'
      end
      object eAA: TEdit
        Left = 205
        Top = 5
        Width = 90
        Height = 21
        TabOrder = 3
        Text = '90'
      end
      object bbSearch: TBitBtn
        Left = 220
        Top = 104
        Width = 75
        Height = 25
        Caption = '&Search'
        TabOrder = 8
        OnClick = bbSearchClick
        Glyph.Data = {
          76010000424D7601000000000000760000002800000020000000100000000100
          04000000000000010000130B0000130B00001000000000000000000000000000
          800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
          333333333333333333FF33333333333330003FF3FFFFF3333777003000003333
          300077F777773F333777E00BFBFB033333337773333F7F33333FE0BFBF000333
          330077F3337773F33377E0FBFBFBF033330077F3333FF7FFF377E0BFBF000000
          333377F3337777773F3FE0FBFBFBFBFB039977F33FFFFFFF7377E0BF00000000
          339977FF777777773377000BFB03333333337773FF733333333F333000333333
          3300333777333333337733333333333333003333333333333377333333333333
          333333333333333333FF33333333333330003333333333333777333333333333
          3000333333333333377733333333333333333333333333333333}
        NumGlyphs = 2
      end
      object cbLattice: TComboBox
        Left = 72
        Top = 77
        Width = 58
        Height = 21
        ItemHeight = 13
        TabOrder = 6
        Text = 'P'
        OnChange = sbUpdateCellClick
        Items.Strings = (
          'P'
          'I'
          'R'
          'F'
          'A'
          'B'
          'C')
      end
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 140
    Width = 693
    Height = 30
    Align = alTop
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
    object lFound: TLabel
      Left = 147
      Top = 1
      Width = 20
      Height = 13
      Caption = 'Cell:'
    end
    object sbSaveAs: TSpeedButton
      Left = 0
      Top = 1
      Width = 23
      Height = 22
      Hint = 'Save File as...'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000130B0000130B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333330070
        7700333333337777777733333333008088003333333377F73377333333330088
        88003333333377FFFF7733333333000000003FFFFFFF77777777000000000000
        000077777777777777770FFFFFFF0FFFFFF07F3333337F3333370FFFFFFF0FFF
        FFF07F3FF3FF7FFFFFF70F00F0080CCC9CC07F773773777777770FFFFFFFF039
        99337F3FFFF3F7F777F30F0000F0F09999937F7777373777777F0FFFFFFFF999
        99997F3FF3FFF77777770F00F000003999337F773777773777F30FFFF0FF0339
        99337F3FF7F3733777F30F08F0F0337999337F7737F73F7777330FFFF0039999
        93337FFFF7737777733300000033333333337777773333333333}
      NumGlyphs = 2
      ParentShowHint = False
      ShowHint = True
      OnClick = sbSaveAsClick
    end
    object lNiggli: TLabel
      Left = 118
      Top = 13
      Width = 49
      Height = 13
      Caption = 'Niggli Cell:'
    end
    object sbUpdateCell: TSpeedButton
      Left = 28
      Top = 1
      Width = 23
      Height = 22
      Hint = 'Update Cell Information'
      Glyph.Data = {
        B6030000424DB60300000000000036000000280000000E000000100000000100
        20000000000080030000C40E0000C40E00000000000000000000FFFFFF00DEEF
        EF00DEEFEF000000000000000000DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEF
        EF00DEEFEF00DEEFEF00DEEFEF00FFFFFF00FFFFFF00DEEFEF00DEEFEF000000
        000000FFFF0000000000DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEF
        EF00DEEFEF00FFFFFF00FFFFFF00DEEFEF00DEEFEF00DEEFEF000000000000FF
        FF0000000000DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00FFFF
        FF00FFFFFF00DEEFEF00DEEFEF00DEEFEF0000000000FFFFFF0000FFFF000000
        0000DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00FFFFFF00FFFFFF00DEEF
        EF00DEEFEF00DEEFEF00DEEFEF0000000000FFFFFF0000FFFF0000000000DEEF
        EF00DEEFEF00DEEFEF00DEEFEF00FFFFFF00FFFFFF00DEEFEF00DEEFEF000000
        0000000000000000000000FFFF00000000000000000000000000DEEFEF00DEEF
        EF00DEEFEF00FFFFFF00FFFFFF00DEEFEF00DEEFEF00DEEFEF000000000000FF
        FF00FFFFFF0000000000DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00FFFF
        FF00FFFFFF00DEEFEF00DEEFEF00DEEFEF00DEEFEF000000000000FFFF00FFFF
        FF0000000000DEEFEF00DEEFEF00DEEFEF00DEEFEF00FFFFFF00FFFFFF00DEEF
        EF00DEEFEF00DEEFEF00DEEFEF0000000000FFFFFF0000FFFF00FFFFFF000000
        0000DEEFEF00DEEFEF00DEEFEF00FFFFFF00FFFFFF00DEEFEF00000000000000
        00000000000000000000FFFFFF0000000000000000000000000000000000DEEF
        EF00DEEFEF00FFFFFF00FFFFFF00DEEFEF00DEEFEF000000000000FFFF00FFFF
        FF0000FFFF00FFFFFF0000000000DEEFEF00DEEFEF00DEEFEF00DEEFEF00FFFF
        FF00FFFFFF00DEEFEF00DEEFEF00DEEFEF000000000000FFFF00FFFFFF0000FF
        FF00FFFFFF0000000000DEEFEF00DEEFEF00DEEFEF00FFFFFF00FFFFFF00DEEF
        EF00DEEFEF00DEEFEF0000000000FFFFFF0000FFFF00FFFFFF0000FFFF00FFFF
        FF0000000000DEEFEF00DEEFEF00FFFFFF00FFFFFF00DEEFEF00DEEFEF00DEEF
        EF00DEEFEF000000000000000000000000000000000000000000000000000000
        0000DEEFEF00FFFFFF00FFFFFF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEF
        EF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00DEEFEF00FFFF
        FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
        FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00}
      OnClick = sbUpdateCellClick
    end
    object sbView: TSpeedButton
      Left = 55
      Top = 1
      Width = 23
      Height = 22
      Hint = 'Run XP'
      Caption = 'XP'
      Enabled = False
      OnClick = sbViewClick
    end
    object sbOlex2: TSpeedButton
      Left = 81
      Top = 1
      Width = 35
      Height = 22
      Hint = 'Run XP'
      Caption = 'Olex2'
      Enabled = False
      OnClick = sbOlex2Click
    end
  end
  object Panel5: TPanel
    Left = 0
    Top = 170
    Width = 693
    Height = 205
    Align = alClient
    Caption = 'Panel5'
    TabOrder = 2
    object Panel4: TPanel
      Left = 1
      Top = 1
      Width = 691
      Height = 203
      Align = alClient
      TabOrder = 0
      object Splitter2: TSplitter
        Left = 225
        Top = 1
        Width = 3
        Height = 201
        Cursor = crHSplit
      end
      object mMemo: TRichEdit
        Left = 228
        Top = 1
        Width = 462
        Height = 201
        Align = alClient
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        PlainText = True
        ReadOnly = True
        ScrollBars = ssBoth
        TabOrder = 0
        WordWrap = False
      end
      object tvTree: TTreeView
        Left = 1
        Top = 1
        Width = 224
        Height = 201
        Align = alLeft
        Indent = 19
        PopupMenu = pmTree
        ReadOnly = True
        TabOrder = 1
        OnGetSelectedIndex = tvTreeGetSelectedIndex
      end
    end
  end
  object mMenu: TMainMenu
    Left = 416
    Top = 296
    object miInfo: TMenuItem
      Caption = 'Tools...'
      OnClick = miInfoClick
    end
    object miPreferences: TMenuItem
      Caption = 'Preferences...'
      OnClick = miPreferencesClick
    end
    object miSearch: TMenuItem
      Caption = 'Search...'
      OnClick = miSearchClick
    end
    object miAbout: TMenuItem
      Caption = 'About...'
      OnClick = miAboutClick
    end
    object miExit: TMenuItem
      Caption = 'Exit'
      OnClick = miExitClick
    end
  end
  object dlgSave: TSaveDialog
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 352
    Top = 124
  end
  object pmTree: TPopupMenu
    Left = 168
    Top = 256
    object Expandall1: TMenuItem
      Caption = 'Expand all'
      OnClick = Expandall1Click
    end
    object Collapseall1: TMenuItem
      Caption = 'Collapse all'
      OnClick = Collapseall1Click
    end
  end
end
