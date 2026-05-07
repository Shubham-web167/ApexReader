// ============================================================
// AnnotationManager.cpp — Plugin API Stub Implementation
// ============================================================
#include "../include/AnnotationManager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDateTime>

// ── Annotation serialization ──

QJsonObject Annotation::toJson() const {
    QJsonObject obj;
    obj["pageNum"]   = pageNum;
    obj["type"]      = static_cast<int>(type);
    obj["rect_x"]    = rect.x();
    obj["rect_y"]    = rect.y();
    obj["rect_w"]    = rect.width();
    obj["rect_h"]    = rect.height();
    obj["text"]      = text;
    obj["color"]     = color.name();
    obj["timestamp"] = timestamp;
    obj["author"]    = author;
    return obj;
}

Annotation Annotation::fromJson(const QJsonObject& obj) {
    Annotation a;
    a.pageNum   = obj["pageNum"].toInt();
    a.type      = static_cast<Annotation::Type>(obj["type"].toInt());
    a.rect      = QRectF(obj["rect_x"].toDouble(), obj["rect_y"].toDouble(),
                         obj["rect_w"].toDouble(), obj["rect_h"].toDouble());
    a.text      = obj["text"].toString();
    a.color     = QColor(obj["color"].toString());
    a.timestamp = obj["timestamp"].toInteger();
    a.author    = obj["author"].toString();
    return a;
}

// ── AnnotationManager ──

AnnotationManager::AnnotationManager(QObject* parent)
    : QObject(parent) {}

AnnotationManager::~AnnotationManager() = default;

void AnnotationManager::addAnnotation(const Annotation& annotation) {
    m_annotations[annotation.pageNum].append(annotation);
    emit annotationAdded(annotation.pageNum, annotation);
}

void AnnotationManager::removeAnnotation(int pageNum, int index) {
    if (m_annotations.contains(pageNum) &&
        index >= 0 && index < m_annotations[pageNum].size()) {
        m_annotations[pageNum].removeAt(index);
        emit annotationRemoved(pageNum, index);
    }
}

QList<Annotation> AnnotationManager::getAnnotations(int pageNum) const {
    return m_annotations.value(pageNum);
}

int AnnotationManager::totalCount() const {
    int count = 0;
    for (auto it = m_annotations.begin(); it != m_annotations.end(); ++it)
        count += it.value().size();
    return count;
}

bool AnnotationManager::saveToFile(const QString& filePath) const {
    QJsonArray arr;
    for (auto it = m_annotations.begin(); it != m_annotations.end(); ++it) {
        for (const auto& a : it.value())
            arr.append(a.toJson());
    }
    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

bool AnnotationManager::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) return false;

    m_annotations.clear();
    const QJsonArray arr = doc.array();
    for (const auto& val : arr) {
        Annotation a = Annotation::fromJson(val.toObject());
        m_annotations[a.pageNum].append(a);
    }
    emit annotationsLoaded(totalCount());
    return true;
}

void AnnotationManager::clearAll() {
    m_annotations.clear();
}
