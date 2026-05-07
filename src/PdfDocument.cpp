#include "../include/PdfDocument.h"
#include "../include/PdfiumWorker.h"

#include "fpdf_text.h"

#include <QEventLoop>
#include <QMetaObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>

PdfDocument::PdfDocument(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QVector<AnnotationData>>();

    m_thread = new QThread(this);
    m_worker = new PdfiumWorker();
    m_worker->moveToThread(m_thread);

    connect(m_worker, &PdfiumWorker::pageRendered, this, &PdfDocument::pageRendered);
    connect(m_worker, &PdfiumWorker::thumbnailRendered, this, &PdfDocument::thumbnailRendered);
    connect(m_worker, &PdfiumWorker::searchFinished, this, &PdfDocument::searchFinished);
    connect(m_worker, &PdfiumWorker::outlineDataReady, this, &PdfDocument::onOutlineData);

    m_thread->start();
}

PdfDocument::~PdfDocument()
{
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "doClose", Qt::BlockingQueuedConnection);
        m_thread->quit();
        m_thread->wait(5000);
        delete m_worker;
        m_worker = nullptr;
    }
}

PdfDocument::OpenResult PdfDocument::open(const QString &filePath, const QString &password)
{
    m_isOpen = false;
    m_pageCount = 0;

    int res = Failed;
    int pc = 0;
    QEventLoop loop;
    QMetaObject::Connection c = connect(
        m_worker, &PdfiumWorker::openFinished, this,
        [&](int r, int p) {
            res = r;
            pc = p;
            loop.quit();
        },
        Qt::QueuedConnection);

    QMetaObject::invokeMethod(m_worker, "doOpen", Qt::QueuedConnection, Q_ARG(QString, filePath),
                              Q_ARG(QString, password));
    loop.exec();
    disconnect(c);

    if (res == static_cast<int>(Success)) {
        m_isOpen = true;
        m_pageCount = pc;
    }
    return static_cast<OpenResult>(res);
}

void PdfDocument::close()
{
    QMetaObject::invokeMethod(m_worker, "doClose", Qt::BlockingQueuedConnection);
    m_isOpen = false;
    m_pageCount = 0;
}

QSizeF PdfDocument::getPageSize(int pageNumber) const
{
    double w = 0, h = 0;
    QMetaObject::invokeMethod(const_cast<PdfiumWorker *>(m_worker), "getPageSizeD",
                              Qt::BlockingQueuedConnection, Q_ARG(int, pageNumber), Q_ARG(double *, &w),
                              Q_ARG(double *, &h));
    return QSizeF(w, h);
}

QImage PdfDocument::renderPageForPrint(int pageNumber, float scale)
{
    QImage img;
    QMetaObject::invokeMethod(m_worker, "getPrintImage", Qt::BlockingQueuedConnection, Q_ARG(int, pageNumber),
                              Q_ARG(float, scale), Q_ARG(QImage *, &img));
    return img;
}

void PdfDocument::requestRenderPage(int pageIndex, int dpi, float layoutZoom, bool darkMode,
                                    quint64 renderToken)
{
    QMetaObject::invokeMethod(m_worker, "doRenderPage", Qt::QueuedConnection, Q_ARG(int, pageIndex),
                              Q_ARG(int, dpi), Q_ARG(float, layoutZoom), Q_ARG(bool, darkMode),
                              Q_ARG(quint64, renderToken));
}

void PdfDocument::requestThumbnail(int pageIndex, float zoomPts, quint64 renderToken)
{
    QMetaObject::invokeMethod(m_worker, "doThumbnail", Qt::QueuedConnection, Q_ARG(int, pageIndex),
                              Q_ARG(float, zoomPts), Q_ARG(quint64, renderToken));
}

void PdfDocument::requestSearch(const QString &query, bool matchCase, float sceneScale)
{
    unsigned long flags = matchCase ? FPDF_MATCHCASE : 0;
    QMetaObject::invokeMethod(m_worker, "doSearch", Qt::QueuedConnection, Q_ARG(QString, query),
                              Q_ARG(unsigned long, flags), Q_ARG(float, sceneScale));
}

void PdfDocument::requestOutline()
{
    QMetaObject::invokeMethod(m_worker, "loadOutlineAsync", Qt::QueuedConnection);
}

int PdfDocument::charIndexAtPos(int pageIndex, double pdfX, double pdfY) const
{
    int r = -1;
    QMetaObject::invokeMethod(const_cast<PdfiumWorker *>(m_worker), "queryCharIndexAtPos",
                              Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, r), Q_ARG(int, pageIndex),
                              Q_ARG(double, pdfX), Q_ARG(double, pdfY));
    return r;
}

int PdfDocument::charCount(int pageIndex) const
{
    int r = -1;
    QMetaObject::invokeMethod(const_cast<PdfiumWorker *>(m_worker), "queryCharCount",
                              Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, r), Q_ARG(int, pageIndex));
    return r;
}

QString PdfDocument::textRange(int pageIndex, int startChar, int charCount) const
{
    QString s;
    QMetaObject::invokeMethod(const_cast<PdfiumWorker *>(m_worker), "queryTextRange",
                              Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, s), Q_ARG(int, pageIndex),
                              Q_ARG(int, startChar), Q_ARG(int, charCount));
    return s;
}

bool PdfDocument::charBox(int pageIndex, int charIndex, double *l, double *r, double *b, double *t) const
{
    bool ok = false;
    QMetaObject::invokeMethod(const_cast<PdfiumWorker *>(m_worker), "queryCharBox",
                              Qt::BlockingQueuedConnection, Q_ARG(int, pageIndex), Q_ARG(int, charIndex),
                              Q_ARG(double *, l), Q_ARG(double *, r), Q_ARG(double *, b), Q_ARG(double *, t),
                              Q_ARG(bool *, &ok));
    return ok;
}

int PdfDocument::countTextRects(int pageIndex, int startChar, int charCount) const
{
    int n = 0;
    QMetaObject::invokeMethod(const_cast<PdfiumWorker *>(m_worker), "queryCountRects",
                              Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, n), Q_ARG(int, pageIndex),
                              Q_ARG(int, startChar), Q_ARG(int, charCount));
    return n;
}

bool PdfDocument::textRect(int pageIndex, int rectIndex, double *l, double *t, double *r,
                           double *b) const
{
    bool ok = false;
    QMetaObject::invokeMethod(const_cast<PdfiumWorker *>(m_worker), "queryTextRect",
                              Qt::BlockingQueuedConnection, Q_ARG(int, pageIndex), Q_ARG(int, rectIndex),
                              Q_ARG(double *, l), Q_ARG(double *, t), Q_ARG(double *, r), Q_ARG(double *, b),
                              Q_ARG(bool *, &ok));
    return ok;
}

void PdfDocument::saveAnnotationsPdf(const QString &path, const QVector<AnnotationData> &items)
{
    QMetaObject::invokeMethod(m_worker, "doSavePdf", Qt::BlockingQueuedConnection, Q_ARG(QString, path),
                              Q_ARG(QVector<AnnotationData>, items));
}

void PdfDocument::loadOutline(QTreeWidget *treeWidget)
{
    m_outlineTree = treeWidget;
    requestOutline();
}

void PdfDocument::onOutlineData(const QVector<QPair<QString, int>> &items)
{
    if (!m_outlineTree)
        return;
    m_outlineTree->clear();
    for (const auto &p : items) {
        auto *it = new QTreeWidgetItem(QStringList() << p.first);
        it->setData(0, Qt::UserRole, p.second);
        m_outlineTree->addTopLevelItem(it);
    }
    m_outlineTree = nullptr;
}
