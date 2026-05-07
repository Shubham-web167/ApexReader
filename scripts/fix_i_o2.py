with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# fix the redeclarations
text = text.replace('QMenu* editMenu = menuBar()->addMenu("&Edit");', '/* QMenu* editMenu */', 1)
text = text.replace('QMenu* viewMenu = menuBar()->addMenu("&View");', '/* QMenu* viewMenu */', 1)

# Fix toolsMenu usage
wrong_block = '''    QAction* pickColorAction = toolsMenu->addAction("Pick Annotation Color...");
    connect(pickColorAction, &QAction::triggered, this, &MainWindow::onPickColor);

    QMenu* toolsMenu = menuBar()->addMenu("&Tools");'''

fixed_block = '''    QMenu* toolsMenu = menuBar()->addMenu("&Tools");

    QAction* pickColorAction = toolsMenu->addAction("Pick Annotation Color...");
    connect(pickColorAction, &QAction::triggered, this, &MainWindow::onPickColor);'''

text = text.replace(wrong_block, fixed_block)

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(text)
