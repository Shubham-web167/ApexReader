with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'a', encoding='utf-8') as f:
    f.write('''
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
        if (m_pdfDoc.open(file)) {
            m_currentFilePath = file;
            m_pdfView->setDocument(&m_pdfDoc);
            m_pdfView->layoutPages();
        }
    }
}
''')