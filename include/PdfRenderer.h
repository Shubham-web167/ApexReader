// ============================================================
// PdfRenderer.h — The Worker Thread
// Renders PDF pages asynchronously on a separate QThread.
// Communicates with UI ONLY via signals/slots.
// ============================================================
#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include <QObject>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QSet>
#include <QHash>
#include <QCache>

class PdfDocument;

/**
 * @brief RenderRequest — Data packet sent to the worker thread.
 */
struct RenderRequest {
    int   pageNum = 0;
    float zoom    = 1.0f;
    int   priority = 0;  // Lower = higher priority (visible pages first)
};

/**
 * @brief PdfRenderer — Async rendering engine.
 *
 * Lives on a dedicated QThread. Receives render requests from the UI thread
 * and emits pageRendered() signals back with the rendered QImage.
 *
 * Implements:
 *   - Request deduplication (won't re-render if already cached)
 *   - Priority queue (visible pages first)
 *   - Render cache with configurable memory limit
 *   - Cancellation of stale requests
 */
class PdfRenderer : public QObject {
    Q_OBJECT

public:
    explicit PdfRenderer(PdfDocument* document, QObject* parent = nullptr);
    ~PdfRenderer() override;

    /** @brief Set the maximum cache size in bytes (default: 256 MB) */
    void setCacheLimit(int bytes);

    /** @brief Clear the entire render cache */
    void clearCache();

public slots:
    /**
     * @brief Request rendering of a specific page.
     * Safe to call from any thread — uses queued connection.
     */
    void requestPage(int pageNum, float zoom);

    /**
     * @brief Request rendering of a batch of pages (e.g. viewport + buffer).
     * Cancels any pending requests not in this batch.
     */
    void requestPages(const QList<int>& pageNums, float zoom);

    /**
     * @brief Cancel all pending render requests.
     */
    void cancelAll();

    /**
     * @brief Notify that a page has left the viewport — evict from cache.
     */
    void evictPage(int pageNum);

signals:
    /**
     * @brief Emitted when a page has been rendered.
     * @param pageNum  The page number that was rendered
     * @param image    The rendered page image
     * @param zoom     The zoom level at which it was rendered
     */
    void pageRendered(int pageNum, const QImage& image, float zoom);

    /**
     * @brief Emitted when rendering encounters an error.
     */
    void renderError(int pageNum, const QString& error);

private:
    void processRequest(int pageNum, float zoom);

    PdfDocument* m_document;            // Non-owning pointer to the model
    QMutex       m_queueMutex;
    QSet<int>    m_pendingPages;        // Pages currently queued for render
    QHash<int, float> m_cachedZoom;     // Track zoom level of cached pages
    QCache<int, QImage> m_cache;        // LRU cache for rendered pages

    static constexpr int DEFAULT_CACHE_BYTES = 256 * 1024 * 1024; // 256 MB
};

#endif // PDFRENDERER_H
