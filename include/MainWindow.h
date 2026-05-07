#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "PdfDocument.h"
#include "PdfView.h"

#include <QLineEdit>
#include <QDockWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QTabBar>
#include <QProgressBar>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onOpenPdfManual(const QString &path);
    void onOpenPdf();
    void onSearchTriggered();
    void onSearchNext();
    void onSearchPrev();
    void onThumbnailClicked(QListWidgetItem *item);
    void onOutlineItemClicked(QTreeWidgetItem *item, int column);
    void toggleFullScreen();
    void onMergePdfs();
    void onSplitPdf();
    void onRotateLeft();
    void onRotateRight();
    void onPrint();
    void onAddWatermark();
    void onCopyPageAsImage();
    void onPresentationMode();
    void onZoomToSelection();
    void onPickColor();
    void onShowProperties();
    void updateRecentFilesMenu();
    void openRecentFile(const QString &path);

private slots:
    void onTabChanged(int index);
    void onTabClosed(int index);
    void rebuildThumbnailStrip();
    void onPdfThumbnail(int page, const QImage &img);
    void onPdfPageChanged(int page);

private:
    struct PdfTab {
        QString filePath;
        PdfDocument *document;
        QString title;
    };

    QMenu *m_fileMenu = nullptr;
    QMenu *m_recentMenu = nullptr;
    QStringList m_recentFiles;
    PdfView *m_pdfView = nullptr;
    QString m_currentFilePath;
    QLineEdit *m_searchBox = nullptr;
    class QCheckBox *m_matchCaseCheck = nullptr;
    QDockWidget *m_thumbnailDock = nullptr;
    QListWidget *m_thumbnailList = nullptr;
    QTabWidget *m_sidebarTabs = nullptr;
    QTreeWidget *m_outlineTree = nullptr;

    QLabel *m_statusToolLabel = nullptr;
    QLabel *m_statusZoomLabel = nullptr;

    QToolBar *m_fileToolBar = nullptr;
    QToolBar *m_navigationToolBar = nullptr;
    QToolBar *m_toolsToolBar = nullptr;
    QTabBar *m_tabBar = nullptr;
    QProgressBar *m_loadingProgress = nullptr;
    QActionGroup *m_toolsActionGroup = nullptr;
    class QSpinBox *m_zoomSpinBox = nullptr;
    class QSpinBox *m_pageSpinBox = nullptr;
    QLabel *m_totalPagesLabel = nullptr;
    QLabel *m_matchLabel = nullptr;

    bool m_isFullScreen = false;
    bool m_isPresentationMode = false;
    QColor m_currentAnnotationColor;
    class QToolButton *m_colorBtn = nullptr;

    QList<PdfTab> m_tabs;
    int m_currentTab = -1;

    void updateColorButton();
    void enterPresentationMode();
    void exitPresentationMode();

protected:
    void keyPressEvent(class QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif
