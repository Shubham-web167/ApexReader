with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# fix the redeclarations
text = text.replace('QMenu* editMenu = menuBar()->addMenu("&Edit");', '')
text = text.replace('QMenu* viewMenu = menuBar()->addMenu("&View");', '')
# because of the above, we need to make sure we didn't remove the original declarations but only the ones the script added.
# actually wait, the original was:
# QMenu* editMenu = menuBar()->addMenu("&Edit");
# The python script added another one. So if we replace all occurrences of `QMenu* editMenu = menuBar()->addMenu("&Edit");` with ``, we'll have 0.
