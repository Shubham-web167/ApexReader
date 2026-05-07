#pragma once

#include <QGraphicsDropShadowEffect>
#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QList>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPointF>
#include <QPushButton>
#include <QRectF>
#include <QScrollBar>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QQueue>
#include <atomic>
#include <memory>
#include <QMutex>

#include <fpdf_annot.h>
#include <fpdf_edit.h>
#include <fpdf_save.h>
#include <fpdf_searchex.h>
#include <fpdf_text.h>
#include <fpdfview.h>

enum class Tool {
    Hand, SelectText, Highlight, Underline, Strikethrough, Pen, Eraser, OcrSnip, AddNote
};

struct PageData {
    QRectF sceneRect;
    QGraphicsPixmapItem *pixmapItem = nullptr;
    QGraphicsRectItem *placeholder = nullptr;
    bool isRendered = false;
    bool isQueued = false;
    std::shared_ptr<std::atomic<bool>> cancelToken;
};

struct AnnotData {
    int pageIndex = -1;
    QString type;
    QColor color;
    QList<QRectF> rects;
    QList<QPointF> inkPoints;
    QString text;
    QGraphicsItem *sceneItem = nullptr;
};

struct SearchResult {
    int page = -1;
    QRectF sceneRect;
};

// 🚀 Nuke-Proof Background Worker
class RenderWorker : public QObject {
    Q_OBJECT
private:
    std::atomic<uint64_t>* m_epochPtr;
public:
    explicit RenderWorker(std::atomic<uint64_t>* epochPtr) : m_epochPtr(epochPtr) {}

public slots:
    void renderPage(uint64_t epoch, void* docPtr, std::shared_ptr<QMutex> docMutex, int index, QRectF sceneRect, float dpiScale, std::shared_ptr<std::atomic<bool>> token);
signals:
    void pageReady(uint64_t epoch, int index, QImage img, QRectF rect);
};

class PdfView : public QGraphicsView {
    Q_OBJECT
public:
    explicit PdfView(QWidget *parent = nullptr);
    ~PdfView() override;

    bool loadDocument(const QString &path, const QString &password = {});
    void closeDocument();
    bool isLoaded() const { return m_doc != nullptr; }
    int pageCount() const { return m_pageCount; }
    int currentPage() const;
    QString filePath() const { return m_filePath; }

    void setTool(Tool t);
    Tool currentTool() const { return m_tool; }
    void setAnnotationColor(const QColor &c) { m_annotColor = c; }
    QColor annotColor() const { return m_annotColor; }

    void zoomIn();
    void zoomOut();
    void setZoomLevel(float z);
    void setZoomPercent(int pct) { setZoomLevel(float(pct) / 100.0f); }
    float zoomLevel() const { return m_zoom; }
    void fitToWidth();
    void fitToPage();
    void scrollToPage(int idx);

    void toggleDarkMode();
    bool isNightMode() const { return m_nightMode; }

    void searchText(const QString &q, bool matchCase);
    void searchNext();
    void searchPrev();
    void clearSearch();

    void saveAnnotations(const QString &outPath);
    void clearAllAnnotations();

    void setActiveTool(Tool t) { setTool(t); }
    void goToNextPage() { scrollToPage(currentPage() + 1); }
    void goToPrevPage() { scrollToPage(currentPage() - 1); }
    void loadAnnotations(const QString &) {}
    class QUndoStack *undoStack() const { return nullptr; }

public slots:
    void onPageReady(uint64_t epoch, int idx, QImage img, QRectF r);
    void processNextThumbnail();

signals:
    void pageChanged(int current, int total);
    void thumbReady(int idx, QPixmap pix);
    void statusMsg(QString msg);
    void statusMessage(const QString &msg);
    void zoomChanged(int pct);
    void docLoaded(int pages);
    void thumbnailReady(int page, const QImage &thumb);
    void currentPageChanged(int page);
    void toolChanged(const QString &toolName);
    void searchResultsChanged(int currentIndex, int totalMatches);

    void reqRender(uint64_t epoch, void* docPtr, std::shared_ptr<QMutex> docMutex, int idx, QRectF rect, float dpi, std::shared_ptr<std::atomic<bool>> token);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void scrollContentsBy(int dx, int dy) override;
    void resizeEvent(QResizeEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    FPDF_DOCUMENT m_doc = nullptr;
    QString m_filePath;
    int m_pageCount = 0;

    std::shared_ptr<QMutex> m_docMutex;
    std::atomic<uint64_t> m_epoch{0};
    QVector<QSizeF> m_pageSizes;

    QGraphicsScene *m_scene = nullptr;
    QVector<PageData> m_pages;

    QThread *m_renderThread = nullptr;
    RenderWorker *m_worker = nullptr;
    QSet<int> m_queued;

    QTimer *m_hdTimer = nullptr;

    QQueue<int> m_thumbQueue;
    QTimer *m_thumbTimer = nullptr;

    Tool m_tool = Tool::Hand;
    float m_zoom = 1.0f;
    bool m_nightMode = false;
    QColor m_annotColor = QColor(255, 255, 0, 160);

    static constexpr float GAP = 16.f;
    static constexpr float MARGIN = 40.f;

    QList<AnnotData> m_annotations;
    bool m_pressing = false;
    QPointF m_pressScene;
    QGraphicsRectItem *m_selOverlay = nullptr;
    QGraphicsRectItem *m_rubberBand = nullptr;
    bool m_penDown = false;
    QPainterPath m_penPath;
    QGraphicsPathItem *m_penItem = nullptr;

    QList<SearchResult> m_searchResults;
    QList<QGraphicsRectItem *> m_searchOverlays;
    int m_searchIdx = -1;

    void layoutPages();
    void renderVisible(float dpiScale = 1.0f);
    int pageAt(QPointF scenePos) const;
    QRectF pdfToScene(int pg, double l, double t, double r, double b) const;
    void applyHighlight(int pg, QRectF sr, const QColor &c, bool line, float lineY);
    void startPen(QPointF sp);
    void contPen(QPointF sp);
    void endPen();
    void eraseAt(QPointF sp);
    void placeNote(QPointF sp);
    void doOcr(QRectF sr);
    void rebuildSearchOverlays();
    void invertPixmap(QGraphicsPixmapItem *item);
    bool saveDocumentCopy(const QString &outPath);
};
