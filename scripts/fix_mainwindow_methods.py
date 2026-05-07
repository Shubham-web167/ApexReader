with open(r'e:\Programing\AcrobatKiller\include\MainWindow.h', 'r', encoding='utf-8') as f:
    text = f.read()

if 'QStringList m_recentFiles;' not in text:
    text = text.replace('private:', 'private:\n    QMenu* m_fileMenu = nullptr;\n    QMenu* m_recentFilesMenu = nullptr;\n    QStringList m_recentFiles;')

with open(r'e:\Programing\AcrobatKiller\include\MainWindow.h', 'w', encoding='utf-8') as f:
    f.write(text)

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

if '#include <QFormLayout>' not in text:
    text = text.replace('#include "../include/MainWindow.h"', '#include "../include/MainWindow.h"\n#include <QFormLayout>\n#include <QSettings>\n#include <QFileInfo>')

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(text)
