import re

pdfview_h = r'e:\Programing\AcrobatKiller\include\PdfView.h'
with open(pdfview_h, 'r', encoding='utf-8') as f:
    pv_h = f.read()

if 'Mode_Strikethrough' not in pv_h:
    pv_h = pv_h.replace('enum ViewMode {', 'enum ViewMode { Mode_Strikethrough, Mode_ZoomRect,')
    pv_h = pv_h.replace('void rotatePage(int pageIndex, int degrees);', 'void rotatePage(int pageIndex, int degrees);\n    void addWatermark(const QString& text);')
    with open(pdfview_h, 'w', encoding='utf-8') as f:
        f.write(pv_h)

pdfview_cpp = r'e:\Programing\AcrobatKiller\src\PdfView.cpp'
with open(pdfview_cpp, 'r', encoding='utf-8') as f:
    pv_cpp = f.read()

if 'addWatermark' not in pv_cpp:
    strikethrough_code = '''        } else if (m_currentMode == Mode_Strikethrough) {
            float y = rect.center().y();
            QGraphicsLineItem* line = new QGraphicsLineItem(rect.left(), y, rect.right(), y);
            line->setPen(QPen(Qt::red, 2.0));
            line->setData(0, "user_annotation");
            m_undoStack->push(new AnnotationManager::AddAnnotationCommand(m_scene, line));
'''
    pv_cpp = pv_cpp.replace('} else if (m_currentMode == Mode_Underline) {', strikethrough_code + '} else if (m_currentMode == Mode_Underline) {')

    watermark_code = '''
void PdfView::addWatermark(const QString& text) {
    if (m_pages.empty()) return;
    int page = getCurrentPage();
    if (page < 0 || page >= m_pages.size()) return;

    QGraphicsTextItem* wm = new QGraphicsTextItem(text);
    QFont font = wm->font();
    font.setPointSize(72);
    font.setBold(true);
    wm->setFont(font);
    wm->setDefaultTextColor(QColor(255, 0, 0, 40));

    QRectF pageRect = m_pages[page].rect;
    wm->setPos(pageRect.center() - wm->boundingRect().center());
    wm->setRotation(-30);
    wm->setData(0, "user_annotation");

    m_undoStack->push(new AnnotationManager::AddAnnotationCommand(m_scene, wm));
}
'''
    pv_cpp += watermark_code
    with open(pdfview_cpp, 'w', encoding='utf-8') as f:
        f.write(pv_cpp)