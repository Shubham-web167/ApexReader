#ifndef ANNOTATIONDATA_H
#define ANNOTATIONDATA_H

#include <QRectF>
#include <QString>
#include <QColor>
#include <QList>
#include <QPointF>
#include <QVector>
#include <QMetaType>

struct AnnotationData {
    int pageIndex = 0;
    QRectF pdfRect;
    QString type;
    QColor color;
    QString text;
    QList<QPointF> inkPoints;
};

Q_DECLARE_METATYPE(QVector<AnnotationData>)

#endif
