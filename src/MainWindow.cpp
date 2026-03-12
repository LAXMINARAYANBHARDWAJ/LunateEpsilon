#include "MainWindow.h"
#include "Logger.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>
#include <QIcon>
#include <QStatusBar>
#include <QtConcurrent/QtConcurrent>

Q_LOGGING_CATEGORY(lcWindow, "le.window")
Q_LOGGING_CATEGORY(lcThread, "le.thread")

namespace LE {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::Window);
    resize(760, 560);
    setMinimumSize(600, 450);

    buildUi();
    connectSignals();

    m_themeManager.applyTheme(Theme::System);
    updateWindowIcon();

    qCInfo(lcWindow) << "MainWindow constructed";
}

void MainWindow::updateWindowIcon()
{
    // Forced dark themes always use the white icon.
    // Forced light theme always uses the black icon.
    // System theme follows the OS setting.
    const Theme current = m_themeManager.currentTheme();

    bool useDarkIcon = false;

    switch (current) {
        case Theme::Dark:
        case Theme::AMOLED:
            useDarkIcon = true;
            break;
        case Theme::Light:
            useDarkIcon = false;
            break;
        case Theme::System:
            useDarkIcon = ThemeManager::systemIsDark();
            break;
    }

    setWindowIcon(QIcon(useDarkIcon ? ":/icons/LEwX.ico" : ":/icons/LEbX.ico"));
}

void MainWindow::buildUi()
{
    buildCentralContent();
}

void MainWindow::buildCentralContent()
{
    auto* central = new QWidget(this);
    central->setObjectName("centralWidget");

    // ── Root layout: vertical, fills the central widget ─────────────────────
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Top bar: theme selector pinned to the right ──────────────────────────
    auto* topBar = new QWidget(central);
    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(8, 8, 12, 4);
    topBarLayout->setSpacing(0);

    m_themeBox = new QComboBox(topBar);
    m_themeBox->addItems({"System", "Light", "Dark", "AMOLED"});
    m_themeBox->setObjectName("themeBox");
    // Fixed size prevents any geometry shift when stylesheets change across themes.
    m_themeBox->setFixedSize(110, 28);

    topBarLayout->addStretch();
    topBarLayout->addWidget(m_themeBox);

    // ── Content area: centred vertically and horizontally ───────────────────
    auto* contentWidget = new QWidget(central);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setAlignment(Qt::AlignCenter);
    contentLayout->setSpacing(12);
    contentLayout->setContentsMargins(40, 0, 40, 20);

    // Select File
    m_selectBtn = new QPushButton("Select File", contentWidget);
    m_selectBtn->setObjectName("selectBtn");
    m_selectBtn->setFixedHeight(34);
    m_selectBtn->setFixedWidth(160);

    m_fileLabel = new QLabel("No file selected", contentWidget);
    m_fileLabel->setObjectName("fileLabel");
    m_fileLabel->setAlignment(Qt::AlignCenter);

    // Base path row (M3U → M3U8)
    m_basePathWidget = new QWidget(contentWidget);
    m_basePathWidget->setFixedWidth(420);
    m_basePathEdit   = new QLineEdit(m_basePathWidget);
    m_basePathEdit->setPlaceholderText("Base folder path");
    m_browseBaseBtn  = new QPushButton("Browse", m_basePathWidget);
    m_browseBaseBtn->setFixedWidth(80);
    {
        auto* row = new QHBoxLayout(m_basePathWidget);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(6);
        row->addWidget(m_basePathEdit);
        row->addWidget(m_browseBaseBtn);
    }
    m_basePathWidget->setVisible(false);

    // Location mode (M3U8 → M3U)
    m_locationModeBox = new QComboBox(contentWidget);
    m_locationModeBox->addItems({"Keep original path", "Use custom base path"});
    m_locationModeBox->setFixedWidth(220);
    m_locationModeBox->setVisible(false);

    // Custom path row
    m_customPathWidget = new QWidget(contentWidget);
    m_customPathWidget->setFixedWidth(420);
    m_customPathEdit   = new QLineEdit(m_customPathWidget);
    m_customPathEdit->setPlaceholderText("Custom base path");
    m_browseCustomBtn  = new QPushButton("Browse", m_customPathWidget);
    m_browseCustomBtn->setFixedWidth(80);
    {
        auto* row = new QHBoxLayout(m_customPathWidget);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(6);
        row->addWidget(m_customPathEdit);
        row->addWidget(m_browseCustomBtn);
    }
    m_customPathWidget->setVisible(false);

    // Convert button
    m_convertBtn = new QPushButton("Convert", contentWidget);
    m_convertBtn->setObjectName("convertBtn");
    m_convertBtn->setFixedHeight(36);
    m_convertBtn->setFixedWidth(160);
    m_convertBtn->setEnabled(false);

    // Progress bar
    m_progressBar = new QProgressBar(contentWidget);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedWidth(320);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(false);

    contentLayout->addWidget(m_selectBtn,        0, Qt::AlignCenter);
    contentLayout->addWidget(m_fileLabel,         0, Qt::AlignCenter);
    contentLayout->addWidget(m_basePathWidget,    0, Qt::AlignCenter);
    contentLayout->addWidget(m_locationModeBox,   0, Qt::AlignCenter);
    contentLayout->addWidget(m_customPathWidget,  0, Qt::AlignCenter);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(m_convertBtn,        0, Qt::AlignCenter);
    contentLayout->addSpacing(8);
    contentLayout->addWidget(m_progressBar,       0, Qt::AlignCenter);

    // ── Assemble root ────────────────────────────────────────────────────────
    rootLayout->addWidget(topBar,        0);
    rootLayout->addWidget(contentWidget, 1);

    // ── Status bar ───────────────────────────────────────────────────────────
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->setSizeGripEnabled(false);
    statusBar()->setVisible(false);

    setCentralWidget(central);
}

void MainWindow::connectSignals()
{
    connect(m_selectBtn,       &QPushButton::clicked, this, &MainWindow::onSelectFile);
    connect(m_browseBaseBtn,   &QPushButton::clicked, this, &MainWindow::onBrowseBasePath);
    connect(m_browseCustomBtn, &QPushButton::clicked, this, &MainWindow::onBrowseCustomPath);
    connect(m_convertBtn,      &QPushButton::clicked, this, &MainWindow::onConvert);

    connect(m_basePathEdit,   &QLineEdit::textChanged,
            this, &MainWindow::onBasePathTextChanged);
    connect(m_customPathEdit, &QLineEdit::textChanged,
            this, &MainWindow::onCustomPathTextChanged);

    connect(m_locationModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLocationModeChanged);

    connect(m_themeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);

    connect(&m_futureWatcher, &QFutureWatcher<void>::finished,
            this, &MainWindow::onConversionFinished);
}

void MainWindow::onSelectFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Select Playlist File", {},
        "Playlist Files (*.m3u *.m3u8)"
    );

    if (path.isEmpty()) return;

    m_filePath = path;
    m_inputExt = QFileInfo(path).suffix().toLower();

    m_fileLabel->setText(QFileInfo(path).fileName());

    if (m_inputExt == "m3u") {
        m_convertBtn->setText("Convert to .m3u8");
        m_basePathWidget->setVisible(true);
        m_locationModeBox->setVisible(false);
        m_customPathWidget->setVisible(false);
    } else {
        m_convertBtn->setText("Convert to .m3u");
        m_basePathWidget->setVisible(false);
        m_locationModeBox->setVisible(true);
        m_customPathWidget->setVisible(m_locationModeBox->currentIndex() == 1);
    }

    updateConvertButtonState();
}

void MainWindow::onBrowseBasePath()
{
    const QString folder = QFileDialog::getExistingDirectory(this, "Select Base Folder");
    if (!folder.isEmpty()) {
        m_basePathEdit->setText(folder);
    }
}

void MainWindow::onBrowseCustomPath()
{
    const QString folder = QFileDialog::getExistingDirectory(this, "Select Custom Base Folder");
    if (!folder.isEmpty()) {
        m_customPathEdit->setText(folder);
    }
}

void MainWindow::onConvert()
{
    if (m_filePath.isEmpty()) return;

    const QString targetExt = (m_inputExt == "m3u") ? ".m3u8" : ".m3u";

    const QString savePath = QFileDialog::getSaveFileName(
        this, "Save Converted File", {},
        targetExt.toUpper().mid(1) + " Files (*" + targetExt + ")"
    );

    if (savePath.isEmpty()) return;

    ConversionParams params;
    params.inputPath  = m_filePath;
    params.outputPath = savePath;

    if (m_inputExt == "m3u") {
        params.basePath = m_basePathEdit->text().trimmed();
    } else {
        if (m_locationModeBox->currentIndex() == 1) {
            params.locationMode = LocationMode::Custom;
            params.basePath = m_customPathEdit->text().trimmed();
            if (params.basePath.isEmpty()) {
                showError("Custom base path is required.");
                return;
            }
        } else {
            params.locationMode = LocationMode::Keep;
        }
    }

    m_conversionError.reset();
    setConversionInProgress(true);

    qCInfo(lcThread) << "Dispatching conversion to thread pool";

    auto future = QtConcurrent::run([this, params]() {
        try {
            m_converter.convert(params);
        } catch (const std::exception& e) {
            m_conversionError = e.what();
        }
    });

    m_futureWatcher.setFuture(future);
}

void MainWindow::onConversionFinished()
{
    qCInfo(lcThread) << "Conversion thread finished";

    if (m_conversionError.has_value()) {
        setConversionInProgress(false);
        showError(QString::fromStdString(*m_conversionError));
        m_conversionError.reset();
        return;
    }

    animateProgressTo(100);
    m_statusLabel->setText("Completed successfully.");

    QTimer::singleShot(1200, this, [this]() {
        setConversionInProgress(false);
    });
}

void MainWindow::onLocationModeChanged(int index)
{
    m_customPathWidget->setVisible(index == 1);
    updateConvertButtonState();
}

void MainWindow::onBasePathTextChanged()
{
    updateConvertButtonState();
}

void MainWindow::onCustomPathTextChanged()
{
    updateConvertButtonState();
}

void MainWindow::onThemeChanged(int index)
{
    const Theme themes[] = {Theme::System, Theme::Light, Theme::Dark, Theme::AMOLED};
    m_themeManager.applyTheme(themes[index]);
    updateWindowIcon();
}

void MainWindow::updateConvertButtonState()
{
    if (m_filePath.isEmpty()) {
        m_convertBtn->setEnabled(false);
        return;
    }

    if (m_inputExt == "m3u") {
        m_convertBtn->setEnabled(!m_basePathEdit->text().trimmed().isEmpty());
        return;
    }

    if (m_locationModeBox->currentIndex() == 1) {
        m_convertBtn->setEnabled(!m_customPathEdit->text().trimmed().isEmpty());
    } else {
        m_convertBtn->setEnabled(true);
    }
}

void MainWindow::setConversionInProgress(bool inProgress)
{
    m_convertBtn->setEnabled(!inProgress);
    m_progressBar->setVisible(inProgress);

    if (inProgress) {
        m_progressBar->setValue(0);
        m_statusLabel->setText("Processing\u2026");
        statusBar()->setVisible(true);
    } else {
        m_progressBar->setValue(0);
        statusBar()->setVisible(false);
        m_statusLabel->clear();
        updateConvertButtonState();
    }
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::animateProgressTo(int targetPercent)
{
    auto* timer = new QTimer(this);
    timer->setInterval(20);
    connect(timer, &QTimer::timeout, this, [this, timer, targetPercent]() {
        const int current = m_progressBar->value();
        if (current >= targetPercent) {
            timer->stop();
            timer->deleteLater();
            return;
        }
        m_progressBar->setValue(current + 2);
    });
    timer->start();
}

} // namespace LE
