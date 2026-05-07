import re

# Read MainWindow.cpp
with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    lines = f.readlines()

new_lines = []
for line in lines:
    if "QAction* undoAction = m_pdfView->undoStack()->createUndoAction" in line:
        new_lines.append('    QMenu* editMenu = menuBar()->addMenu("&Edit");\n')
    new_lines.append(line)

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.writelines(new_lines)
