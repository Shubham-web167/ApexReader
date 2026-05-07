import re

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# Replace the toolbar section
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

text = re.sub(r'    // Setup Toolbar\s+m_mainToolBar = addToolBar\("Main Toolbar"\);[\s\S]*?m_pdfView->clearSearch\(\);\s+\}\);', new_toolbar, text)

# Fix Full Screen hiding/showing
text = text.replace('m_mainToolBar->hide();', 'm_fileToolBar->hide();\n        m_navigationToolBar->hide();\n        m_toolsToolBar->hide();')
text = text.replace('m_mainToolBar->show();', 'm_fileToolBar->show();\n        m_navigationToolBar->show();\n        m_toolsToolBar->show();')


with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(text)
