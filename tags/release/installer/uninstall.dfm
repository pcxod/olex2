object dlgUninstall: TdlgUninstall
  Left = 522
  Top = 356
  Width = 272
  Height = 232
  Caption = 'Olex2 re-installation'
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
  object GroupBox1: TGroupBox
    Left = 0
    Top = 0
    Width = 256
    Height = 113
    Align = alTop
    Caption = 'Uninstallation options'
    TabOrder = 0
    object rgRemove: TRadioButton
      Left = 8
      Top = 16
      Width = 129
      Height = 17
      Caption = 'Remove completely'
      TabOrder = 0
      OnClick = rgRemoveClick
    end
    object rgRename: TRadioButton
      Left = 8
      Top = 64
      Width = 241
      Height = 17
      Caption = 'Rename folders and shortcuts by appending:'
      Checked = True
      TabOrder = 1
      TabStop = True
    end
    object cbRemoveUserSettings: TCheckBox
      Left = 24
      Top = 32
      Width = 145
      Height = 17
      Caption = 'Remove user settings'
      Checked = True
      Enabled = False
      State = cbChecked
      TabOrder = 2
    end
    object eAppend: TEdit
      Left = 24
      Top = 80
      Width = 121
      Height = 21
      TabOrder = 3
    end
  end
  object cbInstall: TCheckBox
    Left = 8
    Top = 127
    Width = 193
    Height = 17
    Caption = 'Install Olex2 after uninstallation'
    Checked = True
    State = cbChecked
    TabOrder = 1
  end
  object bbOK: TBitBtn
    Left = 40
    Top = 159
    Width = 75
    Height = 25
    TabOrder = 2
    Kind = bkOK
  end
  object bbCancel: TBitBtn
    Left = 144
    Top = 159
    Width = 75
    Height = 25
    TabOrder = 3
    Kind = bkCancel
  end
end
