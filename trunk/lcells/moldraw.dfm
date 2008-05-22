object dlgMolDraw: TdlgMolDraw
  Left = 127
  Top = 164
  Width = 467
  Height = 321
  Caption = 'LCELLS: MolDraw'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  PopupMenu = pmStructure
  Position = poMainFormCenter
  OnClick = FormClick
  OnMouseDown = FormMouseDown
  OnMouseMove = FormMouseMove
  OnMouseUp = FormMouseUp
  OnPaint = FormPaint
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object pmStructure: TPopupMenu
    Left = 216
    Top = 144
    object Grow1: TMenuItem
      Caption = 'Grow'
      OnClick = Grow1Click
    end
  end
end
