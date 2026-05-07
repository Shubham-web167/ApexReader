with open(r'e:\Programing\AcrobatKiller\CMakeLists.txt', 'r', encoding='utf-8') as f:
    text = f.read()

text = text.replace('COMPONENTS Core Gui Widgets Concurrent Network)', 'COMPONENTS Core Gui Widgets Concurrent Network PrintSupport)')
text = text.replace('Qt6::Network', 'Qt6::Network\n    Qt6::PrintSupport')
with open(r'e:\Programing\AcrobatKiller\CMakeLists.txt', 'w', encoding='utf-8') as f:
    f.write(text)