import re

pv_cpp = r'e:\Programing\AcrobatKiller\src\PdfView.cpp'
with open(pv_cpp, 'r', encoding='utf-8') as f:
    text = f.read()

if '#include "AnnotationManager.h"' not in text:
    text = text.replace('#include "PdfView.h"', '#include "PdfView.h"\n#include "AnnotationManager.h"')

# fix strikethrough
strikethrough_old = '''        } else if (m_currentMode == Mode_Strikethrough) {
            float y = rect.center().y();'''

strikethrough_new = '''        } else if (m_currentMode == Mode_Strikethrough) {
            QRectF rect = m_selectionOverlay->rect();
            float y = rect.center().y();'''

text = text.replace(strikethrough_old, strikethrough_new)

with open(pv_cpp, 'w', encoding='utf-8') as f:
    f.write(text)

mw_cpp = r'e:\Programing\AcrobatKiller\src\MainWindow.cpp'
with open(mw_cpp, 'r', encoding='utf-8') as f:
    text = f.read()

tools_menu_items = '''    QAction* watermarkAction = toolsMenu->addAction("Add Watermark...");
    connect(watermarkAction, &QAction::triggered, this, &MainWindow::onAddWatermark);
'''
if 'toolsMenu->addAction("Add Watermark...");' not in text:
    text = text.replace('toolsMenu->addAction("Merge PDFs...");', tools_menu_items + '    toolsMenu->addAction("Merge PDFs...");')

with open(mw_cpp, 'w', encoding='utf-8') as f:
    f.write(text)