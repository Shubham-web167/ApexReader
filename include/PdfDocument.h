#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <QObject>
#include <QImage>
#include <QSizeF>
#include <QRectF>
#include <QThread>
#include <QList>
#include <QPair>
#include <QVector>

#include "AnnotationData.h"

class QTreeWidget;
class PdfiumWorker;

class PdfDocument : public QObject {
    Q_OBJECT
public:
    explicit PdfDocument(QObject *parent = nullptr);
    ~PdfDocument() override;

    enum OpenResult { Success = 0, NeedsPassword = 1, Failed = 2 };
    Q_ENUM(OpenResult)

    OpenResult open(const QString &filePath, const QString &password = QString());
    void close();

    bool isOpen() const { return m_isOpen; }
    int getPageCount() const { return m_pageCount; }
    QSizeF getPageSize(int pageNumber) const;

    QImage renderPageForPrint(int pageNumber, float scale);

    void requestRenderPage(int pageIndex, int dpi, float layoutZoom, bool darkMode, quint64 renderToken);
    void requestThumbnail(int pageIndex, float zoomPts, quint64 renderToken);
    void requestSearch(const QString &query, bool matchCase, float sceneScale);
    void requestOutline();

    int charIndexAtPos(int pageIndex, double pdfX, double pdfY) const;
    int charCount(int pageIndex) const;
    QString textRange(int pageIndex, int startChar, int charCount) const;
    bool charBox(int pageIndex, int charIndex, double *l, double *r, double *b, double *t) const;
    int countTextRects(int pageIndex, int startChar, int charCount) const;
    bool textRect(int pageIndex, int rectIndex, double *l, double *t, double *r, double *b) const;

    void saveAnnotationsPdf(const QString &path, const QVector<AnnotationData> &items);

    void loadOutline(QTreeWidget *treeWidget);

signals:
    void pageRendered(int index, QPixmap pixmap, int dpi, quint64 renderToken);
    void thumbnailRendered(int index, QImage image, quint64 renderToken);
    void searchFinished(QList<QPair<int, QRectF>> hits);

private slots:
    void onOutlineData(const QVector<QPair<QString, int>> &items);

private:
    QThread *m_thread = nullptr;
    PdfiumWorker *m_worker = nullptr;
    bool m_isOpen = false;
    int m_pageCount = 0;
    QTreeWidget *m_outlineTree = nullptr;
};

#endif
