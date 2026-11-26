# Android tablet GUI (550 px panel, 44 px controls, row/tag fixes); applied
# after assemble_assets.PATCHES.
# (rundir-relative asset, old text occurring exactly once, new text);
# CRLF assets carry \r\n in old/new, matched on raw bytes
# made for olex2-gui trunk r7881; svn log -qv -r 7882:HEAD <rundir> lists
# the assets changed since
PATCHES = [
    # _global
    ('etc/gui/blocks/tool-help-first-column.htm',
     "<td valign='top' width=",
     "<td valign='center' width="),
    ('etc/gui/blocks/tool-first-column.htm',
     "valign='top' width=",
     "valign='center' width="),
    # change_skin applies the skin's panel width on a first start only, so
    # an existing install kept its saved 480 px panel: apply it every load
    ('util/pyUtil/gui/skin/__init__.py',
     "    deal_with_gui_phil(action='load')\n",
     "    deal_with_gui_phil(action='load')\n"
     "    IT.set_htmlpanel_width(OV.GetParam('gui.htmlpanelwidth'))\n"
     "    # tab/header images are drawn on a first run only and kept in\n"
     "    # global.odb: redraw them once per new APK so skin changes show\n"
     "    import olexex\n"
     "    if olexex.check_for_recent_update() and getattr(olx, 'android_redraw', None) is None:\n"
     "      olx.android_redraw = set()  # read by _android_stale in htmlTools\n"
     "      from PilTools import TI\n"
     "      TI.run_timage(force_images=True)\n"),
    # run_timage skips tabs and h2/h3 headers: MakeHoverButton(On) draws them
    # on demand only when missing, so also redraw each once after a new APK
    ('util/pyUtil/PyToolLib/htmlTools.py',
     "def MakeHoverButton(name, cmds,",
     "def _android_stale(btn_type, btn_name):\r\n"
     "  redo = getattr(olx, 'android_redraw', None)\r\n"
     "  key = btn_type if btn_type in ('tab', 'cbtn', 'toolbar') else btn_name  # drawn as a set\r\n"
     "  if redo is None or key in redo:\r\n"
     "    return False\r\n"
     "  redo.add(key)\r\n"
     "  return True\r\n"
     "\r\n"
     "def MakeHoverButton(name, cmds,"),
    ('util/pyUtil/PyToolLib/htmlTools.py',
     "  if olx.fs.Exists(btn_name + \"off.png\") != \"true\":\r\n    TI.produce_buttons",
     "  if olx.fs.Exists(btn_name + \"off.png\") != \"true\" or _android_stale(btn_type, btn_name):\r\n    TI.produce_buttons"),
    ('util/pyUtil/PyToolLib/htmlTools.py',
     "  if olx.fs.Exists(btn_name + \"off.png\") != \"true\":\r\n    from PilTools import TI",
     "  if olx.fs.Exists(btn_name + \"off.png\") != \"true\" or _android_stale(btn_type, btn_name):\r\n    from PilTools import TI"),
    # tab/h3/button images live in global.odb and are redrawn only when the
    # SVN revision rises - never between APKs, so a new skin kept old images
    ('util/pyUtil/olexex.py',
     "    version = OV.GetSVNVersion()\r\n",
     "    version = OV.GetSVNVersion()\r\n"
     "    stamp = os.path.join(OV.BaseDir(), '.asset-stamp')\r\n"
     "    if os.path.exists(stamp):  # Android: rewritten by every new APK\r\n"
     "      version = int(os.path.getmtime(stamp))\r\n"
     "      # last_version is restored after initpy asks and saved only at a\r\n"
     "      # clean exit (Android kills the process): keep our own record\r\n"
     "      seen = stamp + '-seen'\r\n"
     "      try:\r\n"
     "        last = int(open(seen).read())\r\n"
     "      except Exception:\r\n"
     "        last = 0\r\n"
     "      olx.has_recently_updated = version > last\r\n"
     "      if olx.has_recently_updated:\r\n"
     "        open(seen, 'w').write(str(version))\r\n"
     "      return olx.has_recently_updated\r\n"),
    ('util/pyUtil/gui/skin/__init__.py',
     "table_width = int(olx.html.ClientWidth('self')) - 2 * int(olx.html.GetBorders())",
     "table_width = int(olx.html.ClientWidth('self')) - 2 * int(olx.html.GetBorders()) - 16"),
    # header
    ('etc/gui/blocks/snum-info.htm',
     "<table width='100%' cellspacing='1' cellpadding='0'>",
     "<table width='100%' cellspacing='4' cellpadding='0'>"),
    ('etc/gui/blocks/snum-info.htm',
     '<tr bgcolor=$GetVar(HtmlTableBgColour)>',
     "<tr valign='center' bgcolor=$GetVar(HtmlTableBgColour)>"),
    # tags on their own lines: the call returns an <!-- #include --> that
    # htmlprep expands only at a line start. Size 2: larger sizes wrap the
    # cell lengths in the 60 % column
    ('etc/gui/blocks/snum-info.htm',
     '$spy.gui.tools.make_cell_dimensions_display()',
     "<font size='2'>\n$spy.gui.tools.make_cell_dimensions_display()\n</font>"),
    ('etc/gui/blocks/snum-info.htm',
     "<td width='35%' align='right'",
     "<td width='28%' align='right'"),
    ('util/pyUtil/gui/templates/gui_templates.htm',
     "name='R1_GUI'",
     "name='R1_GUI' height='$GetVar(HtmlButtonHeight)'"),
    ('util/pyUtil/gui/templates/gui_templates.htm',
     "name='wR2_GUI'",
     "name='wR2_GUI' height='$GetVar(HtmlButtonHeight)'"),
    # home
    ('etc/gui/settings.htm',
     '"value=Colour/White [F2]",',
     '"value=Colour/White",'),
    ('etc/gui/settings.htm',
     '"value=Graduated [F4]",',
     '"value=Graduated",'),
    ('etc/gui/settings.htm',
     "  <td width='25%' align='right'>\r\n    Value (e.g. 520, or 0.3)",
     "<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='GUI_WIDTH_VALUE';1; -->\r\n  <td width='23%'>\r\n    Value (e.g. 520, or 0.3)"),
    ('etc/gui/settings.htm',
     '  <td width=\'10%\' align=\'right\'>%Width%:</td>\r\n  <td width="12%" align=\'right\'>',
     '<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'UNITCELL_WIDTH\';1; -->\r\n  <td width=\'23%\'><b>%Width%:</b></td>\r\n  <td width="25%">'),
    ('etc/gui/settings.htm',
     "<td width='75%'>\r\n  Start Olex2 in Debug Mode (Restart required)\r\n",
     "<td width='55%'>\r\n  Start Olex2 in Debug Mode (Restart required)</td>\r\n"),
    ('etc/gui/settings.htm',
     "<td width='75%'>\r\n  Use WSL Adapter and Conda (Restart required)\r\n",
     "<td width='55%'>\r\n  Use WSL Adapter and Conda (Restart required)</td>\r\n"),
    ('etc/gui/extension-modules.htm',
     '<td width="40%" align="right">$+',
     '<td width="37%" align="right">$+'),
    ('util/pyUtil/gui/help/tutorials.htm',
     '    $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Sort",',
     '<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'TUTORIAL_3B\';1; -->\r\n    $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Sort",'),
    ('util/pyUtil/gui/home/__init__.py',
     '((shown_cnt-1) % 4) == 0',
     '((shown_cnt-1) % 3) == 0'),
    ('util/pyUtil/gui/home/__init__.py',
     'cellpadding="1" cellspacing="0" bgcolor="$GetVar(HtmlTableRowBgColour)"><tr>',
     'cellpadding="1" cellspacing="6" bgcolor="$GetVar(HtmlTableRowBgColour)"><tr>'),
    # work-toolbox
    ('etc/gui/toolbox-work.htm',
     "  <td width='13%'>\r\n    <b>Labels</b>",
     "  <td width='20%'>\r\n    <b>Labels</b>"),
    ('etc/gui/toolbox-work.htm',
     '  <td width="10%">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=label_h",',
     '  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Hide",\r\n    "onclick=labels false",\r\n    "td1=<td width=\'30%\'>"\r\n    )\r\n  $-\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'LABELS_2\';1; -->\r\n  <td width="50%">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=label_h",'),
    ('etc/gui/toolbox-work.htm',
     '  <td width="10%">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=labelih",',
     '  <td width="50%">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=labelih",'),
    ('etc/gui/toolbox-work.htm',
     '  </td>\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Hide",\r\n    "onclick=labels false"\r\n    )\r\n  $-\r\n',
     '  </td>\r\n'),
    ('etc/gui/toolbox-work.htm',
     "<td width='10%'>\r\n</td>\r\n\r\n\r\n<td width='4%' align='left'>",
     "<td width='14%' align='right'>"),
    ('etc/gui/toolbox-work.htm',
     "  <td width = '5%' align='right'>",
     "  <td width = '10%' align='right'>"),
    ('etc/gui/toolbox-work.htm',
     '      "onclick=spy.gui.FixFree(free xyz)",\r\n    )$-\r\n',
     '      "onclick=spy.gui.FixFree(free xyz)",\r\n    )$-\r\n\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'REFINE_FIX_2\';1; -->\r\n'),
    ('etc/gui/snippets/snippet-electron-density-map.htm',
     '    <td width="15%">',
     '    <td width="25%">'),
    ('etc/gui/h3-peak-and-uiso-sliders.htm',
     '<!-- #include tool-row gui\\blocks\\tool-row.htm;name=SYMM_GROW_ROW_1; help_ext=Grow;1; -->\r\n',
     ''),
    ('etc/gui/h3-peak-and-uiso-sliders.htm',
     '<!-- #include uiso-slider gui\\snippets\\snippet-uiso-select-slider.htm;1; -->\r\n\r\n\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->',
     '<!-- #include uiso-slider gui\\snippets\\snippet-uiso-select-slider.htm;1; -->'),
    ('etc/gui/snippets/snippet-uiso-select-slider.htm',
     '            name = "UISO_SELECT_SLIDER"\r\n            width="100%"\r\n            height="18"\r\n',
     '            name = "UISO_SELECT_SLIDER"\r\n            width="100%"\r\n            height="$GetVar(HtmlInputHeight)"\r\n'),
    ('etc/gui/snippets/snippet-uiso-select-slider.htm',
     '            name = "UisoSelectVal"\r\n            width="100%"\r\n            height="18"\r\n',
     '            name = "UisoSelectVal"\r\n            width="100%"\r\n            height="$GetVar(HtmlInputHeight)"\r\n'),
    ('etc/gui/h3-growing.htm',
     "<td name=GROW width='32%'>",
     "<td name=GROW width='50%'>"),
    ('etc/gui/h3-growing.htm',
     "<td name=GROW_MODE width='34%'>",
     "<td name=GROW_MODE width='50%'>"),
    ('etc/gui/h3-growing.htm',
     "  <td name=COMPAQ width='34%'>",
     "\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='SYMM_GROW_ROW_2';1; -->\r\n  <td name=COMPAQ width='50%'>"),
    ('etc/gui/h3-finishing.htm',
     '  <td ALIGN=\'left\' width=\'5%\'>\r\n$spy.gui.images.get_action_button_html(\'edit\', "spy.gui.SwitchTool(h2-work-sorting)',
     '  <td ALIGN=\'left\' width=\'10%\'>\r\n$spy.gui.images.get_action_button_html(\'edit\', "spy.gui.SwitchTool(h2-work-sorting)'),
    ('etc/gui/h3-finishing.htm',
     '    "onclick=kill labels",\r\n    )$-\r\n',
     '    "onclick=kill labels",\r\n    )$-\r\n\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'FINISHING_QUICK_IMAGES_2\';1; -->\r\n<td width=\'20%\'></td>\r\n'),
    ('etc/gui/h3-finishing.htm',
     '  <td ALIGN=\'left\' width=\'5%\'>\r\n$spy.gui.images.get_action_button_html(\'edit\', "spy.gui.SwitchTool(h2-tools-images)',
     '  <td ALIGN=\'left\' width=\'10%\'>\r\n$spy.gui.images.get_action_button_html(\'edit\', "spy.gui.SwitchTool(h2-tools-images)'),
    ('util/pyUtil/gui/disorder/templates.htm',
     '<td>\r\n  %(clear_parts)s\r\n</td>\r\n',
     "<td>\r\n  %(clear_parts)s\r\n</td>\r\n</tr></table><table width='100%%' cellpadding='0' cellspacing='2'><tr>\r\n"),
    ('util/pyUtil/gui/disorder/templates.htm',
     "  <td width='4%%' align='right'>\r\n  Sel\r\n",
     "  <td align='left'>\r\n  Auto-select\r\n"),
    ('util/pyUtil/gui/disorder/templates.htm',
     "  <td width='4%%' align='right'>\r\n    Col\r\n",
     "  <td align='left'>\r\n    Colour\r\n"),
    ('util/pyUtil/gui/disorder/templates.htm',
     "  <td width='5%%' align='right'>\r\n  Unique\r\n",
     "  <td align='left'>\r\n  Unique\r\n"),
    ('util/pyUtil/gui/templates/gui_templates.htm',
     '  <td width=\'10%%\'>\r\n  <input\r\n    type="combo"\r\n    name="RESTRAINT_FILTER"\r\n',
     '  <td width=\'20%%\'>\r\n  <input\r\n    type="combo"\r\n    height="$GetVar(HtmlComboHeight)"\r\n    width="100%%"\r\n    name="RESTRAINT_FILTER"\r\n'),
    ('etc/gui/select.htm',
     '  <td width = "30%">\r\n    %Exclusive%:\r\n\r\n    $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Sucrose",\r\n    "onclick=reap GetVar(sample_dir)/sucrose/sucrose",\r\n    )$-\r\n\r\n\r\n    <a href="sel -u>> sel $C">C</a>&nbsp;\r\n    <a href="sel -u>> sel $H">H</a>&nbsp;\r\n    <a href="sel -u>> sel $N">N</a>&nbsp;\r\n    <a href="sel -u>> sel $O">O</a>&nbsp;\r\n  </td>\r\n  <td width = "30%" colspan = 2>\r\n    %Additive%:\r\n    <a href="sel -a">%All%</a>&nbsp;\r\n    <a href="sel $C">C</a>&nbsp;\r\n    <a href="sel $H">H</a>&nbsp;\r\n    <a href="sel $N">N</a>&nbsp;\r\n    <a href="sel $O">O</a>&nbsp;\r\n  </td>\r\n\r\n  <td width = "40%" align=\'right\' colspan = 2>',
     '    $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=All",\r\n    "onclick=sel -a",\r\n    "td1=<td width=\'20%\'>"\r\n    )$-\r\n\r\n  <td width = "80%" align=\'left\'>'),
    ('etc/gui/select.htm',
     '  <td width=\'100%\'>\r\n    <b>%With Selection%:</b>\r\n    <a href="sel -i">%Invert%</a> |\r\n    <a href="sel -u">%Deselect%</a> |\r\n    <a href="kill sel">%Delete%</a> |\r\n    <a href="selback">%Previous%</a>\r\n  </td>',
     '  <td width=\'20%\'>\r\n    <b>%With Selection%:</b>\r\n  </td>\r\n    $+\r\n    html.Snippet(GetVar(default_link), "value=Invert", "onclick=sel -i", "td1=<td width=\'20%\'>")\r\n    $-\r\n    $+\r\n    html.Snippet(GetVar(default_link), "value=Deselect", "onclick=sel -u", "td1=<td width=\'20%\'>")\r\n    $-\r\n    $+\r\n    html.Snippet(GetVar(default_link), "value=Delete", "onclick=kill sel", "td1=<td width=\'20%\'>")\r\n    $-\r\n    $+\r\n    html.Snippet(GetVar(default_link), "value=Previous", "onclick=selback", "td1=<td width=\'20%\'>")\r\n    $-'),
    ('etc/gui/select.htm',
     "  <td width='30%' VALIGN='center' >",
     "  <td width='35%' VALIGN='center' >"),
    ('etc/gui/select.htm',
     "  <td VALIGN='center' width='8%'>",
     "  <td VALIGN='center' width='19%'>"),
    ('etc/gui/select.htm',
     "  <td VALIGN='center' width='10%'>",
     "  <td VALIGN='center' width='16%'>"),
    ('etc/gui/select.htm',
     "  <td width='20%' align='right'>\r\n  $spy.MakeHoverButton('small-Select@uiso'",
     "  <td width='30%' align='right'>\r\n  $spy.MakeHoverButton('small-Select@uiso'"),
    ('etc/gui/select.htm',
     '  <td ALIGN=\'right\' width=\'45%\'>\r\n    <a href="sel rings C6"><b>%Phenyl%</b></a>&nbsp;\r\n    <a href="sel rings C5N"><b>%Pyridine%</b></a>\r\n  </td>\r\n',
     ''),
    ('etc/gui/naming.htm',
     '  <td width="18%">%Start%</td>\r\n  <td name=MODE_NAME_NUMBER>$+\r\n     html.Snippet("gui/snippets/input-text", "name=naming*start", "width=35", "manage=true")\r\n  $-</td>\r\n  <td>%Suffix%</td>\r\n  <td name=MODE_NAME_SUFFIX>$+\r\n     html.Snippet("gui/snippets/input-text", "name=naming*suffix", "width=35", "manage=true")\r\n  $-</td>\r\n  <td>%Type%</td>\r\n  <td name=MODE_NAME_TYPE>$+\r\n     html.Snippet("gui/snippets/input-text", "name=naming*type", "width=35", "manage=true")\r\n  $-</td>\r\n  <td name=IMG_BUTTON_SMALL-NAME align=\'right\'>',
     '  <td width="20%">%Start%</td>\r\n  <td width="30%" name=MODE_NAME_NUMBER>$+\r\n     html.Snippet("gui/snippets/input-text", "name=naming*start", "manage=true")\r\n  $-</td>\r\n  <td width="20%">%Suffix%</td>\r\n  <td width="30%" name=MODE_NAME_SUFFIX>$+\r\n     html.Snippet("gui/snippets/input-text", "name=naming*suffix", "manage=true")\r\n  $-</td>\r\n</tr><tr>\r\n  <td>%Type%</td>\r\n  <td name=MODE_NAME_TYPE>$+\r\n     html.Snippet("gui/snippets/input-text", "name=naming*type", "manage=true")\r\n  $-</td>\r\n  <td name=IMG_BUTTON_SMALL-NAME align=\'right\' colspan=2>'),
    ('etc/gui/naming.htm',
     '  <td width="18%">Autocomplete</td>\r\n  <td width=\'20%\' align="left">$+',
     '  <td width="26%">Autocomplete</td>\r\n  <td width=\'37%\' align="left">$+'),
    ('etc/gui/naming.htm',
     '  <td width=\'20%\' align="left">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=naming*auto*stop_type",',
     '  <td width=\'37%\' align="left">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=naming*auto*stop_type",'),
    ('etc/gui/naming.htm',
     '  <td width=\'20%\' align="left">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=naming*auto*stop_part",',
     '</tr><tr>\r\n  <td></td>\r\n  <td align="left">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=naming*auto*stop_part",'),
    ('etc/gui/naming.htm',
     '  <td align="right">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=naming*auto*stop_branch",',
     '  <td align="left">$+\r\n    html.Snippet("gui/snippets/input-checkbox", "name=naming*auto*stop_branch",'),
    ('etc/gui/naming.htm',
     '  <td>\r\n    <a href="spy.NoSpherA2.match_sel_with_tscb_sync(html.GetValue(\'MATCH_NAME_SUFFIX\'),html.GetState(MATCH_NAME_INVERT))">\r\n    %Equivalent Fragments% (Z\' > 1)</a>\r\n  </td>\r\n  <td>%Suffix%</td>',
     '  <td width="60%">%Equivalent Fragments% (Z\' > 1)</td>\r\n    $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Match",\r\n    "onclick=spy.NoSpherA2.match_sel_with_tscb_sync(html.GetValue(\'MATCH_NAME_SUFFIX\'),html.GetState(MATCH_NAME_INVERT))",\r\n    "td1=<td width=\'40%\'>"\r\n    )$-\r\n\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'MATCH_NAMING_2\';1; -->\r\n  <td width="20%">%Suffix%</td>'),
    ('etc/gui/naming.htm',
     '  <td>\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      height="$GetVar(\'HtmlInputHeight\')"\r\n      width="30"\r\n',
     '  <td width="30%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      height="$GetVar(\'HtmlInputHeight\')"\r\n      width="100%"\r\n'),
    ('etc/gui/naming.htm',
     '  <td>%Invert for matching%</td>\r\n  <td align="right">$+',
     '  <td width="40%">%Invert for matching%</td>\r\n  <td width="10%" align="right">$+'),
    ('etc/gui/sorting.htm',
     '   <td width="40%">$+\r\n     html.Snippet("gui/snippets/input-combo", "name=sorting_moiety",',
     '   <td width="30%">$+\r\n     html.Snippet("gui/snippets/input-combo", "name=sorting_moiety",'),
    ('etc/gui/sorting.htm',
     '   <td width="35%" align=\'left\'>Treat H atoms independently</td>\r\n   <td width="5%" align=\'right\'>',
     '   <td width="40%" align=\'left\'>Treat H atoms independently</td>\r\n   <td width="10%" align=\'right\'>'),
    ('etc/gui/sorting.htm',
     '       <td width="11%">Atoms</td>\r\n       <td width="50%" align="right">$+',
     '       <td width="15%">Atoms</td>\r\n       <td colspan="2" align="right">$+'),
    ('etc/gui/sorting.htm',
     '       <td width="5%" align=\'right\'>$+\r\n         html.Snippet("gui/snippets/input-checkbox", "name=atom_sequence_inplace",',
     '     </tr><tr>\r\n       <td></td>\r\n       <td width="42%" align=\'left\'>$+\r\n         html.Snippet("gui/snippets/input-checkbox", "name=atom_sequence_inplace",'),
    ('etc/gui/sorting.htm',
     '       <td width="5%" align=\'right\'>$+\r\n         html.Snippet("gui/snippets/input-checkbox", "name=atom_sequence_first",',
     '       <td width="43%" align=\'left\'>$+\r\n         html.Snippet("gui/snippets/input-checkbox", "name=atom_sequence_first",'),
    ('etc/gui/sorting.htm',
     '       <td width="11%">Moieties</td>\r\n       <td width="50%" align="right">$+',
     '       <td width="15%">Moieties</td>\r\n       <td width="55%" align="right">$+'),
    ('etc/gui/sorting.htm',
     '      <td width="18%" align=\'right\'>',
     '      <td width="30%" align=\'right\'>'),
    # work-solve
    ('etc/gui/tools/solve-settings.htm',
     "\r\n    <td ALIGN='right' width='13%'>",
     "\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n\r\n<tr ALIGN='left' NAME='SET_Z_AND_ZPRIME' width='100%'>\r\n<!-- #include tool-first-column gui\\blocks\\tool-first-column.htm;1; -->\r\n<!-- #include row_table_on gui\\blocks\\row_table_on.htm;1; -->\r\n  <td width='18%'>\r\n    <b>Z </b>\r\n  </td>\r\n  <td ALIGN='right' width='30%'>"),
    ('etc/gui/tools/solve-settings.htm',
     "width='50%'",
     "width='100%'"),
    ('etc/gui/tools/solve-settings.htm',
     '\r\n      label="Z = "',
     ''),
    ('etc/gui/tools/solve-settings.htm',
     "\r\n  <td ALIGN='right' width='13%'>",
     "\r\n  <td width='18%' align='right'>\r\n    <b>Z' </b>\r\n  </td>\r\n  <td ALIGN='right' width='30%'>"),
    ('etc/gui/tools/solve-settings.htm',
     'width="50%"',
     'width="100%"'),
    ('etc/gui/tools/solve-settings.htm',
     '\r\n      label="Z\' = "',
     ''),
    ('etc/gui/tools/olex2-space-group-row.htm',
     '<a href="sg>>spy.SetParam(\'snum.refinement.sg_list\',SGList())>>html.update"><b>Olex2 </b> </a>',
     '<input type="button" name="SG_FIND_OLEX2" value="Olex2" width="100%" height="$GetVar(HtmlButtonHeight)" onclick="sg>>spy.SetParam(\'snum.refinement.sg_list\',SGList())>>html.update">'),
    ('etc/gui/tools/olex2-space-group-row.htm',
     '<b><a href="zsgh">%ZSGH% </a>&nbsp;</b>',
     '<input type="button" name="SG_FIND_ZSGH" value="%ZSGH%" width="100%" height="$GetVar(HtmlButtonHeight)" onclick="zsgh">'),
    ('util/pyUtil/PyToolLib/method_imp/shelx.py',
     "'width':50,",
     "'width':'25%',"),
    ('util/pyUtil/PyToolLib/method_imp/shelx.py',
     "\r\n      'height':28,",
     ''),
    # work-refine
    ('etc/gui/tools/refine-settings.htm',
     "    <td width='35%'>",
     "    <td width='88%'>"),
    ('etc/gui/tools/refine-settings.htm',
     '      <table cellpadding="0" cellspacing="0">',
     '      <table width=\'100%\' cellpadding="0" cellspacing="0">'),
    ('etc/gui/tools/refine-settings.htm',
     "    <td width='15%'>",
     "<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<tr ALIGN='left' width='100%'>\r\n<!-- #include tool-first-column gui\\blocks\\tool-first-column.htm;1; -->\r\n<!-- #include row_table_on gui\\blocks\\row_table_on.htm;1; -->\r\n    <td width='24%'>"),
    ('etc/gui/tools/refine-settings.htm',
     '"label=Client",\r\n        "height=0",\r\n',
     '"label=Client",\r\n'),
    ('etc/gui/tools/refine-settings.htm',
     "    <td align='left' width='8%'>",
     "    <td align='left' width='12%'>"),
    ('etc/gui/tools/refine-settings.htm',
     '        "name=SET_SNUM_REFINEMENT_MAX_CYCLES",',
     '        "name=SET_SNUM_REFINEMENT_MAX_CYCLES",\r\n        "td1=<td width=\'16%\'>",'),
    ('etc/gui/tools/refine-settings.htm',
     "    <td align='right' width='8%'>",
     "    <td align='right' width='12%'>"),
    ('etc/gui/tools/refine-settings.htm',
     '        "name=SET_SNUM_REFINEMENT_MAX_PEAKS",',
     '        "name=SET_SNUM_REFINEMENT_MAX_PEAKS",\r\n        "td1=<td width=\'16%\'>",'),
    ('etc/gui/tools/refine-settings.htm',
     '    $spy.gui.tools.weightGuiDisplay()',
     "    $spy.gui.tools.weightGuiDisplay()\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<tr ALIGN='left' width='100%'>\r\n<!-- #include tool-first-column gui\\blocks\\tool-first-column.htm;1; -->\r\n<!-- #include row_table_on gui\\blocks\\row_table_on.htm;1; -->\r\n"),
    ('etc/gui/tools/refine-settings.htm',
     "    <td width='18%'>",
     "    <td width='30%'>"),
    ('etc/gui/tools/refine-settings.htm',
     '"td1=<td width=11%>"',
     '"td1=<td width=20%>"'),
    ('etc/gui/tools/refine-settings.htm',
     '"td1=<td width=17%>"',
     '"td1=<td width=22%>"'),
    ('etc/gui/tools/refine-settings.htm',
     '"td1=<td width=13%>"',
     '"td1=<td width=24%>"'),
    ('etc/gui/tools/refine-settings.htm',
     '  $+\r\n    html.Snippet("gui/snippets/input-checkbox-td",\r\n    "name=SNUM_MASKS_OCCUPANCY_WEIGHTING",',
     '<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<tr ALIGN=\'left\' width=\'100%\'>\r\n<!-- #include tool-first-column gui\\blocks\\tool-first-column.htm;1; -->\r\n<!-- #include row_table_on gui\\blocks\\row_table_on.htm;1; -->\r\n  $+\r\n    html.Snippet("gui/snippets/input-checkbox-td",\r\n    "name=SNUM_MASKS_OCCUPANCY_WEIGHTING",'),
    ('etc/gui/tools/refine-settings.htm',
     '"td1=<td width=9%>"',
     '"td1=<td width=14%>"'),
    ('etc/gui/tools/refine-settings.htm',
     '  <td width="9%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      name = "SET_SNUM_MASKS_SOLVENT_RADIUS"',
     '  <td width="15%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      name = "SET_SNUM_MASKS_SOLVENT_RADIUS"'),
    ('etc/gui/tools/refine-settings.htm',
     '  <td width="9%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      name = "SET_SNUM_MASKS_SHRINK_TRUNCATION_RADIUS"',
     '  <td width="15%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      name = "SET_SNUM_MASKS_SHRINK_TRUNCATION_RADIUS"'),
    ('etc/gui/tools/refine-settings.htm',
     '  <td width="9%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      name = "SET_SNUM_MASKS_FLAT_SOLVENT_RADIUS"',
     '  <td width="15%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      name = "SET_SNUM_MASKS_FLAT_SOLVENT_RADIUS"'),
    ('etc/gui/tools/refine-settings.htm',
     '  <td width="9%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      name = "SET_SNUM_MASKS_FLAT_SHRINK_RADIUS"',
     '  <td width="15%">\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="text"\r\n      name = "SET_SNUM_MASKS_FLAT_SHRINK_RADIUS"'),
    ('etc/gui/tools/refine-settings.htm',
     '"label=NoSpherA2 ",\r\n        "height=0",\r\n',
     '"label=NoSpherA2 ",\r\n'),
    ('util/pyUtil/gui/templates/gui_templates.htm',
     "    <td width='20%%' align='left'>",
     "    <td width='84%%' align='left'>"),
    ('util/pyUtil/gui/templates/gui_templates.htm',
     '  height="math.eval(GetVar(\'HtmlInputheight\')-3)"',
     '  height="GetVar(\'HtmlInputHeight\')"'),
    ('util/pyUtil/NoSpherA2/utilities.py',
     '        onclick="spy.NoSpherA2.%(type)s_%(qual)s()"',
     '        width="100%%"\r\n        onclick="spy.NoSpherA2.%(type)s_%(qual)s()"'),
    ('util/pyUtil/NoSpherA2/hybrid_GUI.py',
     '  <td valign=\'top\' width="$GetVar(HtmlTableFirstcolWidth)"',
     '  <td valign=\'center\' width="$GetVar(HtmlTableFirstcolWidth)"'),
    ('util/pyUtil/NoSpherA2/hybrid_GUI.py',
     'width_label=11, width_spinbox=9',
     'width_label=18, width_spinbox=32'),
    ('util/pyUtil/NoSpherA2/hybrid_GUI.py',
     'width_label=14, width_spinbox=9',
     'width_label=18, width_spinbox=32'),
    ('util/pyUtil/NoSpherA2/hybrid_GUI.py',
     'GRID_COLS = ((11, 16), (11, 15), (10, 12), (12, 13))\n',
     '# tablet: two wide columns, _cells_html continues wider rows on the next line\nGRID_COLS = ((18, 32), (18, 32))\n'),
    ('util/pyUtil/NoSpherA2/hybrid_GUI.py',
     'GRID_COLS_3 = ((11, 22), (11, 22), (11, 23))\n',
     'GRID_COLS_3 = GRID_COLS\n'),
    ('util/pyUtil/NoSpherA2/hybrid_GUI.py',
     '    if col + span > frame.n:\n      # a layout bug of the caller: render it anyway (the row wraps) but say so\n      print("NoSpherA2 GUI: row \'%s\' has more than %d grid columns" % (context, frame.n))\n      span = frame.n - col\n      if span <= 0:\n        break\n',
     '    span = min(span, frame.n)\n    if col + span > frame.n:\n      if col < frame.n:\n        html.append(f\'\'\'\n  <td width="{frame.gap(col) + frame.span(col, frame.n - col)}%"></td>\'\'\')\n      html.append(\'\'\'\n  </tr><tr>\'\'\')\n      col = 0\n'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '<td width="13%" align=\'left\'>\r\n  <b>CPUs</b>\r\n</td>\r\n<td width="9%" align=\'center\'>',
     '<td width="18%" align=\'left\'>\r\n  <b>CPUs</b>\r\n</td>\r\n<td width="22%" align=\'center\'>'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  $+\r\n    html.Snippet("gui/snippets/input-checkbox-td",\r\n    "name=NCI",',
     '<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<tr ALIGN=\'left\' NAME=\'SNUM_REFINEMENT_NSFF\' width=\'100%\'>\r\n<!-- #include tool-first-column gui\\blocks\\tool-first-column.htm;1; -->\r\n<!-- #include row_table_on gui\\blocks\\row_table_on.htm;1; -->\r\n  $+\r\n    html.Snippet("gui/snippets/input-checkbox-td",\r\n    "name=NCI",'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width=\'5%\'>\r\n\r\n    <input \r\n      type=\'spin\', \r\n      width=\'50\', \r\n      height="17"\r\n      bgcolor="spy.GetParam(gui.html.input_bg_colour)"\r\n      min=\'1\'\r\n      max=\'$spy.NoSpherA2.get_nmo()\',',
     '  <td width=\'16%\'>\r\n\r\n    <input \r\n      type=\'spin\', \r\n      width=\'100%\', \r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      bgcolor="spy.GetParam(gui.html.input_bg_colour)"\r\n      min=\'1\'\r\n      max=\'$spy.NoSpherA2.get_nmo()\','),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     'width = \'12%\'>",',
     'width = \'22%\'>",'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     "  <td width='39%'> </td>\r\n#ignoreif -->\r\n",
     "  <td width='39%'> </td>\r\n#ignoreif -->\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<tr ALIGN='left' NAME='SNUM_REFINEMENT_NSFF' width='100%'>\r\n<!-- #include tool-first-column gui\\blocks\\tool-first-column.htm;1; -->\r\n<!-- #include row_table_on gui\\blocks\\row_table_on.htm;1; -->\r\n"),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width=\'5%\'>\r\n\r\n    <input \r\n      type=\'spin\', \r\n      width=\'50\', \r\n      height="17"\r\n      bgcolor="spy.GetParam(gui.html.input_bg_colour)"\r\n      min=\'1\'\r\n      max=\'$spy.NoSpherA2.get_ncen()\',',
     '  <td width=\'16%\'>\r\n\r\n    <input \r\n      type=\'spin\', \r\n      width=\'100%\', \r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      bgcolor="spy.GetParam(gui.html.input_bg_colour)"\r\n      min=\'1\'\r\n      max=\'$spy.NoSpherA2.get_ncen()\','),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width="9%" align=\'center\'>\r\n    $+\r\n      html.Snippet("gui/snippets/input-text",\r\n      "name=SET_SNUM_CUBE_RADIUS_NoSpherA2",',
     '  <td width="14%" align=\'center\'>\r\n    $+\r\n      html.Snippet("gui/snippets/input-text",\r\n      "name=SET_SNUM_CUBE_RADIUS_NoSpherA2",'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width="9%" align=\'center\'>\r\n    $+\r\n      html.Snippet("gui/snippets/input-text",\r\n      "name=SET_SNUM_CUBE_RESOLUTION_NoSpherA2",',
     '  <td width="14%" align=\'center\'>\r\n    $+\r\n      html.Snippet("gui/snippets/input-text",\r\n      "name=SET_SNUM_CUBE_RESOLUTION_NoSpherA2",'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width="12%" align=\'center\'>\r\n    $+\r\n      html.Snippet("gui/snippets/input-text",\r\n      "name=SET_SNUM_CUBE_SCALE_MIN",',
     '  <td width="14%" align=\'center\'>\r\n    $+\r\n      html.Snippet("gui/snippets/input-text",\r\n      "name=SET_SNUM_CUBE_SCALE_MIN",'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width="12%" align=\'center\'>\r\n    $+\r\n      html.Snippet("gui/snippets/input-text",\r\n      "name=SET_SNUM_CUBE_SCALE_MAX",',
     '  <td width="14%" align=\'center\'>\r\n    $+\r\n      html.Snippet("gui/snippets/input-text",\r\n      "name=SET_SNUM_CUBE_SCALE_MAX",'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width="25%">',
     '  <td width="21%">'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width="57%" align=\'center\'>',
     '  <td width="51%" align=\'center\'>'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '      name="SET_SNUM_NoSpherA2_MAP_ALPHA_SLIDE"\r\n      width="100%"\r\n      height="18"',
     '      name="SET_SNUM_NoSpherA2_MAP_ALPHA_SLIDE"\r\n      width="100%"\r\n      height="GetVar(\'HtmlInputHeight\')"'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width="10%" align=\'center\'>\r\n    <input\r\n      type="text"\r\n      name="SET_SNUM_NoSpherA2_MAP_ALPHA_VALUE"\r\n      width="100%"\r\n      height="18"',
     '  <td width="16%" align=\'center\'>\r\n    <input\r\n      type="text"\r\n      name="SET_SNUM_NoSpherA2_MAP_ALPHA_VALUE"\r\n      width="100%"\r\n      height="GetVar(\'HtmlInputHeight\')"'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width=\'5%\'>\r\n\r\n    <input \r\n      type=\'spin\', \r\n      width=\'50\', \r\n      height="17"\r\n      bgcolor="spy.GetParam(gui.html.input_bg_colour)"\r\n      min=\'1\'\r\n      name=\'SET_POINTSIZE\',',
     '  <td width=\'14%\'>\r\n\r\n    <input \r\n      type=\'spin\', \r\n      width=\'100%\', \r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      bgcolor="spy.GetParam(gui.html.input_bg_colour)"\r\n      min=\'1\'\r\n      name=\'SET_POINTSIZE\','),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td witdh="83%" align=\'center\'>',
     '  <td width="73%" align=\'center\'>'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '      name="SNUM_XGRID_SCALE_SLIDE"\r\n      width="100%"\r\n      height="18"',
     '      name="SNUM_XGRID_SCALE_SLIDE"\r\n      width="100%"\r\n      height="GetVar(\'HtmlInputHeight\')"'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '  <td width="9%">\r\n    <input\r\n      type="text"\r\n      name="SNUM_XGRID_SCALE_VALUE"\r\n      width="100%"\r\n      height="18"',
     '  <td width="17%">\r\n    <input\r\n      type="text"\r\n      name="SNUM_XGRID_SCALE_VALUE"\r\n      width="100%"\r\n      height="GetVar(\'HtmlInputHeight\')"'),
    ('util/pyUtil/NoSpherA2/h3-refine_NoSpherA2-Properties.htm',
     '<td width=\'10%\'>\r\n\r\n    $+\r\n      html.Snippet("gui/snippets/input-combo",\r\n      "name=SET_SNUM_XGRID_HEAT_COLOURS#suffix",',
     '<td width=\'19%\'>\r\n\r\n    $+\r\n      html.Snippet("gui/snippets/input-combo",\r\n      "name=SET_SNUM_XGRID_HEAT_COLOURS#suffix",'),
    # work-draw
    ('etc/gui/tools/images-header.htm',
     "  <td name=IMAGE_BITMAP_NAME width='35%' align='right'>",
     "  <td name=IMAGE_BITMAP_NAME width='100%' align='left'>"),
    ('etc/gui/tools/images-header.htm',
     '      height="math.eval(GetVar(\'HtmlInputHeight\')*1.3)"',
     '      height="GetVar(\'HtmlInputHeight\')"'),
    ('etc/gui/tools/images-header.htm',
     '  <td width=\'20%\' align=\'right\'>\r\n     If <b>NAME</b> exists\r\n  </td>\r\n\r\n  <td width="17%" align=\'left\'>',
     '<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'IMAGE_SETTINGS_ROW_4B\';1 -->\r\n  <td width=\'25%\' align=\'left\'>\r\n     If <b>NAME</b> exists\r\n  </td>\r\n\r\n  <td width="25%" align=\'left\'>'),
    ('etc/gui/tools/images-header.htm',
     '  <td width=\'10%\' align=\'right\'>\r\n     Default\r\n  </td>\r\n\r\n\r\n  <td width="10%">',
     '  <td width=\'20%\' align=\'right\'>\r\n     Default\r\n  </td>\r\n\r\n\r\n  <td width="30%">'),
    ('etc/gui/tools/images-header.htm',
     "  <td width='20%' align='right'>Symmetry label </td>\r\n\r\n  <td name=IMAGE_SETTINGS_LABEL_SYMM width='12%'>",
     "<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='LABEL_SETTINGS_2';1 -->\r\n  <td width='30%' align='left'>Symmetry label </td>\r\n\r\n  <td name=IMAGE_SETTINGS_LABEL_SYMM width='20%'>"),
    ('etc/gui/tools/images-header.htm',
     "  <td width='10%' align='right'>%Style%</td>\r\n\r\n  <td name=IMAGE_SETTINGS_LABEL_TYPE width='12%' align='right'>",
     "  <td width='20%' align='right'>%Style%</td>\r\n\r\n  <td name=IMAGE_SETTINGS_LABEL_TYPE width='30%' align='left'>"),
    ('etc/gui/tools/images-header.htm',
     '  <td name=LABEL_INSTRUCTIONS_3>\r\n     <a href="kill labels"><b>DELETE all labels</b></a> |\r\n     <a href="SetMaterial \'PLabels.Text\' ChooseMaterial(GetMaterial(\'PLabels.Text\'))">%images_label_colour%</a> |\r\n     <a href="SetMaterial \'PLabels.Plane\' ChooseMaterial(GetMaterial(\'PLabels.Plane\'))">%images_label_box%</a>\r\n  </td>',
     '  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=DELETE all labels",\r\n    "onclick=kill labels",\r\n    )$-\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Label colour",\r\n    "onclick=SetMaterial \'PLabels.Text\' ChooseMaterial(GetMaterial(\'PLabels.Text\'))",\r\n    )$-\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Label box",\r\n    "onclick=SetMaterial \'PLabels.Plane\' ChooseMaterial(GetMaterial(\'PLabels.Plane\'))",\r\n    )$-'),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='8%' align='left'>\r\n    %Type%\r\n  </td>\r\n  <td name=IMAGE_BITMAP_TYPE width='13%'>",
     "  <td width='30%' align='left'>\r\n    %Type%\r\n  </td>\r\n  <td name=IMAGE_BITMAP_TYPE width='20%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     '  <td width=\'5%\' align=\'right\'>\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="checkbox"\r\n      name = "BITMAP_NO_BG"',
     '  <td width=\'15%\' align=\'left\'>\r\n    <font size="$GetVar(\'HtmlFontSizeControls\')">\r\n    <input\r\n      type="checkbox"\r\n      name = "BITMAP_NO_BG"'),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='24%' align='right'>\r\n     Make Image\r\n",
     "<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='IMAGE_BITMAP_ROW_1B';1 -->\r\n  <td width='75%' align='right'>\r\n     Make Image\r\n"),
    ('etc/gui/h3-bitmap-images.htm',
     '    "name=DEFBTN_GO",\r\n',
     '    "name=DEFBTN_GO",\r\n    "td1=<td width=\'25%\'>",\r\n    "height=math.eval(GetVar(HtmlInputHeight)+8)",\r\n'),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='20%' align='left'>\r\n    Resolution (DPI)\r\n  </td>\r\n  <td name=IMAGE_BITMAP_RESOLTUION width='12%'>",
     "  <td width='30%' align='left'>\r\n    Resolution (DPI)\r\n  </td>\r\n  <td name=IMAGE_BITMAP_RESOLTUION width='20%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='10%' align='right'>\r\n    Width\r\n  </td>\r\n  <td name=IMAGE_BITMAP_WIDTH width='12%'>",
     "  <td width='20%' align='right'>\r\n    Width\r\n  </td>\r\n  <td name=IMAGE_BITMAP_WIDTH width='30%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='8%' align='right'>\r\n    Unit\r\n  </td>\r\n  <td name=IMAGE_BITMAP_WIDTH_UNIT width='10%'>",
     "<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='IMAGE_BITMAP_ROW_2B';1 -->\r\n  <td width='30%' align='left'>\r\n    Unit\r\n  </td>\r\n  <td name=IMAGE_BITMAP_WIDTH_UNIT width='20%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='12%' align='right'>\r\n    Size (px)\r\n  </td>\r\n  <td name=IMAGE_BITMAP_SIZE width='15%'>",
     "  <td width='20%' align='right'>\r\n    Size (px)\r\n  </td>\r\n  <td name=IMAGE_BITMAP_SIZE width='30%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='5%'>\r\n     %Trim%\r\n  </td>\r\n  <td width='5%' align='right'>",
     "  <td width='30%' align='left'>\r\n     %Trim%\r\n  </td>\r\n  <td width='20%' align='left'>"),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='16%' align='right'>\r\n    Padding (%)\r\n  </td>\r\n  <td  width='12%'>",
     "  <td width='20%' align='right'>\r\n    Padding (%)\r\n  </td>\r\n  <td  width='30%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='15%' align='right'>\r\n    Border (%)\r\n  </td>\r\n  <td  width='12%'>",
     "<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='IMAGE_BITMAP_ROW_3B';1 -->\r\n  <td width='30%' align='left'>\r\n    Border (%)\r\n  </td>\r\n  <td  width='20%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='10%' align='right'>\r\n    Colour\r\n  </td>\r\n  <td name=IMAGE_BITMAP_SIZE width='20%'>",
     "  <td width='20%' align='right'>\r\n    Colour\r\n  </td>\r\n  <td name=IMAGE_BITMAP_SIZE width='22%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     "  <td width='2%'>",
     "  <td width='8%'>"),
    ('etc/gui/h3-bitmap-images.htm',
     '  <td colspan=2 name-=IMAGE_BITMAP_CHOOSE_ATOM_LABEL_FONTS>\r\n    <a href="setfont AtomLabels ChooseFont(Olex2,spy.GetParam(user.image.bitmap.AtomLabels.font))>>spy.SetParam(user.image.bitmap.AtomLabels.font,GetFont(AtomLabels))"><b>Atom</b> Label Font (Olex2)</a> |\r\n    <a href="setfont AtomLabels ChooseFont(System,spy.GetParam(user.image.bitmap.AtomLabels.font))>>spy.SetParam(user.image.bitmap.AtomLabels.font,GetFont(AtomLabels))">(System Font)</a>\r\n</td>\r\n\r\n <td colspan=2 name-=IMAGE_BITMAP_CHOOSE_BOND_LABEL_FONTS>\r\n\r\n    <a href="setfont BondLabels ChooseFont(Olex2,spy.GetParam(user.image.bitmap.BondLabels.font))>>spy.SetParam(user.image.bitmap.BondLabels.font,GetFont(BondLabels))"><b>Bond</b> Label Font (Olex2)</a> |\r\n    <a href="setfont BondLabels ChooseFont(System,spy.GetParam(user.image.bitmap.BondLabels.font))>>spy.SetParam(user.image.bitmap.BondLabels.font,GetFont(BondLabels))">(System Font)</a>\r\n  </td>',
     '  <td width=\'30%\' align=\'left\'><b>Atom</b> label font</td>\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Olex2",\r\n    "onclick=setfont AtomLabels ChooseFont(Olex2,spy.GetParam(user.image.bitmap.AtomLabels.font))>>spy.SetParam(user.image.bitmap.AtomLabels.font,GetFont(AtomLabels))",\r\n    )$-\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=System",\r\n    "onclick=setfont AtomLabels ChooseFont(System,spy.GetParam(user.image.bitmap.AtomLabels.font))>>spy.SetParam(user.image.bitmap.AtomLabels.font,GetFont(AtomLabels))",\r\n    )$-\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'IMAGE_SETTINGS_ROW_1B\';1 -->\r\n  <td width=\'30%\' align=\'left\'><b>Bond</b> label font</td>\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Olex2",\r\n    "onclick=setfont BondLabels ChooseFont(Olex2,spy.GetParam(user.image.bitmap.BondLabels.font))>>spy.SetParam(user.image.bitmap.BondLabels.font,GetFont(BondLabels))",\r\n    )$-\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=System",\r\n    "onclick=setfont BondLabels ChooseFont(System,spy.GetParam(user.image.bitmap.BondLabels.font))>>spy.SetParam(user.image.bitmap.BondLabels.font,GetFont(BondLabels))",\r\n    )$-'),
    ('etc/gui/h3-postscript-images.htm',
     "  <td name=IMAGE_PS_COLOUR_LINE_LABEL width='18%' align='left'>",
     "  <td name=IMAGE_PS_COLOUR_LINE_LABEL width='30%' align='left'>"),
    ('etc/gui/h3-postscript-images.htm',
     "  <td name=IMAGE_PS_COLOUR_LINE width='5%' align='left'>",
     "  <td name=IMAGE_PS_COLOUR_LINE width='20%' align='left'>"),
    ('etc/gui/h3-postscript-images.htm',
     "  <td name=IMAGE_PS_COLOUR_BOND_LABEL width='18%' align='right'>",
     "  <td name=IMAGE_PS_COLOUR_BOND_LABEL width='30%' align='right'>"),
    ('etc/gui/h3-postscript-images.htm',
     "  <td name=IMAGE_PS_COLOUR_BOND width='5%' align='left'>",
     "  <td name=IMAGE_PS_COLOUR_BOND width='20%' align='left'>"),
    ('etc/gui/h3-postscript-images.htm',
     "  <td name=IMAGE_PS_COLOUR_FILL_LABEL width='18%' align='right'>",
     "<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='PS_ROW_1B';1 -->\r\n  <td name=IMAGE_PS_COLOUR_FILL_LABEL width='30%' align='left'>"),
    ('etc/gui/h3-postscript-images.htm',
     "  <td name=IMAGE_PS_COLOUR_FILL width='5%' align='left'>",
     "  <td name=IMAGE_PS_COLOUR_FILL width='20%' align='left'>"),
    ('etc/gui/h3-postscript-images.htm',
     "  <td name=IMAGE_PS_PERSPECTIVE_LABEL width='10%' align='right'>",
     "  <td name=IMAGE_PS_PERSPECTIVE_LABEL width='30%' align='right'>"),
    ('etc/gui/h3-postscript-images.htm',
     "  <td name=IMAGE_PS_PERSPECTIVE width='5%' align='left'>",
     "  <td name=IMAGE_PS_PERSPECTIVE width='20%' align='left'>"),
    ('etc/gui/h3-postscript-images.htm',
     '  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Go!",\r\n    "name=DEFBTN_PS_GO",\r\n',
     '<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'PS_ROW_1C\';1 -->\r\n  <td width=\'75%\' align=\'right\'>Make PostScript Image</td>\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=Go!",\r\n    "name=DEFBTN_PS_GO",\r\n    "height=math.eval(GetVar(HtmlInputHeight)+8)",\r\n'),
    ('etc/gui/h3-postscript-images.htm',
     '    "td1=<td width=\'10%\'>",',
     '    "td1=<td width=\'25%\'>",'),
    ('etc/gui/h3-povray-images.htm',
     "  <td width='25%' align='right'>\r\n     Make POVRAY Image",
     "  <td width='75%' align='right'>\r\n     Make POVRAY Image"),
    ('etc/gui/h3-povray-images.htm',
     '    "name=DEFBTN_POVRAY_GO",\r\n',
     '    "name=DEFBTN_POVRAY_GO",\r\n    "height=math.eval(GetVar(HtmlInputHeight)+8)",\r\n'),
    # work-report
    ('util/pyUtil/PyToolLib/htmlTools.py',
     "inputDictionary.setdefault('height','35')",
     'inputDictionary.setdefault(\'height\',"math.eval(GetVar(\'HtmlInputHeight\')*2)")'),
    ('util/pyUtil/htmlMaker.py',
     '<a href="spy.contactLetter()>>html.Update" target="Edit Contact Letter"><b>Contact Letter</b></a>',
     '<input type="button" name="CONTACT_LETTER" value="Contact Letter" height="GetVar(HtmlInputHeight)" onclick="spy.contactLetter()>>html.Update" bgcolor="GetVar(linkButton.bgcolor)" fgcolor="GetVar(linkButton.fgcolor)">'),
    ('util/pyUtil/htmlMaker.py',
     '<a href="spy.gui.report.publication.AddAllDefaultAuthors()>>html.Update" target="Add all default authors"><b>Add all default authors</b></a>',
     '<input type="button" name="ADD_DEFAULT_AUTHORS" value="Add all default authors" height="GetVar(HtmlInputHeight)" onclick="spy.gui.report.publication.AddAllDefaultAuthors()>>html.Update" bgcolor="GetVar(linkButton.bgcolor)" fgcolor="GetVar(linkButton.fgcolor)">'),
    # view
    ('etc/gui/quick-drawing-styles.htm',
     '<!-- #include snippet-quick-drawing-styles gui\\snippets\\snippet-quick-drawing-styles.htm;1; -->',
     "<!-- #include snippet-quick-drawing-styles gui\\snippets\\snippet-quick-drawing-styles.htm;1; -->\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='QUICK_DRAWING_STYLES_2';1 -->"),
    ('etc/gui/quick-drawing-styles.htm',
     '    "onclick=default",\r\n    "td1=<td width=\'14%\'>"',
     '    "onclick=default",\r\n    "td1=<td width=\'33%\'>"'),
    ('etc/gui/quick-drawing-styles.htm',
     'etc/styles/)",\r\n    "td1=<td width=\'14%\'>"',
     'etc/styles/)",\r\n    "td1=<td width=\'33%\'>"'),
    ('etc/gui/quick-drawing-styles.htm',
     '"value=...",',
     '"value=More Styles",\r\n    "td1=<td width=\'34%\'>",'),
    # tools
    ('util/pyUtil/htmlMaker.py',
     '  html = ["<td><table width=\'100%%\'><tr>%s</tr></table></td>" % html]',
     '  html = ["<td></td>"]\r\n  nbtn = 3 if name == \'AFIX\' else 1\r\n  rows = [controls[:-nbtn], controls[-nbtn:]] if len(controls) > 3 else [controls]\r\n  for row in rows:\r\n    tds = "".join("<td width=\'%s%%\' align=\'center\'>%s</td>" % (int(100 / len(row)), td) for td in row)\r\n    html.append("</tr><tr bgcolor=\'$GetVar(HtmlTableBgColour)\'>%s<td colspan=\'2\'><table width=\'100%%\' cellspacing=\'6\'><tr>%s</tr></table></td>" % (htmlTools.make_table_first_col(), tds))'),
    ('util/pyUtil/htmlMaker.py',
     "  html.append(htmlTools.make_table_first_col(help_name=name, popout=True, help_image='normal'))",
     '  html.append(htmlTools.make_table_first_col())'),
    ('etc/gui/OLEX2-conres.htm',
     '<td width=\'18%\' bgcolor="$GetVar(HtmlTableBgColour)">',
     '<td width=\'40%\' bgcolor="$GetVar(HtmlTableBgColour)">'),
    ('etc/gui/SHELX-compatible-constraints.htm',
     '<td width=\'18%\' bgcolor="$GetVar(HtmlTableBgColour)">',
     '<td width=\'40%\' bgcolor="$GetVar(HtmlTableBgColour)">'),
    ('etc/gui/SHELX-compatible-restraints.htm',
     '<td width=\'18%\' bgcolor="$GetVar(HtmlTableBgColour)">',
     '<td width=\'40%\' bgcolor="$GetVar(HtmlTableBgColour)">'),
    ('etc/gui/chemical-tools.htm',
     '    "onclick=calcCHN",',
     '    "onclick=calcCHN",\r\n    "td2=</td></tr><tr>",'),
    ('etc/gui/chemical-tools.htm',
     '    "onclick=rsa",',
     '    "onclick=rsa",\r\n    "td2=</td></tr><tr>",'),
    ('etc/gui/chemical-tools.htm',
     '    "onclick=wbox>>sfil",',
     '    "onclick=wbox>>sfil",\r\n    "td2=</td></tr><tr>",'),
    ('etc/gui/tools/folder-view.htm',
     "  <input type='button' value='Select a folder'",
     "  <input type='button' value='Select a folder' width='100%' height='GetVar(HtmlInputHeight)'"),
    ('etc/gui/h3-masks.htm',
     '<td name=REFLECTION_FILE_LABEL width="20%">',
     '<td name=REFLECTION_FILE_LABEL width="30%">'),
    ('etc/gui/h3-masks.htm',
     '<td name=REFLECTION_FILE_INPUT align="right" width="30%">',
     '<td name=REFLECTION_FILE_INPUT align="right" width="70%">'),
    ('etc/gui/h3-masks.htm',
     '  <td width="30%">%Use set completion%</td>',
     '\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<tr bgcolor="$GetVar(HtmlTableBgColour)">\r\n<!-- #include tool-column gui\\blocks\\tool-first-column.htm;1; -->\r\n<!-- #include row_table_on gui\\blocks\\row_table_on.htm;1; -->\r\n  <td width="40%">%Use set completion%</td>'),
    ('etc/gui/h3-masks.htm',
     '  <td align=\'left\' width="5%">',
     '  <td align=\'left\' width="10%">'),
    ('etc/gui/h3-masks.htm',
     '      name="SET_SNUM_MASKS_USE_COMPLETE_SET"',
     '      height="GetVar(\'HtmlCheckboxHeight\')"\r\n      name="SET_SNUM_MASKS_USE_COMPLETE_SET"'),
    # info
    ('etc/gui/recent-files.htm',
     '<!-- #include recent-files-info recent_files.htm;1; -->',
     '<table width="100%" cellpadding="12" cellspacing="0">\r\n<!-- #include recent-files-info recent_files.htm;1; -->\r\n</table>'),
    ('etc/gui/anom-disp.htm',
     '<td width="12%">',
     '<td width="20%">'),
    ('etc/gui/anom-disp.htm',
     '"onclick=spy.MuPlot()",\r\n    "bgcolor=GetVar(HtmlTableRowBgColour)",\r\n    "td1=<td width="44%"',
     '"onclick=spy.MuPlot()",\r\n    "bgcolor=GetVar(HtmlTableRowBgColour)",\r\n    "td1=<td width="30%"'),
    ('etc/gui/anom-disp.htm',
     '"onclick=spy.AnomDispPlot()",\r\n    "bgcolor=GetVar(HtmlTableRowBgColour)",\r\n    "td1=<td width="44%"',
     '"onclick=spy.AnomDispPlot()",\r\n    "bgcolor=GetVar(HtmlTableRowBgColour)",\r\n    "td1=<td width="50%"'),
    ('etc/gui/electron-density-peaks.htm',
     '    height="20"\r\n    value="3"',
     '    height="GetVar(\'HtmlInputHeight\')"\r\n    value="3"'),
    ('etc/gui/electron-density-peaks.htm',
     '<table width="100%" cellsapcing="0" cellpadding="0">\r\n    <font size=1>',
     '<table width="100%" cellspacing="0" cellpadding="6">\r\n    <font size="$GetVar(HtmlGuiFontSize)">'),
    ('etc/gui/bad-reflections.htm',
     '"onclick=omit html.GetValue(omit_filter)",',
     '"onclick=omit html.GetValue(omit_filter)",\r\n    "td1=<td width=\'20%\'>",'),
    ('etc/gui/bad-reflections.htm',
     '"name=omit_filter",',
     '"name=omit_filter",\r\n    "td1=<td width=\'15%\'>",'),
    ('etc/gui/bad-reflections.htm',
     '"onclick=DelIns OMIT",',
     '"onclick=DelIns OMIT",\r\n    "td1=<td width=\'20%\'>",'),
    ('etc/gui/bad-reflections.htm',
     'style="border-collapse: collapse" width="100%" cellpadding="0" cellspacing="1"',
     'style="border-collapse: collapse" width="100%" cellpadding="6" cellspacing="1"'),
    ('etc/gui/bad-reflections.htm',
     '  <td>\r\n    <a href="HklExclude -c=html.GetState(exclude_and) -h=html.GetValue(exclude_h) -k=html.GetValue(exclude_k) -l=html.GetValue(exclude_l)" >%Exclude%:</a>\r\n  </td>\r\n\r\n\r\n  <td>\r\n    <input\r\n      type="text"\r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      name = "exclude_h"\r\n      width="22"\r\n',
     '  <td width="33%">\r\n    <input\r\n      type="text"\r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      name = "exclude_h"\r\n      width="75%"\r\n'),
    ('etc/gui/bad-reflections.htm',
     '  <td>\r\n    <input\r\n      type="text"\r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      name = "exclude_k"\r\n      width="22"\r\n',
     '  <td width="33%">\r\n    <input\r\n      type="text"\r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      name = "exclude_k"\r\n      width="75%"\r\n'),
    ('etc/gui/bad-reflections.htm',
     '  <td>\r\n    <input\r\n      type="text"\r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      name = "exclude_l"\r\n      width="22"\r\n',
     '  <td width="33%">\r\n    <input\r\n      type="text"\r\n      height="GetVar(\'HtmlInputHeight\')"\r\n      name = "exclude_l"\r\n      width="75%"\r\n'),
    ('etc/gui/bad-reflections.htm',
     '      label="l = "\r\n    >\r\n  </td>\r\n  <td>\r\n    <input\r\n      type=\'checkbox\'',
     '      label="l = "\r\n    >\r\n  </td>\r\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\r\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'EXCLUDE_HKL_2\';1 -->\r\n  <td width="30%">\r\n    <input\r\n      type=\'checkbox\''),
    ('etc/gui/bad-reflections.htm',
     '  <td>\r\n    <a href="html.SetValue(\'exclude_h\',\'\')>>html.SetValue(\'exclude_k\',\'\')>>html.SetValue(\'exclude_l\',\'\')>>HklAppend">%Reset%</a>\r\n  </td>',
     '  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=%Exclude%",\r\n    "onclick=HklExclude -c=html.GetState(exclude_and) -h=html.GetValue(exclude_h) -k=html.GetValue(exclude_k) -l=html.GetValue(exclude_l)",\r\n    "td1=<td width=\'35%\'>",\r\n    )\r\n  $-\r\n  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=%Reset%",\r\n    "onclick=html.SetValue(\'exclude_h\',\'\')>>html.SetValue(\'exclude_k\',\'\')>>html.SetValue(\'exclude_l\',\'\')>>HklAppend",\r\n    "td1=<td width=\'35%\'>",\r\n    )\r\n  $-'),
    ('etc/gui/bad-reflections.htm',
     '  <td align=\'left\'>\r\n    <a href="HklEdit">%Edit Reflections%</a>\r\n  </td>',
     '  $+\r\n    html.Snippet(GetVar(default_link),\r\n    "value=%Edit Reflections%",\r\n    "onclick=HklEdit",\r\n    "td1=<td width=\'40%\'>",\r\n    )\r\n  $-\r\n  <td></td>'),
    ('etc/gui/reflection-statistics.htm',
     '<td name=REFLECTION_FILE_LABEL width="10%">',
     '<td name=REFLECTION_FILE_LABEL width="15%">'),
    ('etc/gui/reflection-statistics.htm',
     '<td name=REFLECTION_FILE_INPUT align=\'left\' width="25%">',
     '<td name=REFLECTION_FILE_INPUT align=\'left\' width="40%">'),
    ('etc/gui/reflection-statistics.htm',
     '<td align=\'right\' width="25%">\r\n  </td>',
     '<td align=\'right\' width="5%">\r\n  </td>'),
    ('etc/gui/reflection-statistics.htm',
     '<td align=\'right\' width="25%">\r\n  Write graphics file?',
     '<td align=\'right\' width="30%">\r\n  Write graphics file?'),
    ('etc/gui/reflection-statistics.htm',
     '<td name=SAVE_FILE align=\'left\' width="5%">',
     '<td name=SAVE_FILE align=\'left\' width="10%">'),
    ('util/pyUtil/PyToolLib/Analysis.py',
     "'width': '$math.eval(html.clientwidth(self)-140)',",
     "'width': '100%',"),
    ('util/pyUtil/PyToolLib/Analysis.py',
     "OV.GetParam('gui.htmlpanelwidth_margin_adjust'))/4",
     "OV.GetParam('gui.htmlpanelwidth_margin_adjust'))/5"),
    ('util/pyUtil/PyToolLib/Analysis.py',
     "  options_gui = '\\n<td>%s</td>\\n' %'</td>\\n<td>'.join(options_gui)",
     "  options_gui = '</tr><tr>'.join('<td>%s</td>' % '</td><td>'.join(options_gui[j:j + 2]) for j in range(0, len(options_gui), 2))"),
    ('util/pyUtil/gui/templates/gui_templates.htm',
     '      %(title)s\r\n      </b>\r\n      </font>\r\n    </td>\r\n',
     '      %(title)s\r\n      </b>\r\n      </font>\r\n    </td>\r\n    <td bgcolor=\'%(bg_colour)s\' align=\'right\' valign=\'center\' width=\'30%%\'>\r\n      <a href="html.Hide md_box"><font color=\'white\' size=\'6\'><b>&#215; Close</b></font></a>\r\n    </td>\r\n'),
    # _merged
    # each button row is its own table, so its cellspacing also spaces the rows
    ('etc/gui/blocks/row_table_on.htm',
     'cellspacing="2"',
     'cellspacing="3"'),
    ('util/pyUtil/PyToolLib/htmlTools.py',
     '<td valign=\'top\' align=\'center\' bgcolor="$GetVar(\'HtmlTableFirstcolColour\')">',
     '<td valign=\'center\' width="$GetVar(\'HtmlTableFirstcolWidth\')" align=\'center\' bgcolor="$GetVar(\'HtmlTableFirstcolColour\')">'),
    ('util/pyUtil/PyToolLib/programSettings.py',
     "    row_txt += '''<td valign='bottom' align='left'>%s</td>''' %input_txt",
     "    if count > 1 and count % 2:\r\n      row_txt += '</tr><tr>'\r\n    row_txt += '''<td valign='bottom' align='left' width='50%%'>%s</td>''' %input_txt"),
    ('util/pyUtil/gui/templates/program-output.htm',
     "<table width='100%%' border='0' cellpadding='0'>%(program_output_content)s</table>",
     "<table width='100%%' border='0' cellpadding='8'>%(program_output_content)s</table>"),
    ('etc/gui/snippets/snippet-q-peak-slider.htm',
     '<td width="80%" name=Q_PAEAK_SLIDER_BUTTONS',
     '<td width="60%" name=Q_PAEAK_SLIDER_BUTTONS'),
    ('etc/gui/snippets/snippet-q-peak-slider.htm',
     'name = "SlideQPeaks"\r\n            width="100%"\r\n            height="18"',
     'name = "SlideQPeaks"\r\n            width="100%"\r\n            height="$GetVar(HtmlInputHeight)"'),
    ('etc/gui/snippets/snippet-q-peak-slider.htm',
     'name = "QPeaksValue"\r\n            width="90%"\r\n            height="18"',
     'name = "QPeaksValue"\r\n            width="90%"\r\n            height="$GetVar(HtmlInputHeight)"'),
    ('etc/gui/snippets/snippet-quick-atom-properties.htm',
     "<td width='48%' align='left'>",
     "<td width='38%' align='left'>"),
    ('etc/gui/snippets/snippet-quick-atom-properties.htm',
     "<td width='10%'>",
     "<td width='12%'>"),
    ('etc/gui/snippets/snippet-quick-atom-properties.htm',
     "<td width='50%'>",
     "<td width='38%'>"),
    ('etc/gui/snippets/snippet-quick-bond-properties.htm',
     "<td width='48%' align='left'>",
     "<td width='38%' align='left'>"),
    ('etc/gui/snippets/snippet-quick-bond-properties.htm',
     "<td width='10%'>",
     "<td width='12%'>"),
    ('etc/gui/snippets/snippet-quick-bond-properties.htm',
     "<td width='50%'>",
     "<td width='38%'>"),
    ('etc/gui/snippets/snippet-arad-slider.htm',
     'height="18"',
     'height="$GetVar(HtmlInputHeight)"'),
    ('etc/gui/snippets/snippet-brad-slider.htm',
     'height="18"',
     'height="$GetVar(HtmlInputHeight)"'),
    ('etc/gui/snippets/snippet-quality-slider.htm',
     'height="18"',
     'height="$GetVar(HtmlInputHeight)"'),
    ('etc/gui/snippets/snippet-quality-slider.htm',
     'width="$math.eval(html.clientwidth(self)/6)"',
     'width="100%"'),
    ('etc/gui/snippets/snippet-fit-group.htm',
     'with\r\n<td>',
     'with\r\n</td>'),
]

# the 14 tablet pages that were whole copies in rundir-overlay/etc/gui,
# as diffs against trunk r7881 (pages unchanged since r7680); these pages are
# CRLF, written here with LF
def _crlf(rel, ps):
    return [(rel, o.replace('\n', '\r\n'), n.replace('\n', '\r\n')) for o, n in ps]

PATCHES += _crlf('etc/gui/disorder.htm', [
(r'''    <a href='shell "https://xray.chem.wisc.edu/idealized-molecular-geometry-library/"'>Ilia's Fragment Library</a>
''',
 r'''    <font size="$GetVar('HtmlFontSizeControls')"><b>
    <input
      type="button"
      name="FRAGMENT_LIBRARY_LINK"
      value="Ilia's Fragment Library"
      width="100%"
      height="GetVar(HtmlInputHeight)"
      onclick="shell https://xray.chem.wisc.edu/idealized-molecular-geometry-library/"
      bgcolor="GetVar(linkButton.bgcolor)"
      fgcolor="GetVar(linkButton.fgcolor)"
      fit="false"
      flat="GetVar(linkButton.flat)"
      custom="GetVar(custom_button)"
    >
    </b></font>
'''),
(r'''<!-- #include tool-row-help gui\blocks\tool-row-help.htm;name='LINK_SELECTED';help_ext=Link-Selected;1 -->
  <td width='100%'>
    <b>%Link% %Selected%:</b> 
''',
 r'''<!-- #include tool-row-help gui\blocks\tool-row-help.htm;name='LINK_SELECTED';help_ext=Link-Selected;1 -->
  <td width='30%'>
    <b>%Link% %Selected%:</b> 
'''),
(r'''      <a href="fvar sel">Occupancies</a> | 
      <a href="part -p=2 -lo sel>>fuse">PARTs & Occupancies</a>
''',
 r''''''),
(r'''    <b>%Link% %Selected%:</b> 
  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

''',
 r'''    <b>%Link% %Selected%:</b> 
  </td>
  $+
    html.Snippet(GetVar(default_link),
    "value=Occupancies",
    "name=LINK_SEL_OCC",
    "onclick=fvar sel",
    "td1=<td width='35%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=PARTs & Occupancies",
    "name=LINK_SEL_PARTS",
    "onclick=part -p=2 -lo sel>>fuse",
    "td1=<td width='35%'>"
    )
  $-
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

'''),
(r'''  <td width='100%'>
    <b>Link PARTs & Occupanies + </b> &nbsp;
      <a href="if strcmp(sel(),'') then none else 'link_parts_occupancies_with EADP'">EADP</a>&nbsp;
      <a href="if strcmp(sel(),'') then none else 'link_parts_occupancies_with ISOR'">ISOR</a>
''',
 r'''  <td width='40%'>
    <b>Link PARTs & Occupanies + </b>
'''),
(r'''    <b>Link PARTs & Occupanies + </b>
  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

''',
 r'''    <b>Link PARTs & Occupanies + </b>
  </td>
  $+
    html.Snippet(GetVar(default_link),
    "value=EADP",
    "name=LINK_PARTS_EADP",
    "onclick=if strcmp(sel(),'') then none else 'link_parts_occupancies_with EADP'",
    "td1=<td width='30%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=ISOR",
    "name=LINK_PARTS_ISOR",
    "onclick=if strcmp(sel(),'') then none else 'link_parts_occupancies_with ISOR'",
    "td1=<td width='30%'>"
    )
  $-
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

'''),
(r'''  <td width='100%'>
    <a href="part sel html.GetValue(part)"><b>%Assign% <i>selection</i> to PART</b></a>
  </td>
  <td VALIGN='center'>
    <font size='2'>
''',
 r'''  $+
    html.Snippet(GetVar(default_link),
    "value=%Assign% selection to PART",
    "name=ASSIGN_PART",
    "onclick=part sel html.GetValue(part)",
    "td1=<td width='70%'>"
    )
  $-
  <td width='30%' VALIGN='center'>
    <font size="$GetVar('HtmlFontSizeControls')">
'''),
(r'''      name="part" width="40" height="16" value="1" min=-100>
''',
 r'''      name="part" width="100%" height="GetVar(HtmlSpinHeight)" value="1" min=-100>
'''),
(r'''  <td width='100%'>
    <a href="part sel html.GetValue(part)"><b>%Assign% <i>selection</i> to two PARTs in order</b></a>&nbsp
  </td>
''',
 r'''  $+
    html.Snippet(GetVar(default_link),
    "value=%Assign% selection to two PARTs in order",
    "name=ASSIGN_TWO_PARTS",
    "onclick=part sel html.GetValue(part)",
    "td1=<td width='100%'>"
    )
  $-
'''),
(r'''    <b>%Show% PART </b>
    <a href="showP 0 1">0 1</a>&nbsp;
    <a href="showP 0 2">0 2</a>&nbsp;
    <a href="showP 0 -1">0 -1</a>&nbsp;
    <a href="showP 0 -2">0 -2</a>&nbsp;
    <a href="showP 1 2">1 2</a>&nbsp;
    <a href="showP 1">1</a>&nbsp;
    <a href="showP 2">2</a>&nbsp;
    <a href="showP">ALL</a>&nbsp;
''',
 r'''    <b>%Show% PART</b>
'''),
(r'''  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

!-- #include tool-row-help gui\blocks\tool-row-help.htm;name='SPLIT_GROUP';help_ext=Split-Group;1 -->
''',
 r'''  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='SHOW_PARTS_2';1; -->
  $+
    html.Snippet(GetVar(default_link),
    "value=0 1",
    "name=SHOWP_0_1",
    "onclick=showP 0 1",
    "td1=<td width='33%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=0 2",
    "name=SHOWP_0_2",
    "onclick=showP 0 2",
    "td1=<td width='33%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=0 -1",
    "name=SHOWP_0_M1",
    "onclick=showP 0 -1",
    "td1=<td width='33%'>"
    )
  $-
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='SHOW_PARTS_3';1; -->
  $+
    html.Snippet(GetVar(default_link),
    "value=0 -2",
    "name=SHOWP_0_M2",
    "onclick=showP 0 -2",
    "td1=<td width='33%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=1 2",
    "name=SHOWP_1_2",
    "onclick=showP 1 2",
    "td1=<td width='33%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=1",
    "name=SHOWP_1",
    "onclick=showP 1",
    "td1=<td width='33%'>"
    )
  $-
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='SHOW_PARTS_4';1; -->
  $+
    html.Snippet(GetVar(default_link),
    "value=2",
    "name=SHOWP_2",
    "onclick=showP 2",
    "td1=<td width='33%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=ALL",
    "name=SHOWP_ALL",
    "onclick=showP",
    "td1=<td width='33%'>"
    )
  $-
  <td width='33%'></td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

!-- #include tool-row-help gui\blocks\tool-row-help.htm;name='SPLIT_GROUP';help_ext=Split-Group;1 -->
'''),
(r'''!-- #include tool-row-help gui\blocks\tool-row-help.htm;name='SPLIT_GROUP';help_ext=Split-Group;1 -->
''',
 r'''<!-- #include tool-row-help gui\blocks\tool-row-help.htm;name='SPLIT_GROUP';help_ext=Split-Group;1 -->
'''),
(r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->


<!-- #include tool-footer gui\blocks\tool-footer.htm;colspan=2;1; -->''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->


<!-- #include tool-footer gui\blocks\tool-footer.htm;colspan=2;1; -->'''),
])
PATCHES += _crlf('etc/gui/h3-calculate-solvent-accessible-voids.htm', [
(r'''  <td width="20%">%Probe%/&Aring</td>
  <td width="12%">
''',
 r'''  <td width="25%">%Probe%/&Aring</td>
  <td width="25%">
'''),
(r'''  <td width="20%">%Grid%/&Aring</td>
  <td width="12%">
''',
 r'''  <td width="25%">%Grid%/&Aring</td>
  <td width="25%">
'''),
(r'''  <td width="15%"> </td>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<tr bgcolor="$GetVar(HtmlTableBgColour)">
<!-- #include tool-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
  <td width="50%"> </td>
'''),
])
PATCHES += _crlf('etc/gui/h3-calculate-voids.htm', [
(r'''  <td width="20%">%Resolution%/&Aring</td>
  <td width="12%">
''',
 r'''  <td width="25%">%Resolution%/&Aring</td>
  <td width="25%">
'''),
(r'''  <td width="20%">%Distance%/&Aring</td>
  <td width="12%">
''',
 r'''  <td width="25%">%Distance%/&Aring</td>
  <td width="25%">
'''),
(r'''  <td width="15%">
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<tr bgcolor="$GetVar(HtmlTableBgColour)">
<!-- #include tool-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
  <td width="50%">
'''),
(r'''      type="checkbox"
      value="%Precise%"
''',
 r'''      type="checkbox"
      height="GetVar('HtmlCheckboxHeight')"
      value="%Precise%"
'''),
])
PATCHES += _crlf('etc/gui/h3-electron-density.htm', [
(r'''  <td align='left' width="10%">%Map%</td>
  <td width="15%">
''',
 r'''  <td align='left' width="15%">%Map%</td>
  <td width="35%">
'''),
(r'''  <td align='center' width="10%">%Source%</td>
  <td width="15%">$+
''',
 r'''  <td align='center' width="15%">%Source%</td>
  <td width="35%">$+
'''),
(r'''  <td align='center' width="10%">%Res%/&Aring</td>
  <td width="10%">
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<tr bgcolor="$GetVar(HtmlTableBgColour)">
<!-- #include tool-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
  <td align='left' width="15%">%Res%/&Aring</td>
  <td width="35%">
'''),
(r'''    "onclick=spy.gui.maps.MapView()",
    )$-
''',
 r'''    "onclick=spy.gui.maps.MapView()",
    "td1=<td width='50%'>"
    )$-
'''),
(r'''  <td align='left' width="10%">%Scale%</td>
  <td width="35%">$+
''',
 r'''  <td align='left' width="15%">%Scale%</td>
  <td width="85%">$+
'''),
(r'''    $-</td>
  <td width="35%">$+
    html.Snippet("gui/snippets/input-checkbox", "name=SNUM_MAP_MASK",
''',
 r'''    $-</td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<tr bgcolor="$GetVar(HtmlTableBgColour)">
<!-- #include tool-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
  <td width="70%">$+
    html.Snippet("gui/snippets/input-checkbox", "name=SNUM_MAP_MASK",
'''),
(r'''  <td>
''',
 r'''  <td width="30%">
'''),
])
PATCHES += _crlf('etc/gui/hydrogen-atoms.htm', [
(r'''    <table width="100%">
''',
 r'''    <table width="100%" cellspacing="6">
'''),
(r'''      <td width='5%'>A</td>
      <td VALIGN='center' width='15%'>
''',
 r'''      <td width='10%'>A</td>
      <td VALIGN='center' width='25%'>
'''),
(r'''      <td width='5%'>D</td>
      <td VALIGN='center' width='15%'>
''',
 r'''      <td width='10%'>D</td>
      <td VALIGN='center' width='25%'>
'''),
(r'''      <td VALIGN='center' width='10%'>
''',
 r'''      <td VALIGN='center' width='30%'>
'''),
(r'''      <td width="15%" align="right">
''',
 r'''    </tr>
    <tr>
      <td colspan='2' align='left'>
'''),
(r'''      </td>
      <td>
        <font size="$GetVar('HtmlFontSizeControls')">
''',
 r'''      </td>
      <td colspan='3'>
        <font size="$GetVar('HtmlFontSizeControls')">
'''),
(r'''    <table><tr>
      <td>
        <a href="hadd">%Add Hydrogen%</a>
''',
 r'''    <table width='100%' cellspacing='6'><tr>
  $+
    html.Snippet(GetVar(default_link),
    "value=%Add Hydrogen%",
    "name=H_ADD",
    "onclick=hadd",
    "td1=<td width='33%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=H %Labels%",
    "name=H_LABELS",
    "onclick=labels -h -l",
    "td1=<td width='33%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=No H %Labels%",
    "name=H_NO_LABELS",
    "onclick=labels -l",
    "td1=<td width='33%'>"
    )
  $-
    </tr><tr>
  $+
    html.Snippet(GetVar(default_link),
    "value=%Show% AFIX",
    "name=H_SHOW_AFIX",
    "onclick=labels -h -a",
    "td1=<td width='33%'>"
    )
  $-
      <td width='33%'>
        <font size="$GetVar('HtmlFontSizeControls')"><b>
        <input
          type="button"
          name="H_FREE_ALL"
          value="FREE ALL H"
          width="100%"
          height="GetVar(HtmlInputHeight)"
          onclick="afix 0 $H>>free uiso $H"
          bgcolor="GetVar(linkButton.bgcolor)"
          fgcolor="GetVar(linkButton.fgcolor)"
          fit="false"
          flat="GetVar(linkButton.flat)"
          hint="Remove AFIX instructions from all hydrogen atoms, free Uiso values"
          custom="GetVar(custom_button)"
        >
        </b></font>
'''),
(r'''      <td>
        <a href="labels -h -l">H %Labels%</a>
      </td>
      <td>
        <a href="labels -l">No H %Labels%</a>
      </td>
      <td>
        <a href="labels -h -a">%Show% AFIX</a>
      </td>
      <td>
        <a target="Remove AFIX instructions from all hydrogen atoms, free Uiso values" href="afix 0 $H>>free uiso $H">FREE ALL H</a>
      </td>
      <td>
        <a href="ShowQ a False >> showQ b False">%Hide% Q</a>
      </td>

    </tr>
  </table>
''',
 r'''  $+
    html.Snippet(GetVar(default_link),
    "value=%Hide% Q",
    "name=H_HIDE_Q",
    "onclick=ShowQ a False >> showQ b False",
    "td1=<td width='33%'>"
    )
  $-
    </tr></table>
  </td>
'''),
])
PATCHES += _crlf('etc/gui/metadata-h3-collection.htm', [
(r'''      <font size="2">
''',
 r'''      <font size="$GetVar('HtmlFontSizeControls')">
'''),
(r'''    <font size="2">
    <td align="right" width="80%">
      <table cellpadding='0' cellspacing='0' width='100%'>
        <tr>
''',
 r''''''),
(r'''      <b>Submitted</b>
    </td>
    <td width='17%'>
      <input
        type="date"
''',
 r'''      <b>Submitted</b>
    </td>
    <td width='36%'>
      <input
        type="date"
'''),
(r'''    <td VALIGN="center" ALIGN="right" width="17%">
''',
 r'''    <td VALIGN="center" ALIGN="right" width="14%">
'''),
(r'''      <b>Collected</b>
    </td>
    <td width='17%'>
      <input
        type="date"
''',
 r'''      <b>Collected</b>
    </td>
    <td width='36%'>
      <input
        type="date"
'''),
(r'''    <td VALIGN="center" ALIGN="right" width="18%">
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<!-- #include tool-row gui\blocks\tool-row.htm;name='METADATA_DATES_2';1 -->
    <td VALIGN="center" ALIGN="left" width="14%">
'''),
(r'''    <td width='17%'>
''',
 r'''    <td width='36%'>
'''),
(r'''        </tr>
      </table>
    </td>
    </font>
''',
 r'''    <td width='50%'></td>
'''),
])
PATCHES += _crlf('etc/gui/overlay.htm', [
(r'''  <td width='50%'>
    <a 
      href="match" 
      target="Match All Fragments-target">
      %Match All Fragments%
    </a>
  </td>
  <td width='50%'>
    <a href="match -u">%UnMatch%</a>
  </td>
''',
 r'''  $+
    html.Snippet(GetVar(default_link),
    "value=%Match All Fragments%",
    "name=MATCH_ALL",
    "onclick=match",
    "td1=<td width='50%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=%UnMatch%",
    "name=MATCH_UNMATCH",
    "onclick=match -u",
    "td1=<td width='50%'>"
    )
  $-
'''),
(r'''  <td width='100%'>
    <a 
      target="Match Selected Atoms-target" 
      href="match sel">
      %Match Selected Atoms%</a>
  </td>
''',
 r'''  $+
    html.Snippet(GetVar(default_link),
    "value=%Match Selected Atoms%",
    "name=MATCH_SEL",
    "onclick=match sel",
    "td1=<td width='100%'>"
    )
  $-
'''),
(r'''  <td width='100%'>
    <a href="@reap -*>>mode match>>html.update">%Overlay Structure%</a>&nbsp;|&nbsp;
    <a href="mode match">%Mode Match%</a>
  </td>
''',
 r'''  $+
    html.Snippet(GetVar(default_link),
    "value=%Overlay Structure%",
    "name=MATCH_OVERLAY",
    "onclick=@reap -*>>mode match>>html.update",
    "td1=<td width='50%'>"
    )
  $-
  $+
    html.Snippet(GetVar(default_link),
    "value=%Mode Match%",
    "name=MATCH_MODE",
    "onclick=mode match",
    "td1=<td width='50%'>"
    )
  $-
'''),
])
PATCHES += _crlf('etc/gui/peanut.htm', [
(r'''<!-- #include tool-row-help gui\blocks\tool-row-help.htm;name='Peanut_1';help_ext=Anomalous-Dispersion;1 -->
  <td width="10%" align='left'>
    <b>Type </b>
''',
 r'''<!-- #include tool-row-help gui\blocks\tool-row-help.htm;name='Peanut_1';help_ext=Anomalous-Dispersion;1 -->
  <td width="20%" align='left'>
    <b>Type </b>
'''),
(r'''  <td width="12%" align="'center'"> $+
''',
 r'''  <td width="30%" align="'center'"> $+
'''),
(r'''  </td>
  <td width="10%" align='left'>
    <b>Quality </b>
''',
 r'''  </td>
  <td width="20%" align='left'>
    <b>Quality </b>
'''),
(r'''  <td width='8%' align='left'>
''',
 r'''  <td width='30%' align='left'>
    <font size="$GetVar('HtmlFontSizeControls')">
'''),
(r'''      width="50"
      height="17"
''',
 r'''      width="100%"
      height="GetVar(HtmlSpinHeight)"
'''),
(r'''    >
  </td>
''',
 r'''    >
    </font>
  </td>
'''),
(r'''  <td width="10%" align='right'>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='Peanut_1b';1; -->
  <td width="20%" align='left'>
'''),
(r'''  <td width="20%" align='center'>
''',
 r'''  <td width="30%" align='center'>
'''),
(r'''  <td width="10%" align='left'>
''',
 r'''  <td width="20%" align='left'>
'''),
(r'''  <td width="10%" align="'center'"> $+
''',
 r'''  <td width="30%" align="'center'"> $+
'''),
(r'''    "onclick=spy.Peanut.show_MSDS()",
    "bgcolor=GetVar(HtmlTableRowBgColour)",
    "td1=<td width="20%" align='center'>",
    )
  $-
''',
 r'''    "onclick=spy.Peanut.show_MSDS()",
    "bgcolor=GetVar(HtmlTableRowBgColour)",
    "td1=<td width='50%' align='center'>",
    )
  $-
'''),
(r'''    "onclick=spy.Peanut.remove_MSDS()",
    "bgcolor=GetVar(HtmlTableRowBgColour)",
    "td1=<td width="20%" align='center'>",
    )
  $-  
''',
 r'''    "onclick=spy.Peanut.remove_MSDS()",
    "bgcolor=GetVar(HtmlTableRowBgColour)",
    "td1=<td width='50%' align='center'>",
    )
  $-  
'''),
(r'''  $-  
  $+
''',
 r'''  $-  
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='Peanut_2b';1; -->
  $+
'''),
(r'''    "onclick=spy.Peanut.show_diff()",
    "bgcolor=GetVar(HtmlTableRowBgColour)",
    "td1=<td width="20%" align='center'>",
    )
  $-
''',
 r'''    "onclick=spy.Peanut.show_diff()",
    "bgcolor=GetVar(HtmlTableRowBgColour)",
    "td1=<td width='50%' align='center'>",
    )
  $-
'''),
(r'''    "td1=<td width="20%" align='center'>",
''',
 r'''    "td1=<td width='50%' align='center'>",
'''),
])
PATCHES += _crlf('etc/gui/rotate.htm', [
(r'''<!-- #include tool-top gui\blocks\tool-top.htm;image=#image;onclick=#onclick;1; -->

''',
 r'''<!-- #include tool-top gui\blocks\tool-top.htm;image=#image;onclick=#onclick;1; -->

<!-- Android tablet: one axis per row ([-] spin [+]) -->

'''),
(r'''  <td name=LABEL_ROTATE width='15%'>
''',
 r'''  <td name=LABEL_ROTATE width='25%'>
'''),
(r'''  <td name=SPIN_ROTATION_X align='left'>
''',
 r'''  <td width='25%' align='right'>
'''),
(r'''    $spy.MakeHoverButton('tiny-:x','rota 1 math.eval(html.GetValue(rx)*-1)')
    &thinsp;
    <font size='$GetVar(HtmlFontSizeControls)'>
''',
 r'''    $spy.MakeHoverButton('tiny-:x','rota 1 math.eval(html.GetValue(rx)*-1)')
  </td>
  <td name=SPIN_ROTATION_X width='25%'>
    <font size='$GetVar(HtmlFontSizeControls)'>
'''),
(r'''      name = "rx"
      width="45"
      height="$GetVar(HtmlSPinHeight)"
      value="10"
''',
 r'''      name = "rx"
      width="100%"
      height="$GetVar(HtmlSpinHeight)"
      value="10"
'''),
(r'''    </font>
    &thinsp;
    $spy.MakeHoverButton('tiny-x:','rota 1 html.getvalue(rx)')
''',
 r'''    </font>
  </td>
  <td width='25%' align='left'>
    $spy.MakeHoverButton('tiny-x:','rota 1 html.getvalue(rx)')
'''),
(r'''  <td name=SPIN_ROTATION_Y align='center'>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<!-- #include tool-row gui\blocks\tool-row.htm;name='ROTATE_Y';1 -->
  <td width='25%'></td>
  <td width='25%' align='right'>
'''),
(r'''    $spy.MakeHoverButton('tiny-:y','rota 2 math.eval(html.GetValue(ry)*-1)')
    &thinsp;
    <font size='$GetVar(HtmlFontSizeControls)'>
''',
 r'''    $spy.MakeHoverButton('tiny-:y','rota 2 math.eval(html.GetValue(ry)*-1)')
  </td>
  <td name=SPIN_ROTATION_Y width='25%'>
    <font size='$GetVar(HtmlFontSizeControls)'>
'''),
(r'''      name = "ry"
      width="45"
      height="$GetVar(HtmlSPinHeight)"
      bgcolor="$GetVar(HtmlInputBgColour)"
''',
 r'''      name = "ry"
      width="100%"
      height="$GetVar(HtmlSpinHeight)"
      bgcolor="$GetVar(HtmlInputBgColour)"
'''),
(r'''    </font>
    &thinsp;
    $spy.MakeHoverButton('button_tiny-y:','rota 2 html.getvalue(ry)')
''',
 r'''    </font>
  </td>
  <td width='25%' align='left'>
    $spy.MakeHoverButton('button_tiny-y:','rota 2 html.getvalue(ry)')
'''),
(r'''  <td name=SPIN_ROTATION_Z align='right'>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<!-- #include tool-row gui\blocks\tool-row.htm;name='ROTATE_Z';1 -->
  <td width='25%'></td>
  <td width='25%' align='right'>
'''),
(r'''    $spy.MakeHoverButton('tiny-:z','rota 3 math.eval(html.GetValue(rz)*-1)')
    &thinsp;
    <font size='$GetVar(HtmlFontSizeControls)'>
''',
 r'''    $spy.MakeHoverButton('tiny-:z','rota 3 math.eval(html.GetValue(rz)*-1)')
  </td>
  <td name=SPIN_ROTATION_Z width='25%'>
    <font size='$GetVar(HtmlFontSizeControls)'>
'''),
(r'''      width="45"
      height="$GetVar(HtmlSPinHeight)"
''',
 r'''      width="100%"
      height="$GetVar(HtmlSpinHeight)"
'''),
(r'''    &thinsp;
''',
 r'''  </td>
  <td width='25%' align='left'>
'''),
(r'''  <td name=LABEL_AUTO_ROTATION width='15%'>
''',
 r'''  <td name=LABEL_AUTO_ROTATION width='25%'>
'''),
(r'''  <td name=CHECK_X>
''',
 r'''  <td name=CHECK_X width='25%'>
'''),
(r'''  <td name=CHECK_Y>
''',
 r'''  <td name=CHECK_Y width='25%'>
'''),
(r'''  <td name=CHECK_Z>
''',
 r'''  <td name=CHECK_Z width='25%'>
'''),
(r'''    $-
  </td>

  <td name=LABEL_AUTO_ROTATION_SPEED width='15%'>
''',
 r'''    $-
  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

  <td name=LABEL_AUTO_ROTATION_SPEED width='15%'>
'''),
(r'''  <td name=LABEL_AUTO_ROTATION_SPEED width='15%'>
''',
 r'''<!-- #include tool-row gui\blocks\tool-row.htm;name='AUTO_ROTATION_SPEED';1 -->
  <td name=LABEL_AUTO_ROTATION_SPEED width='25%'>
'''),
(r'''  <td name=SLIDER_SPEED align='center'>
''',
 r'''  <td name=SLIDER_SPEED width='75%'>
'''),
(r'''      width="70"
      height="20"
''',
 r'''      width="100%"
      height="$GetVar(HtmlInputHeight)"
'''),
(r'''  <td name=XYZ align='right'>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<!-- #include tool-row gui\blocks\tool-row.htm;name='VIEW_ALONG';1 -->
  <td width='25%'>
    <b>View along</b>
  </td>
  <td name=XYZ width='25%'>
'''),
(r'''    $spy.MakeHoverButton('tiny-a','matr 1')
    &nbsp
    $spy.MakeHoverButton('tiny-b','matr 2')
''',
 r'''    $spy.MakeHoverButton('tiny-a','matr 1')
  </td>
  <td width='25%'>
    $spy.MakeHoverButton('tiny-b','matr 2')
'''),
(r'''    &nbsp
''',
 r'''  </td>
  <td width='25%'>
'''),
])
PATCHES.append(('etc/gui/start.htm', '    "hint=A small organic structure to get started with",\n    )$-\n', '    "hint=A small organic structure to get started with",\n    "td1=<td width=\'33%\'>"\n    )$-\n'))
PATCHES.append(('etc/gui/start.htm', '    "hint=Simple disorder in a ring",\n    )$-\n', '    "hint=Simple disorder in a ring",\n    "td1=<td width=\'33%\'>"\n    )$-\n'))
PATCHES.append(('etc/gui/start.htm', '    "hint=A simple coordination complex",\n\n', '    "hint=A simple coordination complex",\n    "td1=<td width=\'33%\'>"\n    )$-\n\n'))
PATCHES.append(('etc/gui/start.htm', '    )$-\n\n    )$-\n\n    $+\n', "    )$-\n\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\n\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name='SAMPLE_STRUCTURES_2';1 -->\n\n    $+\n"))
PATCHES.append(('etc/gui/start.htm', '    "hint=A Z\'=2 structure",\n    )$-\n', '    "hint=A Z\'=2 structure",\n    "td1=<td width=\'33%\'>"\n    )$-\n'))
PATCHES.append(('etc/gui/start.htm', '    "hint=Two binding modes for water ligands and water solvent",\n    )$-\n', '    "hint=Two binding modes for water ligands and water solvent",\n    "td1=<td width=\'33%\'>"\n    )$-\n'))
PATCHES.append(('etc/gui/start.htm', '    "hint=Toluene disordered across a symmetry element",\n    )$-\n', '    "hint=Toluene disordered across a symmetry element",\n    "td1=<td width=\'33%\'>"\n    )$-\n'))
PATCHES.append(('etc/gui/start.htm', '    "hint=Toluene disordered across a symmetry element",\n    "td1=<td width=\'33%\'>"\n    )$-\n\n    $+\n    html.Snippet(GetVar(default_link),\n', '    "hint=Toluene disordered across a symmetry element",\n    "td1=<td width=\'33%\'>"\n    )$-\n\n<!-- #include row_table_off gui\\blocks\\row_table_off.htm;1; -->\n\n<!-- #include tool-row gui\\blocks\\tool-row.htm;name=\'SAMPLE_STRUCTURES_3\';1 -->\n\n    $+\n    html.Snippet(GetVar(default_link),\n'))
PATCHES.append(('etc/gui/start.htm', '    "hint=Calcium tartrate, a continuous solid",\n    )$-\n', '    "hint=Calcium tartrate, a continuous solid",\n    "td1=<td width=\'33%\'>"\n    )$-\n'))
PATCHES.append(('etc/gui/start.htm', '    "onclick=spy.internal.get_structure_from_url(TyrosineED.zip)",\n    )$-\n', '    "onclick=spy.internal.get_structure_from_url(TyrosineED.zip)",\n    "td1=<td width=\'33%\'>"\n    )$-\n'))
PATCHES.append(('etc/gui/start.htm', '    "value=• • •",\n', '    "value=More...",\n'))
PATCHES.append(('etc/gui/start.htm', '    "hint=More examples",\n    )$-\n', '    "hint=More examples",\n    "td1=<td width=\'33%\'>"\n    )$-\n'))
PATCHES.append(('etc/gui/start.htm', "  <td width='100%' valign='center'>\n    <b>%Documentation%</b>:\n", ''))
PATCHES.append(('etc/gui/start.htm', '  <a target=\'Documentation at the Olexsys website\' href="shell \'https://www.olexsys.org/olex2/docs/\'"><b>Online</b></a> |\n  <a target=\'Help for line commands\' href="shell BaseDir()/etc/documentation/Commands.pdf">Static %PDF%</a> |\n  <a href="shell https://uwmadison.app.box.com/s/2qq12zzw893vc3n1rspefv9h9p6gw83k">Ilia Guzei\'s Manual&nbsp;</a>\n  </td>\n', '    $+\n    html.Snippet(GetVar(default_link),\n    "value=Online Manual",\n    "onclick=shell \'https://www.olexsys.org/olex2/docs/\'",\n    "td1=<td width=\'33%\'>"\n    )$-\n\n    $+\n    html.Snippet(GetVar(default_link),\n    "value=Commands PDF",\n    "onclick=shell BaseDir()/etc/documentation/Commands.pdf",\n    "td1=<td width=\'33%\'>"\n    )$-\n\n    $+\n    html.Snippet(GetVar(default_link),\n    "value=Guzei Manual",\n    "onclick=shell https://uwmadison.app.box.com/s/2qq12zzw893vc3n1rspefv9h9p6gw83k",\n    "td1=<td width=\'33%\'>"\n    )$-\n\n'))
PATCHES += _crlf('etc/gui/tools/legend-and-placement.htm', [
(r'''
<td width='5%'>
<b>Legend </b>
''',
 r'''
<td width='25%'>
<b>Legend </b>
'''),
(r'''<td width='5%'>
''',
 r'''<td width='15%'>
'''),
(r'''      name = "LEGEND#scope"
      fgcolor="GetVar('HtmlFontColour')"
''',
 r'''      name = "LEGEND#scope"
      height="$GetVar(HtmlCheckboxHeight)"
      width="$GetVar(HtmlCheckboxWidth)"
      fgcolor="GetVar('HtmlFontColour')"
'''),
(r'''    </font>
</td>

  <td width='15%'>
    $+
    html.Snippet(
    "gui/snippets/input-combo",
''',
 r'''    </font>
</td>

  <td width='30%'>
    $+
    html.Snippet(
    "gui/snippets/input-combo",
'''),
(r'''    $-
  </td>

  <td width='15%'>
    $+
    html.Snippet(
    "gui/snippets/input-combo", "name=LEGEND_VERTICAL",
''',
 r'''    $-
  </td>

  <td width='30%'>
    $+
    html.Snippet(
    "gui/snippets/input-combo", "name=LEGEND_VERTICAL",
'''),
(r'''  <td width='15%'>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='LEGEND_SETTINGS_2';1 -->

  <td width='25%'>
    Padding
  </td>

  <td width='35%'>
'''),
(r'''  <td width='24%' align='right'>
     Reset Legend
  </td>
''',
 r''''''),
(r'''    "value=Set Legend",
''',
 r'''    "value=Reset Legend",
    "td1=<td width='40%'>",
'''),
])
PATCHES += _crlf('etc/gui/tools/report-footer.htm', [
(r'''    "onclick=spy.ExtractCifInfo(True,True)>>spy.EditCifInfo()>>html.Update",
    "td1=<td width='33%'>",
    "focus="
''',
 r'''    "onclick=spy.ExtractCifInfo(True,True)>>spy.EditCifInfo()>>html.Update",
    "td1=<td width='50%'>",
    "focus="
'''),
(r'''    "td1=<td width='33%'>",
''',
 r'''    "td1=<td width='50%'>",
'''),
(r'''  )
$-

<td width='12%' align='right'>
''',
 r'''  )
$-
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<td width='12%' align='right'>
'''),
(r'''<td width='12%' align='right'>
  HKL/RES
''',
 r'''<tr align = 'center' name=EDIT_CIF width='100%'>
<!-- #include tool-first-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
<td width='50%' align='left'>
  &nbsp;HKL/RES
'''),
(r'''<td width='25%' align='right'>
''',
 r'''<td width='50%' align='right'>
'''),
(r'''    "td1=<td width='33%'>"
''',
 r'''    "td1=<td width='50%'>"
'''),
(r'''<td name=GET_CHECKCIF_REPORT_FORMAT align='left' VALIGN='center' width='12%'>
''',
 r'''<td name=GET_CHECKCIF_REPORT_FORMAT align='left' VALIGN='center' width='50%'>
'''),
(r'''$-
</td>

<td width='20%' align='center'>
''',
 r'''$-
</td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

<td width='20%' align='center'>
'''),
(r'''<td width='20%' align='center'>
''',
 r'''<tr align = 'center' name=EDIT_CIF width='100%'>
<!-- #include tool-first-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
<td width='50%' align='center'>
'''),
(r'''    "td1=<td width='30%'>"
''',
 r'''    "td1=<td width='50%'>"
'''),
(r'''<td name=AUTO_MERGE_CIF align='right' VALIGN='right' width='5%'>
''',
 r'''<td name=AUTO_MERGE_CIF align='right' VALIGN='right' width='8%'>
'''),
(r'''    <input
      type='checkbox'
      bgcolor="GetVar('HtmlTableBgColour')"
      name = "CIFMERGE_AFTER_REFINE"
''',
 r'''    <input
      type='checkbox'
      height="GetVar(HtmlCheckboxHeight)"
      bgcolor="GetVar('HtmlTableBgColour')"
      name = "CIFMERGE_AFTER_REFINE"
'''),
(r'''  <td align='left' width='15%'>
''',
 r'''  <td align='left' width='22%'>
'''),
(r'''  <td align='left' width='50%'>
''',
 r'''  <td align='left' width='70%'>
'''),
(r'''<a target='Edit the built-in CIF information' href='spy.EditCifInfo()'>metacif</a> $spy.gui.report.publication.DisplayMergeList()
  </td>

  <td align='right' width ='35%'>
''',
 r'''<a target='Edit the built-in CIF information' href='spy.EditCifInfo()'>metacif</a> $spy.gui.report.publication.DisplayMergeList()
  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

  <td align='right' width ='35%'>
'''),
(r'''  <td align='right' width ='35%'>
  <b>Add</b>
<a href="spy.gui.report.publication.add_cif_to_merge_list(FileOpen('Choose definition file','.cif files|*.cif', FilePath()))>>html.Update">
  <b>local</b></a> |
<a href="spy.gui.report.publication.add_cif_to_merge_list(FileOpen('Choose definition file','.cif files|*.cif', BaseDir()/etc/site))>>html.Update">
  <b>default</b></a>&nbsp;<b>CIF</b>
''',
 r'''<tr align = 'center' width='100%'>
<!-- #include tool-first-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
  <td align='left' width='30%'>
    &nbsp;<b>Add CIF</b>
  </td>
  <td width='35%'>
    <snippet src="gui/snippets/gui-link-plain"
     name="add_local_cif"
     value="local"
     onclick="spy.gui.report.publication.add_cif_to_merge_list(FileOpen('Choose definition file','.cif files|*.cif', FilePath()))>>html.Update"  />
  </td>
  <td width='35%'>
    <snippet src="gui/snippets/gui-link-plain"
     name="add_default_cif"
     value="default"
     onclick="spy.gui.report.publication.add_cif_to_merge_list(FileOpen('Choose definition file','.cif files|*.cif', BaseDir()/etc/site))>>html.Update"  />
'''),
(r'''  <td name="Force metacif" align='right' VALIGN='right' width='5%'>
''',
 r'''  <td name="Force metacif" align='right' VALIGN='right' width='8%'>
'''),
(r'''      type='checkbox'
      bgcolor="GetVar('HtmlTableBgColour')"
''',
 r'''      type='checkbox'
      height="GetVar(HtmlCheckboxHeight)"
      bgcolor="GetVar('HtmlTableBgColour')"
'''),
(r'''  </td>
  <td>
    Merge metacif items even if marked for skipping
''',
 r'''  </td>
  <td width='92%'>
    Merge metacif items even if marked for skipping
'''),
(r'''    Merge metacif items even if marked for skipping
  <td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
''',
 r'''    Merge metacif items even if marked for skipping
  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
'''),
(r'''  <td width="10%"><b>HTAB:</b><td>
  <td width="20%">Max D-A</td>
  <td width="10%">
''',
 r'''  <td width="30%">&nbsp;<b>HTAB</b> max D-A</td>
  <td width="20%">
'''),
(r'''  <td width="20%">Min D-H-A angle<td>
  <td width="10%">
''',
 r'''  <td width="30%" align="right">min D-H-A angle</td>
  <td width="20%">
'''),
(r'''     onchange="spy.SetParam('snum.cif.htab_min_angle', html.GetValue('~name~'))"  />
  </td>

  <td width="15%">
''',
 r'''     onchange="spy.SetParam('snum.cif.htab_min_angle', html.GetValue('~name~'))"  />
  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

  <td width="15%">
'''),
(r'''  <td width="15%">
''',
 r'''<tr align = 'center' width='100%'>
<!-- #include tool-first-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
  <td width="50%">
'''),
(r'''  <td width="5%" align="right">
''',
 r'''  <td width="50%" align="left">
'''),
(r'''   <td width="5%" align="right">$+
''',
 r'''   <td width="8%" align="right">$+
'''),
(r'''   $-</td>
  <td>
    Report RTABs
''',
 r'''   $-</td>
  <td width="42%">
    Report RTABs
'''),
(r'''    Report RTABs
  <td>

''',
 r'''    Report RTABs
  </td>

'''),
(r'''  <td width="5%">$+
''',
 r'''  <td width="8%" align="right">$+
'''),
(r'''  $-</td>
  <td>
    Report VARs
''',
 r'''  $-</td>
  <td width="42%">
    Report VARs
'''),
(r'''    Report VARs
  <td>

''',
 r'''    Report VARs
  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

'''),
(r'''  <td>
    Final data name
  <td>
  <td width="30%">$+
''',
 r'''<tr align = 'center' width='100%'>
<!-- #include tool-first-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
  <td width="30%">
    &nbsp;Final data name
  </td>
  <td width="70%">$+
'''),
])
PATCHES += _crlf('etc/gui/tools/report-header.htm', [
(r'''      "height=GetVar(HtmlComboHeight)",
''',
 r'''      "height=math.eval(GetVar(HtmlInputHeight)*1.3)",
'''),
(r'''  <td width='13%' align='left'>
''',
 r'''  <td width='30%' align='left'>
'''),
(r'''  <td width='22%' align='left'>
''',
 r'''  <td width='70%' align='left'>
'''),
(r'''    >
    </font>
  </td>

  <td width='10%' align='right'>
    <b>%Start% <b>
''',
 r'''    >
    </font>
  </td>
<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->

  <td width='10%' align='right'>
    <b>%Start% <b>
'''),
(r'''  <td width='10%' align='right'>
    <b>%Start% <b>
''',
 r'''<tr VALIGN='center' ALIGN='center' valign='center'>
<!-- #include tool-first-column gui\blocks\tool-first-column.htm;1; -->
<!-- #include row_table_on gui\blocks\row_table_on.htm;1; -->
  <td width='15%' align='left'>
    <b>&nbsp;%Start% </b>
'''),
(r'''  <td width='23%'>
''',
 r'''  <td width='35%'>
'''),
(r'''  <td width='10%' align='right'>
    <b>%End% <b>
''',
 r'''  <td width='15%' align='right'>
    <b>%End% </b>
'''),
(r'''  <td width='24%'>
''',
 r'''  <td width='35%'>
'''),
(r'''<td width="20%" align='left'>&nbsp;Table label style:</td>
<td width="30%">$+
''',
 r'''<td width="30%" align='left'><b>&nbsp;Table label style</b></td>
<td width="70%">$+
'''),
])
PATCHES += _crlf('etc/gui/twinning.htm', [
(r'''    "td1=<td width='50%'>"
''',
 r'''    "td1=<td width='33%'>"
'''),
(r''' 
  <td width='14%' align='left'>
     Extended
''',
 r''' 
  <td width='30%' align='left'>
     Extended
'''),
(r'''  <td width='4%' align='left'>
''',
 r'''  <td width='20%' align='left'>
'''),
(r''' 
  <td width='14%' align='right'>
    Min. Laws
''',
 r''' 
  <td width='30%' align='right'>
    Min. Laws
'''),
(r'''  <td width='8%' align='left'>
''',
 r'''  <td width='20%' align='left'>
'''),
(r'''  <td width='40%' align='right'>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='TWINNING_GENERAL_2';1; -->
  <td width='70%' align='left'>
'''),
(r'''  </td>

  <td width='10%' align='left'>
  $+
    html.Snippet("gui/snippets/input-text",
''',
 r'''  </td>

  <td width='30%' align='left'>
  $+
    html.Snippet("gui/snippets/input-text",
'''),
(r'''
  <td width='14%' align='left'>
    Bad Refl'ns
''',
 r'''
  <td width='30%' align='left'>
    Bad Refl'ns
'''),
(r'''    Bad Refl'ns
  </td>
  <td width='10%' align='left'>
  $+
    html.Snippet("gui/snippets/input-combo",
''',
 r'''    Bad Refl'ns
  </td>
  <td width='20%' align='left'>
  $+
    html.Snippet("gui/snippets/input-combo",
'''),
(r'''

  <td align='right'>
    <input
      type="button"
''',
 r'''

  <td width='50%' align='right'>
    <input
      type="button"
'''),
(r'''  <td width='14%' align='left'>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='TWINNING_SPHERICAL_2';1; -->
  <td width='30%' align='left'>
'''),
(r'''  <td width='10%' align='left'>
''',
 r'''  <td width='20%' align='left'>
'''),
(r'''  <td width='14%' align='right'>
''',
 r'''  <td width='30%' align='right'>
'''),
(r'''  <td width='9%' align='left'>
''',
 r'''  <td width='20%' align='left'>
'''),
(r'''  <td align='right'>
''',
 r'''<!-- #include row_table_off gui\blocks\row_table_off.htm;1; -->
<!-- #include tool-row gui\blocks\tool-row.htm;name='TWINNING_SPHERICAL_3';1; -->
  <td width='50%'></td>
  <td width='50%' align='right'>
'''),
])
