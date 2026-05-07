#ifndef PDFOPERATIONS_H
#define PDFOPERATIONS_H

#include <QStringList>
#include <QString>

class PdfOperations {
public:
    static bool mergePdfs(const QStringList& inputPaths, const QString& outputPath);
    static bool splitPdf(const QString& inputPath, int fromPage, int toPage, const QString& outputPath);
};

#endif // PDFOPERATIONS_H