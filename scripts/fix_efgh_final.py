with open(r'e:\Programing\AcrobatKiller\src\PdfView.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

text = text.replace('AnnotationManager::AddAnnotationCommand', 'AddAnnotationCommand')

with open(r'e:\Programing\AcrobatKiller\src\PdfView.cpp', 'w', encoding='utf-8') as f:
    f.write(text)

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# fix the mangled line
mangled = """    QAction* mergeAction =     QAction* watermarkAction = toolsMenu->addAction("Add Watermark...");
    connect(watermarkAction, &QAction::triggered, this, &MainWindow::onAddWatermark);
    toolsMenu->addAction("Merge PDFs...");"""

corrected = """    QAction* watermarkAction = toolsMenu->addAction("Add Watermark...");
    connect(watermarkAction, &QAction::triggered, this, &MainWindow::onAddWatermark);

    QAction* mergeAction = toolsMenu->addAction("Merge PDFs...");"""

text = text.replace(mangled, corrected)

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(text)