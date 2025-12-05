object FormCatObjetosSistema: TFormCatObjetosSistema
  Left = 286
  Top = 130
  Caption = 'Objetos del sistema y sus privilegios'
  ClientHeight = 600
  ClientWidth = 450
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnClose = FormClose
  OnPaint = FormPaint
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 17
    Top = 82
    Width = 417
    Height = 247
    Style = bsRaised
  end
  object Bevel2: TBevel
    Left = 17
    Top = 336
    Width = 417
    Height = 89
    Style = bsRaised
  end
  object LabelGrupo: TLabel
    Left = 169
    Top = 341
    Width = 113
    Height = 13
    Caption = 'Grupo al que pertenece'
  end
  object LabelPrivilegios: TLabel
    Left = 169
    Top = 432
    Width = 98
    Height = 13
    Caption = 'Privilegios del objeto'
  end
  object LabelMensaje: TLabel
    Left = 64
    Top = 568
    Width = 329
    Height = 13
    Caption = 'Presione F5 si desea  modificar los privilegio para el objeto'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
  end
  object EditObjeto: VTLabeledEdit
    Left = 69
    Top = 357
    Width = 93
    Height = 21
    CharCase = ecUpperCase
    Color = 16510433
    EditLabel.Width = 78
    EditLabel.Height = 13
    EditLabel.Caption = 'Clave del objeto'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    MaxLength = 10
    ParentFont = False
    TabOrder = 2
    ActivarOnChange = True
    Modificado = True
  end
  object StringGridObjetos: VTStringGrid
    Left = 31
    Top = 94
    Width = 389
    Height = 223
    ColCount = 3
    DefaultRowHeight = 16
    FixedCols = 0
    RowCount = 7
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goRowSelect, goThumbTracking]
    ScrollBars = ssVertical
    TabOrder = 1
    OnClick = StringGridObjetosClick
    ColWidths = (
      64
      64
      64)
    RowHeights = (
      16
      16
      16
      16
      16
      16
      16)
  end
  object EditNombre: VTLabeledEdit
    Left = 69
    Top = 396
    Width = 317
    Height = 21
    CharCase = ecUpperCase
    EditLabel.Width = 37
    EditLabel.Height = 13
    EditLabel.Caption = 'Nombre'
    MaxLength = 40
    TabOrder = 4
    ActivarOnChange = True
    Precision = 2
    Modificado = True
  end
  object ComboBoxGrupo: VTComboBox
    Left = 171
    Top = 356
    Width = 215
    Height = 22
    Style = csOwnerDrawFixed
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    OnKeyDown = ComboBoxGrupoKeyDown
    AnchoColumnas.Strings = (
      '50'
      '120')
  end
  inline FrameNav: TFrameNavegacion
    Left = 83
    Top = 19
    Width = 297
    Height = 49
    TabOrder = 0
    TabStop = True
    ExplicitLeft = 83
    ExplicitTop = 19
    ExplicitWidth = 297
    ExplicitHeight = 49
    inherited ButtonAgregar: VTBitBtn
      OnClick = FrameNavButtonAgregarClick
    end
    inherited ButtonQuitar: VTBitBtn
      OnClick = FrameNavButtonQuitarClick
    end
    inherited ButtonGrabar: VTBitBtn
      OnClick = FrameNavButtonGrabarClick
    end
    inherited ButtonCancelar: VTBitBtn
      OnClick = FrameNavButtonCancelarClick
    end
  end
  object StringGridPrivilegios: VTStringGrid
    Left = 65
    Top = 448
    Width = 329
    Height = 113
    ColCount = 2
    DefaultRowHeight = 16
    FixedCols = 0
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goThumbTracking]
    ScrollBars = ssVertical
    TabOrder = 5
    OnEnter = StringGridPrivilegiosEnter
    OnExit = StringGridPrivilegiosExit
    OnKeyDown = StringGridPrivilegiosKeyDown
    ColWidths = (
      64
      64)
    RowHeights = (
      16
      16
      16
      16
      16)
  end
end
