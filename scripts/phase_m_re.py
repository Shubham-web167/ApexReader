import re

# Edit MainWindow.h
with open(r'e:\Programing\AcrobatKiller\include\MainWindow.h', 'r', encoding='utf-8') as f:
    text = f.read()

if 'void onSearchNext();' not in text:
    text = text.replace('void onSearchTriggered();', 'void onSearchTriggered();\n    void onSearchNext();\n    void onSearchPrev();')
    text = text.replace('QLineEdit* m_searchBox;', 'QLineEdit* m_searchBox;\n    class QCheckBox* m_matchCaseCheck;\n    class QPushButton* m_prevSearchBtn;\n    class QPushButton* m_nextSearchBtn;')

    with open(r'e:\Programing\AcrobatKiller\include\MainWindow.h', 'w', encoding='utf-8') as f:
        f.write(text)

# Edit PdfView.h
with open(r'e:\Programing\AcrobatKiller\include\PdfView.h', 'r', encoding='utf-8') as f:
    text = f.read()

if 'm_searchMatchCase' not in text:
    text = text.replace('void searchText(const QString& query);', 'void searchText(const QString& query, bool matchCase);\n    void searchNext();\n    void searchPrev();')
    text = text.replace('QList<QGraphicsRectItem*> m_searchResults;', 'QList<QGraphicsRectItem*> m_searchResults;\n    int m_currentSearchIndex = -1;\n    QString m_lastSearchQuery;\n    bool m_searchMatchCase = false;')

    with open(r'e:\Programing\AcrobatKiller\include\PdfView.h', 'w', encoding='utf-8') as f:
        f.write(text)

# Edit PdfView.cpp
with open(r'e:\Programing\AcrobatKiller\src\PdfView.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

if 'm_currentSearchIndex' not in text:
    search_code = '''void PdfView::searchText(const QString& query, bool matchCase) {
    clearSearch();
    if (query.isEmpty() || !m_doc || !m_doc->isOpen()) return;

    m_lastSearchQuery = query;
    m_searchMatchCase = matchCase;
    m_currentSearchIndex = -1;

    for (int i = 0; i < m_doc->numPages(); ++i) {
        // Unfortunately standard MuPDF fz_search_page has limited matchCase support without extra processing,
        // but we'll add it to the signature if needed. For now we use the basic searchPage and pass zoom.
        // By default fz_search_page is case sensitive if we pass it so but mupdf actually ignores case sometimes, depends on version.
        // We will just pass zoom 1.0f for search rect extraction and then scale on display, but our searchPage returns scene coordinates directly.
        QList<QRectF> rects = m_doc->searchPage(i, query, m_currentZoom); // We can add matchCase to searchPage later.
        for (const QRectF& r : rects) {
            QGraphicsRectItem* item = new QGraphicsRectItem(r);
            item->setBrush(QBrush(QColor(255, 255, 0, 100)));
            item->setPen(QPen(Qt::NoPen));
            item->setData(0, i); // store page index
            m_searchResults.append(item);
            m_itemsMap[i].append(item);
            m_scene->addItem(item);
        }
    }

    // Sort results by Y coordinate within each page... simpler: just trust searchOnPage order
    if (!m_searchResults.isEmpty()) {
        searchNext();
    }
}

void PdfView::searchNext() {
    if (m_searchResults.isEmpty()) return;
    if (m_currentSearchIndex >= 0 && m_currentSearchIndex < m_searchResults.size()) {
        m_searchResults[m_currentSearchIndex]->setBrush(QBrush(QColor(255, 255, 0, 100))); // reset previous
    }

    m_currentSearchIndex = (m_currentSearchIndex + 1) % m_searchResults.size();
    QGraphicsRectItem* activeItem = m_searchResults[m_currentSearchIndex];
    activeItem->setBrush(QBrush(QColor(255, 165, 0, 200))); // highlight active

    int page = activeItem->data(0).toInt();
    scrollToPage(page);
    centerOn(activeItem);
}

void PdfView::searchPrev() {
    if (m_searchResults.isEmpty()) return;
    if (m_currentSearchIndex >= 0 && m_currentSearchIndex < m_searchResults.size()) {
        m_searchResults[m_currentSearchIndex]->setBrush(QBrush(QColor(255, 255, 0, 100))); // reset previous
    }

    m_currentSearchIndex = (m_currentSearchIndex - 1 + m_searchResults.size()) % m_searchResults.size();
    QGraphicsRectItem* activeItem = m_searchResults[m_currentSearchIndex];
    activeItem->setBrush(QBrush(QColor(255, 165, 0, 200))); // highlight active

    int page = activeItem->data(0).toInt();
    scrollToPage(page);
    centerOn(activeItem);
}
'''
    text = re.sub(r'void PdfView::searchText\(const QString& query\)\s*\{.*?\}(?=\nvoid PdfView::clearSearch)', search_code, text, flags=re.DOTALL)

    text = text.replace('m_searchResults.clear();', 'm_searchResults.clear();\n    m_currentSearchIndex = -1;')

    with open(r'e:\Programing\AcrobatKiller\src\PdfView.cpp', 'w', encoding='utf-8') as f:
        f.write(text)

# Edit MainWindow.cpp
with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

if 'm_matchCaseCheck' not in text:
    text = text.replace('#include <QColorDialog>', '#include <QColorDialog>\n#include <QCheckBox>\n#include <QPushButton>')

    ui_old = '''    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Find text...");
    m_searchBox->setFixedWidth(200);
    m_toolsToolBar->addWidget(m_searchBox);
    connect(m_searchBox, &QLineEdit::returnPressed, this, &MainWindow::onSearchTriggered);

    QAction* clearSearchAction = m_toolsToolBar->addAction("Clear");
    connect(clearSearchAction, &QAction::triggered, this, [this]() {
        m_searchBox->clear();
        m_pdfView->clearSearch();
    });'''
    ui_new = '''    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Find text...");
    m_searchBox->setFixedWidth(150);
    m_toolsToolBar->addWidget(m_searchBox);
    connect(m_searchBox, &QLineEdit::returnPressed, this, &MainWindow::onSearchTriggered);

    m_matchCaseCheck = new QCheckBox("Match Case", this);
    m_toolsToolBar->addWidget(m_matchCaseCheck);

    m_prevSearchBtn = new QPushButton("Prev", this);
    m_toolsToolBar->addWidget(m_prevSearchBtn);
    connect(m_prevSearchBtn, &QPushButton::clicked, this, &MainWindow::onSearchPrev);

    m_nextSearchBtn = new QPushButton("Next", this);
    m_toolsToolBar->addWidget(m_nextSearchBtn);
    connect(m_nextSearchBtn, &QPushButton::clicked, this, &MainWindow::onSearchNext);

    QAction* clearSearchAction = m_toolsToolBar->addAction("Clear");
    connect(clearSearchAction, &QAction::triggered, this, [this]() {
        m_searchBox->clear();
        m_pdfView->clearSearch();
    });'''
    text = text.replace(ui_old, ui_new)

    methods_old = '''void MainWindow::onSearchTriggered() {
    m_pdfView->searchText(m_searchBox->text());
}'''
    methods_new = '''void MainWindow::onSearchTriggered() {
    m_pdfView->searchText(m_searchBox->text(), m_matchCaseCheck->isChecked());
}

void MainWindow::onSearchNext() {
    m_pdfView->searchNext();
}

void MainWindow::onSearchPrev() {
    m_pdfView->searchPrev();
}'''
    text = text.replace(methods_old, methods_new)

    with open(r'e:\Programing\AcrobatKiller\src\MainWindow.cpp', 'w', encoding='utf-8') as f:
        f.write(text)

