#!/usr/bin/env python3
"""
Generate the LMS themes into Sources/Resources/themes/<Theme>/.

    Sources/Resources/themes/
        Light/  css/<XClass>.css ...  data/variables.json  images/
        Dark/   css/<XClass>.css ...  data/variables.json  images/

One css file per xQt class (XPushButton.css, XLineEdit.css, ...). Selectors use
the X* class names; a selector also matches subclasses (NxTableView is styled
by XTableView). The only Qt names kept are elements Qt creates itself and that
have no X* counterpart: QToolTip, QMessageBox (NxMessage), the header of item
views, the popup list of a combo box, the calendar popup of a date edit.

The css is shared by all themes and uses @variables (see XStyleManager); only
data/variables.json differs per theme. Run again after editing the templates:

    python Sources/tools/generate_theme.py
"""

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent          # .../Sources
THEMES_DIR = ROOT / "Resources" / "themes"

# ---------------------------------------------------------------------------
# Variables
# ---------------------------------------------------------------------------
COMMON = {
    "fontFamily": "\"Segoe UI\", \"Roboto\", sans-serif",
    "fontSize": "10pt",
    "fontSizeSmall": "9pt",
    "fontSizeTitle": "14pt",
    "radius": "4px",
    "radiusLarge": "6px",
    "controlHeight": "28px",
    "scrollbarSize": "12px",
}

LIGHT = {
    "window": "#f3f4f6",
    "windowAlt": "#e9eaee",
    "base": "#ffffff",
    "baseAlt": "#f7f8fa",
    "text": "#1f2937",
    "textMuted": "#6b7280",
    "textInverted": "#ffffff",
    "border": "#d1d5db",
    "borderStrong": "#9ca3af",
    "hover": "#e5e7eb",
    "pressed": "#d1d5db",
    "primary": "#2563eb",
    "primaryHover": "#1d4ed8",
    "primaryPressed": "#1e40af",
    "primaryText": "#ffffff",
    "selection": "#2563eb",
    "selectionText": "#ffffff",
    "selectionSoft": "#dbeafe",
    "focus": "#60a5fa",
    "disabledText": "#9ca3af",
    "disabledBg": "#eef0f3",
    "success": "#16a34a",
    "warning": "#d97706",
    "danger": "#dc2626",
    "dangerText": "#ffffff",
    "tooltipBg": "#111827",
    "tooltipText": "#f9fafb",
}

DARK = {
    "window": "#1e1f24",
    "windowAlt": "#25262c",
    "base": "#15161a",
    "baseAlt": "#1b1c21",
    "text": "#e5e7eb",
    "textMuted": "#9ca3af",
    "textInverted": "#111827",
    "border": "#3a3c44",
    "borderStrong": "#52555f",
    "hover": "#2a2c33",
    "pressed": "#35373f",
    "primary": "#3b82f6",
    "primaryHover": "#60a5fa",
    "primaryPressed": "#2563eb",
    "primaryText": "#ffffff",
    "selection": "#3b82f6",
    "selectionText": "#ffffff",
    "selectionSoft": "#1e3a5f",
    "focus": "#60a5fa",
    "disabledText": "#6b7280",
    "disabledBg": "#25262c",
    "success": "#22c55e",
    "warning": "#f59e0b",
    "danger": "#ef4444",
    "dangerText": "#ffffff",
    "tooltipBg": "#f9fafb",
    "tooltipText": "#111827",
}

THEMES = {"Light": LIGHT, "Dark": DARK}

# ---------------------------------------------------------------------------
# CSS per class (shared by all themes)
# ---------------------------------------------------------------------------
CSS = {}

CSS["XWidget"] = """\
XWidget {
    background-color: @window;
    color: @text;
    font-family: @fontFamily;
    font-size: @fontSize;
    selection-background-color: @selection;
    selection-color: @selectionText;
}
XWidget:disabled {
    color: @disabledText;
}
XWidget[class~="panel"] {
    background-color: @base;
    border: 1px solid @border;
    border-radius: @radiusLarge;
}
"""

CSS["XMainWindow"] = """\
XMainWindow {
    background-color: @window;
    color: @text;
    font-family: @fontFamily;
    font-size: @fontSize;
}
XMainWindow::separator {
    background: @border;
    width: 1px;
    height: 1px;
}

/* Login button living in the tab row (NxMainWindow, objectName btnLogin) */
#btnLogin {
    background: transparent;
    border: none;
    border-right: 1px solid @border;
    border-radius: 0;
    padding: 0 12px;
    color: @textMuted;
}
#btnLogin:hover {
    background-color: @hover;
    color: @text;
}
#btnLogin:pressed {
    background-color: @pressed;
}
"""

CSS["XDialog"] = """\
XDialog {
    background-color: @window;
    color: @text;
    font-family: @fontFamily;
    font-size: @fontSize;
}
XDialogButtonBox {
    background: transparent;
    dialogbuttonbox-buttons-have-icons: 0;
}
"""

CSS["XLabel"] = """\
XLabel {
    background: transparent;
    color: @text;
}
XLabel:disabled {
    color: @disabledText;
}
XLabel[class~="title"] {
    font-size: @fontSizeTitle;
    font-weight: bold;
}
XLabel[class~="muted"] {
    color: @textMuted;
    font-size: @fontSizeSmall;
}
XLabel[class~="success"] { color: @success; font-weight: bold; }
XLabel[class~="warning"] { color: @warning; font-weight: bold; }
XLabel[class~="danger"]  { color: @danger;  font-weight: bold; }
XLabel[class~="badge"] {
    background-color: @danger;
    color: @dangerText;
    border-radius: 9px;
    padding: 0 6px;
    font-size: @fontSizeSmall;
    font-weight: bold;
}
"""

CSS["XPushButton"] = """\
XPushButton {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 4px 14px;
    min-height: @controlHeight;
}
XPushButton:hover {
    background-color: @hover;
    border-color: @borderStrong;
}
XPushButton:pressed {
    background-color: @pressed;
}
XPushButton:focus {
    border-color: @focus;
}
XPushButton:disabled {
    background-color: @disabledBg;
    color: @disabledText;
    border-color: @border;
}
XPushButton:default, XPushButton[class~="primary"] {
    background-color: @primary;
    color: @primaryText;
    border-color: @primary;
}
XPushButton:default:hover, XPushButton[class~="primary"]:hover {
    background-color: @primaryHover;
    border-color: @primaryHover;
}
XPushButton:default:pressed, XPushButton[class~="primary"]:pressed {
    background-color: @primaryPressed;
}
XPushButton[class~="danger"] {
    background-color: @danger;
    color: @dangerText;
    border-color: @danger;
}
XPushButton[class~="success"] {
    background-color: @success;
    color: @primaryText;
    border-color: @success;
}
XPushButton[class~="flat"] {
    background: transparent;
    border-color: transparent;
}
XPushButton[class~="flat"]:hover {
    background-color: @hover;
}
"""

CSS["XToolButton"] = """\
XToolButton {
    background: transparent;
    color: @text;
    border: 1px solid transparent;
    border-radius: @radius;
    padding: 4px;
}
XToolButton:hover {
    background-color: @hover;
    border-color: @border;
}
XToolButton:pressed, XToolButton:checked {
    background-color: @pressed;
    border-color: @borderStrong;
}
XToolButton:disabled {
    color: @disabledText;
}
XToolButton::menu-indicator {
    subcontrol-position: bottom right;
    bottom: 2px;
    right: 2px;
}
"""

CSS["XCheckBox"] = """\
XCheckBox {
    background: transparent;
    color: @text;
    spacing: 8px;
}
XCheckBox:disabled {
    color: @disabledText;
}
XCheckBox::indicator {
    width: 18px;
    height: 18px;
    border: 1px solid @borderStrong;
    border-radius: 3px;
    background-color: @base;
}
XCheckBox::indicator:hover {
    border-color: @primary;
}
XCheckBox::indicator:checked {
    background-color: @primary;
    border-color: @primary;
}
XCheckBox::indicator:indeterminate {
    background-color: @selectionSoft;
    border-color: @primary;
}
XCheckBox::indicator:disabled {
    background-color: @disabledBg;
    border-color: @border;
}
"""

CSS["XRadioButton"] = """\
XRadioButton {
    background: transparent;
    color: @text;
    spacing: 8px;
}
XRadioButton:disabled {
    color: @disabledText;
}
XRadioButton::indicator {
    width: 18px;
    height: 18px;
    border: 1px solid @borderStrong;
    border-radius: 9px;
    background-color: @base;
}
XRadioButton::indicator:hover {
    border-color: @primary;
}
XRadioButton::indicator:checked {
    background-color: @primary;
    border: 5px solid @base;
    outline: 1px solid @primary;
}
XRadioButton::indicator:disabled {
    background-color: @disabledBg;
    border-color: @border;
}
"""

CSS["XLineEdit"] = """\
XLineEdit {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 2px 8px;
    min-height: @controlHeight;
    selection-background-color: @selection;
    selection-color: @selectionText;
}
XLineEdit:hover {
    border-color: @borderStrong;
}
XLineEdit:focus {
    border-color: @focus;
}
XLineEdit:read-only {
    background-color: @baseAlt;
    color: @textMuted;
}
XLineEdit:disabled {
    background-color: @disabledBg;
    color: @disabledText;
}
XLineEdit[class~="error"] {
    border-color: @danger;
}
"""

CSS["XTextEdit"] = """\
XTextEdit, XPlainTextEdit, XTextBrowser {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 4px;
    selection-background-color: @selection;
    selection-color: @selectionText;
}
XTextEdit:focus, XPlainTextEdit:focus, XTextBrowser:focus {
    border-color: @focus;
}
XTextEdit:disabled, XPlainTextEdit:disabled, XTextBrowser:disabled {
    background-color: @disabledBg;
    color: @disabledText;
}
"""

CSS["XSpinBox"] = """\
XSpinBox, XDoubleSpinBox {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 2px 6px;
    padding-right: 22px;
    min-height: @controlHeight;
    selection-background-color: @selection;
    selection-color: @selectionText;
}
XSpinBox:hover, XDoubleSpinBox:hover {
    border-color: @borderStrong;
}
XSpinBox:focus, XDoubleSpinBox:focus {
    border-color: @focus;
}
XSpinBox:disabled, XDoubleSpinBox:disabled {
    background-color: @disabledBg;
    color: @disabledText;
}
XSpinBox::up-button, XDoubleSpinBox::up-button,
XSpinBox::down-button, XDoubleSpinBox::down-button {
    subcontrol-origin: border;
    width: 18px;
    background-color: @windowAlt;
    border-left: 1px solid @border;
}
XSpinBox::up-button, XDoubleSpinBox::up-button {
    subcontrol-position: top right;
    border-top-right-radius: @radius;
}
XSpinBox::down-button, XDoubleSpinBox::down-button {
    subcontrol-position: bottom right;
    border-bottom-right-radius: @radius;
}
XSpinBox::up-button:hover, XDoubleSpinBox::up-button:hover,
XSpinBox::down-button:hover, XDoubleSpinBox::down-button:hover {
    background-color: @hover;
}
"""

CSS["XDateEdit"] = """\
XDateEdit, XTimeEdit, XDateTimeEdit {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 2px 6px;
    min-height: @controlHeight;
    selection-background-color: @selection;
    selection-color: @selectionText;
}
XDateEdit:hover, XTimeEdit:hover, XDateTimeEdit:hover {
    border-color: @borderStrong;
}
XDateEdit:focus, XTimeEdit:focus, XDateTimeEdit:focus {
    border-color: @focus;
}
XDateEdit:disabled, XTimeEdit:disabled, XDateTimeEdit:disabled {
    background-color: @disabledBg;
    color: @disabledText;
}
XDateEdit::drop-down, XDateTimeEdit::drop-down {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 22px;
    border-left: 1px solid @border;
    background-color: @windowAlt;
    border-top-right-radius: @radius;
    border-bottom-right-radius: @radius;
}

/* calendar popup created by Qt inside the date edit */
XDateEdit QCalendarWidget QWidget, XDateTimeEdit QCalendarWidget QWidget, XCalendarWidget QWidget {
    background-color: @base;
    color: @text;
    alternate-background-color: @baseAlt;
}
XDateEdit QCalendarWidget QToolButton, XDateTimeEdit QCalendarWidget QToolButton, XCalendarWidget QToolButton {
    background: transparent;
    color: @text;
    min-height: 24px;
}
XDateEdit QCalendarWidget QAbstractItemView, XDateTimeEdit QCalendarWidget QAbstractItemView, XCalendarWidget QAbstractItemView {
    selection-background-color: @selection;
    selection-color: @selectionText;
}
"""

CSS["XComboBox"] = """\
XComboBox {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 2px 8px;
    min-height: @controlHeight;
}
XComboBox:hover {
    border-color: @borderStrong;
}
XComboBox:focus, XComboBox:on {
    border-color: @focus;
}
XComboBox:disabled {
    background-color: @disabledBg;
    color: @disabledText;
}
XComboBox::drop-down {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 24px;
    border-left: 1px solid @border;
    background-color: @windowAlt;
    border-top-right-radius: @radius;
    border-bottom-right-radius: @radius;
}
XComboBox::drop-down:hover {
    background-color: @hover;
}

/* popup list created by Qt */
XComboBox QAbstractItemView {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    outline: none;
    selection-background-color: @selection;
    selection-color: @selectionText;
}
XComboBox QAbstractItemView::item {
    min-height: 26px;
    padding: 0 6px;
}
"""

CSS["XGroupBox"] = """\
XGroupBox {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radiusLarge;
    margin-top: 14px;
    padding: 10px 8px 8px 8px;
    font-weight: bold;
}
XGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 10px;
    padding: 0 6px;
    color: @textMuted;
    background-color: @base;
}
XGroupBox:disabled {
    color: @disabledText;
}
"""

CSS["XProgressBar"] = """\
XProgressBar {
    background-color: @baseAlt;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    text-align: center;
    min-height: 20px;
}
XProgressBar::chunk {
    background-color: @primary;
    border-radius: 3px;
}
XProgressBar[class~="success"]::chunk { background-color: @success; }
XProgressBar[class~="warning"]::chunk { background-color: @warning; }
XProgressBar[class~="danger"]::chunk  { background-color: @danger; }
"""

CSS["XScrollBar"] = """\
XScrollBar:vertical {
    background: @windowAlt;
    width: @scrollbarSize;
    margin: 0;
}
XScrollBar:horizontal {
    background: @windowAlt;
    height: @scrollbarSize;
    margin: 0;
}
XScrollBar::handle:vertical, XScrollBar::handle:horizontal {
    background: @borderStrong;
    border-radius: 5px;
    margin: 2px;
}
XScrollBar::handle:vertical { min-height: 24px; }
XScrollBar::handle:horizontal { min-width: 24px; }
XScrollBar::handle:hover {
    background: @textMuted;
}
XScrollBar::add-line, XScrollBar::sub-line {
    width: 0;
    height: 0;
    background: none;
    border: none;
}
XScrollBar::add-page, XScrollBar::sub-page {
    background: none;
}
"""

CSS["XScrollArea"] = """\
XScrollArea {
    background-color: @window;
    border: 1px solid @border;
    border-radius: @radius;
}
XScrollArea > QWidget > XWidget {
    background-color: @window;
}
"""

CSS["XStackedWidget"] = """\
XStackedWidget {
    background-color: @window;
    border: none;
}
"""

CSS["XStatusBar"] = """\
XStatusBar {
    background-color: @windowAlt;
    color: @textMuted;
    border-top: 1px solid @border;
}
XStatusBar::item {
    border: none;
}
"""

CSS["XToolTip"] = """\
/* XToolTip forwards to QToolTip; the tip window itself is created by Qt and
   keeps the selector "QToolTip". */
QToolTip {
    background-color: @tooltipBg;
    color: @tooltipText;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 4px 8px;
    font-size: @fontSizeSmall;
}
"""

CSS["XTableView"] = """\
/* also matches NxTableView and XTableWidget (subclasses) */
XTableView {
    background-color: @base;
    alternate-background-color: @baseAlt;
    color: @text;
    gridline-color: @border;
    border: 1px solid @border;
    border-radius: @radius;
    selection-background-color: @selection;
    selection-color: @selectionText;
    outline: none;
}
XTableView::item {
    padding: 2px 6px;
    border: none;
}
XTableView::item:hover {
    background-color: @selectionSoft;
}
XTableView::item:selected {
    background-color: @selection;
    color: @selectionText;
}

/* header and corner button are created by Qt inside the view */
XTableView QHeaderView {
    background-color: @windowAlt;
    border: none;
}
XTableView QHeaderView::section {
    background-color: @windowAlt;
    color: @textMuted;
    border: none;
    border-right: 1px solid @border;
    border-bottom: 1px solid @border;
    padding: 4px 6px;
    font-weight: bold;
    min-height: 26px;
}
XTableView QHeaderView::section:hover {
    background-color: @hover;
    color: @text;
}
XTableView QTableCornerButton::section {
    background-color: @windowAlt;
    border: none;
    border-right: 1px solid @border;
    border-bottom: 1px solid @border;
}
"""

CSS["XHeaderView"] = """\
/* stand-alone XHeaderView (headers inside XTableView are styled in XTableView.css) */
XHeaderView {
    background-color: @windowAlt;
    border: none;
}
XHeaderView::section {
    background-color: @windowAlt;
    color: @textMuted;
    border: none;
    border-right: 1px solid @border;
    border-bottom: 1px solid @border;
    padding: 4px 6px;
    font-weight: bold;
    min-height: 26px;
}
XHeaderView::section:hover {
    background-color: @hover;
    color: @text;
}
"""

CSS["XListView"] = """\
XListView {
    background-color: @base;
    alternate-background-color: @baseAlt;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    outline: none;
}
XListView::item {
    padding: 4px 6px;
    min-height: 24px;
}
XListView::item:hover {
    background-color: @selectionSoft;
}
XListView::item:selected {
    background-color: @selection;
    color: @selectionText;
}
"""

CSS["XTreeView"] = """\
XTreeView {
    background-color: @base;
    alternate-background-color: @baseAlt;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    outline: none;
}
XTreeView::item {
    padding: 2px 4px;
    min-height: 24px;
}
XTreeView::item:hover {
    background-color: @selectionSoft;
}
XTreeView::item:selected {
    background-color: @selection;
    color: @selectionText;
}
XTreeView::branch {
    background: transparent;
}
XTreeView QHeaderView::section {
    background-color: @windowAlt;
    color: @textMuted;
    border: none;
    border-right: 1px solid @border;
    border-bottom: 1px solid @border;
    padding: 4px 6px;
    font-weight: bold;
}
"""

CSS["XTabWidget"] = """\
XTabWidget::pane {
    background-color: @window;
    border: 1px solid @border;
    border-radius: @radius;
    top: -1px;
}
XTabBar {
    background: transparent;
    qproperty-drawBase: 0;
}
XTabBar::tab {
    background-color: @windowAlt;
    color: @textMuted;
    border: 1px solid @border;
    border-bottom: none;
    border-top-left-radius: @radius;
    border-top-right-radius: @radius;
    padding: 6px 12px;
    margin-right: 2px;
}
XTabBar::tab:hover {
    background-color: @hover;
    color: @text;
}
XTabBar::tab:selected {
    background-color: @window;
    color: @primary;
    border-bottom: 2px solid @primary;
    font-weight: bold;
}
XTabBar::tab:!selected {
    margin-top: 3px;
}
XTabBar::tab:disabled {
    color: @disabledText;
}
"""

CSS["XToolBar"] = """\
XToolBar {
    background-color: @windowAlt;
    border: none;
    border-bottom: 1px solid @border;
    padding: 3px;
    spacing: 4px;
}
XToolBar::separator {
    background: @border;
    width: 1px;
    margin: 4px 6px;
}

/* buttons Qt creates for QActions inside a tool bar are plain QToolButtons */
XToolBar QToolButton, XToolBar XToolButton {
    background: transparent;
    border: 1px solid transparent;
    border-radius: @radius;
    padding: 4px 8px;
    color: @text;
}
XToolBar QToolButton:hover, XToolBar XToolButton:hover {
    background-color: @hover;
    border-color: @border;
}
XToolBar QToolButton:pressed, XToolBar XToolButton:pressed,
XToolBar QToolButton:checked, XToolBar XToolButton:checked {
    background-color: @pressed;
    border-color: @borderStrong;
}
XToolBar QToolButton:disabled, XToolBar XToolButton:disabled {
    color: @disabledText;
}
XToolBar XLineEdit, XToolBar XDateEdit {
    min-height: 26px;
}
XToolBar XLabel {
    background: transparent;
    padding: 0 4px;
}
"""

CSS["XMenuBar"] = """\
XMenuBar {
    background-color: @windowAlt;
    color: @text;
    border-bottom: 1px solid @border;
}
XMenuBar::item {
    background: transparent;
    padding: 4px 10px;
}
XMenuBar::item:selected {
    background-color: @hover;
    border-radius: @radius;
}
XMenuBar::item:pressed {
    background-color: @pressed;
}
"""

CSS["XMenu"] = """\
XMenu {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 4px;
}
XMenu::item {
    padding: 6px 24px 6px 12px;
    border-radius: 3px;
}
XMenu::item:selected {
    background-color: @selection;
    color: @selectionText;
}
XMenu::item:disabled {
    color: @disabledText;
}
XMenu::separator {
    height: 1px;
    background: @border;
    margin: 4px 8px;
}
XMenu::icon {
    margin-left: 6px;
}
"""

CSS["XMessageBox"] = """\
/* NxMessage builds XMessageBox with object names HxMsgNormal / HxMsgInfo / HxMsgQuestion /
   HxMsgWarning / HxMsgCritical, suffix "High" for the alarm variant, buttons HxMsgButton<Name>.
   The label and buttons inside are created by Qt, hence QLabel / QPushButton below. */
XMessageBox {
    background-color: @window;
    color: @text;
    font-family: @fontFamily;
    font-size: @fontSize;
}
XMessageBox QLabel {
    background: transparent;
    color: @text;
}
XMessageBox QPushButton {
    background-color: @base;
    color: @text;
    border: 1px solid @border;
    border-radius: @radius;
    padding: 4px 14px;
    min-width: 88px;
    min-height: @controlHeight;
}
XMessageBox QPushButton:hover {
    background-color: @hover;
    border-color: @borderStrong;
}
XMessageBox QPushButton:default {
    background-color: @primary;
    color: @primaryText;
    border-color: @primary;
}
#HxMsgWarning QLabel {
    color: @warning;
}
#HxMsgCritical QLabel {
    color: @danger;
}
#HxMsgWarningHigh, #HxMsgCriticalHigh {
    background-color: @danger;
}
#HxMsgWarningHigh QLabel, #HxMsgCriticalHigh QLabel {
    color: @dangerText;
    font-size: @fontSizeTitle;
    font-weight: bold;
}
#HxMsgWarningHigh QPushButton, #HxMsgCriticalHigh QPushButton {
    background-color: @base;
    color: @text;
    border-color: @base;
    min-width: 120px;
    min-height: 36px;
    font-weight: bold;
}
"""


def write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def main() -> None:
    for name, palette in THEMES.items():
        theme_dir = THEMES_DIR / name
        variables = {**COMMON, **palette}
        write(theme_dir / "data" / "variables.json", json.dumps(variables, indent=4, ensure_ascii=False) + "\n")
        # remove css files of classes that no longer exist in the template set
        css_dir = theme_dir / "css"
        if css_dir.is_dir():
            for old in css_dir.glob("*.css"):
                if old.stem not in CSS:
                    old.unlink()
        for css_name, css in CSS.items():
            write(css_dir / f"{css_name}.css", f"/* {name} theme - generated by tools/generate_theme.py */\n" + css)
        (theme_dir / "images").mkdir(exist_ok=True)
        print(f"{theme_dir.relative_to(ROOT).as_posix()}: {len(CSS)} css files, {len(variables)} variables")


if __name__ == "__main__":
    main()
