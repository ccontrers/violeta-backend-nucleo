object FormSisCambiarClave: TFormSisCambiarClave
  Left = 444
  Top = 269
  ParentCustomHint = False
  Caption = 'Cambio de clave del usuario '
  ClientHeight = 274
  ClientWidth = 441
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnPaint = FormPaint
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 22
    Top = 8
    Width = 406
    Height = 39
    Caption = 
      'Las claves de acceso nuevas,  deben ser m'#237'nimo de 8 caracteres y' +
      ' m'#225'ximo 13, deben tener al menos una letra. Adem'#225's deben contene' +
      'r al menos un d'#237'gito. Solo se admiten letras y d'#237'gitos.'
    Color = clWhite
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentColor = False
    ParentFont = False
    Transparent = False
    WordWrap = True
  end
  object EditPasswordAnterior: VTLabeledEdit
    Left = 120
    Top = 104
    Width = 201
    Height = 21
    EditLabel.Width = 68
    EditLabel.Height = 13
    EditLabel.Caption = 'Clave anterior'
    MaxLength = 13
    PasswordChar = '*'
    TabOrder = 0
    ActivarOnChange = True
    Modificado = True
  end
  object EditPassword: VTLabeledEdit
    Left = 120
    Top = 144
    Width = 201
    Height = 21
    EditLabel.Width = 111
    EditLabel.Height = 13
    EditLabel.Caption = 'Clave de acceso nueva'
    MaxLength = 13
    PasswordChar = '*'
    TabOrder = 1
    ActivarOnChange = True
    Modificado = True
  end
  object EditPasswordRepetida: VTLabeledEdit
    Left = 120
    Top = 184
    Width = 201
    Height = 21
    EditLabel.Width = 170
    EditLabel.Height = 13
    EditLabel.Caption = 'Reescriba la clave de acceso nueva'
    MaxLength = 13
    PasswordChar = '*'
    TabOrder = 2
    ActivarOnChange = True
    Modificado = True
  end
  object ButtonCambiar: VTBitBtn
    Left = 120
    Top = 224
    Width = 75
    Height = 25
    Caption = 'Ca&mbiar'
    TabOrder = 3
    OnClick = ButtonCambiarClick
  end
  object ButtonCancelar: VTBitBtn
    Left = 248
    Top = 224
    Width = 75
    Height = 25
    Caption = 'C&ancelar'
    TabOrder = 4
    OnClick = ButtonCancelarClick
  end
end
