// ============================================================
// PdfRenderer.cpp — Async Render Engine Implementation
// ============================================================
#include "../include/PdfRenderer.h"
#include "../include/PdfDocument.h"

#include <QDebug>
#include <QtConcurrent>

PdfRenderer::PdfRenderer(PdfDocument* document, QObject* parent)
    : QObject(parent)
    , m_document(document)
    , m_cache(DEFAULT_CACHE_BYTES / (1024 * 4))  // Rough estimate: avg page ~4KB cache cost
{
}

PdfRenderer::~PdfRenderer() {
    clearCache();
}

void PdfRenderer::setCacheLimit(int bytes) {
    m_cache.setMaxCost(bytes / (1024 * 4));
}

void PdfRenderer::clearCache() {
    QMutexLocker lock(&m_queueMutex);
    m_cache.clear();
    m_cachedZoom.clear();
}

void PdfRenderer::requestPage(int pageNum, float zoom) {
    {
        QMutexLocker lock(&m_queueMutex);

        // Check if we already have this page at the correct zoom
        if (m_cachedZoom.value(pageNum, -1.0f) == zoom) {
            QImage* cached = m_cache.object(pageNum);
            if (cached) {
                emit pageRendered(pageNum, *cached, zoom);
                return;
            }
        }

        // Check if already queued
        if (m_pendingPages.contains(pageNum)) {
            return;
        }

        m_pendingPages.insert(pageNum);
    }

    // Process on this thread (which should be the worker thread)
    processRequest(pageNum, zoom);
}

void PdfRenderer::requestPages(const QList<int>& pageNums, float zoom) {
    // Cancel pages not in the new set
    {
        QMutexLocker lock(&m_queueMutex);
        QSet<int> newSet(pageNums.begin(), pageNums.end());

        // Remove pending pages not in the new viewport
        QSet<int> toRemove;
        for (int p : m_pendingPages) {
            if (!newSet.contains(p)) {
                toRemove.insert(p);
            }
        }
        m_pendingPages -= toRemove;
    }

    // Request each page
    for (int pageNum : pageNums) {
        requestPage(pageNum, zoom);
    }
}

void PdfRenderer::cancelAll() {
    QMutexLocker lock(&m_queueMutex);
    m_pendingPages.clear();
}

void PdfRenderer::evictPage(int pageNum) {
    QMutexLocker lock(&m_queueMutex);
    m_cache.remove(pageNum);
    m_cachedZoom.remove(pageNum);
}

void PdfRenderer::processRequest(int pageNum, float zoom) {
    if (!m_document || !m_document->isOpen()) {
        {
            QMutexLocker lock(&m_queueMutex);
            m_pendingPages.remove(pageNum);
        }
        emit renderError(pageNum, "No document is open");
        return;
    }

    // Check if request was cancelled while we were waiting
    {
        QMutexLocker lock(&m_queueMutex);
        if (!m_pendingPages.contains(pageNum)) {
            return;  // Cancelled — skip
        }
    }

    // === THE HEAVY LIFTING — Render on worker thread ===
    QImage image = m_document->renderPage(pageNum, zoom);

    {
        QMutexLocker lock(&m_queueMutex);
        m_pendingPages.remove(pageNum);

        if (!image.isNull()) {
            // Cache the result — cost is approximate pixel data size
            int cost = image.sizeInBytes() / 1024;  // cost in KB
            auto* cached = new QImage(image);
            m_cache.insert(pageNum, cached, qMax(1, cost));
            m_cachedZoom[pageNum] = zoom;
        }
    }

    if (image.isNull()) {
        emit renderError(pageNum, QString("Render returned null for page %1").arg(pageNum));
    } else {
        emit pageRendered(pageNum, image, zoom);
    }
}
