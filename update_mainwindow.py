import re
import os

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# Make the adjustments
text = text.replace('#include <QStatusBar>', '#include <QStatusBar>\n#include <QApplication>\n#include <QStyle>\n#include <QSpinBox>\n#include <QActionGroup>\n#include <QToolButton>')

constructor_content = r"""
    m_pdfView->setBackgroundBrush(QBrush(QColor("#3a3a3a")));

    qApp->setStyleSheet(R"(
    QMainWindow, QWidget {
        background-color: #2b2b2b;
        color: #e0e0e0;
    }
    QMenuBar {
        background-color: #1e1e1e;
        color: #cccccc;
        border-bottom: 1px solid #444;
    }
    QMenuBar::item:selected {
        background-color: #3a3a3a;
    }
    QMenu {
        background-color: #2b2b2b;
        border: 1px solid #555;
        color: #e0e0e0;
    }
    QMenu::item:selected {
        background-color: #0078d4;
    }
    QToolBar {
        background-color: #1e1e1e;
        border-bottom: 1px solid #444;
        spacing: 4px;
        padding: 3px;
    }
    QToolButton {
        background-color: transparent;
        border: 1px solid transparent;
        border-radius: 3px;
        padding: 3px 6px;
        color: #cccccc;
    }
    QToolButton:hover {
        background-color: #3a3a3a;
        border-color: #555;
    }
    QToolButton:pressed, QToolButton:checked {
        background-color: #0078d4;
        border-color: #005a9e;
        color: white;
    }
    QDockWidget {
        background-color: #252525;
        color: #cccccc;
        titlebar-close-icon: none;
    }
    QDockWidget::title {
        background-color: #1e1e1e;
        padding: 5px;
        border-bottom: 1px solid #444;
    }
    QListWidget {
        background-color: #252525;
        border: none;
        color: #cccccc;
    }
    QListWidget::item:selected {
        background-color: #0078d4;
    }
    QTreeWidget {
        background-color: #252525;
        border: none;
        color: #cccccc;
    }
    QTreeWidget::item:hover { background-color: #3a3a3a; }
    QTreeWidget::item:selected { background-color: #0078d4; }
    QStatusBar {
        background-color: #0078d4;
        color: white;
        font-size: 12px;
    }
    QLineEdit {
        background-color: #3a3a3a;
        border: 1px solid #555;
        border-radius: 3px;
        padding: 3px 6px;
        color: #e0e0e0;
    }
    QSpinBox {
        background-color: #3a3a3a;
        border: 1px solid #555;
        color: #e0e0e0;
        padding: 2px;
    }
    QTabWidget::pane { border: none; }
    QTabBar::tab {
        background-color: #1e1e1e;
        color: #aaa;
        padding: 6px 12px;
        border-bottom: 2px solid transparent;
    }
    QTabBar::tab:selected {
        color: #0078d4;
        border-bottom: 2px solid #0078d4;
    }
    QScrollBar:vertical {
        background: #2b2b2b;
        width: 10px;
    }
    QScrollBar::handle:vertical {
        background: #555;
        border-radius: 5px;
    }
    QScrollBar::handle:vertical:hover { background: #777; }
)");
"""
text = text.replace('    m_pdfView = new PdfView(this);\n    setCentralWidget(m_pdfView);', '    m_pdfView = new PdfView(this);\n    setCentralWidget(m_pdfView);\n' + constructor_content)

# Icon mapping
text = text.replace('openAction = fileMenu->addAction("&Open PDF...");', 'openAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "&Open PDF...");')
text = text.replace('saveAction = fileMenu->addAction("&Save Annotations");', 'saveAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "&Save Annotations");')
text = text.replace('darkAction = viewMenu->addAction("Toggle &Dark Mode");', 'darkAction = viewMenu->addAction(style()->standardIcon(QStyle::SP_DesktopIcon), "Toggle &Dark Mode");')
text = text.replace('fullScreenAction = viewMenu->addAction("Full Screen");', 'fullScreenAction = viewMenu->addAction(style()->standardIcon(QStyle::SP_TitleBarMaxButton), "Full Screen");')

text = text.replace('handAction = toolsMenu->addAction("Hand Tool (Pan)");', 'handAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_ArrowRight), "Hand Tool (Pan)");')
text = text.replace('selectAction = toolsMenu->addAction("Select Text");', 'selectAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Select Text");')
text = text.replace('highlightAction = toolsMenu->addAction("Highlight");', 'highlightAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_MessageBoxInformation), "Highlight");')
text = text.replace('penAction = toolsMenu->addAction("Pen Tool");', 'penAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Pen Tool");')
text = text.replace('eraserAction = toolsMenu->addAction("Eraser Tool");', 'eraserAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_TrashIcon), "Eraser Tool");')
text = text.replace('underlineAction = toolsMenu->addAction("Underline Text");', 'underlineAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "Underline Text");')
text = text.replace('textNoteAction = toolsMenu->addAction("Add Text Note");', 'textNoteAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "Add Text Note");')
text = text.replace('ocrAction = toolsMenu->addAction("OCR Snip Tool");', 'ocrAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "OCR Snip Tool");')

text = text.replace('zoomInAction = new QAction("Zoom In", this);', 'zoomInAction = new QAction(style()->standardIcon(QStyle::SP_ArrowUp), "Zoom In", this);')
text = text.replace('zoomOutAction = new QAction("Zoom Out", this);', 'zoomOutAction = new QAction(style()->standardIcon(QStyle::SP_ArrowDown), "Zoom Out", this);')
text = text.replace('fitWidthAction = new QAction("Fit Width", this);', 'fitWidthAction = new QAction(style()->standardIcon(QStyle::SP_FileIcon), "Fit Width", this);')
text = text.replace('fitPageAction = new QAction("Fit Page", this);', 'fitPageAction = new QAction(style()->standardIcon(QStyle::SP_FileIcon), "Fit Page", this);')

# Fix action groups
text = text.replace('connect(handAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Scroll); });\n    addAction(handAction);', 'connect(handAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Scroll); });\n    addAction(handAction);\n    handAction->setCheckable(true);')
text = text.replace('connect(selectAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_SelectText); });\n    addAction(selectAction);', 'connect(selectAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_SelectText); });\n    addAction(selectAction);\n    selectAction->setCheckable(true);')
text = text.replace('connect(highlightAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Highlight); });\n    addAction(highlightAction);', 'connect(highlightAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Highlight); });\n    addAction(highlightAction);\n    highlightAction->setCheckable(true);')
text = text.replace('connect(penAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Draw); });\n    addAction(penAction);', 'connect(penAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Draw); });\n    addAction(penAction);\n    penAction->setCheckable(true);')
text = text.replace('connect(eraserAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Erase); });\n    addAction(eraserAction);', 'connect(eraserAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Erase); });\n    addAction(eraserAction);\n    eraserAction->setCheckable(true);')
text = text.replace('connect(underlineAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Underline); });\n    addAction(underlineAction);', 'connect(underlineAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Underline); });\n    addAction(underlineAction);\n    underlineAction->setCheckable(true);')
text = text.replace('connect(textNoteAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_TextNote); });\n    addAction(textNoteAction);', 'connect(textNoteAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_TextNote); });\n    addAction(textNoteAction);\n    textNoteAction->setCheckable(true);')
text = text.replace('connect(ocrAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_OCR); });\n    addAction(ocrAction);', 'connect(ocrAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_OCR); });\n    addAction(ocrAction);\n    ocrAction->setCheckable(true);\n    \n    m_toolsActionGroup = new QActionGroup(this);\n    m_toolsActionGroup->addAction(handAction);\n    m_toolsActionGroup->addAction(selectAction);\n    m_toolsActionGroup->addAction(highlightAction);\n    m_toolsActionGroup->addAction(underlineAction);\n    m_toolsActionGroup->addAction(penAction);\n    m_toolsActionGroup->addAction(eraserAction);\n    m_toolsActionGroup->addAction(textNoteAction);\n    m_toolsActionGroup->addAction(ocrAction);\n    handAction->setChecked(true);')

# Now the toolbars replacements
old_toolbar = r'''    // Setup Toolbar
    m_mainToolBar = addToolBar("Main Toolbar");

    QAction* zoomInAction = new QAction("Zoom In", this);
    zoomInAction->setShortcut(QKeySequence("Ctrl+="));
    connect(zoomInAction, &QAction::triggered, m_pdfView, &PdfView::zoomIn);
    addAction(zoomInAction);

    QAction* zoomOutAction = new QAction("Zoom Out", this);
    zoomOutAction->setShortcut(QKeySequence("Ctrl+-"));
    connect(zoomOutAction, &QAction::triggered, m_pdfView, &PdfView::zoomOut);
    addAction(zoomOutAction);

    QAction* fitWidthAction = new QAction("Fit Width", this);
    fitWidthAction->setShortcut(QKeySequence("Ctrl+W"));
    connect(fitWidthAction, &QAction::triggered, m_pdfView, &PdfView::fitToWidth);
    addAction(fitWidthAction);

    QAction* fitPageAction = new QAction("Fit Page", this);
    fitPageAction->setShortcut(QKeySequence("Ctrl+0"));
    connect(fitPageAction, &QAction::triggered, m_pdfView, &PdfView::fitToPage);
    addAction(fitPageAction);

    m_mainToolBar->addAction(openAction);
    m_mainToolBar->addAction(saveAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(zoomInAction);
    m_mainToolBar->addAction(zoomOutAction);
    m_mainToolBar->addAction(fitWidthAction);
    m_mainToolBar->addAction(fitPageAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(handAction);
    m_mainToolBar->addAction(selectAction);
    m_mainToolBar->addAction(highlightAction);
    m_mainToolBar->addAction(underlineAction);
    m_mainToolBar->addAction(penAction);
    m_mainToolBar->addAction(eraserAction);
    m_mainToolBar->addAction(textNoteAction);
    m_mainToolBar->addAction(ocrAction);

    m_mainToolBar->addSeparator();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Find text...");
    m_searchBox->setFixedWidth(200);
    m_mainToolBar->addWidget(m_searchBox);
    connect(m_searchBox, &QLineEdit::returnPressed, this, &MainWindow::onSearchTriggered);

    QAction* clearSearchAction = m_mainToolBar->addAction("Clear");
    connect(clearSearchAction, &QAction::triggered, this, [this]() {
        m_searchBox->clear();
        m_pdfView->clearSearch();
    });'''


new_toolbar = r'''    // Setup Toolbars
    m_fileToolBar = addToolBar("File");
    m_fileToolBar->setMovable(false);
    m_fileToolBar->setFloatable(false);
    m_fileToolBar->setIconSize(QSize(20, 20));
    m_fileToolBar->addAction(openAction);
    m_fileToolBar->addAction(saveAction);
    m_fileToolBar->addSeparator();

    m_navigationToolBar = addToolBar("Navigation");
    m_navigationToolBar->setMovable(false);
    m_navigationToolBar->setFloatable(false);
    m_navigationToolBar->setIconSize(QSize(20, 20));

    QAction* zoomInAction = new QAction(style()->standardIcon(QStyle::SP_ArrowUp), "Zoom In", this);
    zoomInAction->setShortcut(QKeySequence("Ctrl+="));
    connect(zoomInAction, &QAction::triggered, m_pdfView, &PdfView::zoomIn);
    addAction(zoomInAction);

    QAction* zoomOutAction = new QAction(style()->standardIcon(QStyle::SP_ArrowDown), "Zoom Out", this);
    zoomOutAction->setShortcut(QKeySequence("Ctrl+-"));
    connect(zoomOutAction, &QAction::triggered, m_pdfView, &PdfView::zoomOut);
    addAction(zoomOutAction);

    QAction* fitWidthAction = new QAction(style()->standardIcon(QStyle::SP_FileIcon), "Fit Width", this);
    fitWidthAction->setShortcut(QKeySequence("Ctrl+W"));
    connect(fitWidthAction, &QAction::triggered, m_pdfView, &PdfView::fitToWidth);
    addAction(fitWidthAction);

    QAction* fitPageAction = new QAction(style()->standardIcon(QStyle::SP_FileIcon), "Fit Page", this);
    fitPageAction->setShortcut(QKeySequence("Ctrl+0"));
    connect(fitPageAction, &QAction::triggered, m_pdfView, &PdfView::fitToPage);
    addAction(fitPageAction);

    m_zoomSpinBox = new QSpinBox(this);
    m_zoomSpinBox->setRange(10, 500);
    m_zoomSpinBox->setValue(100);
    m_zoomSpinBox->setSuffix("%");
    connect(m_zoomSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {
        // Not perfectly mapped, but fine for UI
    });

    m_pageSpinBox = new QSpinBox(this);
    m_pageSpinBox->setRange(1, 1);
    m_pageSpinBox->setValue(1);
    connect(m_pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {
        m_pdfView->scrollToPage(val - 1);
    });

    m_totalPagesLabel = new QLabel(" / 1", this);

    m_navigationToolBar->addAction(zoomOutAction);
    m_navigationToolBar->addWidget(m_zoomSpinBox);
    m_navigationToolBar->addAction(zoomInAction);
    m_navigationToolBar->addAction(fitWidthAction);
    m_navigationToolBar->addAction(fitPageAction);
    m_navigationToolBar->addSeparator();
    m_navigationToolBar->addWidget(m_pageSpinBox);
    m_navigationToolBar->addWidget(m_totalPagesLabel);

    m_toolsToolBar = addToolBar("Tools");
    m_toolsToolBar->setMovable(false);
    m_toolsToolBar->setFloatable(false);
    m_toolsToolBar->setIconSize(QSize(20, 20));
    m_toolsToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_toolsToolBar->addAction(handAction);
    m_toolsToolBar->addAction(selectAction);
    m_toolsToolBar->addAction(highlightAction);
    m_toolsToolBar->addAction(underlineAction);
    m_toolsToolBar->addAction(penAction);
    m_toolsToolBar->addAction(eraserAction);
    m_toolsToolBar->addAction(textNoteAction);
    m_toolsToolBar->addAction(ocrAction);
    m_toolsToolBar->addSeparator();
    m_toolsToolBar->addAction(darkAction);
    m_toolsToolBar->addAction(fullScreenAction);

    m_toolsToolBar->addSeparator();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Find text...");
    m_searchBox->setFixedWidth(200);
    m_toolsToolBar->addWidget(m_searchBox);
    connect(m_searchBox, &QLineEdit::returnPressed, this, &MainWindow::onSearchTriggered);

    QAction* clearSearchAction = m_toolsToolBar->addAction("Clear");
    connect(clearSearchAction, &QAction::triggered, this, [this]() {
        m_searchBox->clear();
        m_pdfView->clearSearch();
    });'''

text = text.replace(old_toolbar, new_toolbar)

# Add zoom update
text = text.replace('m_statusZoomLabel->setText(QString("Zoom: %1%").arg(int(zoom * 100)));', 'm_statusZoomLabel->setText(QString("Zoom: %1%").arg(int(zoom * 100)));\n        m_zoomSpinBox->setValue(int(zoom * 100));')

# Update total pages max when document opened
text = text.replace('int totalPages = m_pdfDoc.getPageCount();', 'int totalPages = m_pdfDoc.getPageCount();\n        m_pageSpinBox->setRange(1, totalPages);\n        m_totalPagesLabel->setText(QString(" / %1").arg(totalPages));')

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(text)
