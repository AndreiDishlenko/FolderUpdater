#include <QApplication>
#include "window.h"
#include "aupdater.h"

#include <QDebug>
#include <QDir>
#include <QTime>
#include <QTextCodec>

void myMessageOutput(QtMsgType type, const char *msg) {
    QString filename = QDir::currentPath()+"/updater.log";
    QString prefix = "["+QTime::currentTime().toString("hh:mm:ss")+"]";
    FILE *file;
    file = fopen(filename.toStdString().c_str(), "a");
    fprintf(file, "%s", prefix.toStdString().c_str());
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "%s\n", msg);
        fprintf(file, "%s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
        fprintf(file, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
        fprintf(file, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
        fprintf(file, "Fatal: %s\n", msg);
        abort();
    }
    fclose(file);
}


int main(int argc, char *argv[])
{
    #ifdef QT_NO_DEBUG
        qInstallMsgHandler(myMessageOutput);
    #endif
        bool silentmode = false;
        for (int i=1;i<=argc;i++) {
            QString arg = argv[i];
            if (i < argc) {
                if (arg == "-silentmode") {silentmode=true;}
            }
        }


    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("CP1251"));

    QApplication a(argc, argv);

    Window w;
    AUpdater *aUpd = new AUpdater(&w);
    //qDebug() << "-- Updater started ---------------------";
    if (silentmode==false) {
        w.show();

        QPushButton *pbCheckUpdates = w.findChild<QPushButton*>("pbCheckUpdates");
        QPushButton *pbUpdate = w.findChild<QPushButton*>("pbUpdate");
        QPushButton *pbExit = w.findChild<QPushButton*>("pbExit");

        QObject::connect( pbUpdate, SIGNAL(clicked()), aUpd, SLOT(beginUpdate()) );
        QObject::connect( pbExit, SIGNAL(clicked()), aUpd, SLOT(exitApp()) );

    } else {
        aUpd->beginUpdate();
        exit(0);
        //        qDebug() << "1";
        //        aUpd->exitApp();
        //        qDebug() << "2";
        //        w.close();
        //        qDebug() << "3";
        //        w.~QWidget();
        //        qDebug() << "4";
    }



    return a.exec();
}
