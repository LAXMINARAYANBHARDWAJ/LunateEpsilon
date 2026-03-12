#include "ThemeManager.h"
#include "Logger.h"

#include <QApplication>
#include <QStyleFactory>
#include <QStyle>
#include <QSettings>

Q_LOGGING_CATEGORY(lcTheme, "le.theme")

namespace LE {

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{}

void ThemeManager::applyTheme(Theme theme)
{
    qCInfo(lcTheme) << "Applying theme:" << static_cast<int>(theme);

    m_current = theme;

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    if (theme == Theme::System) {
        QApplication::setPalette(QApplication::style()->standardPalette());
        qApp->setStyleSheet(buildStyleSheet(Theme::System));
    } else {
        QPalette palette;

        switch (theme) {
            case Theme::Light:  palette = buildLightPalette();  break;
            case Theme::Dark:   palette = buildDarkPalette();   break;
            case Theme::AMOLED: palette = buildAmoledPalette(); break;
            default: break;
        }

        QApplication::setPalette(palette);
        qApp->setStyleSheet(buildStyleSheet(theme));
    }

    emit themeChanged(theme);
    qCDebug(lcTheme) << "Theme applied successfully";
}

// ─── System dark detection ───────────────────────────────────────────────────
// Reads the AppsUseLightTheme registry DWORD. Value 0 → dark; 1 or absent → light.

bool ThemeManager::systemIsDark()
{
    const QSettings reg(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat
    );
    // AppsUseLightTheme: 0 = dark apps, 1 = light apps. Default to light if key absent.
    return reg.value("AppsUseLightTheme", 1).toInt() == 0;
}

// ─── Palette Builders ────────────────────────────────────────────────────────

QPalette ThemeManager::buildLightPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(QRgb(0xF3F3F3)));
    p.setColor(QPalette::WindowText,      QColor(QRgb(0x1A1A1A)));
    p.setColor(QPalette::Base,            QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::AlternateBase,   QColor(QRgb(0xEAEAEA)));
    p.setColor(QPalette::Text,            QColor(QRgb(0x1A1A1A)));
    p.setColor(QPalette::Button,          QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::ButtonText,      QColor(QRgb(0x1A1A1A)));
    p.setColor(QPalette::Highlight,       QColor(QRgb(0x0067C0)));
    p.setColor(QPalette::HighlightedText, QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::PlaceholderText, QColor(QRgb(0x888888)));
    p.setColor(QPalette::ToolTipBase,     QColor(QRgb(0xFFFFE1)));
    p.setColor(QPalette::ToolTipText,     QColor(QRgb(0x000000)));
    return p;
}

// Windows 11 native dark palette
// Base layer:    #202020  (solid app background — Settings, Explorer, etc.)
// Surface layer: #2B2B2B  (cards, panels, input fields)
// Raised layer:  #313131  (elevated controls, button resting state)
// Text primary:  #FFFFFF
// Text secondary:#C7C7C7
// Accent:        #0067C0  (default Windows blue; hover #005BA5, pressed #004E8C)
QPalette ThemeManager::buildDarkPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(QRgb(0x202020)));
    p.setColor(QPalette::WindowText,      QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::Base,            QColor(QRgb(0x2B2B2B)));
    p.setColor(QPalette::AlternateBase,   QColor(QRgb(0x313131)));
    p.setColor(QPalette::Text,            QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::Button,          QColor(QRgb(0x313131)));
    p.setColor(QPalette::ButtonText,      QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::Highlight,       QColor(QRgb(0x0067C0)));
    p.setColor(QPalette::HighlightedText, QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::PlaceholderText, QColor(QRgb(0x6E6E6E)));
    p.setColor(QPalette::ToolTipBase,     QColor(QRgb(0x2B2B2B)));
    p.setColor(QPalette::ToolTipText,     QColor(QRgb(0xFFFFFF)));
    return p;
}

QPalette ThemeManager::buildAmoledPalette() const
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(QRgb(0x000000)));
    p.setColor(QPalette::WindowText,      QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Base,            QColor(QRgb(0x0A0A0A)));
    p.setColor(QPalette::AlternateBase,   QColor(QRgb(0x111111)));
    p.setColor(QPalette::Text,            QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Button,          QColor(QRgb(0x1A1A1A)));
    p.setColor(QPalette::ButtonText,      QColor(QRgb(0xE0E0E0)));
    p.setColor(QPalette::Highlight,       QColor(QRgb(0x0078D4)));
    p.setColor(QPalette::HighlightedText, QColor(QRgb(0xFFFFFF)));
    p.setColor(QPalette::PlaceholderText, QColor(QRgb(0x3A3A3A)));
    p.setColor(QPalette::ToolTipBase,     QColor(QRgb(0x0A0A0A)));
    p.setColor(QPalette::ToolTipText,     QColor(QRgb(0xE0E0E0)));
    return p;
}

// ─── Stylesheet ──────────────────────────────────────────────────────────────
//
// Structure: each theme block defines rules for the following components in order:
//   1. Central widget background (#centralWidget)
//   2. Select and Convert buttons (#selectBtn, #convertBtn)
//   3. Browse buttons (generic QPushButton inside path widgets)
//   4. QLineEdit (path input fields)
//   5. QComboBox (location mode selector, theme selector)
//   6. QProgressBar
//   7. Status / file labels (#statusLabel, #fileLabel)
//   8. QScrollBar
//   9. QToolTip
//  10. QMenu (context menu)

QString ThemeManager::buildStyleSheet(Theme theme) const
{
    switch (theme) {

    // ── System ───────────────────────────────────────────────────────────────
    case Theme::System:
        return QStringLiteral(
            "QPushButton#selectBtn, QPushButton#convertBtn {"
            "  border-radius: 4px;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "}"
            // min-height prevents descender clipping on all QComboBox instances
            "QComboBox { min-height: 28px; }"
            "QProgressBar {"
            "  border: none;"
            "  border-radius: 3px;"
            "  background-color: palette(mid);"
            "  max-height: 6px;"
            "}"
            "QProgressBar::chunk {"
            "  border-radius: 3px;"
            "  background-color: palette(highlight);"
            "}"
            "QMenu {"
            "  border: 1px solid palette(mid);"
            "  border-radius: 4px;"
            "  padding: 4px 0;"
            "}"
            "QMenu::item { padding: 5px 28px 5px 16px; font-size: 12px; }"
            "QMenu::item:selected {"
            "  background-color: palette(highlight);"
            "  color: palette(highlighted-text);"
            "}"
            "QMenu::separator { height: 1px; background: palette(mid); margin: 3px 8px; }"
        );

    // ── Light ─────────────────────────────────────────────────────────────────
    case Theme::Light:
        return QStringLiteral(
            "QWidget#centralWidget { background-color: #F3F3F3; }"

            "QPushButton#selectBtn {"
            "  background-color: #E8E8E8;"
            "  border: 1px solid #C0C0C0;"
            "  border-radius: 4px;"
            "  color: #1A1A1A;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "}"
            "QPushButton#selectBtn:hover   { background-color: #D8D8D8; border-color: #A0A0A0; }"
            "QPushButton#selectBtn:pressed { background-color: #C8C8C8; }"

            "QPushButton#convertBtn {"
            "  background-color: #0067C0;"
            "  border: none;"
            "  border-radius: 4px;"
            "  color: #FFFFFF;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "}"
            "QPushButton#convertBtn:hover   { background-color: #005BA5; }"
            "QPushButton#convertBtn:pressed { background-color: #004E8C; }"
            "QPushButton#convertBtn:disabled { background-color: #C8C8C8; color: #909090; }"

            "QPushButton { border-radius: 4px; font-size: 12px; }"

            "QLineEdit {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #1A1A1A;"
            "  selection-background-color: #0067C0;"
            "  selection-color: #FFFFFF;"
            "}"
            "QLineEdit:focus { border-color: #0067C0; }"

            "QComboBox {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  border-radius: 4px;"
            "  padding: 3px 8px;"
            "  font-size: 12px;"
            "  color: #1A1A1A;"
            "  min-height: 28px;"
            "}"
            "QComboBox::drop-down { border: none; width: 20px; }"
            "QComboBox QAbstractItemView {"
            "  background-color: #FFFFFF;"
            "  border: 1px solid #C0C0C0;"
            "  selection-background-color: #0067C0;"
            "  selection-color: #FFFFFF;"
            "}"

            "QProgressBar {"
            "  border: none;"
            "  border-radius: 3px;"
            "  background-color: #DCDCDC;"
            "  max-height: 6px;"
            "}"
            "QProgressBar::chunk { border-radius: 3px; background-color: #0067C0; }"

            "QLabel#fileLabel { color: #444444; font-size: 12px; }"

            "QStatusBar { background-color: #E8E8E8; border-top: 1px solid #D0D0D0; }"
            "QStatusBar QLabel#statusLabel { color: #444444; font-size: 12px; padding: 0 6px; }"

            "QScrollBar:vertical { background: #EBEBEB; width: 8px; border-radius: 4px; }"
            "QScrollBar::handle:vertical { background: #BBBBBB; border-radius: 4px; min-height: 20px; }"
            "QScrollBar::handle:vertical:hover { background: #999999; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"

            "QToolTip {"
            "  background-color: #FFFBE6; color: #1A1A1A;"
            "  border: 1px solid #C8C8C8; border-radius: 3px;"
            "  padding: 3px 6px; font-size: 12px;"
            "}"

            "QMenu {"
            "  background-color: #FFFFFF; border: 1px solid #C0C0C0;"
            "  border-radius: 4px; padding: 4px 0;"
            "}"
            "QMenu::item { padding: 5px 28px 5px 16px; font-size: 12px; color: #1A1A1A; }"
            "QMenu::item:selected { background-color: #0067C0; color: #FFFFFF; border-radius: 3px; margin: 0 4px; }"
            "QMenu::item:disabled { color: #AAAAAA; }"
            "QMenu::separator { height: 1px; background: #D8D8D8; margin: 3px 8px; }"
        );

    // ── Dark ──────────────────────────────────────────────────────────────────
    // Native Windows 11 dark:
    //   Background  #202020 | Surface #2B2B2B | Elevated #313131
    //   Border      #3D3D3D | Text #FFFFFF | Secondary #C7C7C7 | Accent #0067C0
    case Theme::Dark:
        return QStringLiteral(
            "QWidget#centralWidget { background-color: #202020; }"

            "QPushButton#selectBtn {"
            "  background-color: #313131;"
            "  border: 1px solid #3D3D3D;"
            "  border-radius: 4px;"
            "  color: #FFFFFF;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "}"
            "QPushButton#selectBtn:hover   { background-color: #3A3A3A; border-color: #4A4A4A; }"
            "QPushButton#selectBtn:pressed { background-color: #424242; }"

            "QPushButton#convertBtn {"
            "  background-color: #0067C0;"
            "  border: none;"
            "  border-radius: 4px;"
            "  color: #FFFFFF;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "}"
            "QPushButton#convertBtn:hover   { background-color: #005BA5; }"
            "QPushButton#convertBtn:pressed { background-color: #004E8C; }"
            "QPushButton#convertBtn:disabled { background-color: #2E2E2E; color: #5A5A5A; }"

            "QPushButton { border-radius: 4px; font-size: 12px; }"

            "QLineEdit {"
            "  background-color: #2B2B2B;"
            "  border: 1px solid #3D3D3D;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #FFFFFF;"
            "  selection-background-color: #0067C0;"
            "  selection-color: #FFFFFF;"
            "}"
            "QLineEdit:focus { border-color: #0067C0; }"

            "QComboBox {"
            "  background-color: #2B2B2B;"
            "  border: 1px solid #3D3D3D;"
            "  border-radius: 4px;"
            "  padding: 3px 8px;"
            "  font-size: 12px;"
            "  color: #FFFFFF;"
            "  min-height: 28px;"
            "}"
            "QComboBox::drop-down { border: none; width: 20px; }"
            "QComboBox QAbstractItemView {"
            "  background-color: #2B2B2B;"
            "  border: 1px solid #3D3D3D;"
            "  selection-background-color: #0067C0;"
            "  selection-color: #FFFFFF;"
            "}"

            "QProgressBar {"
            "  border: none; border-radius: 3px;"
            "  background-color: #313131; max-height: 6px;"
            "}"
            "QProgressBar::chunk { border-radius: 3px; background-color: #0067C0; }"

            "QLabel#fileLabel { color: #C7C7C7; font-size: 12px; }"

            "QStatusBar { background-color: #181818; border-top: 1px solid #2E2E2E; }"
            "QStatusBar QLabel#statusLabel { color: #C7C7C7; font-size: 12px; padding: 0 6px; }"

            "QScrollBar:vertical { background: #2B2B2B; width: 8px; border-radius: 4px; }"
            "QScrollBar::handle:vertical { background: #4A4A4A; border-radius: 4px; min-height: 20px; }"
            "QScrollBar::handle:vertical:hover { background: #606060; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"

            "QToolTip {"
            "  background-color: #2B2B2B; color: #FFFFFF;"
            "  border: 1px solid #3D3D3D; border-radius: 3px;"
            "  padding: 3px 6px; font-size: 12px;"
            "}"

            "QMenu {"
            "  background-color: #2B2B2B; border: 1px solid #3D3D3D;"
            "  border-radius: 4px; padding: 4px 0;"
            "}"
            "QMenu::item { padding: 5px 28px 5px 16px; font-size: 12px; color: #FFFFFF; }"
            "QMenu::item:selected { background-color: #0067C0; color: #FFFFFF; border-radius: 3px; margin: 0 4px; }"
            "QMenu::item:disabled { color: #555555; }"
            "QMenu::separator { height: 1px; background: #3D3D3D; margin: 3px 8px; }"
        );

    // ── AMOLED ────────────────────────────────────────────────────────────────
    case Theme::AMOLED:
        return QStringLiteral(
            "QWidget#centralWidget { background-color: #000000; }"

            "QPushButton#selectBtn {"
            "  background-color: #1A1A1A;"
            "  border: 1px solid #2A2A2A;"
            "  border-radius: 4px;"
            "  color: #E0E0E0;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "}"
            "QPushButton#selectBtn:hover   { background-color: #242424; border-color: #3A3A3A; }"
            "QPushButton#selectBtn:pressed { background-color: #2E2E2E; }"

            "QPushButton#convertBtn {"
            "  background-color: #0078D4;"
            "  border: none;"
            "  border-radius: 4px;"
            "  color: #FFFFFF;"
            "  padding: 0 20px;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "}"
            "QPushButton#convertBtn:hover   { background-color: #006BBD; }"
            "QPushButton#convertBtn:pressed { background-color: #005FA6; }"
            "QPushButton#convertBtn:disabled { background-color: #181818; color: #3A3A3A; }"

            "QPushButton { border-radius: 4px; font-size: 12px; }"

            "QLineEdit {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #222222;"
            "  border-radius: 4px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "  color: #E0E0E0;"
            "  selection-background-color: #0078D4;"
            "  selection-color: #FFFFFF;"
            "}"
            "QLineEdit:focus { border-color: #0078D4; }"

            "QComboBox {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #222222;"
            "  border-radius: 4px;"
            "  padding: 3px 8px;"
            "  font-size: 12px;"
            "  color: #E0E0E0;"
            "  min-height: 28px;"
            "}"
            "QComboBox::drop-down { border: none; width: 20px; }"
            "QComboBox QAbstractItemView {"
            "  background-color: #0A0A0A;"
            "  border: 1px solid #222222;"
            "  selection-background-color: #0078D4;"
            "  selection-color: #FFFFFF;"
            "}"

            "QProgressBar {"
            "  border: none; border-radius: 3px;"
            "  background-color: #1A1A1A; max-height: 6px;"
            "}"
            "QProgressBar::chunk { border-radius: 3px; background-color: #0078D4; }"

            "QLabel#fileLabel { color: #606060; font-size: 12px; }"

            "QStatusBar { background-color: #000000; border-top: 1px solid #1A1A1A; }"
            "QStatusBar QLabel#statusLabel { color: #606060; font-size: 12px; padding: 0 6px; }"

            "QScrollBar:vertical { background: #000000; width: 8px; border-radius: 4px; }"
            "QScrollBar::handle:vertical { background: #333333; border-radius: 4px; min-height: 20px; }"
            "QScrollBar::handle:vertical:hover { background: #484848; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"

            "QToolTip {"
            "  background-color: #0A0A0A; color: #E0E0E0;"
            "  border: 1px solid #222222; border-radius: 3px;"
            "  padding: 3px 6px; font-size: 12px;"
            "}"

            "QMenu {"
            "  background-color: #0A0A0A; border: 1px solid #222222;"
            "  border-radius: 4px; padding: 4px 0;"
            "}"
            "QMenu::item { padding: 5px 28px 5px 16px; font-size: 12px; color: #E0E0E0; }"
            "QMenu::item:selected { background-color: #0078D4; color: #FFFFFF; border-radius: 3px; margin: 0 4px; }"
            "QMenu::item:disabled { color: #333333; }"
            "QMenu::separator { height: 1px; background: #1C1C1C; margin: 3px 8px; }"
        );

    default:
        return {};
    }
}

} // namespace LE
