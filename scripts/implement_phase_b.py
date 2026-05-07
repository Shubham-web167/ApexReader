import re
import os

cmakelists_path = r'e:\Programing\AcrobatKiller\CMakeLists.txt'
with open(cmakelists_path, 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace('src/AnnotationManager.cpp assets/icons.qrc)', 'src/AnnotationManager.cpp src/PdfOperations.cpp assets/icons.qrc)')
content = content.replace('include/AnnotationManager.h)', 'include/AnnotationManager.h include/PdfOperations.h)')

with open(cmakelists_path, 'w', encoding='utf-8') as f:
    f.write(content)

mainwindow_h_path = r'e:\Programing\AcrobatKiller\include\MainWindow.h'
with open(mainwindow_h_path, 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace('void toggleFullScreen();', 'void toggleFullScreen();\n    void onMergePdfs();\n    void onSplitPdf();')

with open(mainwindow_h_path, 'w', encoding='utf-8') as f:
    f.write(content)

mainwindow_cpp_path = r'e:\Programing\AcrobatKiller\src\MainWindow.cpp'
with open(mainwindow_cpp_path, 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace('#include <QUndoStack>', '#include <QUndoStack>\n#include "../include/PdfOperations.h"\n#include <QInputDialog>')

tools_menu_items = r'''    QAction* ocrAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "OCR Snip Tool");
    ocrAction->setShortcut(QKeySequence("O"));
    connect(ocrAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_OCR); });
    addAction(ocrAction);
    ocrAction->setCheckable(true);

    toolsMenu->addSeparator();

    QAction* mergeAction = toolsMenu->addAction("Merge PDFs...");
    connect(mergeAction, &QAction::triggered, this, &MainWindow::onMergePdfs);

    QAction* splitAction = toolsMenu->addAction("Split PDF...");
    connect(splitAction, &QAction::triggered, this, &MainWindow::onSplitPdf);
'''

content = content.replace(r'''    QAction* ocrAction = toolsMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "OCR Snip Tool");
    ocrAction->setShortcut(QKeySequence("O"));
    connect(ocrAction, &QAction::triggered, this, [this]() { m_pdfView->setViewMode(PdfView::Mode_OCR); });
    addAction(ocrAction);
    ocrAction->setCheckable(true);''', tools_menu_items)

merge_split_implementations = r'''
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

    int totalPages = m_pdfDoc.getPageCount();
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
'''
content = content + merge_split_implementations

with open(mainwindow_cpp_path, 'w', encoding='utf-8') as f:
    f.write(content)
