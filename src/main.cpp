#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QIcon>
#include "../include/MainWindow.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFile file("debug_log.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") << msg << Qt::endl;
    }
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(myMessageOutput);
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app_icon.ico"));

    try {
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Startup Error",
            QString("Fatal error: %1").arg(e.what()));
        return -1;
    } catch (...) {
        QMessageBox::critical(nullptr, "Startup Error",
            "Unknown fatal error on startup!");
        return -1;
    }
}
