#include "../include/MainWindow.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <QFileInfo>
#include <QColorDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QTabBar>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QMenuBar>
#include <QUndoStack>
#include "../include/PdfOperations.h"
#include <QInputDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QToolBar>
#include <QLineEdit>
#include <QMouseEvent>
#include <QTabWidget>
#include <QTreeWidget>
#include <QStatusBar>
#include <QApplication>
#include <QStyle>
#include <QSpinBox>
#include <QActionGroup>
#include <QToolButton>
#include <QSignalBlocker>
#include <QEvent>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    try {
        setWindowTitle("Apex Reader");
        resize(1000, 800);

        QSettings settings("ApexReader", "App");
        m_recentFiles = settings.value("recentFiles").toStringList();

        m_tabBar = new QTabBar(this);
        m_tabBar->setTabsClosable(true);
        m_tabBar->setMovable(true);
        m_tabBar->setExpanding(false);
        m_tabBar->setDrawBase(false);
        m_tabBar->setStyleSheet(R"(
            QTabBar::tab {
                background: #1e1e1e;
                color: #aaa;
                padding: 6px 12px;
                border-right: 1px solid #333;
                min-width: 100px;
                max-width: 200px;
                font-family: 'Segoe UI', sans-serif;
                font-size: 11px;
            }
            QTabBar::tab:selected {
                background: #2b2b2b;
                color: #0078d4;
                border-top: 2px solid #0078d4;
                font-weight: bold;
            }
            QTabBar::tab:hover:!selected {
                background: #333;
            }
        )");

        m_pdfView = new PdfView(this);
        m_pdfView->setBackgroundBrush(QBrush(QColor("#3a3a3a")));
        m_currentAnnotationColor = QColor(255, 255, 0);
        m_pdfView->setAnnotationColor(m_currentAnnotationColor);
        connect(m_pdfView, &PdfView::thumbnailReady, this, &MainWindow::onPdfThumbnail);
        connect(m_pdfView, &PdfView::currentPageChanged, this, &MainWindow::onPdfPageChanged);

        QWidget *central = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(central);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(m_tabBar);
        layout->addWidget(m_pdfView);
        setCentralWidget(central);

        connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::onTabChanged);
        connect(m_tabBar, &QTabBar::tabCloseRequested, this, &MainWindow::onTabClosed);
        m_tabBar->installEventFilter(this);

        qApp->setStyleSheet(R"(
            QMainWindow, QWidget {
                background-color: #1a1a1a;
                color: #e0e0e0;
                font-family: 'Segoe UI', sans-serif;
            }
            QMenuBar {
                background: #141414;
                color: #cccccc;
                font-size: 13px;
                padding: 2px;
                border-bottom: 1px solid #333;
            }
            QMenuBar::item {
                padding: 4px 10px;
                border-radius: 4px;
            }
            QMenuBar::item:selected {
                background: #2d2d2d;
                color: white;
            }
            QMenu {
                background: #1e1e1e;
                border: 1px solid #444;
                color: #cccccc;
                font-size: 13px;
            }
            QMenu::item {
                padding: 6px 25px;
            }
            QMenu::item:selected {
                background: #0078d4;
                color: white;
            }
            QMenu::separator {
                height: 1px;
                background: #444;
                margin: 3px 0;
            }
            QToolBar {
                background: #1a1a1a;
                border-bottom: 2px solid #0078d4;
                padding: 3px 5px;
                spacing: 2px;
            }
            QToolBar::separator {
                background: #444;
                width: 1px;
                margin: 4px 6px;
            }
            QStatusBar {
                background: #0078d4;
                color: white;
                font-size: 12px;
                font-weight: bold;
                padding: 2px 8px;
            }
            QSpinBox {
                background: #2d2d2d;
                border: 1px solid #555;
                border-radius: 4px;
                color: white;
                padding: 2px 5px;
                min-width: 70px;
                font-size: 13px;
            }
            QSpinBox::up-button, QSpinBox::down-button {
                background: #3d3d3d;
                border-left: 1px solid #555;
                width: 18px;
            }
            QSpinBox::up-button:hover, QSpinBox::down-button:hover {
                background: #0078d4;
            }
            QDockWidget {
                background-color: #1a1a1a;
                color: #cccccc;
            }
            QDockWidget::title {
                background-color: #141414;
                padding: 5px;
                border-bottom: 1px solid #333;
            }
        )");

        m_thumbnailDock = new QDockWidget("Navigation", this);
        m_sidebarTabs = new QTabWidget(this);

        m_thumbnailList = new QListWidget(this);
        m_thumbnailList->setViewMode(QListView::IconMode);
        m_thumbnailList->setIconSize(QSize(120, 160));
        m_thumbnailList->setGridSize(QSize(130, 190));
        m_thumbnailList->setResizeMode(QListView::Adjust);
        m_thumbnailList->setSpacing(5);

        m_outlineTree = new QTreeWidget(this);
        m_outlineTree->setHeaderHidden(true);

        m_sidebarTabs->addTab(m_thumbnailList, "Thumbnails");
        m_sidebarTabs->addTab(m_outlineTree, "Bookmarks");

        m_thumbnailDock->setWidget(m_sidebarTabs);
        addDockWidget(Qt::LeftDockWidgetArea, m_thumbnailDock);

        connect(m_thumbnailList, &QListWidget::itemClicked, this, &MainWindow::onThumbnailClicked);
        connect(m_outlineTree, &QTreeWidget::itemClicked, this, &MainWindow::onOutlineItemClicked);

        QMenu* fileMenu = menuBar()->addMenu("&File");
        QAction* openAction = fileMenu->addAction("&Open PDF...");
        openAction->setShortcut(QKeySequence("Ctrl+O"));
        connect(openAction, &QAction::triggered, this, &MainWindow::onOpenPdf);

        m_recentMenu = new QMenu("Recent Files", this);
        fileMenu->addMenu(m_recentMenu);
        updateRecentFilesMenu();

        QAction* saveAction = fileMenu->addAction("&Save Annotations");
        saveAction->setShortcut(QKeySequence("Ctrl+S"));
        connect(saveAction, &QAction::triggered, this, [this]() { if(m_pdfView) m_pdfView->saveAnnotations(m_currentFilePath); });

        QMenu* editMenu = menuBar()->addMenu("&Edit");
        QAction* copyAction = editMenu->addAction("Copy Page as Image");
        connect(copyAction, &QAction::triggered, this, &MainWindow::onCopyPageAsImage);

        QMenu* viewMenu = menuBar()->addMenu("&View");
        QAction* darkAction = viewMenu->addAction("Toggle &Dark Mode");
        darkAction->setShortcut(QKeySequence("Ctrl+D"));
        connect(darkAction, &QAction::triggered, m_pdfView, &PdfView::toggleDarkMode);

        QAction* fullScreenAction = viewMenu->addAction("Full Screen");
        fullScreenAction->setShortcut(QKeySequence("F11"));
        connect(fullScreenAction, &QAction::triggered, this, &MainWindow::toggleFullScreen);

        QAction* presentationAction = viewMenu->addAction("Presentation Mode");
        presentationAction->setShortcut(QKeySequence("F5"));
        connect(presentationAction, &QAction::triggered, this, &MainWindow::onPresentationMode);

        QMenu* toolsMenu = menuBar()->addMenu("&Tools");
        QAction* watermarkAction = toolsMenu->addAction("Add Watermark...");
        connect(watermarkAction, &QAction::triggered, this, &MainWindow::onAddWatermark);

        QAction* mergeAction = toolsMenu->addAction("Merge PDFs...");
        connect(mergeAction, &QAction::triggered, this, &MainWindow::onMergePdfs);

        QAction* splitAction = toolsMenu->addAction("Split PDF...");
        connect(splitAction, &QAction::triggered, this, &MainWindow::onSplitPdf);

        auto makeToolBtn = [](QString icon, QString tip, QWidget *parent) {
            QToolButton *btn = new QToolButton(parent);
            btn->setText(icon);
            btn->setToolTip(tip);
            btn->setFixedSize(34, 34);
            btn->setFont(QFont("Segoe UI Emoji", 14));
            btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setStyleSheet(R"(
                QToolButton {
                    background: transparent;
                    border: 1px solid transparent;
                    border-radius: 5px;
                    color: #cccccc;
                }
                QToolButton:hover {
                    background: #3d3d3d;
                    border-color: #555;
                }
                QToolButton:checked, QToolButton:pressed {
                    background: #0078d4;
                    border-color: #005a9e;
                    color: white;
                }
            )");
            return btn;
        };

        m_fileToolBar = addToolBar("Main Toolbar");
        m_fileToolBar->setMovable(false);
        m_fileToolBar->setFloatable(false);

        QToolButton* openBtn = makeToolBtn("📂", "Open PDF (Ctrl+O)", this);
        connect(openBtn, &QToolButton::clicked, this, &MainWindow::onOpenPdf);
        m_fileToolBar->addWidget(openBtn);

        QToolButton* saveBtn = makeToolBtn("💾", "Save Annotations (Ctrl+S)", this);
        connect(saveBtn, &QToolButton::clicked, this, [this]() { if(m_pdfView) m_pdfView->saveAnnotations(m_currentFilePath); });
        m_fileToolBar->addWidget(saveBtn);

        m_fileToolBar->addSeparator();

        if (m_pdfView && m_pdfView->undoStack()) {
            QAction* undoAct = m_pdfView->undoStack()->createUndoAction(this);
            QToolButton* undoBtn = makeToolBtn("↩️", "Undo (Ctrl+Z)", this);
            connect(undoBtn, &QToolButton::clicked, undoAct, &QAction::trigger);
            m_fileToolBar->addWidget(undoBtn);

            QAction* redoAct = m_pdfView->undoStack()->createRedoAction(this);
            QToolButton* redoBtn = makeToolBtn("↪️", "Redo (Ctrl+Y)", this);
            connect(redoBtn, &QToolButton::clicked, redoAct, &QAction::trigger);
            m_fileToolBar->addWidget(redoBtn);
        }

        m_fileToolBar->addSeparator();

        QToolButton* zOutBtn = makeToolBtn("🔍", "Zoom Out (Ctrl+-)", this);
        connect(zOutBtn, &QToolButton::clicked, m_pdfView, &PdfView::zoomOut);
        m_fileToolBar->addWidget(zOutBtn);

        m_zoomSpinBox = new QSpinBox(this);
        m_zoomSpinBox->setRange(10, 500);
        m_zoomSpinBox->setValue(100);
        m_zoomSpinBox->setSuffix("%");
        m_fileToolBar->addWidget(m_zoomSpinBox);

        QToolButton* zInBtn = makeToolBtn("🔎", "Zoom In (Ctrl++)", this);
        connect(zInBtn, &QToolButton::clicked, m_pdfView, &PdfView::zoomIn);
        m_fileToolBar->addWidget(zInBtn);

        QToolButton* fitWidthBtn = makeToolBtn("↔️", "Fit Width (Ctrl+W)", this);
        connect(fitWidthBtn, &QToolButton::clicked, m_pdfView, &PdfView::fitToWidth);
        m_fileToolBar->addWidget(fitWidthBtn);

        QToolButton* fitPageBtn = makeToolBtn("⤢", "Fit Page (Ctrl+0)", this);
        connect(fitPageBtn, &QToolButton::clicked, m_pdfView, &PdfView::fitToPage);
        m_fileToolBar->addWidget(fitPageBtn);

        m_fileToolBar->addSeparator();

        QToolButton* prevBtn = makeToolBtn("◀", "Previous Page", this);
        connect(prevBtn, &QToolButton::clicked, m_pdfView, &PdfView::goToPrevPage);
        m_fileToolBar->addWidget(prevBtn);

        m_pageSpinBox = new QSpinBox(this);
        m_pageSpinBox->setMinimum(1);
        m_fileToolBar->addWidget(m_pageSpinBox);

        m_totalPagesLabel = new QLabel(" / 0", this);
        m_fileToolBar->addWidget(m_totalPagesLabel);

        QToolButton* nextBtn = makeToolBtn("▶", "Next Page", this);
        connect(nextBtn, &QToolButton::clicked, m_pdfView, &PdfView::goToNextPage);
        m_fileToolBar->addWidget(nextBtn);

        m_fileToolBar->addSeparator();

        m_toolsActionGroup = new QActionGroup(this);
        m_toolsActionGroup->setExclusive(true);

        auto addTool = [&](QString icon, QString tip, Tool mode, QString shortcut, bool checked = false) {
            QToolButton *btn = makeToolBtn(icon, tip, this);
            btn->setCheckable(true);
            btn->setChecked(checked);
            connect(btn, &QToolButton::toggled, this, [this, mode](bool c) {
                if (c)
                    m_pdfView->setActiveTool(mode);
            });
            m_fileToolBar->addWidget(btn);

            QAction *act = new QAction(icon + " " + tip, this);
            act->setCheckable(true);
            act->setChecked(checked);
            if (!shortcut.isEmpty())
                act->setShortcut(QKeySequence(shortcut));
            connect(act, &QAction::triggered, btn, &QToolButton::animateClick);
            m_toolsActionGroup->addAction(act);
            toolsMenu->addAction(act);
        };

        addTool(QStringLiteral("✋"), QStringLiteral("Hand Tool"), Tool::Hand, "H", true);
        addTool(QStringLiteral("T"), QStringLiteral("Select Text"), Tool::SelectText, "T");
        addTool(QStringLiteral("▮"), QStringLiteral("Highlight"), Tool::Highlight, "M");
        addTool(QStringLiteral("U"), QStringLiteral("Underline"), Tool::Underline, "U");
        addTool(QStringLiteral("S"), QStringLiteral("Strikethrough"), Tool::Strikethrough, "K");
        addTool(QStringLiteral("✏"), QStringLiteral("Pen"), Tool::Pen, "P");
        addTool(QStringLiteral("⌫"), QStringLiteral("Eraser"), Tool::Eraser, "E");
        addTool(QStringLiteral("📝"), QStringLiteral("Add Note"), Tool::AddNote, "N");
        addTool(QStringLiteral("👁"), QStringLiteral("OCR Snip"), Tool::OcrSnip, "O");

        m_fileToolBar->addSeparator();

        m_colorBtn = new QToolButton(this);
        m_colorBtn->setFixedSize(28, 28);
        m_colorBtn->setToolTip("Annotation Color");
        updateColorButton();
        connect(m_colorBtn, &QToolButton::clicked, this, &MainWindow::onPickColor);
        m_fileToolBar->addWidget(m_colorBtn);

        QToolButton* nightBtn = makeToolBtn("🌙", "Night Mode (Ctrl+I)", this);
        connect(nightBtn, &QToolButton::clicked, m_pdfView, &PdfView::toggleDarkMode);
        m_fileToolBar->addWidget(nightBtn);

        QToolButton* fsBtn = makeToolBtn("⛶", "Full Screen (F11)", this);
        connect(fsBtn, &QToolButton::clicked, this, &MainWindow::toggleFullScreen);
        m_fileToolBar->addWidget(fsBtn);

        QToolButton* presBtn = makeToolBtn("▶️", "Presentation Mode (F5)", this);
        connect(presBtn, &QToolButton::clicked, this, &MainWindow::onPresentationMode);
        m_fileToolBar->addWidget(presBtn);

        QToolButton* helpBtn = makeToolBtn("?", "Keyboard Shortcuts (?)", this);
        connect(helpBtn, &QToolButton::clicked, this, [this]() {
            QMessageBox::information(this, "Keyboard Shortcuts",
                "H: Hand Tool\n"
                "V: Select Text\n"
                "M: Highlight\n"
                "U: Underline\n"
                "K: Strikethrough\n"
                "P: Pen Tool\n"
                "E: Eraser\n"
                "N: Text Note\n"
                "O: OCR Snip\n"
                "F5: Presentation Mode\n"
                "F11: Full Screen\n"
                "Ctrl+O: Open\n"
                "Ctrl+S: Save\n"
                "Ctrl+I: Night Mode\n"
                "Ctrl++, Ctrl+-: Zoom\n"
                "Ctrl+W: Fit Width\n"
                "Ctrl+0: Fit Page");
        });
        m_fileToolBar->addWidget(helpBtn);

        m_fileToolBar->addSeparator();

        m_searchBox = new QLineEdit(this);
        m_searchBox->setPlaceholderText("Search...");
        m_searchBox->setFixedWidth(150);
        m_fileToolBar->addWidget(m_searchBox);

        m_matchCaseCheck = new QCheckBox("Aa", this);
        m_matchCaseCheck->setToolTip("Match Case");
        m_fileToolBar->addWidget(m_matchCaseCheck);

        m_matchLabel = new QLabel("0 of 0", this);
        m_matchLabel->setFixedWidth(60);
        m_fileToolBar->addWidget(m_matchLabel);

        QToolButton* sPrevBtn = makeToolBtn("▲", "Previous Match", this);
        connect(sPrevBtn, &QToolButton::clicked, this, &MainWindow::onSearchPrev);
        m_fileToolBar->addWidget(sPrevBtn);

        QToolButton *sNextBtn = makeToolBtn(QStringLiteral("▼"), QStringLiteral("Next Match"), this);
        connect(sNextBtn, &QToolButton::clicked, this, &MainWindow::onSearchNext);
        m_fileToolBar->addWidget(sNextBtn);

        connect(m_searchBox, &QLineEdit::returnPressed, this, &MainWindow::onSearchTriggered);
        connect(m_searchBox, &QLineEdit::textChanged, this, [this](const QString &t) {
            if (t.isEmpty() && m_pdfView)
                m_pdfView->clearSearch();
        });

        m_statusToolLabel = new QLabel("Tool: Hand", this);
        m_statusZoomLabel = new QLabel("Zoom: 100%", this);
        statusBar()->addPermanentWidget(m_statusToolLabel);
        statusBar()->addPermanentWidget(m_statusZoomLabel);

        if (m_pdfView) {
            connect(m_pdfView, &PdfView::searchResultsChanged, this, [this](int cur, int total) {
                if (total > 0)
                    m_matchLabel->setText(QString("%1 of %2").arg(cur + 1).arg(total));
                else
                    m_matchLabel->setText("0 of 0");
            });

            connect(m_pdfView, &PdfView::toolChanged, this, [this](const QString& tool) {
                m_statusToolLabel->setText(QString("Tool: %1").arg(tool));
            });

            connect(m_pdfView, &PdfView::zoomChanged, this, [this](int pct) {
                m_statusZoomLabel->setText(QString("Zoom: %1%").arg(pct));
                if (m_zoomSpinBox) {
                    QSignalBlocker b(m_zoomSpinBox);
                    m_zoomSpinBox->setValue(pct);
                }
            });

            connect(m_zoomSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
                if (m_pdfView)
                    m_pdfView->setZoomPercent(v);
            });

            connect(m_pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int p) {
                if (m_pdfView && p >= 1)
                    m_pdfView->scrollToPage(p - 1);
            });

            connect(m_pdfView, &PdfView::statusMessage, this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 5000);
            });
            connect(m_pdfView, &PdfView::pageChanged, this, [this](int cur, int total) {
                if (m_pageSpinBox) {
                    QSignalBlocker b(m_pageSpinBox);
                    m_pageSpinBox->setValue(cur + 1);
                }
                if (m_totalPagesLabel)
                    m_totalPagesLabel->setText(QString("/ %1").arg(total));
            });
            connect(m_pdfView, &PdfView::statusMsg, this, [this](const QString &msg) {
                statusBar()->showMessage(msg, 5000);
            });
            connect(m_pdfView, &PdfView::thumbReady, this, [this](int idx, const QPixmap &pix) {
                if (!m_thumbnailList)
                    return;
                auto *item = new QListWidgetItem();
                item->setIcon(QIcon(pix));
                item->setText(QString("Page %1").arg(idx + 1));
                item->setData(Qt::UserRole, idx);
                if (idx < m_thumbnailList->count())
                    m_thumbnailList->item(idx)->setIcon(QIcon(pix));
                else
                    m_thumbnailList->addItem(item);
            });
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Startup Error", QString("MainWindow initialization partially failed: %1").arg(e.what()));
    } catch (...) {
        QMessageBox::warning(this, "Startup Error", "MainWindow initialization failed with an unknown error.");
    }
}

MainWindow::~MainWindow() {}

void MainWindow::onOpenPdf() {
    QStringList filePaths = QFileDialog::getOpenFileNames(this, "Open PDF Documents", "", "PDF Files (*.pdf)");
    if (filePaths.isEmpty()) return;

    for (const QString& filePath : filePaths) {
        onOpenPdfManual(filePath);

        m_recentFiles.removeAll(filePath);
        m_recentFiles.prepend(filePath);
    }

    while (m_recentFiles.size() > 10) m_recentFiles.removeLast();
    QSettings("ApexReader", "App").setValue("recentFiles", m_recentFiles);
    updateRecentFilesMenu();
}

void MainWindow::onSearchTriggered() {
    m_pdfView->searchText(m_searchBox->text(), m_matchCaseCheck->isChecked());
}

void MainWindow::onSearchNext() {
    m_pdfView->searchNext();
}

void MainWindow::onSearchPrev() {
    m_pdfView->searchPrev();
}

void MainWindow::onThumbnailClicked(QListWidgetItem* item) {
    if (!item) return;
    int pageIndex = item->data(Qt::UserRole).toInt();
    m_pdfView->scrollToPage(pageIndex);
}

void MainWindow::onOutlineItemClicked(QTreeWidgetItem* item, int column) {
    if (!item) return;
    int pageIndex = item->data(0, Qt::UserRole).toInt();
    if (pageIndex >= 0) {
        m_pdfView->scrollToPage(pageIndex);
    }
}

void MainWindow::toggleFullScreen()
{
    m_isFullScreen = !m_isFullScreen;
    if (m_isFullScreen) {
        if (m_fileToolBar) m_fileToolBar->hide();
        if (m_thumbnailDock) m_thumbnailDock->hide();
        menuBar()->hide();
        statusBar()->hide();
        showFullScreen();
    } else {
        if (m_fileToolBar) m_fileToolBar->show();
        if (m_thumbnailDock) m_thumbnailDock->show();
        menuBar()->show();
        statusBar()->show();
        showNormal();
    }
}

void MainWindow::onMergePdfs() {
    QStringList inputPaths = QFileDialog::getOpenFileNames(this, "Select PDFs to Merge", "", "PDF Files (*.pdf)");
    if (inputPaths.size() < 2) {
        if (!inputPaths.isEmpty()) {
            QMessageBox::warning(this, "Merge PDFs", "Please select at least 2 PDF files to merge.");
        }
        return;
    }

    QString outputPath = QFileDialog::getSaveFileName(this, "Save Merged PDF", "", "PDF Files (*.pdf)");
    if (outputPath.isEmpty()) return;

    if (PdfOperations::mergePdfs(inputPaths, outputPath)) {
        QMessageBox::information(this, "Success", "PDFs merged successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to merge PDFs.");
    }
}

void MainWindow::onSplitPdf() {
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "Split PDF", "Please open a PDF first to split it.");
        return;
    }

    if (m_currentTab == -1) return;
    int totalPages = m_pdfView ? m_pdfView->pageCount() : 0;
    if (totalPages == 0) return;

    bool ok1, ok2;
    int fromPage = QInputDialog::getInt(this, "Split PDF", QString("Start Page (1-%1):").arg(totalPages), 1, 1, totalPages, 1, &ok1);
    if (!ok1) return;

    int toPage = QInputDialog::getInt(this, "Split PDF", QString("End Page (%1-%2):").arg(fromPage).arg(totalPages), fromPage, fromPage, totalPages, 1, &ok2);
    if (!ok2) return;

    QString outputPath = QFileDialog::getSaveFileName(this, "Save Split PDF", "", "PDF Files (*.pdf)");
    if (outputPath.isEmpty()) return;

    if (PdfOperations::splitPdf(m_currentFilePath, fromPage - 1, toPage - 1, outputPath)) {
        QMessageBox::information(this, "Success", "PDF split successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to split PDF.");
    }
}

void MainWindow::onRotateLeft() {}
void MainWindow::onRotateRight() {}

void MainWindow::onPrint() {
    if (m_currentTab == -1) return;

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QPixmap snap = m_pdfView->viewport()->grab();
        QRect rect = painter.viewport();
        QSize size = snap.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(snap.rect());
        painter.drawPixmap(0, 0, snap);
    }
}

void MainWindow::onAddWatermark() {}

void MainWindow::onShowProperties() {
    if (m_currentFilePath.isEmpty()) return;
    QDialog dialog(this);
    dialog.setWindowTitle("Document Properties");
    QFormLayout layout(&dialog);
    QFileInfo fi(m_currentFilePath);
    layout.addRow("File Name:", new QLabel(fi.fileName()));
    layout.addRow("File Size:", new QLabel(QString::number(fi.size() / 1024) + " KB"));
    if (m_pdfView)
        layout.addRow("Pages:", new QLabel(QString::number(m_pdfView->pageCount())));
    dialog.exec();
}

void MainWindow::updateRecentFilesMenu() {
    if (!m_recentMenu) return;
    m_recentMenu->clear();

    if (m_recentFiles.isEmpty()) {
        QAction* none = m_recentMenu->addAction("No recent files");
        none->setEnabled(false);
        return;
    }

    for (const QString& path : m_recentFiles) {
        QAction* a = m_recentMenu->addAction(QFileInfo(path).fileName());
        a->setToolTip(path);
        connect(a, &QAction::triggered, this, [this, path]() {
            openRecentFile(path);
        });
    }

    m_recentMenu->addSeparator();
    m_recentMenu->addAction("Clear Recent Files", this, [this]() {
        m_recentFiles.clear();
        QSettings("ApexReader", "App").remove("recentFiles");
        updateRecentFilesMenu();
    });
}

void MainWindow::openRecentFile(const QString& path) {
    for (int i = 0; i < m_tabs.size(); i++) {
        if (m_tabs[i].filePath == path) {
            m_tabBar->setCurrentIndex(i);
            return;
        }
    }

    if (m_pdfView->loadDocument(path)) {
        PdfTab tab;
        tab.filePath = path;
        tab.document = nullptr;
        tab.title = QFileInfo(path).fileName();
        m_tabs.append(tab);

        int idx = m_tabBar->addTab(tab.title);
        m_tabBar->setCurrentIndex(idx);

        m_currentFilePath = path;
        m_pdfView->loadAnnotations(m_currentFilePath);
        int totalPages = m_pdfView->pageCount();
        m_pageSpinBox->setRange(1, qMax(1, totalPages));
        m_totalPagesLabel->setText(QString(" / %1").arg(totalPages));
        rebuildThumbnailStrip();
        m_outlineTree->clear();

        m_recentFiles.removeAll(path);
        m_recentFiles.prepend(path);
        QSettings("ApexReader", "App").setValue("recentFiles", m_recentFiles);
        updateRecentFilesMenu();
    } else {
        QMessageBox::warning(this, "Recent File", "Failed to open recent file.");
        m_recentFiles.removeAll(path);
        updateRecentFilesMenu();
    }
}

void MainWindow::onCopyPageAsImage() {}

void MainWindow::onPresentationMode() {
    if (!m_isPresentationMode)
        enterPresentationMode();
    else
        exitPresentationMode();
}

void MainWindow::enterPresentationMode() {
    m_isPresentationMode = true;
    menuBar()->hide();
    m_fileToolBar->hide();
    if(m_thumbnailDock) m_thumbnailDock->hide();
    statusBar()->hide();
    if(m_pdfView) {
        m_pdfView->setBackgroundBrush(Qt::black);
        m_pdfView->setFocus();
    }
    showFullScreen();
}

void MainWindow::exitPresentationMode() {
    m_isPresentationMode = false;
    menuBar()->show();
    m_fileToolBar->show();
    if(m_thumbnailDock) m_thumbnailDock->show();
    statusBar()->show();
    if(m_pdfView) {
        m_pdfView->setBackgroundBrush(QColor("#525659"));
    }
    showNormal();
}

void MainWindow::onZoomToSelection() {
    if (m_pdfView) m_pdfView->setActiveTool(Tool::SelectText);
}

void MainWindow::onPickColor() {
    QColor c = QColorDialog::getColor(m_currentAnnotationColor, this, "Pick Color");
    if (c.isValid()) {
        m_currentAnnotationColor = c;
        updateColorButton();
        m_pdfView->setAnnotationColor(c);
    }
}

void MainWindow::updateColorButton() {
    if (m_colorBtn) {
        m_colorBtn->setStyleSheet(QString(
            "background-color: %1; border: 2px solid #888; border-radius: 4px;"
        ).arg(m_currentAnnotationColor.name()));
    }
}

void MainWindow::onTabChanged(int index) {
    if (index < 0 || index >= m_tabs.size()) return;

    m_currentTab = index;
    PdfTab &tab = m_tabs[index];

    m_currentFilePath = tab.filePath;
    setWindowTitle("Apex Reader - " + tab.title);
    m_pdfView->loadDocument(tab.filePath);

    int totalPages = m_pdfView->pageCount();
    m_pageSpinBox->setRange(1, qMax(1, totalPages));
    m_totalPagesLabel->setText(QString(" / %1").arg(totalPages));
    rebuildThumbnailStrip();
    m_outlineTree->clear();
}

void MainWindow::onTabClosed(int index) {
    if (index < 0 || index >= m_tabs.size()) return;

    delete m_tabs[index].document;
    m_tabs.removeAt(index);
    m_tabBar->removeTab(index);

    if (m_tabs.isEmpty()) {
        m_currentTab = -1;
        m_pdfView->closeDocument();
        setWindowTitle("Apex Reader");
        m_currentFilePath.clear();
        m_thumbnailList->clear();
        m_outlineTree->clear();
        m_pageSpinBox->setRange(1, 1);
        m_totalPagesLabel->setText(QStringLiteral(" / 0"));
        if (m_matchLabel)
            m_matchLabel->setText(QStringLiteral("0 of 0"));
    } else {
        int ci = m_tabBar->currentIndex();
        if (ci < 0 || ci >= m_tabs.size())
            m_tabBar->setCurrentIndex(0);
        onTabChanged(m_tabBar->currentIndex());
    }
}

void MainWindow::onOpenPdfManual(const QString& filePath) {
    if (m_pdfView->loadDocument(filePath)) {
        PdfTab tab;
        tab.filePath = filePath;
        tab.document = nullptr;
        tab.title = QFileInfo(filePath).fileName();
        m_tabs.append(tab);

        int idx = m_tabBar->addTab(tab.title);
        m_tabBar->setCurrentIndex(idx);

        m_currentFilePath = filePath;
        m_pdfView->loadAnnotations(m_currentFilePath);
        int totalPages = m_pdfView->pageCount();
        m_pageSpinBox->setRange(1, qMax(1, totalPages));
        m_totalPagesLabel->setText(QStringLiteral(" / %1").arg(totalPages));
        rebuildThumbnailStrip();
        m_outlineTree->clear();
    }
}

void MainWindow::rebuildThumbnailStrip()
{
    if (!m_thumbnailList) return;
    m_thumbnailList->clear();
    if (m_currentTab < 0 || m_currentTab >= m_tabs.size()) return;
    if (!m_pdfView || !m_pdfView->isLoaded()) return;
    const int n = m_pdfView->pageCount();
    for (int i = 0; i < n; ++i) {
        auto *it = new QListWidgetItem(QStringLiteral("Page %1").arg(i + 1), m_thumbnailList);
        it->setData(Qt::UserRole, i);
        it->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    }
}

void MainWindow::onPdfThumbnail(int page, const QImage &img)
{
    if (!m_thumbnailList || img.isNull()) return;
    for (int j = 0; j < m_thumbnailList->count(); ++j) {
        QListWidgetItem *li = m_thumbnailList->item(j);
        if (!li || li->data(Qt::UserRole).toInt() != page) continue;
        QPixmap pm = QPixmap::fromImage(img);
        const QSize target = m_thumbnailList->iconSize();
        li->setIcon(QIcon(pm.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        break;
    }
}

void MainWindow::onPdfPageChanged(int page)
{
    if (!m_thumbnailList) return;
    for (int j = 0; j < m_thumbnailList->count(); ++j) {
        QListWidgetItem *li = m_thumbnailList->item(j);
        if (li && li->data(Qt::UserRole).toInt() == page) {
            m_thumbnailList->setCurrentItem(li);
            break;
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_tabBar && event->type() == QEvent::MouseButtonRelease) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::MiddleButton) {
            const int i = m_tabBar->tabAt(me->pos());
            if (i >= 0) {
                onTabClosed(i);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (!m_isPresentationMode && m_pdfView) {
        if (event->modifiers() == Qt::NoModifier) {
            switch (event->key()) {
                case Qt::Key_H: m_pdfView->setActiveTool(Tool::Hand); return;
                case Qt::Key_T:
                case Qt::Key_V: m_pdfView->setActiveTool(Tool::SelectText); return;
                case Qt::Key_M: m_pdfView->setActiveTool(Tool::Highlight); return;
                case Qt::Key_U: m_pdfView->setActiveTool(Tool::Underline); return;
                case Qt::Key_K: m_pdfView->setActiveTool(Tool::Strikethrough); return;
                case Qt::Key_P: m_pdfView->setActiveTool(Tool::Pen); return;
                case Qt::Key_E: m_pdfView->setActiveTool(Tool::Eraser); return;
                case Qt::Key_N: m_pdfView->setActiveTool(Tool::AddNote); return;
                case Qt::Key_O: m_pdfView->setActiveTool(Tool::OcrSnip); return;
            }
        }
        if (event->modifiers() & Qt::ControlModifier) {
            if (event->key() == Qt::Key_Equal || event->key() == Qt::Key_Plus) { m_pdfView->zoomIn(); return; }
            if (event->key() == Qt::Key_Minus) { m_pdfView->zoomOut(); return; }
            if (event->key() == Qt::Key_0) { m_pdfView->fitToPage(); return; }
            if (event->key() == Qt::Key_W) { m_pdfView->fitToWidth(); return; }
        }
    }

    if (m_isPresentationMode) {
        switch(event->key()) {
            case Qt::Key_Right:
            case Qt::Key_Space:
            case Qt::Key_Down:
            case Qt::Key_PageDown:
                if(m_pdfView) m_pdfView->goToNextPage();
                break;
            case Qt::Key_Left:
            case Qt::Key_Up:
            case Qt::Key_PageUp:
                if(m_pdfView) m_pdfView->goToPrevPage();
                break;
            case Qt::Key_Escape:
                exitPresentationMode();
                break;
            default:
                QMainWindow::keyPressEvent(event);
                break;
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}
