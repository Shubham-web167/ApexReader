#ifndef PDFIUMWORKER_H
#define PDFIUMWORKER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <QRectF>
#include <QSizeF>
#include <QList>
#include <QPair>
#include <QVector>

#include "AnnotationData.h"

class PdfiumWorker : public QObject {
    Q_OBJECT
public:
    explicit PdfiumWorker(QObject *parent = nullptr);
    ~PdfiumWorker() override;

public slots:
    void doOpen(const QString &path, const QString &password);
    void doClose();

    void doRenderPage(int pageIndex, int dpi, float layoutZoom, bool darkMode, quint64 renderToken);
    void doThumbnail(int pageIndex, float zoomPts, quint64 renderToken);

    void doSearch(const QString &query, unsigned long flags, float sceneScale);

    void doSavePdf(const QString &path, QVector<AnnotationData> annotations);

    int queryPageCount();
    void getPageSizeD(int pageIndex, double *w, double *h);
    void getPrintImage(int page, float scale, QImage *out);
    int queryCharIndexAtPos(int pageIndex, double pdfX, double pdfY);
    int queryCharCount(int pageIndex);
    QString queryTextRange(int pageIndex, int startChar, int charCount);
    void queryCharBox(int pageIndex, int charIndex, double *left, double *right, double *bottom,
                      double *top, bool *ok);
    int queryCountRects(int pageIndex, int startChar, int charCount);
    void queryTextRect(int pageIndex, int rectIndex, double *left, double *top, double *right,
                       double *bottom, bool *ok);

signals:
    void openFinished(int result, int pageCount);
    void pageRendered(int index, QPixmap pixmap, int dpi, quint64 renderToken);
    void thumbnailRendered(int index, QImage image, quint64 renderToken);
    void searchFinished(QList<QPair<int, QRectF>> hits);
    void saveFinished(bool ok, QString errorMessage);
    void outlineDataReady(QVector<QPair<QString, int>> items);

public:
    void loadOutlineAsync();

private:
    void *m_document = nullptr;
    QString m_path;
};

#endif
