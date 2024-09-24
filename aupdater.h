#ifndef AUPDATER_H
#define AUPDATER_H

#include <QObject>
#include <QMap>

#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QProgressBar>

#include "aftp.h"

class AUpdater : public QObject
{
    Q_OBJECT

public:
    explicit AUpdater(QObject *parent = 0);
    QMultiMap<QString, QString> readIniFile(QString filename);
    QStringList getIniParamList(QString varname);
    QString unionFolderNames(QString firstname, QString secondname);
    AFtp *ftp;

    void updateFtp();
    void updateFolder();

    QMultiMap<QString, QString> iniFile;

    QLineEdit *leServerDir;
    QLineEdit *leDestDir;
    QLineEdit *leFtpConnection;
    QCheckBox *cbFtp;
    QPlainTextEdit *pteLog;
    QProgressBar *progressBar;
    QProgressBar *progressBar2;
    QPushButton *pbCheckUpdates;
    QPushButton *pbUpdate;
    QPushButton *pbExit;

    bool FtpUpdate;
    bool mode_updateCheck;
    int updateSize;
    bool ifUpdateAvailable;
    
signals:
    
public slots:
    void checkUpdates();
    void beginUpdate();
    void exitApp();
    void writeLog(QString text);
    void updateFileDownloadStatus(qint64 done, qint64 total);
};

#endif // AUPDATER_H
