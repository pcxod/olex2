object MF: TMF
  Left = 129
  Top = 98
  Width = 878
  Height = 656
  Caption = 'BTest'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object reEdit: TRichEdit
    Left = 0
    Top = 0
    Width = 870
    Height = 615
    Align = alClient
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssBoth
    TabOrder = 0
  end
  object MainMenu1: TMainMenu
    Left = 424
    Top = 312
    object Test1: TMenuItem
      Caption = 'Test'
      OnClick = Test1Click
    end
    object CPP1: TMenuItem
      Caption = 'CPP'
      OnClick = CPP1Click
    end
  end
  object dlgOpen: TOpenDialog
    Filter = 
      'C Header file|*.h|C++ Header file|*.hpp|C File|*.c|C++ File|*.cp' +
      'p'
    Left = 488
    Top = 288
  end
end
