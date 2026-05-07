with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

text = text.replace('#include <QInputDialog>', '#include <QInputDialog>\n#include <QPrinter>\n#include <QPrintDialog>\n#include <QPainter>')

with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(text)