object Form1: TForm1
  Left = 226
  Top = 286
  Width = 777
  Height = 583
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poScreenCenter
  ShowHint = True
  OnMouseDown = FormMouseDown
  OnMouseMove = FormMouseMove
  OnMouseUp = FormMouseUp
  OnPaint = FormPaint
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object sbStatus: TStatusBar
    Left = 0
    Top = 508
    Width = 761
    Height = 19
    Panels = <
      item
        Width = 50
      end>
    SimplePanel = False
  end
  object MainMenu1: TMainMenu
    Left = 280
    Top = 208
    object File1: TMenuItem
      Caption = 'File'
      object Listen1: TMenuItem
        Caption = 'Listen...'
        OnClick = Listen1Click
      end
      object miClear: TMenuItem
        Caption = 'Clear'
        OnClick = miClearClick
      end
      object miTest: TMenuItem
        Caption = 'Test'
        OnClick = miTestClick
      end
    end
    object DrawStyle1: TMenuItem
      Caption = 'Draw Style'
      object Telp1: TMenuItem
        Caption = 'Telp'
        OnClick = Telp1Click
      end
      object Pers1: TMenuItem
        Caption = 'Pers'
        OnClick = Pers1Click
      end
      object Sfil1: TMenuItem
        Caption = 'Sfil'
        OnClick = Sfil1Click
      end
    end
    object View1: TMenuItem
      Caption = 'Labels'
      object miLabels: TMenuItem
        Caption = 'Labels'
        ShortCut = 114
        OnClick = miLabelsClick
      end
      object miH: TMenuItem
        Caption = 'H-Atoms'
        GroupIndex = 1
        OnClick = miHClick
      end
      object miQ: TMenuItem
        Caption = 'Q-Atoms'
        GroupIndex = 2
        OnClick = miQClick
      end
    end
    object View2: TMenuItem
      Caption = 'View'
      object miCell: TMenuItem
        Caption = 'Cell'
        RadioItem = True
        OnClick = miCellClick
      end
    end
  end
  object dlgOpen: TOpenDialog
    Filter = 'Shelx instruction file (*.ins, *.res)|*.ins;*.res'
    Left = 400
    Top = 144
  end
  object tTimer: TTimer
    Interval = 100
    OnTimer = tTimerTimer
    Left = 160
    Top = 104
  end
  object dlgOpen1: TOpenDialog
    FileName = 'ptablex.dat'
    Filter = 'Olex2 data files|ptablex.dat'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 312
    Top = 152
  end
end
