#include "../include/PdfOperations.h"

#include <QFile>
#include <QDebug>

#include "fpdfview.h"
#include "fpdf_save.h"
#include "fpdf_ppo.h"
#include "fpdf_edit.h"

namespace {

struct QIODeviceWriteContext {
    FPDF_FILEWRITE base;
    QFile *file = nullptr;
};

int writeBlockCb(FPDF_FILEWRITE *pThis, const void *pData, unsigned long size)
{
    auto *ctx = reinterpret_cast<QIODeviceWriteContext *>(pThis);
    if (!ctx || !ctx->file)
        return 0;
    return ctx->file->write(static_cast<const char *>(pData), static_cast<qint64>(size))
                   == static_cast<qint64>(size)
               ? 1
               : 0;
}

static bool saveDocument(FPDF_DOCUMENT doc, const QString &path)
{
    QFile out(path);
    if (!out.open(QIODevice::WriteOnly))
        return false;
    QIODeviceWriteContext ctx{};
    ctx.base.version = 1;
    ctx.base.WriteBlock = &writeBlockCb;
    ctx.file = &out;
    const FPDF_BOOL ok = FPDF_SaveAsCopy(doc, &ctx.base, FPDF_NO_INCREMENTAL);
    out.close();
    return ok;
}

} // namespace

bool PdfOperations::mergePdfs(const QStringList &inputPaths, const QString &outputPath)
{
    if (inputPaths.size() < 2 || outputPath.isEmpty())
        return false;

    FPDF_DOCUMENT dst = FPDF_CreateNewDocument();
    if (!dst)
        return false;

    for (const QString &path : inputPaths) {
        QByteArray utf8 = path.toUtf8();
        FPDF_DOCUMENT src = FPDF_LoadDocument(utf8.constData(), nullptr);
        if (!src) {
            FPDF_CloseDocument(dst);
            return false;
        }
        const int n = FPDF_GetPageCount(src);
        QVector<int> indices(n);
        for (int i = 0; i < n; ++i)
            indices[i] = i;
        if (!FPDF_ImportPagesByIndex(dst, src, indices.constData(), static_cast<unsigned long>(n),
                                     FPDF_GetPageCount(dst))) {
            FPDF_CloseDocument(src);
            FPDF_CloseDocument(dst);
            return false;
        }
        FPDF_CloseDocument(src);
    }

    const bool ok = saveDocument(dst, outputPath);
    FPDF_CloseDocument(dst);
    return ok;
}

bool PdfOperations::splitPdf(const QString &inputPath, int fromPage, int toPage,
                               const QString &outputPath)
{
    if (inputPath.isEmpty() || outputPath.isEmpty() || fromPage > toPage || fromPage < 0)
        return false;

    QByteArray utf8 = inputPath.toUtf8();
    FPDF_DOCUMENT src = FPDF_LoadDocument(utf8.constData(), nullptr);
    if (!src)
        return false;

    FPDF_DOCUMENT dst = FPDF_CreateNewDocument();
    if (!dst) {
        FPDF_CloseDocument(src);
        return false;
    }

    const int total = FPDF_GetPageCount(src);
    int end = qMin(toPage, total - 1);
    QVector<int> indices;
    for (int i = fromPage; i <= end; ++i)
        indices.append(i);

    const bool imp = FPDF_ImportPagesByIndex(dst, src, indices.constData(),
                                             static_cast<unsigned long>(indices.size()), 0);
    FPDF_CloseDocument(src);
    if (!imp) {
        FPDF_CloseDocument(dst);
        return false;
    }
    const bool ok = saveDocument(dst, outputPath);
    FPDF_CloseDocument(dst);
    return ok;
}
