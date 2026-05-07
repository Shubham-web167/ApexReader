import re
with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# Restore original declarations and remove the one our script added at the bottom
text = text.replace('/* QMenu* editMenu */', 'QMenu* editMenu = menuBar()->addMenu("&Edit");')
text = text.replace('/* QMenu* viewMenu */', 'QMenu* viewMenu = menuBar()->addMenu("&View");')

# Now remove the duplicates from the bottom
text = text.replace('QMenu* editMenu = menuBar()->addMenu("&Edit");\n', '', 1) # Replace the second one (since the first matched above might have been just restored)
# wait, .replace replaces ALL occurrences if we don't pass count.
# let's be more precise:
text = text.replace('QMenu* editMenu = menuBar()->addMenu("&Edit");', '', 2)
# wait, if I do that both might be removed!

# Better way: replace the EXACT chunk added by the python script:
chunk = """    QMenu* editMenu = menuBar()->addMenu("&Edit");
    QAction* copyAction = editMenu->addAction("Copy Page as Image");"""
new_chunk = """    QAction* copyAction = editMenu->addAction("Copy Page as Image");"""
text = text.replace(chunk, new_chunk)

chunk2 = """    QMenu* viewMenu = menuBar()->addMenu("&View");
    QAction* zoomSelAction = viewMenu->addAction("Zoom to Selection");"""
new_chunk2 = """    QAction* zoomSelAction = viewMenu->addAction("Zoom to Selection");"""
text = text.replace(chunk2, new_chunk2)

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(text)
