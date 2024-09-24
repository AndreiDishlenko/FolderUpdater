#include "aftp.h"
#include <QString>
#include <QDebug>
#include <QUrl>
#include <QtCore>
#include <QNetworkAccessManager>
#include <QStringList>
#include <QIODevice>

AFtp::AFtp(QObject *parent) : QFtp(parent) {
    connect(this, SIGNAL(commandFinished(int,bool)),
            this, SLOT(ftpCommandFinished(int,bool)));
    connect(this, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToList(QUrlInfo)));
    connect(this, SIGNAL(stateChanged(int)),
            this, SLOT(stateChanged_slot(int)));
//    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
//            this, SLOT(updateDataTransferProgress(qint64,qint64)));
    connectionstate = false;
}

void AFtp::downloadFile(QString serverFile, QString destFile) {
//    if (QFile::exists(destFile)) {
//        QFile::remove(destFile);
//        writeLog("Файл '"+serverFile+"' присутствует в папке назначения.");
//        return;
//    }
    QFile *file = new QFile(destFile);
    if (!file->open(QIODevice::WriteOnly)) {
        writeLog("Невозможно сохранить файл '"+destFile+"' по причине: '"+file->errorString()+"'");
        delete file;
        return;
    }
//    writeLog(serverFile+" "+destFile+" "+QString::number(this->get(serverFile, file)));
    this->get(serverFile, file);

    while(hasPendingCommands() || currentCommand()!=QFtp::None) {
        qApp->processEvents();
    }
    file->close();
}

void AFtp::writeLog(QString text) {
    emit signal_writeLog(text);
}

//void AFtp::updateStatus(QString text) {
//    emit signal_writeLog(text);
//}

void AFtp::connectServer(QString connectionstring) {
//    writeLog("AFtp::connectServer");
    QUrl url(connectionstring);
    if (!url.isValid() || url.scheme().toLower() != QLatin1String("ftp")) {
        this->connectToHost(connectionstring, 21);
        login();
    } else {
        connectToHost(url.host(), url.port(21));
        if (!url.userName().isEmpty())
            login(QUrl::fromPercentEncoding(url.userName().toLatin1()), url.password());
        else
            login();
        if (!url.path().isEmpty())
            this->cd(url.path());
    }
    while(hasPendingCommands() || currentCommand()!=QFtp::None) {
        qApp->processEvents();
    }
}

void AFtp::ftpCommandFinished(int, bool error) {
    if (currentCommand() == QFtp::ConnectToHost) {
        if (error) {
            writeLog("Невозможно установить связь с FTP сервером. Проверьте адрес и порт подключения.");
            return;
        }
        writeLog("Соединение с сервером установлено.");
        return;
    }
    if (currentCommand() == QFtp::Login) {
        if (error) {
            qDebug() << "Невозможно войти на FTP сервер. Проверьте имя пользователя и пароль.";
            return;
        }
        connectionstate = true;
        writeLog("Вход на сервер выполнен.");

        return;
    }
    if (currentCommand() == QFtp::Get) {
        if (error) {
            qDebug() << "Canceled download of %1.";
        } else {
            qDebug() << "Downloaded %1 to current directory.";
        }
    } else if (currentCommand() == QFtp::List) {
//        qDebug() << "List finished";
//        this->processList();
    }
}

void AFtp::addToList(const QUrlInfo &urlInfo) {
    temp1 << urlInfo;
//    else temp1 <<
}

QMultiMap<QString, QUrlInfo> AFtp::getFtpFilesTree(QString startdir, QString dir) {
//    qDebug() << "AFtp::getFtpFilesTree()";
    QMultiMap<QString, QUrlInfo> result;
    QString tempdir = startdir+"/";
    if (dir!="") {tempdir = tempdir+dir;}
    list(tempdir+"/*.*");
    while(hasPendingCommands() || currentCommand()!=QFtp::None) {
        qApp->processEvents();
    }
    QList<QUrlInfo> temp_dirs;
    for (int i=0;i<temp1.length();i++) {
        if (temp1.at(i).isDir() && temp1.at(i).name()!="." && temp1.at(i).name()!="..") {
            temp_dirs << temp1.at(i);
        } else {
            result.insert(dir, temp1.at(i));
        }
    }
//        result[dir] = temp_files;
    temp1.clear();
    for (int i=0;i<temp_dirs.length();i++) {
        result.unite(AFtp::getFtpFilesTree(startdir, dir + "/" + temp_dirs.at(i).name()));
    }
    return result;
}

void AFtp::stateChanged_slot(int state) {
    if (state==QFtp::Unconnected) {
        writeLog("... Соединение преравно!");
        connectionstate = false;
    }
    if (state==QFtp::Closing) {
        writeLog("... trabl!");
        connectionstate = false;
    }

    //    writeLog(QString::number(state));
}


