with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'a', encoding='utf-8') as f:
    f.write('''
void MainWindow::onPrint() {
    if (!m_pdfDoc.isOpen()) return;

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        int pages = m_pdfDoc.getPageCount();
        for (int i = 0; i < pages; ++i) {
            if (i > 0) printer.newPage();
            QImage img = m_pdfDoc.renderPage(i, 1.0f);
            QRect rect = painter.viewport();
            QSize size = img.size();
            size.scale(rect.size(), Qt::KeepAspectRatio);
            painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
            painter.setWindow(img.rect());
            painter.drawImage(0, 0, img);
        }
    }
}
''')