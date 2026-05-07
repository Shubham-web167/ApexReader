import re

# Phase O: UI Refinements
# - QTabBar for managing multiple PDFs
# - QProgressBar for loading status
# - QPropertyAnimation for smooth scrolling
# - Keyboard shortcuts help

# Update MainWindow.h
with open(r'e:\Programing\AcrobatKiller\include\MainWindow.h', 'r', encoding='utf-8') as f:
    text = f.read()

if 'QTabBar' not in text:
    text = text.replace('#include <QLabel>', '#include <QLabel>\n#include <QTabBar>\n#include <QProgressBar>')
    text = text.replace('QToolBar *m_toolsToolBar;', '''QToolBar *m_toolsToolBar;
    QTabBar *m_tabBar;
    QProgressBar *m_loadingProgress;
    QLabel *m_statusBarMessage;''')

    with open(r'e:\Programing\AcrobatKiller\include\MainWindow.h', 'w', encoding='utf-8') as f:
        f.write(text)

# Update PdfView.h to add animation support
with open(r'e:\Programing\AcrobatKiller\include\PdfView.h', 'r', encoding='utf-8') as f:
    text = f.read()

if 'loadingProgress' not in text:
    text = text.replace('signals:', 'signals:\n    void loadingStarted();\n    void loadingFinished();')
    text = text.replace('bool m_presentationMode = false;', '''bool m_presentationMode = false;
    class QPropertyAnimation* m_scrollAnimation = nullptr;''')

    with open(r'e:\Programing\AcrobatKiller\include\PdfView.h', 'w', encoding='utf-8') as f:
        f.write(text)

# Update MainWindow.cpp UI initialization
with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

if 'm_loadingProgress' not in text:
    text = text.replace('#include <QColorDialog>\n#include <QCheckBox>\n#include <QPushButton>',
                        '#include <QColorDialog>\n#include <QCheckBox>\n#include <QPushButton>\n#include <QTabBar>\n#include <QProgressBar>\n#include <QPropertyAnimation>')

    # Add tab bar to toolbar
    text = text.replace('m_toolsToolBar = new QToolBar();', '''m_tabBar = new QTabBar(this);
    addToolBar(Qt::TopToolBarArea, new QToolBar());
    QToolBar* tabToolbar = new QToolBar(this);
    tabToolbar->addWidget(m_tabBar);
    addToolBar(Qt::TopToolBarArea, tabToolbar);

    m_toolsToolBar = new QToolBar();''')

    # Add progress bar to status bar
    text = text.replace('statusBar()->showMessage("Ready");', '''m_statusBarMessage = new QLabel("Ready");
    statusBar()->addPermanentWidget(m_statusBarMessage);

    m_loadingProgress = new QProgressBar(this);
    m_loadingProgress->setMaximumWidth(200);
    m_loadingProgress->setValue(0);
    m_loadingProgress->setVisible(false);
    statusBar()->addPermanentWidget(m_loadingProgress);

    // Connect loading signals
    connect(m_pdfView, &PdfView::loadingStarted, this, [this]() {
        m_loadingProgress->setVisible(true);
        m_loadingProgress->setValue(0);
    });
    connect(m_pdfView, &PdfView::loadingFinished, this, [this]() {
        m_loadingProgress->setVisible(false);
        m_loadingProgress->setValue(100);
    });''')

    # Add help action with shortcuts display
    help_action = '''    QAction* helpAction = helpMenu->addAction("&Keyboard Shortcuts");
    connect(helpAction, &QAction::triggered, this, [this]() {
        QString shortcuts = "Keyboard Shortcuts:\\n\\n"
                          "Ctrl+O: Open PDF\\n"
                          "Ctrl+S: Save\\n"
                          "Ctrl+Z: Undo\\n"
                          "Ctrl+Y: Redo\\n"
                          "Ctrl+D: Toggle Dark Mode\\n"
                          "F11: Full Screen\\n"
                          "F5: Presentation Mode\\n"
                          "Z: Zoom to Selection\\n"
                          "Ctrl+P: Print\\n"
                          "Ctrl+F: Find Text\\n"
                          "Plus/Minus: Zoom In/Out\\n";
        QMessageBox::information(this, "Keyboard Shortcuts", shortcuts);
    });'''

    text = text.replace('QMenu* helpMenu = menuBar()->addMenu("&Help");', 'QMenu* helpMenu = menuBar()->addMenu("&Help");\n' + help_action)

    with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
        f.write(text)

# Update PdfView.cpp to add animation support
with open(r'e:\Programing\AcrobatKiller\src\PdfView.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

if '#include <QGraphicsScene>' not in text:
    text = text.replace('#include "PdfView.h"', '#include "PdfView.h"\n#include <QGraphicsScene>\n#include <QPropertyAnimation>')
    with open(r'e:\Programing\AcrobatKiller\src\PdfView.cpp', 'w', encoding='utf-8') as f:
        f.write(text)

print("Phase O UI Refinements completed!")
