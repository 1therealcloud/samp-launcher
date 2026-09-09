object fmMain: TfmMain
  Left = 542
  Top = 282
  Caption = 'San Andreas Multiplayer 0.3.7'
  ClientHeight = 532
  ClientWidth = 832
  Color = clBtnFace
  Constraints.MinHeight = 420
  Constraints.MinWidth = 525
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clSilver
  Font.Height = -11
  Font.Name = 'Arial'
  Font.Style = []
  Menu = mmMain
  Position = poScreenCenter
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnResize = FormResize
  OnShow = FormShow
  TextHeight = 14
  object spRight: TSplitter
    Left = 588
    Top = 28
    Width = 10
    Height = 345
    Align = alRight
    Beveled = True
    Color = clMenu
    ParentColor = False
    ResizeStyle = rsUpdate
    ExplicitLeft = 594
    ExplicitHeight = 372
  end
  object sbMain: TStatusBar
    Left = 0
    Top = 493
    Width = 818
    Height = 21
    Panels = <
      item
        Width = 500
      end>
    SimplePanel = True
    SimpleText = 'Client Loaded.'
    SizeGrip = False
    OnDrawPanel = sbMainDrawPanel
  end
  object tbMain: TToolBar
    Left = 0
    Top = 0
    Width = 818
    Height = 28
    ButtonHeight = 24
    Caption = 'tbMain'
    Color = clBtnFace
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clSilver
    Font.Height = -11
    Font.Name = 'Arial'
    Font.Style = []
    Images = VirtualImageList1
    ParentColor = False
    ParentFont = False
    TabOrder = 1
    Transparent = False
    Wrapable = False
    OnResize = tbMainResize
    DesignSize = (
      818
      28)
    object tbConnect: TToolButton
      Left = 0
      Top = 0
      Hint = 'Connect'
      ImageIndex = 13
      ImageName = 'tbConnect'
      ParentShowHint = False
      ShowHint = True
      OnClick = ConnectClick
    end
    object tbSpacer1: TToolButton
      Left = 23
      Top = 0
      Width = 8
      ImageIndex = 15
      ImageName = 'tbDeleteServer'
      Style = tbsSeparator
    end
    object tbAddServer: TToolButton
      Left = 31
      Top = 0
      Hint = 'Add Server'
      ImageIndex = 12
      ImageName = 'tbAddServer'
      ParentShowHint = False
      ShowHint = True
      OnClick = AddServerClick
    end
    object tbDeleteServer: TToolButton
      Left = 54
      Top = 0
      Hint = 'Delete Server'
      ImageIndex = 15
      ImageName = 'tbDeleteServer'
      ParentShowHint = False
      ShowHint = True
      OnClick = DeleteServerClick
    end
    object tbRefreshServer: TToolButton
      Left = 77
      Top = 0
      Hint = 'Refresh Server'
      ImageIndex = 18
      ImageName = 'tbRefreshServer'
      ParentShowHint = False
      ShowHint = True
      OnClick = RefreshServerClick
    end
    object tbSpacer2: TToolButton
      Left = 100
      Top = 0
      Width = 8
      ImageIndex = 15
      ImageName = 'tbDeleteServer'
      Style = tbsSeparator
    end
    object tbMasterServerUpdate: TToolButton
      Left = 108
      Top = 0
      Hint = 'Master Server Update'
      ImageIndex = 17
      ImageName = 'tbMasterServerUpdate'
      ParentShowHint = False
      ShowHint = True
    end
    object tbSpacer3: TToolButton
      Left = 131
      Top = 0
      Width = 8
      ImageIndex = 15
      ImageName = 'tbDeleteServer'
      Style = tbsSeparator
    end
    object tbCopyServerInfo: TToolButton
      Left = 139
      Top = 0
      Hint = 'Copy Server Info'
      ImageIndex = 14
      ImageName = 'tbCopyServerInfo'
      ParentShowHint = False
      ShowHint = True
      OnClick = CopyServerInfoClick
    end
    object tbServerProperties: TToolButton
      Left = 162
      Top = 0
      Hint = 'Server Properties'
      ImageIndex = 19
      ImageName = 'tbServerProperties'
      ParentShowHint = False
      ShowHint = True
      OnClick = ServerPropertiesClick
    end
    object tbSpacer4: TToolButton
      Left = 185
      Top = 0
      Width = 8
      ImageIndex = 15
      ImageName = 'tbDeleteServer'
      Style = tbsSeparator
    end
    object tbSettings: TToolButton
      Left = 193
      Top = 0
      Hint = 'Settings'
      ImageIndex = 20
      ImageName = 'tbSettings'
      ParentShowHint = False
      ShowHint = True
      OnClick = SettingsClick
    end
    object tbSpacer5: TToolButton
      Left = 216
      Top = 0
      Width = 8
      ImageIndex = 15
      ImageName = 'tbDeleteServer'
      Style = tbsSeparator
    end
    object tbHelp: TToolButton
      Left = 224
      Top = 0
      Hint = 'Help'
      ImageIndex = 16
      ImageName = 'tbHelp'
      ParentShowHint = False
      ShowHint = True
      OnClick = HelpTopicsClick
    end
    object tbAbout: TToolButton
      Left = 247
      Top = 0
      Hint = 'About'
      ImageIndex = 11
      ImageName = 'tbAbout'
      ParentShowHint = False
      ShowHint = True
      OnClick = AboutClick
    end
    object lblPlayerName: TLabel
      Left = 270
      Top = 0
      Width = 50
      Height = 24
      Anchors = [akLeft]
      AutoSize = False
      Caption = '   Name: '
      Color = clBtnFace
      Font.Charset = ANSI_CHARSET
      Font.Color = clBtnText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      Layout = tlCenter
    end
    object edName: TEdit
      Left = 320
      Top = 0
      Width = 160
      Height = 24
      Anchors = [akLeft]
      AutoSize = False
      BevelInner = bvNone
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Arial'
      Font.Style = []
      MaxLength = 30
      ParentFont = False
      TabOrder = 0
    end
    object ToolButton1: TToolButton
      Left = 480
      Top = 0
      Width = 345
      ImageIndex = 4
      ImageName = 'imUpArrow'
      Style = tbsSeparator
    end
    object imLogo: TVirtualImage
      Left = 825
      Top = 0
      Width = 57
      Height = 26
      Cursor = crHandPoint
      Anchors = []
      Center = True
      ImageCollection = ImageCollection1
      ImageWidth = 0
      ImageHeight = 0
      ImageIndex = -1
      ImageName = 'imLogo'
      OnClick = imLogoClick
    end
  end
  object tsServerLists: TTabSet
    Left = 0
    Top = 472
    Width = 818
    Height = 21
    Align = alBottom
    DitherBackground = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBtnText
    Font.Height = -12
    Font.Name = 'Arial'
    Font.Style = [fsBold]
    StartMargin = 0
    SoftTop = True
    TabHeight = 25
    Tabs.Strings = (
      '    Favorites   '
      '    Internet    '
      '     Hosted    ')
    TabIndex = 0
    UnselectedColor = clBtnShadow
    OnChange = tsServerListsChange
  end
  object pnBreakable: TPanel
    Left = 0
    Top = 373
    Width = 818
    Height = 99
    Align = alBottom
    BevelOuter = bvNone
    Ctl3D = True
    Font.Charset = OEM_CHARSET
    Font.Color = clSilver
    Font.Height = -12
    Font.Name = 'Arial'
    Font.Style = []
    ParentCtl3D = False
    ParentFont = False
    ParentShowHint = False
    ShowHint = False
    TabOrder = 3
    OnResize = pnBreakableResize
    object gbFilter: TGroupBox
      Left = 0
      Top = 0
      Width = 241
      Height = 99
      Align = alLeft
      Caption = ' Filter '
      Color = clBtnFace
      Ctl3D = False
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBtnText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentColor = False
      ParentCtl3D = False
      ParentFont = False
      TabOrder = 0
      OnDblClick = ToggleFilterServerInfo
      object edFilterMode: TLabeledEdit
        Left = 8
        Top = 32
        Width = 121
        Height = 15
        TabStop = False
        BevelInner = bvNone
        BevelKind = bkFlat
        BorderStyle = bsNone
        Ctl3D = True
        EditLabel.Width = 30
        EditLabel.Height = 13
        EditLabel.Caption = 'Mode:'
        EditLabel.Font.Charset = DEFAULT_CHARSET
        EditLabel.Font.Color = clWindowText
        EditLabel.Font.Height = -12
        EditLabel.Font.Name = 'MS Sans Serif'
        EditLabel.Font.Style = []
        EditLabel.ParentFont = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -12
        Font.Name = 'Arial'
        Font.Style = []
        ParentCtl3D = False
        ParentFont = False
        TabOrder = 0
        Text = ''
        OnChange = FilterChange
        OnDblClick = ToggleFilterServerInfo
      end
      object edFilterMap: TLabeledEdit
        Left = 8
        Top = 72
        Width = 121
        Height = 15
        TabStop = False
        BevelInner = bvNone
        BevelKind = bkFlat
        BorderStyle = bsNone
        Ctl3D = True
        EditLabel.Width = 51
        EditLabel.Height = 13
        EditLabel.Caption = 'Language:'
        EditLabel.Font.Charset = DEFAULT_CHARSET
        EditLabel.Font.Color = clWindowText
        EditLabel.Font.Height = -12
        EditLabel.Font.Name = 'MS Sans Serif'
        EditLabel.Font.Style = []
        EditLabel.ParentFont = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -12
        Font.Name = 'Arial'
        Font.Style = []
        ParentCtl3D = False
        ParentFont = False
        TabOrder = 1
        Text = ''
        OnChange = FilterChange
        OnDblClick = ToggleFilterServerInfo
      end
      object cbFilterEmpty: TCheckBox
        Left = 136
        Top = 52
        Width = 97
        Height = 17
        TabStop = False
        Caption = 'Not Empty'
        Ctl3D = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        ParentCtl3D = False
        ParentFont = False
        TabOrder = 2
        OnClick = FilterChange
      end
      object cbFilterPassworded: TCheckBox
        Left = 136
        Top = 72
        Width = 97
        Height = 17
        TabStop = False
        Caption = 'No Password'
        Ctl3D = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        ParentCtl3D = False
        ParentFont = False
        TabOrder = 3
        OnClick = FilterChange
      end
      object cbFilterFull: TCheckBox
        Left = 136
        Top = 32
        Width = 97
        Height = 17
        TabStop = False
        Caption = 'Not Full'
        Ctl3D = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        ParentCtl3D = False
        ParentFont = False
        TabOrder = 4
        OnClick = FilterChange
      end
    end
    object gbInfo: TGroupBox
      Left = 343
      Top = 0
      Width = 485
      Height = 99
      Align = alRight
      Caption = ' Server Info '
      Ctl3D = False
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBtnText
      Font.Height = -12
      Font.Name = 'Arial'
      Font.Style = []
      ParentCtl3D = False
      ParentFont = False
      PopupMenu = pmServers
      TabOrder = 1
      OnContextPopup = lbServersContextPopup
      OnDblClick = ToggleFilterServerInfo
      object lbSIAddressLab: TLabel
        Left = 8
        Top = 16
        Width = 51
        Height = 14
        Caption = 'Address:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = [fsBold]
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object lbSIModeLab: TLabel
        Left = 8
        Top = 64
        Width = 34
        Height = 14
        Caption = 'Mode:'
        Font.Charset = ANSI_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = [fsBold]
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object lbSIMapLab: TLabel
        Left = 8
        Top = 80
        Width = 57
        Height = 14
        Caption = 'Language:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = [fsBold]
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object lbSIPlayersLab: TLabel
        Left = 8
        Top = 32
        Width = 44
        Height = 14
        Caption = 'Players:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = [fsBold]
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object lbSIPingLab: TLabel
        Left = 8
        Top = 48
        Width = 27
        Height = 14
        Caption = 'Ping:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = [fsBold]
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object lbSIPing: TLabel
        Left = 72
        Top = 47
        Width = 175
        Height = 13
        AutoSize = False
        Caption = '- - -'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object lbSIPlayers: TLabel
        Left = 72
        Top = 32
        Width = 175
        Height = 13
        AutoSize = False
        Caption = '- - -'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object lbSIMap: TLabel
        Left = 72
        Top = 80
        Width = 175
        Height = 13
        AutoSize = False
        Caption = '- - -'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object lbSIMode: TLabel
        Left = 72
        Top = 64
        Width = 175
        Height = 13
        AutoSize = False
        Caption = '- - -'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        ParentFont = False
        OnDblClick = ToggleFilterServerInfo
      end
      object edSIAddress: TEdit
        Left = 72
        Top = 16
        Width = 175
        Height = 13
        TabStop = False
        BorderStyle = bsNone
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBtnText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        ParentColor = True
        ParentFont = False
        ParentShowHint = False
        PopupMenu = pmCopy
        ReadOnly = True
        ShowHint = False
        TabOrder = 0
        Text = '- - -'
      end
      object chSIPingChart: TChart
        Left = 264
        Top = 17
        Width = 220
        Height = 73
        AllowPanning = pmNone
        BackWall.Color = clBlack
        BackWall.Transparent = False
        BottomWall.Color = 16384
        Foot.Visible = False
        LeftWall.Color = 8454016
        Legend.Shadow.Color = clSilver
        Legend.Visible = False
        Title.Text.Strings = (
          'Time')
        Title.Visible = False
        Title.AdjustFrame = False
        BottomAxis.Automatic = False
        BottomAxis.AutomaticMaximum = False
        BottomAxis.AutomaticMinimum = False
        BottomAxis.Maximum = 60.000000000000000000
        BottomAxis.Visible = False
        LeftAxis.Automatic = False
        LeftAxis.AutomaticMaximum = False
        LeftAxis.AutomaticMinimum = False
        LeftAxis.Axis.Visible = False
        LeftAxis.Grid.Color = clBlue
        LeftAxis.Grid.SmallDots = True
        LeftAxis.Grid.Visible = False
        LeftAxis.LabelsFormat.Font.Height = -11
        LeftAxis.LabelsFormat.Font.Name = 'Tahoma'
        LeftAxis.Maximum = 50.000000000000000000
        LeftAxis.MinorGrid.Color = 7303023
        LeftAxis.MinorGrid.SmallDots = True
        LeftAxis.MinorGrid.Visible = True
        LeftAxis.MinorTickCount = 1
        LeftAxis.MinorTickLength = 3
        LeftAxis.MinorTicks.SmallDots = True
        LeftAxis.EndPosition = 97.000000000000000000
        LeftAxis.PositionPercent = -2.000000000000000000
        LeftAxis.TickLength = 6
        LeftAxis.TicksInner.Visible = False
        LeftAxis.Title.Caption = 'Ping'
        LeftAxis.Title.Font.Height = -11
        Pages.MaxPointsPerPage = 30
        RightAxis.Visible = False
        TopAxis.Labels = False
        TopAxis.LabelsFormat.Font.Color = clWindowText
        TopAxis.LabelsFormat.Font.Height = -11
        TopAxis.LabelsFormat.Visible = False
        TopAxis.Ticks.Color = clWindowText
        TopAxis.Visible = False
        View3D = False
        Zoom.Allow = False
        BevelOuter = bvNone
        TabOrder = 1
        OnDblClick = ToggleFilterServerInfo
        DefaultCanvas = 'TGDIPlusCanvas'
        ColorPaletteIndex = 13
        object chSIPingLineChart: TFastLineSeries
          HoverElement = []
          Marks.Style = smsValue
          SeriesColor = 33023
          LinePen.Color = 33023
          LinePen.Width = 2
          XValues.Name = 'X'
          XValues.Order = loAscending
          YValues.Name = 'Y'
          YValues.Order = loNone
        end
      end
    end
  end
  object pnLine: TPanel
    Left = 0
    Top = 28
    Width = 818
    Height = 0
    Align = alTop
    BevelOuter = bvNone
    Color = clBlack
    TabOrder = 4
  end
  object pnRight: TPanel
    Left = 598
    Top = 28
    Width = 220
    Height = 345
    Align = alRight
    BevelOuter = bvNone
    Color = 4933703
    Ctl3D = True
    ParentCtl3D = False
    TabOrder = 5
    object Splitter1: TSplitter
      Left = 0
      Top = 203
      Width = 220
      Height = 10
      Cursor = crVSplit
      Align = alBottom
      Beveled = True
      Color = clMenu
      ParentColor = False
      ResizeStyle = rsUpdate
      ExplicitTop = 212
    end
    object pnPlayers: TPanel
      Left = 0
      Top = 0
      Width = 220
      Height = 203
      Align = alClient
      BevelOuter = bvNone
      TabOrder = 0
      object lbPlayers: TListBox
        Left = 0
        Top = 17
        Width = 220
        Height = 186
        TabStop = False
        Style = lbOwnerDrawFixed
        Align = alClient
        BorderStyle = bsNone
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ItemHeight = 19
        ParentFont = False
        TabOrder = 0
        OnDrawItem = lbPlayersDrawItem
        OnExit = lbPlayersExit
      end
      object hcPlayers: THeaderControl
        Left = 0
        Top = 0
        Width = 220
        Height = 17
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        Images = ilMain
        Sections = <
          item
            AllowClick = False
            ImageIndex = -1
            MinWidth = 30
            Text = 'Player'
            Width = 140
          end
          item
            AllowClick = False
            ImageIndex = -1
            MinWidth = 30
            Text = 'Score'
            Width = 60
          end>
        Style = hsFlat
        OnSectionResize = hcPlayersSectionResize
        ParentFont = False
      end
    end
    object pnRules: TPanel
      Left = 0
      Top = 195
      Width = 220
      Height = 150
      Align = alBottom
      BevelOuter = bvNone
      TabOrder = 1
      object label_url: TLabel
        Left = 0
        Top = 132
        Width = 220
        Height = 18
        Cursor = crHandPoint
        Align = alBottom
        Alignment = taCenter
        AutoSize = False
        Color = clWindow
        Font.Charset = ANSI_CHARSET
        Font.Color = clBlue
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = [fsItalic, fsUnderline]
        ParentColor = False
        ParentFont = False
        Layout = tlCenter
        OnClick = label_urlClick
      end
      object lbRules: TListBox
        Left = 0
        Top = 17
        Width = 220
        Height = 115
        TabStop = False
        Style = lbOwnerDrawFixed
        Align = alClient
        BorderStyle = bsNone
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ItemHeight = 19
        ParentFont = False
        TabOrder = 0
        OnDrawItem = lbRulesDrawItem
        OnExit = lbRulesExit
      end
      object hcRules: THeaderControl
        Left = 0
        Top = 0
        Width = 220
        Height = 17
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Arial'
        Font.Style = []
        Images = ilMain
        Sections = <
          item
            AllowClick = False
            ImageIndex = -1
            MinWidth = 30
            Text = 'Rule'
            Width = 120
          end
          item
            AllowClick = False
            ImageIndex = -1
            MinWidth = 30
            Text = 'Value'
            Width = 100
          end>
        Style = hsFlat
        OnSectionResize = hcRulesSectionResize
        ParentFont = False
      end
    end
  end
  object pnMain: TPanel
    Left = 0
    Top = 28
    Width = 588
    Height = 345
    Align = alClient
    BevelOuter = bvNone
    Color = 4933703
    TabOrder = 6
    object hcServers: THeaderControl
      Left = 0
      Top = 0
      Width = 598
      Height = 17
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBtnFace
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      Images = VirtualImageList1
      Sections = <
        item
          AllowClick = False
          ImageIndex = 21
          ImageName = 'THeaderSection'
          MaxWidth = 28
          MinWidth = 28
          Width = 28
        end
        item
          ImageIndex = -1
          MinWidth = 30
          Text = 'HostName'
          Width = 250
        end
        item
          ImageIndex = -1
          MinWidth = 30
          Text = 'Players'
          Width = 60
        end
        item
          ImageIndex = -1
          MinWidth = 30
          Text = 'Ping'
          Width = 40
        end
        item
          ImageIndex = -1
          MinWidth = 30
          Text = 'Mode'
          Width = 120
        end
        item
          ImageIndex = -1
          MinWidth = 30
          Text = 'Language'
          Width = 90
        end>
      OnDrawSection = hcServersDrawSection
      OnSectionClick = hcServersSectionClick
      OnSectionResize = hcServersSectionResize
      ParentFont = False
    end
    object lbServers: TListBox
      Left = 0
      Top = 17
      Width = 598
      Height = 346
      Style = lbOwnerDrawFixed
      Align = alClient
      BorderStyle = bsNone
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ItemHeight = 19
      ParentFont = False
      PopupMenu = pmServers
      TabOrder = 0
      OnClick = lbServersClick
      OnContextPopup = lbServersContextPopup
      OnDblClick = ServerPropertiesClick
      OnDrawItem = lbServersDrawItem
    end
  end
  object mmMain: TMainMenu
    BiDiMode = bdLeftToRight
    Images = VirtualImageList1
    ParentBiDiMode = False
    Left = 48
    Top = 61
    object miFile: TMenuItem
      Caption = 'File'
      object miImportFavoritesList: TMenuItem
        Caption = 'Import Favorites List'
        ImageIndex = 7
        ImageName = 'miImportFavoritesList'
        OnClick = ImportFavoritesClick
      end
      object miExportFavoritesList: TMenuItem
        Caption = 'Export Favorites List'
        ImageIndex = 6
        ImageName = 'miExportFavoritesList'
        OnClick = ExportFavoritesClick
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object miExit: TMenuItem
        Caption = 'Exit'
        ImageIndex = 5
        ImageName = 'miExit'
        OnClick = ExitClick
      end
    end
    object miView: TMenuItem
      Caption = 'View'
      OnClick = miViewClick
      object miFilterServerInfo: TMenuItem
        Caption = 'Filter / Server Info'
        OnClick = ToggleFilterServerInfo
      end
      object N10: TMenuItem
        Caption = '-'
      end
      object miStatusBar: TMenuItem
        Caption = 'Status Bar'
        OnClick = ToggleStatusBar
      end
    end
    object miServers: TMenuItem
      Caption = 'Servers'
      object miConnect: TMenuItem
        Caption = 'Connect'
        ImageIndex = 13
        ImageName = 'tbConnect'
        OnClick = ConnectClick
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object miAddServer: TMenuItem
        Caption = 'Add Server'
        ImageIndex = 12
        ImageName = 'tbAddServer'
        OnClick = AddServerClick
      end
      object miDeleteServer: TMenuItem
        Caption = 'Delete Server'
        ImageIndex = 15
        ImageName = 'tbDeleteServer'
        OnClick = DeleteServerClick
      end
      object miRefreshServer: TMenuItem
        Caption = 'Refresh Server'
        ImageIndex = 18
        ImageName = 'tbRefreshServer'
        OnClick = RefreshServerClick
      end
      object N3: TMenuItem
        Caption = '-'
      end
      object miMasterServerUpdate: TMenuItem
        Caption = 'Master Server Update'
        ImageIndex = 17
        ImageName = 'tbMasterServerUpdate'
        OnClick = MasterServerUpdateClick
      end
      object N4: TMenuItem
        Caption = '-'
      end
      object miCopyServerInfo: TMenuItem
        Caption = 'Copy Server Info'
        ImageIndex = 14
        ImageName = 'tbCopyServerInfo'
        OnClick = CopyServerInfoClick
      end
      object miServerProperties: TMenuItem
        Caption = 'Server Properties'
        ImageIndex = 19
        ImageName = 'tbServerProperties'
        OnClick = ServerPropertiesClick
      end
    end
    object miTools: TMenuItem
      Caption = 'Tools'
      object RemoteConsole: TMenuItem
        Caption = 'Remote Console'
        ImageIndex = 9
        ImageName = 'RemoteConsole'
        OnClick = RemoteConsoleClick
      end
      object miSettings: TMenuItem
        Caption = 'Settings'
        ImageIndex = 20
        ImageName = 'tbSettings'
        OnClick = SettingsClick
      end
    end
    object miHelp: TMenuItem
      Caption = 'Help'
      object miHelpTopics: TMenuItem
        Caption = 'Help Topics'
        ImageIndex = 16
        ImageName = 'tbHelp'
        OnClick = HelpTopicsClick
      end
      object N6: TMenuItem
        Caption = '-'
      end
      object miSamp: TMenuItem
        Caption = 'SA-MP.com'
        ImageIndex = 8
        ImageName = 'miSamp'
        OnClick = miSampClick
      end
      object N11: TMenuItem
        Caption = '-'
      end
      object miAbout: TMenuItem
        Caption = 'About'
        ImageIndex = 11
        ImageName = 'tbAbout'
        OnClick = AboutClick
      end
    end
  end
  object pmServers: TPopupMenu
    Images = ilMain
    Left = 80
    Top = 61
    object piConnect: TMenuItem
      Caption = 'Connect'
      ImageIndex = 6
      OnClick = ConnectClick
    end
    object N7: TMenuItem
      Caption = '-'
    end
    object AddtoFavorites1: TMenuItem
      Caption = 'Add to Favorites'
      ImageIndex = 8
      OnClick = AddServerClick
    end
    object piDeleteServer: TMenuItem
      Caption = 'Delete Server'
      ImageIndex = 9
      OnClick = DeleteServerClick
    end
    object piRefreshServer: TMenuItem
      Caption = 'Refresh Server'
      ImageIndex = 7
      OnClick = RefreshServerClick
    end
    object N9: TMenuItem
      Caption = '-'
    end
    object piCopyServerInfo: TMenuItem
      Caption = 'Copy Server Info'
      ImageIndex = 1
      OnClick = CopyServerInfoClick
    end
    object piServerProperties: TMenuItem
      Caption = 'Server Properties'
      ImageIndex = 10
      OnClick = ServerPropertiesClick
    end
  end
  object tmSIPingUpdate: TTimer
    Interval = 1500
    OnTimer = tmSIPingUpdateTimer
    Left = 176
    Top = 61
  end
  object pmCopy: TPopupMenu
    OnPopup = pmCopyPopup
    Left = 112
    Top = 61
    object piCopy: TMenuItem
      Caption = 'Copy'
      OnClick = piCopyClick
    end
  end
  object tmrQueryQueueProcess: TTimer
    Enabled = False
    Interval = 10
    OnTimer = tmrQueryQueueProcessTimer
    Left = 208
    Top = 61
  end
  object tmrServerListUpdate: TTimer
    Enabled = False
    Interval = 50
    OnTimer = tmServerListUpdate
    Left = 240
    Top = 61
  end
  object ImageCollection1: TImageCollection
    Images = <
      item
        Name = 'imDownArrow'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000036000000480806000000BD6D06
              6C000000E8494441546843EDD9B10D02310C46E1DC2AB008EC03D3C03EB008AC
              02CDFD546759318EA2E4DE575162D92FCD2DA5944F99D0C260836163A3D9DFC6
              0E97C7104B7ADFCF9BFFD3DC188375C6C68453ECACDB29BE6EA7CDD18FD7E7EF
              F73F184CB21A6363419CA2708A0E1A0BA231A131078D05D198D09883C682684C
              68CC416341342634E6A0B1201A131A73D058108DC9EE1AB39A69CDFA3A93768A
              0C968C8DAD384599B6314BD6C0564B96B457D1C2600E36B6E21485C61CD6A352
              DB92A5F9295A182C888D49EDE361E11483BA9D626B0C26598DB5C6C6848D7556
              7D8AA363B0D1B0B1D14CBBB12F537C8C10C0C519F90000001064654247343141
              30343039393345414239373644F4D008980000000049454E44AE426082}
          end>
      end
      item
        Name = 'imLogo'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D49484452000000E4000000680806000000E1255C
              2E000008EA49444154785EED9D4D8B5C45148627869190A04402216EB23012A2
              C2809B2CDCE517E4AF883F44F2575C096EB25370391B257EE032440625213064
              6823359183F5F44C9FBC54F5BDB7A7DE6775B4EBAD5BB7E63C335DDE9EF1D29E
              5934AFBFDA7BFDFF055EFA7AEF52FCC319703C91F3BF46F5868FA33A459ECF6C
              64E3669AF96143B70A20E72DE4A46CFCE298F9A120B25040CE5BC849D9F8C531
              F3434164A1809CB79093B2F18B63A68742C8020139DF5B40CE676AB8BF519945
              C086960500729E02B1615AE73335DCDFA8CC226043CB0200394F81D830ADF399
              1AEE6F546611B0A16501809CA7406C98D6F94C0DF7372A330B6C60B9E1819CA7
              306C90D6F966E6E8DEC38D2B58AD5E457D16377FF936EAADC0FD8ECACC021B5A
              1600C8790AC406699D6F662CA4916043CB0200394F812C64857F420E06059185
              0272DE42C6569C8585BCE05008592020E70713F0DAD555DC7A61FFCA7ED485CB
              7B97A32E1C1F1F475D78FEA2CE771794EF48A23293404164A1809CB790B11505
              0B39381444160AC8790B195B51B09083434164A1809CB790B115050B39181442
              1608C8F9E105BC125B51383AAECF847F3C791175E1FEC17B51178E8F4FA22E74
              3F53FA0C392D1444160AC8790B195B51B09083434164A1809CB790B115050B39
              381444160AC8790B195B51B0908341216481809CB780B115050AF8E7D37FA22E
              DCB9F54ED485C3DF8EA22EDCFFEC66D485EECF297D86DC2E1444160AC8790B19
              5B51B09083434164A1809CB790B115050B39381444160AC8790B195B51B09083
              41216481809C1F5CC02B78CEF82C3933BE7CF957D4856B373E88BAF0F2D9CBA8
              CFA2FB734A9F21FB424164A1809CB790B115050B39381444160AC8790B195B51
              B09083434164A1809CB790B115050B391814421608C8F9C105DC4F3E9BBA7666
              7C5E9F090FEED66740FC3AE4DEE193FA39E4DEEADD28CFE2E06EBD1E929E297D
              866C8382C84201396F21632B0A16727028882C1490F31632B6A26021078782C8
              4201396F21632B0A16723028842C1090F3175C4072F469FD7754AF5FAF3F6B7A
              547F9474FDCC88E78C6B67C0CBF51FB53AB87323EAC2217E3F92E3B3F9EEDDAE
              BF611C1FD787549F211BA120B25040CE5BC8D88A82851C1C0A220B05E4BC858C
              AD2858C8C1A120B25040CE5BC8D88A82851C0C0A210B04E4FC6002129E216F5C
              AFFF28D58F877F477D4A76C64B38F8A47E2E79F813CE9004D77B7E525FEFF3DB
              519EB25AD5EBF719528482C84201396F21632B0A16727028882C1490F31632B6
              A26021078782C84201396F21632B0A16723028842C1090F3830B48F8BF8FE3FF
              9B83BFFF289F29B3E790F89B3ACC3FAF1F7BAE9D19AF5DADD7E7CFB28A501059
              2820E72D6485851C1C0A220B05E4BC85ACB09083434164A1809CB79015167230
              28842C1090F31650A2B7A03C33F2F72157ABFA5FFCF0733D5FF399918CFEFB90
              1444160AC8790B2961212F381444160AC8790B2961212F381444160AC8790B29
              61212F1814421608C8790BD8956641019F2BBEBF5F9F39EF7C583F876C3E3392
              D1CE901444160AC8790BD9150BB9E35010592820E72D64572CE48E434164A180
              9CB7905DB1903B068590050272DE024E4A2E287F9F72F3DFCCD9FA99915CF433
              240591850272DE424E8A855C381444160AC8790B39291672E15010592820E72D
              E4A458C88541216481809CB7808B2213F4D5497D663C39A91F44767FCE9871D1
              CE901444160AC8790BB9282CE4CC5010592820E72DE4A2B09033434164A1809C
              B7908BC2424E0C8590050272DE02EE141474B5AACF9097F7EBCFAAAE70C6EC7E
              6624BB7E86A420B25040CE5BC89DC2426E190A220B05E4BC85DC292CE496A120
              B25040CE5BC89DC242768642C80201396F01CD36D9B533240591850272DE429A
              6D62212DA4591016D2429A05B17421F996517E8B09E47CE7B7A88FBE8BF2946F
              9E46D98587B7A23C936D5FEFCB8FA27C2B1EFD1EE529BDD797C1F593DEEBE1F5
              D6F68BFD15D5426043AB0210396F213792365882858CAD788385ACB1901A1652
              23DD2F0B59632135D2064BF04FC8D88A372C4D480A21BFC50472BEF35B54CEF7
              E0FB284F79FCF871D43D78F0E041D467B1EDEB51509EC1B2D77BAF2F83EB27BD
              D7C3EB3DFE22CA37B0DFA29A0936B42C0090F31652820D960997BDDE5B800CAE
              9FF45E0FAF672181856C830D960997BDDE5B800CAE9FF45E0FAF672181856C83
              0D960997BDDE5B800CAE9FF45E0FAFB738212984FC1613C8F92DBF4525D91992
              5F3015B5E1D5EBA9F9D1C693ECFE2D64269085DC48D66064B4F124BB7F0B6921
              A3190A6C908CACC1C868E34976FF16D242463314D82019598391D1C693ECFE67
              179242C8673420E7277E8B4AB233642BFC82936D5F2F3BC366AF737D9C9F4C3D
              BE15DE8F85B490D10C3D608365C265AF4F2D18D74FB89E56783D0B6921A3197A
              C006CB84CB5EA7009C9F4C3DBE15DE8F85B490D10C3D608365C265AF4F2D18D7
              4FB89E5678BDC985E4194E3EF301393FB380A4F787A9D930FC8213753C51F3A3
              8DCFE037A4B50FE3F3B15B549DA020B25040CE5BC80A5510A2E6471B9F61212D
              64852A0851F3A38DCFB09016B2421584A8F9D1C6674C2E24DF32CA6F31819C5F
              98806469CF212914FF2343B65EE6D9703C2367AF67F393B9C773BF9AE97D86A4
              20B25040CE5BC8D88AB3C81A980D6621B5FD6AC642D6A482379235782B6C18C2
              EB713C1B2C5B2FF3D94FC0ECF56C7E32F778EE573316B2C642C6569C6221377F
              83589C906C60F92D2690F3FC09C6EF30ADF375A6F77348C29F4084D7E3783E17
              CBD69BE533D4F9C9DCE3D5FB4D61FF46F596B0A16501809CA740BCA1D6F93A93
              35602BAD0DCC06CBD69BE533D4F9C9DCE3D5FB4D61FF46F596B0A16501809CA7
              40BCA1D6F93A9335602BAD0DCC06CBD69BE533D4F9C9DCE3D5FB4D61FF46F596
              B0A16501809CA740BCA1D6F93A9335602BAD0DCC06CBD69BE533D4F9C9DCE3D5
              FB4D61FF46750E6C60B9E1819CA730BC81D6F98C9913F67354E7C08696050072
              9E02F1065AE733664ED8CF519D031B5A1600C8790AC41B689DCF9839613F4775
              0E6C68590020E729106FA0753E63E684FD1CD57FB081B386276A9EE3D784E182
              D5F98CD921D69A9B0D9D0940D43CC75B4833326BB250904C28A2E639DE429A91
              5993858264421135CFF116D28CCCBFD1033E8EB61A1493000000106465424743
              3130313243463546434335423031428B3B41A10000000049454E44AE426082}
          end>
      end
      item
        Name = 'imPadlock'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000034000000380806000000B2BADB
              9000000321494441546843ED9A5F48536118C63F5D6EB999AB2D832273546456
              101141049941660A9617251AD445687FAE328CBA8E2EEC26220BBAE826EFAC41
              12919546358AF23A84D22485CA8865B9696D733B339E73F1C47B0689882793EF
              77F51B836DEFDEF7D939DFF72D4B2935A9E61159BAA039CEFCEFD0DAF6C2BFF6
              C0617E077F3861F8E96087771D1D38D5041DB8E2113AE8FED64F0737BCF2F50D
              F9308381BA8F7490D1215D90EED03F1E39B7E52AD56878E9A03E504C072ED74A
              3A184BE5D281C72933A546DF5041DBFB777470335F8628964D359976867441BA
              43368FDC066321C70F5C0EECA58325C6281DB4F43EA683C11C99812263011D34
              6F3B4C07A9AF1FE8A0E9CB6B3AE8F5E4D0C1B433A40BD21DB279E44A26E40FFF
              D9B88F0E5289281DB478137430E494195A93A49A5C0BD4D0812B29AF53A707EF
              D1C18C33A40BD21D9AE5915B6F19B91596EB46B6E5DECE312943117738E920A1
              0C3A284ACAE72FE66FA18378B67CBDEBE11E3A789A273F8F2E487768AE8D5C73
              5B80F3091AF24AE82055B09A0EDCE91F7460E42EA5039F536668ECBB830E9223
              293A882D9319F24486E8A035FC960E5A1B3ED141C68F822E4877C8E691EB0AEE
              E3F881F2DDB57413BFDC33506999919E57E37470A7FD3E1DD41EAAA483ED5B5D
              7493B49B6A1291EBAB50E76D3A283B394C071919D205E90ED93C72A10E99A1D2
              9A26BA49546E142A25D74B5507AFD2C1C3EE97745059B1930E3A8367E8263F65
              2655DA9AA14E3A286B9C2243BA20DD219B47EE51C77E8E1FA828ADA39B24E585
              5159D63B232332637B6A2FD1C193E0053AF0FB3D7493C473AA497A900A425DF2
              5EAEECB87C3E2343BA20DD219B47EE4A9B5CEF1C5DB5990E7C25D57413A73C63
              FD158DD1C1AE03E7E92074F7181DB83D61BAC984CC44B84FEEF3DD1A90E747E7
              4E4DB11ED205E90ED93C72C5967DB92371B90F56EDDB4407CB0B37D241814FCE
              FC8367F27CA7AA549ED1468665E63E0FC93D8AE0B87CDC6EF91F435FFD141B8D
              BA20DD219B47CE7A9C922B065229B792F772353179065B9E9467AECEC58BE820
              1E4DD3C18B94DC9C0FE6C9371C77C8CCC467FA3F055D90EED02C8FDCFF8E2E68
              AE33EF3AF41BBBF7702E71F0756C000000106465424738374345373741424435
              4445413936436C116E540000000049454E44AE426082}
          end>
      end
      item
        Name = 'imPadlocked'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000034000000380806000000B2BADB
              900000032D494441546843ED994B4894611486FF191DA711D3F182D2A4108856
              88922D229572A008372DC492025705612511144245BB56152D2A5AB8AA305024
              C245966D2AB3A92C5249233049D006CDD49CC9CB8C7389F75BBC7066BA3020A3
              C9F7AC1E19FFC5F1BCC7EFFBCF980CC3081B6B08932E6895B3F63BD492B7FBEF
              3D48A429526B027460DB99430769C139BAC22F7F9EE89EA4034FA77C3E1CC4DF
              FCCF1C1AEDA283A80EE9827487563872665B88710429554B74505AE3A083D48C
              223AF08753E8C0625EA4039FFB391DBC6919A303EF8B6C3A082F26D041CC33A4
              0BD21D8A73E4123607193FB0FDDC263AC8592767ACEBB68B0E7CB3F2735B9A3C
              572AEBF7D081DB3D4107BD97BFD24160248D0E629E215D90EE50BC23972BEF6A
              65B7AAE86061EC231DBC3F3D4A0761BF3C370CAB7CFD2AB9574E07F6790F1DB8
              1ADFD241603C990E629F215D90EE10D300963D72AD8532721975D374602D9077
              ABA5803CA70243F2F7CD11EF331ECB063A284897CF7FCF9633EBFBFC830EBC77
              E5DDB076A89B0E7441BA432B1DB98737F7328FA0B23C9F0EC2B91574600AF8E9
              C09C28970EFE1939E4964439D416DF3C1DF892E5E7A629F94FE15D47271D382F
              8ED041D40CE9827487E21CB9DE1679D06D3B789EAE08CA73C308CA73E3D2B54F
              7470E5EA0D3A683CDB4007174E15D2150B36AA22D04105AFDADBE9A0FCB89B0E
              A2664817A43B14E7C8B95AF7317EA0ACF60C5D312DEF6A4692950A8ED55FA783
              E6FB2FE9A0AE46CE685353235DE199A22A424FA8C0F5E0191D5434C83D5ED40C
              E9827487E21CB9A76D7267E07456D315C14CAA22B8405564A5524151F1493A18
              1CB843574CCA19883C778C905CEEBF7E3448076527BED041D40CE9827487E21C
              B9C775A58C1F283E9C4E078ED20374857523551171D7CBDE7A840EBEF51DA52B
              CCF23B5663499E73A3FDB374D0DF3C4307FBDB06E8206A867441BA43718E5CE4
              2AD8BECB4B07F6EA2C3AC8CBDB42070E878F0E7AFAE4EE7B47C97A3A981CFE49
              0723C3E37430DE29EF8AF33D1974F0CF45A32E48776885231789395D24D4B039
              E55E2D335FBEE327A7C81DC1DC827CDEFB41CE88A7DB4E07214F12FD77C43C43
              91E8827487963972FF3BBAA0D5CE9AEBD02F05B1902EDDCAD7FB000000106465
              4247414434353839394332303230353537310065BA320000000049454E44AE42
              6082}
          end>
      end
      item
        Name = 'imUpArrow'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D49484452000000240000003008060000007947D8
              150000009C494441545847EDD6410E45301485E1762B6C84FDB01AF6C346D80A
              3169A206BFA60D979C6FF466EFE4EF1DF0CEB9CD19E23508D82F54755358FB84
              756C4F7F7329A441BF2FB40C4D78EE43DDCFE1F71DC50B691079BD503C80D04D
              65DF9006992F94FA4424BEA9E41BD2A0CF1522F193C63742926F8868105121A2
              424485880A1115222A445488A81031572857F16FEA5C1A44B0D0DB3488982BB4
              036E4208104FFADF6C0000001064654247413834423635393746303545363731
              37600AD9F80000000049454E44AE426082}
          end>
      end
      item
        Name = 'miExit'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000024E49444154785EED9BAD73E25014C553475CE34046C27FB02B63EBB6
              8E4A6CE5CA4A6C655D6371482A2317B7389091915D97B8766EC499DE9B0CAFD9
              7D744BDFF9A993498099CB39F77D24B9883CB398AF5F70D0C3FEB08416B6BBDD
              050EFE03DE7F9C05A003BE7804EC3F3C9E8DA085389A40F7B1592FA085D9F40E
              FA3DD4D1015A88A329741FF9EAFA68CC8F9EEC8305A003BE7804EC3F6CF97695
              410B650DD992C6902DF67CF1A03F7F9BFF8216C6C75B4887B268A085EDA68016
              06F70016800E6004F438FF3D856E19E9639BF9A82E218532D2D7FF6B0F882B9D
              F9BD9E1644C5E6015A58ADEE3A317F4BE7A48D000B4007841E814C67B6937983
              1DF793B28216D64F37D0C26DFE1B5A184F74C65D9C7C1EC002D001A147C0D104
              3B347A1E90547ABF60680FB0E37E3DD1DF77FA1EC002D001B05B4B7011304D70
              281F3D0FF0BF166001E800B8E16F38FF08384601BB3658FCD0E3F4FD52CF0B5C
              F3807DF507FA3DA4E672EF7B822C001D107A04CC28609BDAE54C6FE25D65BA07
              E4B9DEB4B3F706B78B1D742F8ED38FD933B4E0BF07B0007400DC20041781697A
              FCFEBB8B26D6EBF9C13DC0C163A27B80F7B5000B4007041E817AA22742AE6780
              A6239DC98FEE01DEE7012C001DC008200E2D26E3713383EEA31EEDA185F2E927
              B4E07B1E70F21EC002D001A14520BD44BE5A9A04B21753207B7D59E8B54076AD
              DF1F486ADD73EC7E830BEF6B0116800E082C02F3F95245C03E8F3FD41171A5EF
              0BD877863EDDFB022C001D1078045CB81CE2C2F680B37B6F9005A003028FC050
              AC63CAC31A5A38BB1E301416800EF8DC117805C850906EEFFA73170000001064
              65424743363432353637463430343936413043A04413BE0000000049454E44AE
              426082}
          end>
      end
      item
        Name = 'miExportFavoritesList'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000021649444154785EED9BA16FC24018C58BABA48EC9EE3F0037249239E6
              6627874362378903894416C724B212DCEA56090E64DD966BC897DCA3E9E55620
              DDEEFDD48392403FDE7B57D2A3E1394EC3F1F3E700E8008C40FC197DCB03CFF3
              56592CBA886D9A8ABE05876C5BFA36E94164CE4B77205AF1DE9968E7CC01C828
              4ED001AE4760BC19691D101EBAA2156950DE09B65CBA43B023B6F14EB4229B65
              E51DC001D0018C80D60155C18C47BD8568C576F7215AF196E8C74D60E6912354
              56324BED3AA02A1C001DF0C722D08F065A04DA6128BA08FC8611CCBCDFF44517
              F1B87C12AD3065DC847507700074002350A903567D58C77DBBCC9BD8ECEC3A21
              4844E65CBD0338003AE09F45C044D48B442BFCA6C8426C338FDCBC034C700074
              806311580DF413AE1B77B37BED2355BE0E4038003A80119038D411EC00643F74
              AC03100E800E3044E061116ACB60E0B7451761BB2A54FD2D806CB2B5E822F687
              A3E89CA17ECE1C808CE2041DE07A045A53BD033A77E51D80DCBA132EDE011C00
              1DC008681D100622734CD705C8B53B013B0033EF27FA7D09E31E218C00074007
              8819729C8B00828EB0257EFE12AD98AF97A215CBA3DD1EA1B8A9EF313A2699E8
              1CBC9538B7BC0E4038003A40CCF02B6A1F016FEA69116879E5FB034C8EC0FDFB
              F87A3CEE05F84439B8EE9F61DB011C001DE0780470ABEC643E135D47B20CD67D
              60FC3A12AD30FE678803A0031C8F806B9C75806B7000AE7DE3C80F3D83BC5F85
              F3610B000000106465424733443341373838323344384431413231248A5ED200
              00000049454E44AE426082}
          end>
      end
      item
        Name = 'miImportFavoritesList'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000021B49444154785EED9B2154C2501885479B710DE26C4668128D446C1A
              89D288586DD25824422352898BD2A4B9280DE26B7ADE0EE73FE75D76369E6308
              E7DD2F5D9DE778F6EFDEBB5F7DD63CC7A9397EFF1C001D8011883FE73FF281E7
              790B158BCE629524A2CFC156AD72BF4DB21599D26B77456BDE5A23E39E390019
              C51E3AC0F5080C3F06460784DBB6684D12E477822DA7EE10EC8855FC2D5AA322
              95DF011C001DC008181D5016CCF8EBDDB3684DB3D111AD795C9AD78BC0CC233B
              A8AC7594D875405938003AE0CA22D099778D0834C3507416F8848B5874A6A253
              7C5F6416EDD9ADE8BF60DD011C001DC00854DA01C8A23B177D0C8DC8AE1382B5
              C8947FEF008403A0032E3C025583BB7CFCF425FA18704FC0DF0996EE80AAE100
              E8802B8B80ED2E7F6E704F38790770007480EB11B05C5CCE0D7600B2E997ED00
              0E800E10375C22A523703F0D8D08047E53741594DD041BB34074169BED4E744A
              DFBC670E4046B1870E703D02F5B1D901AD46B51D60FB5A0D9637A235FE26FF67
              11EB0EE000E80046C0E880105EB365F702DBCCE37B5ED595680D760066DE5F9B
              D70BCF08610438003A40CC90E25C041074842DBDFABB684DEFC13CCB8B9947B0
              0390DD1AAEE311A289E51E807000F90FA8103AE0D223E08D3D2302752FFF7C40
              9123F06F75F8F578DD0BF013F9E07BFF00DB0EE000E800C723804765479348F4
              25A254FE5E307C1988D614FECF10074007381E01D738E800D7E0005C7BE2C82F
              E1EF8C5FDA4C9B95000000106465424730324135383834344435373532443746
              3D1CC6920000000049454E44AE426082}
          end>
      end
      item
        Name = 'miSamp'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000027E49444154785EED9A5DAEE2300C46C39ACC9ADC35C56BAAD7C42817
              148D8F115135BCCC8DBFA73429A1323EFE09BDB52FAB9FFD312F5A6B32474FF9
              1C3D6569265E2B7638EE769B175FD057371B2A039407FC720456BF70139DC3A1
              C38F391EE2FDEE8C01516E715DDA39C743DA19236E97B0BE74F35019A03C6033
              04F88B2B18671E3730DF6C8E9E5A309FA4E1EB539DC0FD45E284D9E7BAE1E3E2
              5019A03C603304CEF30CD01921432677C73A1127F33DE6F15C17CCE17B5D5C5F
              C5841403CA00E501BB23F0788418C05ADD0C799E10B37607F3943B3610404CE6
              C938261C318BBD04B1CF31A00C501EB037021D4130410E468FFB7D8E8704BD82
              68BC26912BE49378BEA0AC4BB043EA15E2FD29069401CA0336436099F6E6E8BD
              5806642D185D09BD06424C12B74FBD0A2ECB00E501BB23A05D420C601E676636
              42D6FA1C0D21CD262657EB9C10456D8FDE8131C85949F012BD4119A03C607704
              56A7BE648A79DC986711132E8B4101677AACE5DD1023B02ED88F8F5B06280FD8
              1D014D060063648879FC88FDB923334BCAD4A833B8CC3A00310011224FE080C1
              8D0F1CD7CB00E501BB2330617849CFF8873C7B03E6FD8C30A0E4B9BCE07F026C
              C0334293CFEF185129241C718667946580698A97CA03764740D01C113AE6F5C4
              1CEB86450C20D3693FF6220D75C7C5DA9FD82704CA00E5019B23D01104F31920
              15D773BF3E872FC53C2CA937C0F7E1F37C1A414CE0F32AF2FEF21DA1324079C0
              E60850DAE37981F35DE0C43C6282837930B9D23204A51834873F323B12E67FEB
              E3E25019A03C60730428ED089207CE07D8EF7F59EEF19DA4559E5FE9D2CD4365
              80F280CD1158491506C27F7757A5780F9027085799A7FEE9C3EF5406280FF8BF
              10F803D07CCDE6E0C7D139000000106465424739313636373732443142463731
              4537300273E2DE0000000049454E44AE426082}
          end>
      end
      item
        Name = 'RemoteConsole'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE000001A349444154785EED9A215384501485DD46C4B891884D486E86F26CD8
              AC569B8160B5F0038C468D364D986DD8A4B971E3DAA4E9BC0D77BC779C659987
              33BCBDE74B27C00CEFBC73EE00F36607CA99295F3F0C4002CEAF5FBE7FD7A0FD
              3C24BD135D4372439090DC897FBE3F0ED7A42D97A77C7D300009D05E81E0EA83
              CD80309E939E02C95130E8318AF707D296DBD72FD2161375A42D300009D05E01
              3904DFDA3BD27F310F788756DDF68EBA5E1F7403DF4B04EB2E226D8942921B60
              0012A0BD02A6B86033E0F971FB0CF08D302ED8239B63FEAD000390005440D70C
              3833CA67000C4002065620CB32D296B22C495BF23C273D45E47BC0E004C00024
              40790524BE25C2790648608067954002C6AE8064EA8990068CFE3F00064C7C26
              2001635740EE785555A42D699A929E02D200E72108039000651570DDF1BEFB93
              8477B28FA6E16782E4FF89BAAE495B9C6740DF02600012B0E715F01DE719E03B
              300009E8A9C089B961156897E2ECADEF040BB602138B7382300009505E81FBA7
              86CD80D5B225BD8F2C92982D0B062001DA2B4065500A0C50BAF1040C202B9402
              03946E3CA1DE801F36FBCD28EC67C85D00000010646542473244444439343033
              45334643334638413ABCF1100000000049454E44AE426082}
          end>
      end
      item
        Name = 'sbBrowse'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000027649444154785EED9A2F731B410CC52F6CCDCEAC86860D6B590253D6
              B0C014A61FA1B0B0B41FA1D0B066310C4C58C272F0A0CD72CCCBD2D176E64DF5
              948972E3C934E3D50F3D8FCEFF747A5AEDDA074DE51C54FEFD230151019E05EE
              17EF1EF1E00986ED145AD80E0FD0C2A76F1BF73DFE27EE878B044405546601BE
              E3EF8F7E4017E687904FD2DF430ADDCD776861BD862C4CDAE77B08C7DB898E1F
              7ED9ADC798274702A202C202BA079C2FA10BF916B29012E45F3E421532C5F30A
              52E070A21E90538616FAD5025A78FD1E1009880A40B915EAB300CD01597BD2C0
              71EA1179E8A185D4E87877F50B5AE0B96136837C11BC5739FEDAA9A5DF9F0322
              015101A886029738C3F17DB780E7E9DCE884E4AD7EBCBEF90D2DCC8F7E420B69
              AA5FDFA78512BA95BE813C37ECDC03220151019559206F3A94BFD05FE9D99CD7
              5D6F7FFFE1449F173433BDF0276EAA4CDE420ADC73EE2EF5DE63E73920121015
              509B05CEB447BBA5B604AFDB8DB6B821B5DAB3CD3040BE849CA947F0DC717B07
              2D4C4E57C6E6FF6282A6074402A202500D427D1638B9C0971778BFCECBA41995
              695D66CFF2F52E99CE0CE9F5AF177A2EF17E9B3441D30322015101A806A13A0B
              30BCEEA799F6E4684F7B389E6F7ABD199B1C2F8DAD9FC35CCC3D808904440554
              668179AB7FEB4BB42AE4860E44C6B600CFE30F34370CB419EBAFA1053EF3F330
              17730F8804440584056007217D3E832EF0BA3FE8FD3A7B9AF7021CE7759D3DCE
              678E7CC63716F364B7074402A202500D857DB780F99FE0740329E44EFF67C8F3
              ACF73BC1AE9E1E8B7933EE019180A880CA2DE03176F67E6B980FCF3DC0231210
              15B06716A80DD3036A231250DB1D6722014845A544022ABDF1E00F1500486E15
              244110000000106465424737333846424435333845393632353642AA87AE7D00
              00000049454E44AE426082}
          end>
      end
      item
        Name = 'tbAbout'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000022549444154785EED9A2173C2401484C1A50E6424B21264253FA14824
              75D415D7CAD6B512D7C848EA82ACC50559472475C181A373B4F366DE92C9351C
              332DDC7E6AE14880776F975C8E7ACD73EA9E7F7F16801D706C0BE4EBED561E14
              908B2A265B89DC912573D186F12C156D9847374EDFC1E9E022580076806716B0
              CD38D2BC78125DC4627D2FDAB0DA88DCD10844EE788FDD32A274F037B000EC80
              33B7806D8637E0D13DE005EDEBA96843D86989364C9EDBA28BC04C0803FD4412
              7F8836D832C19A012C003BC0330BB8CEF81A7EA75D41CFE375008EB7607C62B9
              4ED8CB0016801DE099052ACF383CB1465302BDDB48B4214BF5827F9589DCB158
              DE893E04CC0464D8D76B1116801D400BE80C70F53C7AB0DB7D115D44D50CC0F3
              E3C7C171C49A012C0056901D70E61688D385B2C0654BAFCF1BD8110D5107D1B9
              D29950350310F47C0E8F715F217AD3F72358007680EF16680F5E55068C865DD1
              866367826B06B87A1E6101D801BE5B40CCF083AD20086604321895DF0F08427D
              7C08E78B1EF53E410E176659A2F7016C9E9F4D1FCAF70558007680E71640B023
              82E5A768C338D6FBF9B83E47F03F40B59AF67433D0274827E5F7F5F1F320E879
              A474D0C002B0033CB7808D65AE6FA92D457D831EB6FD4E57C5E6711B4E071B58
              0076002DA03220A9E879570FBBE2FCE6680116801DE0990516B0B5D6EF95FF17
              F8AF3D8F3867000BC00EF0DC02B6D5E27FF33CE29C012C003BC0730B9C3ACE19
              70EAB000A73E83AE7C01D3C7498A6CB1B66B0000001064654247313344363434
              313234453136303230385A8D2D230000000049454E44AE426082}
          end>
      end
      item
        Name = 'tbAddServer'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000027C49444154785EED9AA153E34014C6A9DBB8D43532B238244870E53F
              A8ADBC7395AD2C0E1CC8CAE238777520397727E32E32718DEBBA32BB306FE67D
              E9F4B1138619D8F7535FE8A4B45FDFF7653793DE51E4F422FFFE6A804E004660
              57AD7674E0F8BB22E9694A929ECD86E45E1A52AFA4A4F6621B7E426332D28EAD
              F0FFCAD290769C4FCE493B7A17F7EC3BAB0164C51B3A01D147E0F78877407642
              D2636B929E724BD29342E82D64D662290042C6B1532CFCA1DAF20E5843A7FCBC
              E3B16F47400DD009883C020FA760C031490F66DEC0BA40CA388299174E97328F
              E0BAE0E2BE163A400DD009883C028B013720E76B690BD779F3C17B83D0BD40B6
              E0C7D5BC4FDA11DE016A804E40E411B80503B29CE45E0AE80061BF8F74CDBCF9
              3727ED281FEE483BC23B400DD009D008501E1CC2755B04CF07706DDF62C6F71E
              52E691EE1D809F4F0D202BDE071A08E804480E7D7604B60BC3226052E1273F9B
              90F4AC97243DC2E92D7E54243D819947823B400DD009883C02D575CE3A6060E0
              1E20667EC8337A542C487A9EA113B0F36687336FD737A41DD23D4024B803D400
              9D80C823F07F96B20EC813C81C5ED7B1134EA00320D3E2EBC23AA2B6096907DE
              2F40823B400DD009D008B00E48FAFC3E7B6B5D80D7F511EF043BE499374558E6
              11A90330F3459F3FCF203E1F801150037402228BC0E398DF12CB734BDA9125FC
              58C28CA6A43DB83790808EA94D5807FCA9F91BCC9FECE10E500374023402073B
              2094E0AB88B00E282BFE7930F3C8877740286A804EC0178F00D235125D91328F
              74EE00440DD009F8E611D8ADC62C0257CB5FA41DE930CC80E166407A1FB85F47
              9A222CF3C874CA9F734C2ED7873B400DD009883C02B1D1EA80D8500362FBC591
              170A4BC86EE92309A00000001064654247393845343545363743364234314634
              42B5E05B3A0000000049454E44AE426082}
          end>
      end
      item
        Name = 'tbConnect'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000021A49444154785EED9B2D73C24018840F17D9382AF313C015595989AC
              AD2C8E3A2A5B89031959090E5B1909AE718D6C5C9071ED5C8679676EC3E41A2E
              65CADC3E6AF99881BCECEE1D24F494E7F43C3F7E0E800EC008241FAB6FB9A194
              DA9489E863ECB24CF43928CA5DE3CB6485C88A87D158B4E67538378E99039051
              1CA0037C8FC06C3B353A202A46A23559D8DC096DE9BA43B02376C997684DB92C
              9B3B8003A0031801A3035C79899E446BE2ADD921EBFD9BE853C0CC237BA8AC74
              99B5EB005738003AE0C22270B71A1B11184491E863D8D6F1D5ED4AB426B81259
              11BFAF456B6C9D60CB3CD2BA0338003A8011E8B4033663B3036C6027C4B959A2
              087EFF47C25464C5D93B8003A0032E3C02AEB4750062EB843FEF005738003A80
              119038740176C273DABC4F70DE07B8E21A018403A00318018983C6D601483EF1
              AC03100E800EB044E0E62D3296C13018883E05D765105BBFF65D4035FF209017
              7BD11513F398390019C5013AC0F708F41766070CAFCFDB0198793C4F80E7053A
              EF000E800E60048C0E884291156DF705B67D802DF348AD03A00272659EA708D2
              40B4C67A8D10468003A003C40C15DE45004147D848EE3F456BDA661EA97500EC
              036AEB3E5E4E10B7DC07201C001D2066F815171701B5504604FAAAF9FA009B23
              709DC6E7E3E32A843B0A6C614BE691B61DC001D0019E47002F959DC74BD1FF91
              B22C1BDFD6EC712A5A63FDCF10074007781E01DFA875806F7000BE7DE2C80F00
              9F246E7B2C992000000010646542473539363730373645373542303438323793
              B36A510000000049454E44AE426082}
          end>
      end
      item
        Name = 'tbCopyServerInfo'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000022149444154785EED992F73C24010C5C1514765251FA1C87E84D65516
              491DB856461607AE48243870ADACAC04878D0C2E71E0DAD9889DB95D26D79B3B
              FE947B3FB51048989DF7DEDD1EF59A60F83EFDE11747202F9AC65306C9439D5F
              1C01F53034000A80058C0C78E93D711D82D178C63591B7EEB92E49532E8941BF
              AD6C1A127573690134000A8005D80E7F61C7D57EC622035C79ED77946D7D5037
              F3CD0034805BB11F2820760B149B0DAB8148D38C6B22CDB92C59AFCCEBA16705
              75B34367001A0005FC330BF4840532F3F72B1673BF75DE15DFF304F561990168
              0014000BB01D888539AE2BB24F33035C97511BA1CF13D4455B06A00150008B61
              2F1767816ED7B4402AF6BAF9CE7C63355F704DB866806D2B1D7A98B266001A00
              05C0026C07625954BBF4D019107A9A74CE0034000AA816E9C559A0233CFCE5B8
              11925B69DB7982A4687059D214FDBF926F1405974432FCE09A988C9EDD32000D
              800262B740E791ED40ACAB335085A0CC00DB34E9CB6DCD0C99D1D83703D00028
              80D540C46701B997DF881010EBF4746A6680DC4ADBCE135C69891FB0DB86CE00
              34000A603594C46681E5B7DBC2DDBE6B714DC80CB04D93AE5C37CC0C68E48133
              000D800222B3802FD2427219B59D2728646634C5C643206781B764C235319B25
              D519E00B1A0005C0025EE3B42B375BF30C5072FA0C4003A0009623119F051CA7
              49F5E7A0BC6E21135F18F6065C1327DF07A00150406416709D260FCDD133000D
              8002CEDB02BF3B66586E927D8540000000106465424746313730324139313033
              3146413942462B2FDD8E0000000049454E44AE426082}
          end>
      end
      item
        Name = 'tbDeleteServer'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000028B49444154785EED9A2153C34010855B7775A96B6464714890D4957F
              804582AB6C2538249595AD03471D581C481C91896B5CCE95B983D9997DC974E7
              2618B8FDD42B9D0279DDF77297A4DF8B9C7EE4C7AF06E8046004F6C57A4F2F1C
              6F6B929E2A27E9D9ED48B65291FA2621D58AADF8072A939276D4C2DFCB7343DA
              71767946DAD19F6CD831AB0164C50F3A01D147E069CA3B203D26E9B125494F5E
              93F424107A0B99B5580A809071EC140B3F286ADE015BE894EB258F7D33026A80
              4E40E411783801038E487A30F306D60552C611CCBCF07129F308AE0B269B52E8
              0035402720F208DC8FB80143BE96B6709E37D327D29E255FBB3732DD712F90CC
              3E493B8AC590741BE11DA006E8046804280F8E3423D94A061D31BE21ED302BE8
              042034F3E67D41DA913F2C49B7D1BD03D4009D001A8656FE7D04A4F338BE3FBD
              24E939E69DD0BBE519C7B5BD99C35E2330F348F70EC0035403C88A6FD0209D80
              3F1E81FAC6B0089804BFF2404EB9218D75026EE721F3E5CB86B443BA2F800477
              801AA0131079048ABB8C75C0C8C079195B1F2B427AFFAA20E981CCF7B62B920E
              5C2748D70091E00E50037402228FC0E73C611D90A561996B20651EC1BD035C63
              2CED80B4435A170477801AA013A011601D3018F2EBEEE2BA602E641ECEF38D75
              02EC1D7A63D81B2CF9BD4AEC00CCFCC7903FCF203E1F801150037402228BC0F3
              05BF24966596B4231DF0D7D2353C31F308760A5C60B1B04EC07B83D801AF25FF
              858B177BB803D4009D008DC0C10E08453C8B484027E435FF7F30F3C8AF774028
              6A804EC01F8F00D235125D91328F74EE00440DD009F8E711D8AF2F58046E578F
              A41DC938CC80F16E44BA0DDCAF23D54758E691D98C3FC33438DF1EEE00354027
              20F208C446A30362430D88ED1B47BE0022C1B46ED85F69660000001064654247
              31423346353642453345374241373741F917DA090000000049454E44AE426082}
          end>
      end
      item
        Name = 'tbHelp'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000036E49444154785EED9B2170E240148653171C3890A96B1D3890D44516
              7748EAC035EE2253475DEB1A191C75C55D24E78A2BEE22830B0E1C379BEBBC99
              FF4F870CC399BBBC4FFD61B31BFA78FFDBCD26BDB0CE24CD760739B02CCBB66D
              D186FD7E2FDA706E7BAB51BB9083BFC0D983690034032A66818F5F2978BE51AB
              8B36349BE8E173D96C36A20DD94E64CEF565EB2C1B9FDC5903A01950310BF02F
              7EE5A0E72D0B3DBFDA8ACC596722FFB0C5799DFB5B34FC6D4B640E2D13AC7582
              35E2D49A507AB2064033A0621628FEE24DD106F6F87C491F10930E7ABE5E4713
              6FA9263CBD61FBDAC2767F80DFA74D35E3D49A5068D400680654DC027C77B7A3
              89D78BD0638DB5C89C97A9C84FD0B3618CFD473D32B18D35A53349457F453C6D
              8B3664549216F39968C3643204DB176A80064033A06216381C0EE079C60FD154
              8B5522DAE05E89CC990C70F17E3F450FC74BEC3F1A3AA20DC1183D7DE761CD98
              A7D87FDCC72F108CB066C5315EFFE6E6126B80064033A0E216E0AACFB83E4DF4
              252419DFEF1F67E1A187BB6DF4706782D7E7F19D069EBFA4754192620D891E7D
              D1060D8066805A006BC007597870BF126D60CF95C19E7D1E62FF6FB490F0433A
              3FC61AC0D7E7F193B02BDA90520DF0BC916843C1021A00CD0049869CCA5920A3
              1BEA9E8F6B6FF620C39E1C5FE38381E0BB2BDA10CEF07ADEDB71CF337CBDB36B
              80064033A0E216A06D7BABE79DB616E7F6710FDB47035CAB0F025C6770FFB2F1
              DD16B647535C57FC5CE1F94130146D28D4000D8066802443CE7F6F01F94B3FD9
              EDD012AFB4161F47C73D5A86DBC63D40DE6364D8F3F616F7F8A27BAC29D71DDC
              939C47AFA20DCB552CDAA00190507CA21950750B149F0EE3B3BB21AD0B16297A
              9499F6691D3042CFF27307BEFF67CF8F5C7E8E80F3FE3AC1F18207DC03EC77FB
              A20D050B68003403AA6E818F0C6A80DDC01AE034D163FE33CEE3E1028F5B0E7A
              D6C3ED80C2BA8229EE21E278C906BFDFC3F441F457848F3ED8BE580334009A01
              D5B600B35CBE43409A0ECEE3AD3A7A78BBC51A11BF1F7FC7C76988CC69D3BDC2
              9E5E1EE63DBE6836179D93D2738C36CEFB777725EF08311A00CD808A5B80F931
              9F434DA839E8B13AD504BB86EF0932FB1D7A9A4913FC27A17086F37CAD8E35A3
              7B85C7EC79E668E357680034032A6E01E6E9E5096A42BAC27978951E5F079CCA
              AD8BFBFA651E2FE3ACCE060D8066C0BF6D81DF88DBCC45C259A6F30000001064
              654247373732393134324539433633364342453F3EBA000000000049454E44AE
              426082}
          end>
      end
      item
        Name = 'tbMasterServerUpdate'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000027C49444154785EED9A2173E250148553475C715B894406472575BB6E
              575662715DB795F41FACA4929575A923323870E03692B8C405B73B2F9D3933F7
              E60DAF69C2CE2C399F3A194898B973CF79F7BD70E529E238FB830B430FEA4314
              59066DF0FD3E74897ABEFE7EB4DE431B9E1EBF5CE1A2052A0F6301D8011DB7C0
              6AF55B64C0E876006DE3E81DA10D3D6D6A07FAFE2C91D7CB2881B6F1341B556C
              5C87CACD2C003BA0E31688373204C7C135F4BF60BDCDC5CF84B1CC007F12401B
              8A680B6DA89B09952FB300EC80AE5B400D42C1586640BD55DE8D5CF53D2FD9CB
              0C58BEC4D025C12DA4617F904F18A61B68836BEF50F990056007D0022733204F
              5368431846D036A6FD9FD0259FEF21DF9841D9D8CF27D03696FE03B48DD63380
              0560075CB805F46E7032B9817EC33109BCC875BA6DCFE7B305B421D9C8B961B7
              3D401B6A67000BC00EA00554069C3E133CB7E70FB310FA3D44CF722E69210358
              0014C30A3BE0E22D20E7803495EBEEA7F82B748923035C9E8F6EA6D0869E1A3B
              8EEA0061F84DFEFE1932800540310CEC80EE5940AD023543CFE5F9BAEBBC262D
              6448EC7EBD421B5AC8001600C528610774CD02B9F2B8C3F34D7165C676BA82B6
              D17E06B000EC007443C9A55B60196E840582F83BF447183EAABD42CD39C1B537
              F006F2FE64DDF0DD200BC00EA005D4283C843614B934E1206E7730AAACF3B9FE
              07C16976AF0DCF03B40558007640C72D301EC9FFE5553DBF8634B83CAED19E2F
              12F9F2D5EFCBD7F34526CF24F5E7AD67000BC00EE89805160BB50CDEAB0228D7
              B83CEF9CE51BE28FE45EA37106B000EC005AE064061CE777D036F439BF3EB7D7
              B8D67DCDD9E7009705580076C0855BE0C73C1416F8DFA93D07B000EC806E59E0
              2F5688946E7DA0C6A30000001064654247424230383330313337363837454333
              3335D6F8830000000049454E44AE426082}
          end>
      end
      item
        Name = 'tbRefreshServer'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE0000025249444154785EED9A2D6C02411484C11DEE709C445257594B5D2BEB
              B0C8D6214182AC2B12098EBAE25A5B578BEB4970E05847B3D7E6256FEEC2B2E1
              27293B9F1A081018DECCEE5EAE5C0A9C72E0BF9F0670023002DBC5782B0F2C5F
              639119EB5464C66A25B290B5A85F62518598B57EC3621389DE8734D5AF6FB69B
              A22DE5DB89FACD3440ACF88313107C04DEEE740724D72233CC526446BA119911
              43E80D7484C152003C3BC5C013D81933E894A7A18E7D3E02348013107804A637
              60C095C80CCC7C04FB02CC78B327722F461D9119DD85488B1954445B30F308EE
              0B6E274B4707D0004E40E01178A96903AA7A2F6D609D8FF06C8099AFB5441E83
              B45715BD0FFE1D400338018C80E4C192D4451632870E8075BB344C441681E7FF
              A80BFB0CE0FC1D40033801327E855C5C04FA50828E6B784EE06890033BC3C1E9
              3B80067002C28EC0A61F2903A2184A00337DE48E705DE3F3C5FB2C4003380181
              47E0BB1BAB0EA8277E19346DBDAE47AEB77B9E157C3BC1BB036800278011D8DD
              01AE7DC0A3636F8F99F7FCBC939F059C11F0FCC239680027408621C373A2CE1E
              01241709C415015F96139116336A8BB6E0BE00D7FD7955DFCFE0BC3F003B00A1
              019C800B8FC07B4B5F10A9D78D684B52D18F91DC757D6CFD8767917B31D5F70B
              B8CE06D8019F4BFDFADE87D9DD01348013C008ECEC8043C9AD22AE8D10902EF4
              F7C1CC2347EF8043A1019C807F1601E4D891F0C59579E4E00E40680027E0C223
              B01DB7540406A357D196B8E167406355135D049ED791F5DC2FF348A7A3EF73AC
              DCCF7677000DE004041E81D0C8754068D080D0FE71E407044B3C6E8032921200
              0000106465424734324138443446363731323641363046F952B5720000000049
              454E44AE426082}
          end>
      end
      item
        Name = 'tbServerProperties'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE000002E249444154785EED9A2190DA501086738E3870545277E78803890CAE
              F290E0C0DDB943826B65DD55525907EE9089230E5C9F242E71C4B5B377333BF7
              EF637843C9DC4CC97EEACF2424CCCEFEBB9BF772E309A228FBC307448DD53F71
              C832D684EF3758BF22EE2FAF5FC73BD6C47CDABFE18312B06EA601D00CA8B805
              5E5E7E430DE8765BAC89DC2B58133561E2C2715E22AFCF0C1E2FD68635D19DF5
              581FA39FE696AD4F615DAC01D00CA8B805A20D16C14EBBCEFA2388931C1E9385
              5883C249C8FA95EC274BC2FCC0FFFB393F5D13AC931A00CD808A5B4076815E0F
              3D5836D8F53D6F365FB126A6ADEFAC89FD38627D8C9D183BA21C9F302F0AB0BD
              550334009A0115B7807C1B6C77B0AF2671C2BA0C96EB3D6BE22EC4593F8B7EB1
              26C68D196B62F580BF77795E629DD400680654DC026577814234FAD937ECF3D2
              F3A6868D7C13A5AC0993A0E76F33BC5F7F80EF0A837E60D9FC3DD6490D806680
              5A40D4804FACDF70ACF1393D8F1E35E276E77A7E22D6070C6E2B94510334001C
              8C3734033814C7B8420BE01C103BDE055CB37DD97DBE59C3771563704D7134BA
              B80668003818846640F52C805DA028D0C3724DEF713A674D347B63D6C46E8FBF
              B03C5FE01AE0F0FE8935613C9F35E137B0066C576BD684EB7B02EBA45D033400
              1C0C4233A07A16C02E2096DCBCAF8F23D64450C7FDFC8DFF8535B1DA775913B2
              CF0F87F7AC89B4212751E49061DF37F1863551420DD000703008CD806BB7C062
              B9010B84E12D6B6227BEDB5B2EB06FCB1AB08B712F6FDB9CB226E46CDF129F11
              BA3035FC7F17CF011A00CD00B5806883E8B1E160C29AC80CF6DD5610B026E42C
              EFDD615F375BECE372B697C8BE2FAF2FBD06680034032A6E814ED0663B108310
              3DDE68E1F1C3F8B4E7537CFDB77079DC45E9354003A01950310B3C3F8B3638C0
              00A4C9E97D818F667B1073C5A5EB011A00CD00B5C0C91A60E48A8803ABEFD771
              5FE160706FF0DCBE2FB9780E70594003A01970E516789A2DC102FF3B67D7000D
              806640B52CF017E094347DE33E3A5E0000001064654247363938303732423535
              3443323739344341D675F70000000049454E44AE426082}
          end>
      end
      item
        Name = 'tbSettings'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE000002D249444154785EED9B2170A3501086A9E35C704122A9232E913987CC
              B98BCCC9BAD435927317974A64EA1A17642571C515774F06070E5C6FB673B3D3
              5D327019B8B4336F3FF5533224B3B3FFFFF601BD32CEE4FB6FF7150F0CC31845
              2BD440AA146AE03831510379758F1A38CCD5151E7C00677FB914403A40330BDC
              3D2F89E7D520420DDC645BD4401C3FA3069E1CFA798EE738A8819FA3F5D9B6EC
              42EB974901A40334B3004FF93656D5236A204952D4C0D6A473425E55A80165E4
              A8819139450D44DF76AD36ED42EDE25200E900CD2CC053BE8DB63940B1BD4010
              DFA20652979E1F5A03D4A758B80BD440DF738214403A40770B8CB74E63063863
              BA9FE738858FFA1461BC43FD8645D77D4E9617A801DF9EA106FA9E0BA400D201
              BA5B00CDF097E18666C2D46FCE004E4197F91A79D6BC1730720B25304DE8DEE0
              210C6BBFB90BB58B4901A40334B7000F453E07A803F330B3F0C865CF0198E72D
              93678A8B0A98B3FB07DB901EDBCE0435B070E95E8433F991D66CFE9EDA492980
              7480E616F01F672403A2239BE513546F0C3D7A5F9F67C08ACD059E4D3FAFF8FD
              8001F57CB05EA306AECD17D4C06CD23C785CCFB39ACDDF533B2905900ED0DC02
              5DE1ABC8BAA499C033C0B46CD440E604A8814D4033E18BD19C014549F7126541
              0795AFB734136A19D01529807480E616788D172403AADA7E9FEE0DCECE8094BE
              7F30F68FA84F61D3CB1B4539470DF49E015200E900CD2D104C3D9201CB2533A1
              C5EE07B464C26E4FD779D388510326DF4B305295A13E45EF192005900E100B90
              0CE0EBF4C41DA1FE175441D7FD368E6C2C3844F459A3E17A2881FF9E015200E9
              00CD2CC029F73EC9044E18D1778E5C67881AE0B3BC7AA19E6E9B03F8FE9FD378
              B20FA400D2019A5B607F6736664072A0CF06F9AAC1336017D23F0C5CF6A08271
              73DF6CF3C6937D2005900ED0DC02E7B2F945DF5D2E22FA7F86ABA7AA57DBF67A
              B13E90024807686E814BF3E932E0D248012E5DF1CFC61FFF1FF84327C8A67A00
              0000106465424744323039444444453536453741323530D0E655A90000000049
              454E44AE426082}
          end>
      end
      item
        Name = 'THeaderSection'
        SourceImages = <
          item
            Image.Data = {
              89504E470D0A1A0A0000000D4948445200000040000000400806000000AA6971
              DE000001F849444154785EED9AA153C33018C53337246EC8CAE290AB0437DC90
              9343E2C031391C12097272FC074C5622C131B939E636372E88EFC86BEFBEE692
              1DD7CBFBA9B76B9283AFEF7DCDD2754CE27412FFFF59003A207604CAE77C2F1F
              8C31ABA34CB4E564BB145DC776F32DDA7271B78EFE37FE25FAE22C001D905804
              E6B38193F941D6156DE96657A29BB05BCE455BCAB2146D89DD1382176301E880
              C422805DFEEC34175D076658A3280AD175E07AA13DC17B320B4007241E01ECFA
              C3F39168CB62762BDAE29B517458FF7222DA12BA3EE23D9905A003128FC0A133
              7A688721DE9359003A20B108E01DD7CEF8B4EB088ECF3F76A22D9B6C25DA82E3
              71FDE2FAD32BD6EA6016800E482C02F81CAE9CF1E5EE73D9B8978D71235CBDBE
              C10161849E21562EB2007440E211C0AEDF1F4D453702323E7971DFF53D4C6E44
              5BEEA74FA22DD3F1B1E8466C67222D8BD777D116EF1EC002D0018C805F0F509E
              EB9D13D83728EC576EA655FEBD07B0007480B8B18ED645007782C3E158F42FCA
              1DD7C08284661E09DE07B0007440E211C09D20BEFFF7FDCD0FE2DD0394CCEFD6
              EE8143F079000B4007241E0124B6232A3DE06B20BA09A19947D4C12C001D9078
              04107404BEABABBC47E8F97D77D0328EF8661EF19ECC02D001894740E3EDB1E7
              F488504233AE117D7116800E483C026D237A0F681B2C40DBEE586C7E007D28BC
              5F0DE7A181000000106465424732423846453735373145423334413033904CEE
              7B0000000049454E44AE426082}
          end>
      end>
    Left = 400
    Top = 280
  end
  object VirtualImageList1: TVirtualImageList
    Images = <
      item
        CollectionIndex = 0
        CollectionName = 'imDownArrow'
        Name = 'imDownArrow'
      end
      item
        CollectionIndex = 1
        CollectionName = 'imLogo'
        Name = 'imLogo'
      end
      item
        CollectionIndex = 2
        CollectionName = 'imPadlock'
        Name = 'imPadlock'
      end
      item
        CollectionIndex = 3
        CollectionName = 'imPadlocked'
        Name = 'imPadlocked'
      end
      item
        CollectionIndex = 4
        CollectionName = 'imUpArrow'
        Name = 'imUpArrow'
      end
      item
        CollectionIndex = 5
        CollectionName = 'miExit'
        Name = 'miExit'
      end
      item
        CollectionIndex = 6
        CollectionName = 'miExportFavoritesList'
        Name = 'miExportFavoritesList'
      end
      item
        CollectionIndex = 7
        CollectionName = 'miImportFavoritesList'
        Name = 'miImportFavoritesList'
      end
      item
        CollectionIndex = 8
        CollectionName = 'miSamp'
        Name = 'miSamp'
      end
      item
        CollectionIndex = 9
        CollectionName = 'RemoteConsole'
        Name = 'RemoteConsole'
      end
      item
        CollectionIndex = 10
        CollectionName = 'sbBrowse'
        Name = 'sbBrowse'
      end
      item
        CollectionIndex = 11
        CollectionName = 'tbAbout'
        Name = 'tbAbout'
      end
      item
        CollectionIndex = 12
        CollectionName = 'tbAddServer'
        Name = 'tbAddServer'
      end
      item
        CollectionIndex = 13
        CollectionName = 'tbConnect'
        Name = 'tbConnect'
      end
      item
        CollectionIndex = 14
        CollectionName = 'tbCopyServerInfo'
        Name = 'tbCopyServerInfo'
      end
      item
        CollectionIndex = 15
        CollectionName = 'tbDeleteServer'
        Name = 'tbDeleteServer'
      end
      item
        CollectionIndex = 16
        CollectionName = 'tbHelp'
        Name = 'tbHelp'
      end
      item
        CollectionIndex = 17
        CollectionName = 'tbMasterServerUpdate'
        Name = 'tbMasterServerUpdate'
      end
      item
        CollectionIndex = 18
        CollectionName = 'tbRefreshServer'
        Name = 'tbRefreshServer'
      end
      item
        CollectionIndex = 19
        CollectionName = 'tbServerProperties'
        Name = 'tbServerProperties'
      end
      item
        CollectionIndex = 20
        CollectionName = 'tbSettings'
        Name = 'tbSettings'
      end
      item
        CollectionIndex = 21
        CollectionName = 'THeaderSection'
        Name = 'THeaderSection'
      end>
    ImageCollection = ImageCollection1
    Left = 368
    Top = 280
  end
end
