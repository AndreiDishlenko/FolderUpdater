#ifndef AFTP_H
#define AFTP_H

#include <QFtp>
#include <QNetworkRequest>
#include <QApplication>

class AFtp : public QFtp {
    Q_OBJECT
public:
    AFtp(QObject *parent=0);
    void connectServer(QString connectionstring);

    QList<QUrlInfo> temp1;
//     entryList;
//    QString curDir;
//    QApplication qApp;

    bool connectionstate;

    QMultiMap<QString, QUrlInfo> getFtpFilesTree(QString startdir, QString dir="");
    void downloadFile(QString serverFile, QString destFile);

    void processList();
    void endAction();
    void writeLog(QString text);

private:

public slots:
    void stateChanged_slot(int state);
    void ftpCommandFinished(int, bool error);
    void addToList(const QUrlInfo &urlInfo);

signals:
    void signal_writeLog(QString text);

};

#endif // AFTP_H
