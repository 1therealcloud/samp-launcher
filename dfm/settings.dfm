object fmSettings: TfmSettings
  Left = 473
  Top = 185
  BorderStyle = bsDialog
  Caption = 'Settings'
  ClientHeight = 298
  ClientWidth = 318
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Verdana'
  Font.Style = []
  Position = poScreenCenter
  OnCreate = FormCreate
  TextHeight = 13
  object bnSave: TButton
    Left = 8
    Top = 264
    Width = 75
    Height = 25
    Caption = 'Save'
    Default = True
    TabOrder = 0
    OnClick = bnSaveClick
  end
  object bnCancel: TButton
    Left = 88
    Top = 264
    Width = 75
    Height = 25
    Caption = 'Cancel'
    TabOrder = 1
    OnClick = bnCancelClick
  end
  object bnLaunchDebug: TButton
    Left = 198
    Top = 264
    Width = 111
    Height = 25
    Caption = 'Launch Debug'
    TabOrder = 2
    OnClick = bnLaunchDebugClick
  end
  object gbPasswords: TGroupBox
    Left = 8
    Top = 8
    Width = 301
    Height = 249
    Caption = ' Passwords '
    TabOrder = 3
    object Label1: TLabel
      Left = 16
      Top = 80
      Width = 196
      Height = 13
      Caption = 'San Andreas Installation Location:'
    end
    object lblModelCacheTag: TLabel
      Left = 16
      Top = 120
      Width = 129
      Height = 13
      Caption = 'Model Cache Location:'
    end
    object lblProxyAddr: TLabel
      Left = 16
      Top = 160
      Width = 254
      Height = 13
      Caption = 'Download Proxy: (http://,https://,socks5://)'
    end
    object lblDebugScript: TLabel
      Left = 16
      Top = 200
      Width = 82
      Height = 13
      Caption = 'Debug Script:'
    end
    object sbBrowse: TSpeedButton
      Left = 260
      Top = 96
      Width = 23
      Height = 21
      ImageIndex = 10
      ImageName = 'sbBrowse'
      Images = fmMain.VirtualImageList1
      Flat = True
      OnClick = sbBrowseClick
    end
    object sbBrowseCache: TSpeedButton
      Left = 260
      Top = 136
      Width = 23
      Height = 21
      ImageIndex = 10
      ImageName = 'sbBrowse'
      Images = fmMain.VirtualImageList1
      Flat = True
      OnClick = sbBrowseCacheClick
    end
    object sbBrowseDebugScript: TSpeedButton
      Left = 260
      Top = 216
      Width = 23
      Height = 21
      ImageIndex = 10
      ImageName = 'sbBrowse'
      Images = fmMain.VirtualImageList1
      Flat = True
      OnClick = sbBrowseDebugScriptClick
    end
    object cbSaveServerPasswords: TCheckBox
      Left = 16
      Top = 24
      Width = 201
      Height = 17
      Caption = 'Auto-Save Server Passwords'
      TabOrder = 0
    end
    object cbSaveRconPasswords: TCheckBox
      Left = 16
      Top = 48
      Width = 201
      Height = 17
      Caption = 'Auto-Save Rcon Passwords'
      TabOrder = 1
    end
    object edInstallLoc: TEdit
      Left = 16
      Top = 96
      Width = 245
      Height = 21
      Color = clBtnFace
      ReadOnly = True
      TabOrder = 2
    end
    object edCacheLoc: TEdit
      Left = 16
      Top = 136
      Width = 245
      Height = 21
      Color = clBtnFace
      ReadOnly = True
      TabOrder = 3
    end
    object edProxyAddress: TEdit
      Left = 16
      Top = 176
      Width = 245
      Height = 21
      HideSelection = False
      MaxLength = 64
      TabOrder = 4
    end
    object edDebugScript: TEdit
      Left = 16
      Top = 216
      Width = 245
      Height = 21
      Color = clBtnFace
      HideSelection = False
      MaxLength = 255
      ReadOnly = True
      TabOrder = 5
    end
  end
end
