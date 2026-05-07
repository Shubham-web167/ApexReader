#include "../include/PdfiumWorker.h"
#include "../include/PdfDocument.h"

#include <QFile>
#include <QDebug>
#include <QtMath>
#include <cstring>
#include <atomic>

#include "fpdfview.h"
#include "fpdf_text.h"
#include "fpdf_save.h"
#include "fpdf_annot.h"
#include "fpdf_doc.h"

namespace {

std::atomic<int> g_pdfiumUsers{0};

void pdfiumAddRef()
{
    if (g_pdfiumUsers.fetch_add(1) == 0) {
        FPDF_LIBRARY_CONFIG cfg{};
        cfg.version = 2;
        FPDF_InitLibraryWithConfig(&cfg);
    }
}

void pdfiumRelease()
{
    if (g_pdfiumUsers.fetch_sub(1) == 1)
        FPDF_DestroyLibrary();
}

static QImage bitmapToImage(FPDF_BITMAP bm, int w, int h, bool invertRgb)
{
    if (!bm)
        return {};
    const int stride = FPDFBitmap_GetStride(bm);
    const unsigned char *src = static_cast<const unsigned char *>(FPDFBitmap_GetBuffer(bm));
    QImage img(w, h, QImage::Format_RGBA8888);
    for (int y = 0; y < h; ++y) {
        const unsigned char *row = src + y * stride;
        uchar *dst = img.scanLine(y);
        for (int x = 0; x < w; ++x) {
            const unsigned char b = row[0], g = row[1], r = row[2], a = row[3];
            dst[0] = r;
            dst[1] = g;
            dst[2] = b;
            dst[3] = a;
            row += 4;
            dst += 4;
        }
    }
    if (invertRgb)
        img.invertPixels(QImage::InvertRgb);
    return img;
}

static QString utf16leToQString(const unsigned short *buf, int len)
{
    return QString::fromUtf16(reinterpret_cast<const char16_t *>(buf), len);
}

} // namespace

PdfiumWorker::PdfiumWorker(QObject *parent) : QObject(parent)
{
    pdfiumAddRef();
}

PdfiumWorker::~PdfiumWorker()
{
    doClose();
    pdfiumRelease();
}

void PdfiumWorker::doOpen(const QString &path, const QString &password)
{
    doClose();
    m_path = path;
    QByteArray p = path.toUtf8();
    QByteArray pw = password.toUtf8();
    FPDF_DOCUMENT doc = FPDF_LoadDocument(p.constData(), pw.isEmpty() ? nullptr : pw.constData());
    if (!doc) {
        unsigned long err = FPDF_GetLastError();
        if (err == FPDF_ERR_PASSWORD) {
            emit openFinished(static_cast<int>(PdfDocument::NeedsPassword), 0);
            return;
        }
        qWarning() << "FPDF_LoadDocument failed" << err;
        emit openFinished(static_cast<int>(PdfDocument::Failed), 0);
        return;
    }
    m_document = doc;
    int n = FPDF_GetPageCount(doc);
    emit openFinished(static_cast<int>(PdfDocument::Success), n);
}

void PdfiumWorker::doClose()
{
    if (m_document) {
        FPDF_CloseDocument(static_cast<FPDF_DOCUMENT>(m_document));
        m_document = nullptr;
    }
    m_path.clear();
}

void PdfiumWorker::doRenderPage(int pageIndex, int dpi, float layoutZoom, bool darkMode,
                                quint64 renderToken)
{
    if (!m_document)
        return;
    FPDF_PAGE page = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), pageIndex);
    if (!page)
        return;

    const double pw = FPDF_GetPageWidth(page);
    const double ph = FPDF_GetPageHeight(page);
    const double scale = (static_cast<double>(dpi) / 72.0) * static_cast<double>(layoutZoom);
    int w = qMax(1, int(qCeil(pw * scale)));
    int h = qMax(1, int(qCeil(ph * scale)));

    FPDF_BITMAP bm = FPDFBitmap_Create(w, h, 0);
    FPDFBitmap_FillRect(bm, 0, 0, w, h, 0xFFFFFFFF);
    FPDF_RenderPageBitmap(bm, page, 0, 0, w, h, 0, FPDF_ANNOT | FPDF_LCD_TEXT);
    QImage img = bitmapToImage(bm, w, h, darkMode);
    FPDFBitmap_Destroy(bm);
    FPDF_ClosePage(page);

    emit pageRendered(pageIndex, QPixmap::fromImage(std::move(img)), dpi, renderToken);
}

void PdfiumWorker::doThumbnail(int pageIndex, float zoomPts, quint64 renderToken)
{
    if (!m_document)
        return;
    FPDF_PAGE page = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), pageIndex);
    if (!page)
        return;
    const double pw = FPDF_GetPageWidth(page);
    const double ph = FPDF_GetPageHeight(page);
    const double scale = static_cast<double>(zoomPts);
    int w = qMax(1, int(qCeil(pw * scale)));
    int h = qMax(1, int(qCeil(ph * scale)));
    FPDF_BITMAP bm = FPDFBitmap_Create(w, h, 0);
    FPDFBitmap_FillRect(bm, 0, 0, w, h, 0xFFFFFFFF);
    FPDF_RenderPageBitmap(bm, page, 0, 0, w, h, 0, FPDF_ANNOT | FPDF_LCD_TEXT);
    QImage img = bitmapToImage(bm, w, h, false);
    FPDFBitmap_Destroy(bm);
    FPDF_ClosePage(page);
    emit thumbnailRendered(pageIndex, std::move(img), renderToken);
}

void PdfiumWorker::doSearch(const QString &query, unsigned long flags, float sceneScale)
{
    QList<QPair<int, QRectF>> all;
    if (!m_document || query.isEmpty()) {
        emit searchFinished(all);
        return;
    }
    FPDF_DOCUMENT doc = static_cast<FPDF_DOCUMENT>(m_document);
    const int pages = FPDF_GetPageCount(doc);
    const QVector<ushort> utf16 = query.utf16();
    const FPDF_WIDESTRING wstr = reinterpret_cast<FPDF_WIDESTRING>(utf16.constData());

    for (int pi = 0; pi < pages; ++pi) {
        FPDF_PAGE page = FPDF_LoadPage(doc, pi);
        if (!page)
            continue;
        FPDF_TEXTPAGE tp = FPDFText_LoadPage(page);
        if (!tp) {
            FPDF_ClosePage(page);
            continue;
        }
        FPDF_SCHHANDLE h = FPDFText_FindStart(tp, wstr, flags, 0);
        if (h) {
            while (FPDFText_FindNext(h)) {
                const int si = FPDFText_GetSchResultIndex(h);
                const int cnt = FPDFText_GetSchCount(h);
                if (cnt <= 0)
                    continue;
                const int nrect = FPDFText_CountRects(tp, si, cnt);
                for (int ri = 0; ri < nrect; ++ri) {
                    double l, t, r, b;
                    if (FPDFText_GetRect(tp, ri, &l, &t, &r, &b)) {
                        QRectF scene(l * sceneScale, (FPDF_GetPageHeight(page) - t) * sceneScale,
                                     qMax(0.0, r - l) * sceneScale,
                                     qMax(0.0, t - b) * sceneScale);
                        all.append(qMakePair(pi, scene));
                    }
                }
            }
            FPDFText_FindClose(h);
        }
        FPDFText_ClosePage(tp);
        FPDF_ClosePage(page);
    }
    emit searchFinished(all);
}

struct QIODeviceWriteContext {
    QFile *file = nullptr;
};

static int writeBlockCb(FPDF_FILEWRITE *pThis, const void *pData, unsigned long size)
{
    auto *ctx = reinterpret_cast<QIODeviceWriteContext *>(pThis->stream);
    if (!ctx || !ctx->file)
        return 0;
    return ctx->file->write(static_cast<const char *>(pData), static_cast<qint64>(size)) == static_cast<qint64>(size)
               ? 1
               : 0;
}

void PdfiumWorker::doSavePdf(const QString &path, QVector<AnnotationData> annotations)
{
    if (m_path.isEmpty() || path.isEmpty()) {
        emit saveFinished(false, QStringLiteral("No document"));
        return;
    }
    QByteArray srcPath = m_path.toUtf8();
    FPDF_DOCUMENT doc = FPDF_LoadDocument(srcPath.constData(), nullptr);
    if (!doc) {
        emit saveFinished(false, QStringLiteral("Reload failed"));
        return;
    }

    for (const AnnotationData &a : annotations) {
        if (a.pageIndex < 0 || a.pageIndex >= FPDF_GetPageCount(doc))
            continue;
        FPDF_PAGE page = FPDF_LoadPage(doc, a.pageIndex);
        if (!page)
            continue;

        if (a.type == QLatin1String("highlight")) {
            FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(page, FPDF_ANNOT_HIGHLIGHT);
            if (annot) {
                FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, a.color.red(), a.color.green(),
                                   a.color.blue(), a.color.alpha());
                const float l = float(a.pdfRect.left());
                const float r = float(a.pdfRect.right());
                const float b = float(a.pdfRect.bottom());
                const float t = float(a.pdfRect.top());
                FS_QUADPOINTSF q{};
                q.x1 = l;
                q.y1 = t;
                q.x2 = r;
                q.y2 = t;
                q.x3 = r;
                q.y3 = b;
                q.x4 = l;
                q.y4 = b;
                FPDFAnnot_AppendAttachmentPoints(annot, &q);
            }
        } else if (a.type == QLatin1String("underline")) {
            FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(page, FPDF_ANNOT_UNDERLINE);
            if (annot) {
                FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, a.color.red(), a.color.green(),
                                   a.color.blue(), a.color.alpha());
                FS_RECTF rect{};
                rect.left = float(a.pdfRect.left());
                rect.right = float(a.pdfRect.right());
                rect.bottom = float(a.pdfRect.bottom());
                rect.top = float(a.pdfRect.top());
                FPDFAnnot_SetRect(annot, &rect);
            }
        } else if (a.type == QLatin1String("strikethrough")) {
            FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(page, FPDF_ANNOT_STRIKEOUT);
            if (annot) {
                FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, a.color.red(), a.color.green(),
                                   a.color.blue(), a.color.alpha());
                FS_RECTF rect{};
                rect.left = float(a.pdfRect.left());
                rect.right = float(a.pdfRect.right());
                rect.bottom = float(a.pdfRect.bottom());
                rect.top = float(a.pdfRect.top());
                FPDFAnnot_SetRect(annot, &rect);
            }
        }
        // ink / freetext: skip if API surface too large for this pass

        FPDF_ClosePage(page);
    }

    QFile out(path);
    if (!out.open(QIODevice::WriteOnly)) {
        FPDF_CloseDocument(doc);
        emit saveFinished(false, QStringLiteral("Cannot write"));
        return;
    }

    FPDF_FILEWRITE fw{};
    fw.version = 1;
    fw.WriteBlock = &writeBlockCb;
    QIODeviceWriteContext ctx{&out};
    fw.stream = &ctx;

    const FPDF_BOOL ok = FPDF_SaveAsCopy(doc, &fw, FPDF_NO_INCREMENTAL);
    out.close();
    FPDF_CloseDocument(doc);
    emit saveFinished(ok, ok ? QString() : QStringLiteral("Save failed"));
}

int PdfiumWorker::queryPageCount()
{
    if (!m_document)
        return 0;
    return FPDF_GetPageCount(static_cast<FPDF_DOCUMENT>(m_document));
}

void PdfiumWorker::getPageSizeD(int pageIndex, double *w, double *h)
{
    if (!w || !h)
        return;
    *w = 0;
    *h = 0;
    if (!m_document)
        return;
    FPDF_GetPageSizeByIndex(static_cast<FPDF_DOCUMENT>(m_document), pageIndex, w, h);
}

void PdfiumWorker::getPrintImage(int page, float scale, QImage *out)
{
    if (!out)
        return;
    *out = QImage();
    if (!m_document)
        return;
    FPDF_PAGE p = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), page);
    if (!p)
        return;
    const double pw = FPDF_GetPageWidth(p);
    const double ph = FPDF_GetPageHeight(p);
    const double sc = static_cast<double>(scale) * (150.0 / 72.0);
    int w = qMax(1, int(qCeil(pw * sc)));
    int h = qMax(1, int(qCeil(ph * sc)));
    FPDF_BITMAP bm = FPDFBitmap_Create(w, h, 0);
    FPDFBitmap_FillRect(bm, 0, 0, w, h, 0xFFFFFFFF);
    FPDF_RenderPageBitmap(bm, p, 0, 0, w, h, 0, FPDF_ANNOT | FPDF_LCD_TEXT);
    *out = bitmapToImage(bm, w, h, false);
    FPDFBitmap_Destroy(bm);
    FPDF_ClosePage(p);
}

int PdfiumWorker::queryCharIndexAtPos(int pageIndex, double pdfX, double pdfY)
{
    if (!m_document)
        return -1;
    FPDF_PAGE page = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), pageIndex);
    if (!page)
        return -1;
    FPDF_TEXTPAGE tp = FPDFText_LoadPage(page);
    int idx = -1;
    if (tp)
        idx = FPDFText_GetCharIndexAtPos(tp, pdfX, pdfY, 5.0, 5.0);
    if (tp)
        FPDFText_ClosePage(tp);
    FPDF_ClosePage(page);
    return idx;
}

int PdfiumWorker::queryCharCount(int pageIndex)
{
    if (!m_document)
        return -1;
    FPDF_PAGE page = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), pageIndex);
    if (!page)
        return -1;
    FPDF_TEXTPAGE tp = FPDFText_LoadPage(page);
    int n = tp ? FPDFText_CountChars(tp) : -1;
    if (tp)
        FPDFText_ClosePage(tp);
    FPDF_ClosePage(page);
    return n;
}

QString PdfiumWorker::queryTextRange(int pageIndex, int startChar, int charCount)
{
    if (!m_document || charCount <= 0)
        return {};
    FPDF_PAGE page = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), pageIndex);
    if (!page)
        return {};
    FPDF_TEXTPAGE tp = FPDFText_LoadPage(page);
    if (!tp) {
        FPDF_ClosePage(page);
        return {};
    }
    const int need = FPDFText_GetText(tp, startChar, charCount, nullptr);
    if (need <= 0) {
        FPDFText_ClosePage(tp);
        FPDF_ClosePage(page);
        return {};
    }
    QVector<unsigned short> buf(need + 1);
    FPDFText_GetText(tp, startChar, charCount, buf.data());
    FPDFText_ClosePage(tp);
    FPDF_ClosePage(page);
    return utf16leToQString(buf.constData(), need - 1);
}

void PdfiumWorker::queryCharBox(int pageIndex, int charIndex, double *left, double *right,
                                double *bottom, double *top, bool *ok)
{
    if (ok)
        *ok = false;
    if (!m_document || !ok)
        return;
    FPDF_PAGE page = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), pageIndex);
    if (!page)
        return;
    FPDF_TEXTPAGE tp = FPDFText_LoadPage(page);
    if (tp) {
        *ok = FPDFText_GetCharBox(tp, charIndex, left, right, bottom, top);
        FPDFText_ClosePage(tp);
    }
    FPDF_ClosePage(page);
}

int PdfiumWorker::queryCountRects(int pageIndex, int startChar, int charCount)
{
    if (!m_document)
        return 0;
    FPDF_PAGE page = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), pageIndex);
    if (!page)
        return 0;
    FPDF_TEXTPAGE tp = FPDFText_LoadPage(page);
    int n = 0;
    if (tp)
        n = FPDFText_CountRects(tp, startChar, charCount);
    if (tp)
        FPDFText_ClosePage(tp);
    FPDF_ClosePage(page);
    return n;
}

void PdfiumWorker::queryTextRect(int pageIndex, int rectIndex, double *left, double *top, double *right,
                                 double *bottom, bool *ok)
{
    if (ok)
        *ok = false;
    if (!m_document || !ok)
        return;
    FPDF_PAGE page = FPDF_LoadPage(static_cast<FPDF_DOCUMENT>(m_document), pageIndex);
    if (!page)
        return;
    FPDF_TEXTPAGE tp = FPDFText_LoadPage(page);
    if (tp) {
        *ok = FPDFText_GetRect(tp, rectIndex, left, top, right, bottom);
        FPDFText_ClosePage(tp);
    }
    FPDF_ClosePage(page);
}

static void collectBookmarks(FPDF_DOCUMENT doc, FPDF_BOOKMARK bm, int level,
                             QVector<QPair<QString, int>> *out)
{
    while (bm) {
        unsigned long len = FPDFBookmark_GetTitle(bm, nullptr, 0);
        QVector<unsigned short> title(len + 1);
        FPDFBookmark_GetTitle(bm, title.data(), (unsigned long)title.size() * sizeof(ushort));
        QString text = QString::fromUtf16(reinterpret_cast<const char16_t *>(title.constData()));

        FPDF_DEST dest = FPDFBookmark_GetDest(doc, bm);
        int page = 0;
        if (dest) {
            page = FPDFDest_GetDestPageIndex(doc, dest);
            if (page < 0)
                page = 0;
        }

        out->append(qMakePair(QString(level * 2, QChar(' ')) + text, page));

        FPDF_BOOKMARK child = FPDFBookmark_GetFirstChild(doc, bm);
        if (child)
            collectBookmarks(doc, child, level + 1, out);

        bm = FPDFBookmark_GetNextSibling(doc, bm);
    }
}

void PdfiumWorker::loadOutlineAsync()
{
    QVector<QPair<QString, int>> items;
    if (m_document) {
        FPDF_BOOKMARK root = FPDFBookmark_GetFirstBookmark(static_cast<FPDF_DOCUMENT>(m_document));
        if (root)
            collectBookmarks(static_cast<FPDF_DOCUMENT>(m_document), root, 0, &items);
    }
    emit outlineDataReady(items);
}
