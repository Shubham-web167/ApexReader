// ============================================================
// AnnotationManager.h — Plugin API Stub (Phase 4)
// Provides a clean C++ interface where future annotation,
// notes, or AI modules can plug in.
// ============================================================
#ifndef ANNOTATIONMANAGER_H
#define ANNOTATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QRectF>
#include <QColor>
#include <QList>
#include <QJsonObject>

/**
 * @brief Annotation — Represents a single annotation on a PDF page.
 */
struct Annotation {
    enum class Type {
        Highlight,
        Underline,
        StrikeOut,
        FreeText,
        StickyNote,
        Drawing
    };

    int     pageNum = 0;
    Type    type    = Type::Highlight;
    QRectF  rect;                     // Bounding rect in page coordinates
    QString text;                     // Annotation text / note content
    QColor  color   = Qt::yellow;
    qint64  timestamp = 0;            // Unix timestamp of creation
    QString author;

    /** @brief Serialize to JSON */
    [[nodiscard]] QJsonObject toJson() const;
    /** @brief Deserialize from JSON */
    static Annotation fromJson(const QJsonObject& obj);
};

/**
 * @brief AnnotationManager — Clean interface for annotation CRUD.
 *
 * This is a stub API designed for future integration with:
 *   - Python-based AI note generation modules
 *   - External annotation storage backends
 *   - Collaborative editing features
 *
 * All methods are virtual so they can be overridden by plugins.
 */
class AnnotationManager : public QObject {
    Q_OBJECT

public:
    explicit AnnotationManager(QObject* parent = nullptr);
    ~AnnotationManager() override;

    /** @brief Add an annotation to a page */
    virtual void addAnnotation(const Annotation& annotation);

    /** @brief Remove an annotation by index */
    virtual void removeAnnotation(int pageNum, int index);

    /** @brief Get all annotations for a specific page */
    [[nodiscard]] virtual QList<Annotation> getAnnotations(int pageNum) const;

    /** @brief Get total annotation count across all pages */
    [[nodiscard]] virtual int totalCount() const;

    /** @brief Save annotations to a JSON file */
    virtual bool saveToFile(const QString& filePath) const;

    /** @brief Load annotations from a JSON file */
    virtual bool loadFromFile(const QString& filePath);

    /** @brief Clear all annotations */
    virtual void clearAll();

signals:
    void annotationAdded(int pageNum, const Annotation& annotation);
    void annotationRemoved(int pageNum, int index);
    void annotationsLoaded(int count);

private:
    QHash<int, QList<Annotation>> m_annotations;  // pageNum -> list
};

#endif // ANNOTATIONMANAGER_H
