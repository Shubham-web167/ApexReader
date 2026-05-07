import re

# CMakeLists.txt
cmake_path = r'e:\Programing\AcrobatKiller\CMakeLists.txt'
with open(cmake_path, 'r', encoding='utf-8') as f:
    content = f.read()
content = content.replace('COMPONENTS Core Gui Widgets REQUIRED', 'COMPONENTS Core Gui Widgets PrintSupport REQUIRED')
content = content.replace('Qt6::Core Qt6::Gui Qt6::Widgets', 'Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport')
with open(cmake_path, 'w', encoding='utf-8') as f:
    f.write(content)

# PdfDocument.h
h_path = r'e:\Programing\AcrobatKiller\include\PdfDocument.h'
with open(h_path, 'r', encoding='utf-8') as f:
    content = f.read()

if 'void rotatePage' not in content:
    content = content.replace('#include <QTreeWidget>', '#include <QTreeWidget>\n#include <QMap>')
    content = content.replace('QSizeF getPageSize(int pageNumber) const;', 'QSizeF getPageSize(int pageNumber) const;\n    void rotatePage(int pageNumber, int degrees);\n    int getPageRotation(int pageNumber) const;')
    content = content.replace('mutable QMutex m_renderMutex;', 'mutable QMutex m_renderMutex;\n    QMap<int, int> m_pageRotations;')
    with open(h_path, 'w', encoding='utf-8') as f:
        f.write(content)

# PdfDocument.cpp
cpp_path = r'e:\Programing\AcrobatKiller\src\PdfDocument.cpp'
with open(cpp_path, 'r', encoding='utf-8') as f:
    content = f.read()

if 'rotatePage' not in content:
    content = content.replace('return success;\n}', 'return success;\n}\n\nvoid PdfDocument::rotatePage(int pageNumber, int degrees) {\n    m_pageRotations[pageNumber] = (m_pageRotations.value(pageNumber, 0) + degrees) % 360;\n    if (m_pageRotations[pageNumber] < 0) m_pageRotations[pageNumber] += 360;\n}\n\nint PdfDocument::getPageRotation(int pageNumber) const {\n    return m_pageRotations.value(pageNumber, 0);\n}')
    content = content.replace('fz_matrix transform = fz_scale(zoom * HD_FACTOR, zoom * HD_FACTOR);', 'fz_matrix transform = fz_scale(zoom * HD_FACTOR, zoom * HD_FACTOR);\n        transform = fz_concat(transform, fz_rotate(m_pageRotations.value(pageNumber, 0)));')

    # getPageSize replacement
    size_new = '''QSizeF PdfDocument::getPageSize(int pageNumber) const {
    if (!m_ctx || !m_doc) return QSizeF();
    QMutexLocker locker(&m_renderMutex);
    fz_page* page = fz_load_page(m_ctx, m_doc, pageNumber);
    fz_rect rect = fz_bound_page(m_ctx, page);
    fz_drop_page(m_ctx, page);
    int rot = m_pageRotations.value(pageNumber, 0);
    if (rot == 90 || rot == 270) {
        return QSizeF(rect.y1 - rect.y0, rect.x1 - rect.x0);
    }
    return QSizeF(rect.x1 - rect.x0, rect.y1 - rect.y0);
}'''
    # if it's there
    content = re.sub(r'QSizeF PdfDocument::getPageSize.*?return QSizeF\(.*?\);\n}', size_new, content, flags=re.DOTALL)

    with open(cpp_path, 'w', encoding='utf-8') as f:
        f.write(content)

# mainWindow.h
mw_h = r'e:\Programing\AcrobatKiller\include\MainWindow.h'
with open(mw_h, 'r', encoding='utf-8') as f:
    content = f.read()
if 'void onRotateLeft();' not in content:
    content = content.replace('void onSplitPdf();', 'void onSplitPdf();\n    void onRotateLeft();\n    void onRotateRight();\n    void onPrint();')
    with open(mw_h, 'w', encoding='utf-8') as f:
        f.write(content)

# MainWindow.cpp
mw_cpp = r'e:\Programing\AcrobatKiller\src\MainWindow.cpp'
with open(mw_cpp, 'r', encoding='utf-8') as f:
    content = f.read()

if 'onRotateLeft' not in content:
    content = content.replace('#include <QInputDialog>', '#include <QInputDialog>\n#include <QPrinter>\n#include <QPrintDialog>\n#include <QPainter>')

    menus_new = '''
    // Page menu
    QMenu* pageMenu = menuBar()->addMenu("&Page");

    QAction* rotateLeftAction = pageMenu->addAction("Rotate Left");
    rotateLeftAction->setShortcut(QKeySequence("Ctrl+Shift+L"));
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::onRotateLeft);

    QAction* rotateRightAction = pageMenu->addAction("Rotate Right");
    rotateRightAction->setShortcut(QKeySequence("Ctrl+Shift+R"));
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::onRotateRight);

    QAction* printAction = m_fileMenu->addAction("Print...");
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, &QAction::triggered, this, &MainWindow::onPrint);
'''
    content = content.replace('setupToolBars();', 'setupToolBars();' + menus_new)

    impls = '''
void MainWindow::onRotateLeft() {
    if (!m_pdfDoc.isOpen()) return;
    int currentPage = m_pdfView->getCurrentPage();
    m_pdfDoc.rotatePage(currentPage, -90);
    m_pdfView->renderPage(currentPage);
}

void MainWindow::onRotateRight() {
    if (!m_pdfDoc.isOpen()) return;
    int currentPage = m_pdfView->getCurrentPage();
    m_pdfDoc.rotatePage(currentPage, 90);
    m_pdfView->renderPage(currentPage);
}

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
'''
    content += impls
    with open(mw_cpp, 'w', encoding='utf-8') as f:
        f.write(content)
