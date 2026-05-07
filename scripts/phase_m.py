import re

# Edit MainWindow.h
with open(r'e:\Programing\AcrobatKiller\include\MainWindow.h', 'r', encoding='utf-8') as f:
    text = f.read()

if 'onSearchNext' not in text:
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
        QList<QRectF> rects = m_doc->searchOnPage(i, query, matchCase);
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

# Edit PdfDocument.h
with open(r'e:\Programing\AcrobatKiller\include\PdfDocument.h', 'r', encoding='utf-8') as f:
    text = f.read()

if 'bool matchCase' not in text:
    text = text.replace('QList<QRectF> searchOnPage(int pageIndex, const QString& query);', 'QList<QRectF> searchOnPage(int pageIndex, const QString& query, bool matchCase);')
    with open(r'e:\Programing\AcrobatKiller\include\PdfDocument.h', 'w', encoding='utf-8') as f:
        f.write(text)

# Edit PdfDocument.cpp
with open(r'e:\Programing\AcrobatKiller\src\PdfDocument.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

if 'bool matchCase' not in text:
    # Modify searchOnPage to accept matchCase. Unfortunately MuPDF fz_search_page doesn't natively do regex or matchcase. Wait!
    # MuPDF fz_search_page is case-insensitive usually. Or maybe we can't easily implement matchCase without custom text extraction?
    # fz_search_page ignores case by default in recent versions or has a specific behaviour. Let's look at fz_search_page!
    pass

# We will just write a wrapper script.
