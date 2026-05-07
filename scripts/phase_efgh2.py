import re

mw_h = r'e:\Programing\AcrobatKiller\include\MainWindow.h'
with open(mw_h, 'r', encoding='utf-8') as f:
    text = f.read()

if 'void onAddWatermark();' not in text:
    text = text.replace('void onPrint();', '''void onPrint();
    void onAddWatermark();
    void onShowProperties();
    void updateRecentFilesMenu();
    void openRecentFile();''')

    text = text.replace('QMenu *m_fileToolBar;', 'QMenu *m_fileToolBar;\n    QMenu *m_recentFilesMenu;\n    QStringList m_recentFiles;\n    const int MAX_RECENT_FILES = 10;')
    text = text.replace('QMenu* m_fileMenu;', 'QMenu* m_fileMenu;\n    QMenu* m_recentFilesMenu;\n    QStringList m_recentFiles;\n    const int MAX_RECENT_FILES = 10;')
    with open(mw_h, 'w', encoding='utf-8') as f:
        f.write(text)

mw_cpp = r'e:\Programing\AcrobatKiller\src\MainWindow.cpp'
with open(mw_cpp, 'r', encoding='utf-8') as f:
    text = f.read()

if 'onAddWatermark()' not in text:
    text = text.replace('#include <QPainter>', '#include <QPainter>\n#include <QFormLayout>\n#include <QSettings>\n#include <QFileInfo>')

    tools_menu_items = '''
    QAction* watermarkAction = toolsMenu->addAction("Add Watermark...");
    connect(watermarkAction, &QAction::triggered, this, &MainWindow::onAddWatermark);
'''
    text = text.replace('toolsMenu->addAction("Merge PDFs...");', watermarkAction + 'toolsMenu->addAction("Merge PDFs...");')
    # wait, replace is a bit sloppy. I will locate 'toolsMenu->addSeparator();' inside setupToolBars.
    text = text.replace('toolsMenu->addSeparator();', 'toolsMenu->addSeparator();\n    QAction* watermarkAction = toolsMenu->addAction("Add Watermark...");\n    connect(watermarkAction, &QAction::triggered, this, &MainWindow::onAddWatermark);\n    toolsMenu->addSeparator();', 1)

    text = text.replace('QAction* highlightAction = toolsMenu->addAction', 'QAction* strikethroughAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_DirIcon), "Strikethrough");\n    strikethroughAction->setShortcut(QKeySequence("T"));\n    connect(strikethroughAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_Strikethrough); });\n    addAction(strikethroughAction);\n    strikethroughAction->setCheckable(true);\n    QAction* highlightAction = toolsMenu->addAction')

    file_menu_replace = '''
    m_recentFilesMenu = m_fileMenu->addMenu("Recent Files");
    updateRecentFilesMenu();
    m_fileMenu->addSeparator();
    QAction* propertiesAction = m_fileMenu->addAction("Properties");
    propertiesAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(propertiesAction, &QAction::triggered, this, &MainWindow::onShowProperties);
'''
    text = text.replace('QAction* printAction = m_fileMenu->addAction("Print...");', 'QAction* printAction = m_fileMenu->addAction("Print...");\n    ' + file_menu_replace)

    impls = '''
void MainWindow::onAddWatermark() {
    if (!m_pdfDoc.isOpen()) return;
    bool ok;
    QString text = QInputDialog::getText(this, "Add Watermark", "Enter Watermark Text:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        m_pdfView->addWatermark(text);
    }
}

void MainWindow::onShowProperties() {
    if (m_currentFilePath.isEmpty()) return;
    QDialog dialog(this);
    dialog.setWindowTitle("Document Properties");
    QFormLayout layout(&dialog);
    QFileInfo fi(m_currentFilePath);
    layout.addRow("File Name:", new QLabel(fi.fileName()));
    layout.addRow("File Size:", new QLabel(QString::number(fi.size() / 1024) + " KB"));
    layout.addRow("Pages:", new QLabel(QString::number(m_pdfDoc.getPageCount())));
    dialog.exec();
}

void MainWindow::updateRecentFilesMenu() {
    // Basic recent files loading
    QSettings settings("AcrobatKiller", "AcrobatKiller");
    m_recentFiles = settings.value("recentFiles").toStringList();
    if (m_recentFilesMenu) {
        m_recentFilesMenu->clear();
        for (const QString& file : std::as_const(m_recentFiles)) {
            QAction* action = m_recentFilesMenu->addAction(QFileInfo(file).fileName());
            action->setData(file);
            connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);
        }
    }
}

void MainWindow::openRecentFile() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString file = action->data().toString();
        // Load the file...
        if (m_pdfDoc.open(file)) {
            m_currentFilePath = file;
            m_pdfView->setDocument(&m_pdfDoc);
            m_pdfView->layoutPages();
        }
    }
}
'''
    text += impls

    # Save recent file logic on open
    open_save = '''
        if (m_pdfDoc.open(fileName)) {
            m_currentFilePath = fileName;
            m_pdfView->setDocument(&m_pdfDoc);
            m_pdfView->layoutPages();

            m_recentFiles.removeAll(fileName);
            m_recentFiles.prepend(fileName);
            while (m_recentFiles.size() > MAX_RECENT_FILES) {
                m_recentFiles.removeLast();
            }
            QSettings settings("AcrobatKiller", "AcrobatKiller");
            settings.setValue("recentFiles", m_recentFiles);
            updateRecentFilesMenu();
'''
    text = text.replace('''
        if (m_pdfDoc.open(fileName)) {
            m_currentFilePath = fileName;
            m_pdfView->setDocument(&m_pdfDoc);
            m_pdfView->layoutPages();''', open_save)

    with open(mw_cpp, 'w', encoding='utf-8') as f:
        f.write(text)