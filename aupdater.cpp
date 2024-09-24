#include <QtCore>
#include "QFile"
#include <QUrlInfo>

#include "aupdater.h"
#include "aftp.h"
#include "alocal.h"

AUpdater::AUpdater(QObject *parent) : QObject(parent) {
    leServerDir = parent->findChild<QLineEdit*>("leServerDir");
    leDestDir = parent->findChild<QLineEdit*>("leDestDir");
    leFtpConnection = parent->findChild<QLineEdit*>("leFtpConnection");
    cbFtp = parent->findChild<QCheckBox*>("cbFtp");
    pteLog = parent->findChild<QPlainTextEdit*>("pteLog");
    progressBar = parent->findChild<QProgressBar*>("progressBar");
    progressBar2 = parent->findChild<QProgressBar*>("progressBar2");
    pbUpdate = parent->findChild<QPushButton*>("pbUpdate");
    pbExit = parent->findChild<QPushButton*>("pbExit");

    QString iniFileName = QDir::currentPath()+"/updater.ini";

    if (!QFile(iniFileName).exists()) {writeLog("Не возможно открыть файл конфигурации '"+iniFileName+"'"); return;}
    iniFile = this->readIniFile(QDir::currentPath()+"/updater.ini");
    if (iniFile.find("ServerDir").value()!="") { leServerDir->setText(iniFile.find("ServerDir").value());}
    if (iniFile.find("DestDir").value()!="") {leDestDir->setText(iniFile.find("DestDir").value());}
    if (iniFile.find("FtpConnectionString").value()!="") {leFtpConnection->setText(iniFile.find("FtpConnectionString").value());}
    if (iniFile.find("FtpUpdate").value()=="1") {cbFtp->setCheckState(Qt::Checked);} else {cbFtp->setCheckState(Qt::Unchecked);}
    writeLog("Файл конфигурации: данные прочитаны.");

    ifUpdateAvailable = false;

    ftp = new AFtp(this);
    QObject::connect(ftp, SIGNAL(signal_writeLog(QString)), this, SLOT(writeLog(QString)));
    QObject::connect(ftp, SIGNAL(dataTransferProgress(qint64, qint64)), this, SLOT(updateFileDownloadStatus(qint64, qint64)));
}

void AUpdater::updateFileDownloadStatus(qint64 done, qint64 total){
    progressBar2->setMinimum(0);
    progressBar2->setMaximum(total);
    progressBar2->setValue(done);
}

QStringList AUpdater::getIniParamList(QString varname) {
    QStringList result;
    QMultiMap<QString, QString>::iterator iter = iniFile.begin();
    while (iter!=iniFile.end()) {
        if (iter.key()==varname) {result << iter.value();}
        iter++;
    }
    return result;
}

void AUpdater::updateFtp() {
    qDebug() << "updateFtp";
    if (ftp->connectionstate==true) {
        QString ServerDir = leServerDir->text();
        QString DestDir = leDestDir->text();

        QMultiMap<QString , QUrlInfo> :: iterator it;
        QMultiMap<QString , QString> :: iterator it2;

        QMultiMap<QString, QUrlInfo> ServerDirFiles = ftp->getFtpFilesTree(ServerDir);
//        for ( it = ServerDirFiles.begin(); it != ServerDirFiles.end(); ++it) {
//            qDebug() << "1" << it.key() << it.value().name();
//        }
        QMultiMap<QString, QString> DestDirFiles = ALocal::getFilesTree(DestDir, DestDir/*, Start_DirHt*/);
//        for ( it2 = DestDirFiles.begin(); it2 != DestDirFiles.end(); ++it2) {
//            qDebug() << "2" << it2.key() << it2.value();
//        }

        QStringList exceptFiles = this->getIniParamList("exceptFile");
        QStringList exceptDirs = this->getIniParamList("exceptDir");

        QDir tempDir;
        bool temp=false;

        // Удаление файлов на хосте, если их нет на сервере
        for ( it2 = DestDirFiles.begin(); it2 != DestDirFiles.end(); ++it2) {
            QString tempDestDir = it2.key();     // внутренняя папка
            QString tempDestFile = it2.value();  // голое имя файла
//            qDebug() << "1" << it2.key() << it2.value();
// Проверка на исключения exceptDir
            for (int i=0;i<exceptDirs.count();i++) { if ( tempDestDir.left(exceptDirs.at(i).length()) == exceptDirs.at(i) ) temp=true; }
            if (temp==true) {temp=false; continue;}
            if (exceptFiles.contains(tempDestDir+"/"+tempDestFile)) {continue;}
// Удаляет файл, если он отсутствует на сервере
            bool temp_file=false, temp_dir=false;
            for ( it = ServerDirFiles.begin(); it != ServerDirFiles.end(); ++it) {
//                qDebug() << "2" << it.key() << it.value().name();
                if (tempDestDir==it.key()) temp_dir=true;
                if (tempDestDir==it.key() && tempDestFile==it.value().name()) temp_file=true;
            }
            if(temp_file==false && tempDestFile!="") {
                QFile targetFile(DestDir+tempDestDir+"/"+tempDestFile);       // конечный файл (полный путь)
                if (!mode_updateCheck){
                    writeLog("- Удаление файла: "+targetFile.fileName());
                    targetFile.remove();
                } else {ifUpdateAvailable=true;}
            }
// Удаляет папку, если он отсутствует на сервере
            if(!temp_dir && tempDestFile=="") {
                if (!mode_updateCheck){
                    writeLog("- Удаление папки: "+DestDir+tempDestDir);
                    tempDir.rmpath(DestDir + tempDestDir);
                } else {ifUpdateAvailable=true;}
            }
        }

        DestDirFiles = ALocal::getFilesTree(DestDir, DestDir/*, Start_DirHt*/);
        for ( it = ServerDirFiles.begin(); it != ServerDirFiles.end(); ++it) {
            QString tempServerDir = it.key();     // внутренняя папка
            QString tempServerFile = it.value().name();  // голое имя файла

            for (int i=0;i<exceptDirs.count();i++) { if ( tempServerDir.left(exceptDirs.at(i).length()) == exceptDirs.at(i) ) temp=true; }
            if (temp==true) {temp=false; continue;}
            if (exceptFiles.contains(tempServerDir+"/"+tempServerFile)) {continue;}

//            QFile serverFile(ServerDir+tempServerDir+"/"+tempServerFile);       // исходный файл
            QString serverFileName = ServerDir+tempServerDir+"/"+tempServerFile;
            QString targetFileName = DestDir+tempServerDir+"/"+tempServerFile;
            QFile targetFile(targetFileName);       // конечный файл (полный путь)
// Создает папку на Хосте, если ее нет
            if(!tempDir.exists(DestDir+"/"+tempServerDir)){
                if (!mode_updateCheck){
                    writeLog("- Новая папка: "+DestDir+tempServerDir);
                    tempDir.mkdir(DestDir+"/"+tempServerDir);
                } else {ifUpdateAvailable=true;}
            }
            if (tempServerFile=="." || tempServerFile==".." || tempServerFile=="") continue;
// Копирует файл в хост, если его нет
            if(!DestDirFiles.contains(tempServerDir, tempServerFile)){
                if (!mode_updateCheck){
                    writeLog("- Новый файл: "+targetFileName);
    //                serverFile.copy(targetFile.fileName());
                    ftp->downloadFile(serverFileName, targetFileName);
                    progressBar->setValue(progressBar->value()+it.value().size());
                } else {ifUpdateAvailable=true; updateSize += it.value().size(); }
            }
            else if (tempServerFile!="") {
            // Заменяет файл на хосте, если не совпадают даты
//                QFileInfo servF_inf(serverFile);
                QFileInfo hostF_inf(targetFile);
//                QDateTime serverFileModified = it.value().lastModified();//servF_inf.lastModified();
//                QDateTime targetFileModified = hostF_inf.lastModified();
                int serverFileSize = it.value().size(); //serverFile.size();
                int targetFileSize = targetFile.size();
                if (serverFileSize != targetFileSize){
                    if (!mode_updateCheck){
                        writeLog("- Обновление файла: "+targetFileName);
                        ftp->downloadFile(serverFileName, targetFileName);
                        progressBar->setValue(progressBar->value()+it.value().size());
    //                    serverFile.copy(targetFile.fileName());
                    } else {ifUpdateAvailable=true; updateSize += it.value().size(); }
                }
            }
        }
    }
}

void AUpdater::updateFolder() {
        QString ServerDir = leServerDir->text();
        QString DestDir = leDestDir->text();

        QMultiMap<QString, QString> ServerDirFiles = ALocal::getFilesTree(ServerDir, ServerDir);
        QMultiMap<QString, QString> DestDirFiles = ALocal::getFilesTree(DestDir, DestDir);

        QStringList exceptFiles = this->getIniParamList("exceptFile");
        QStringList exceptDirs = this->getIniParamList("exceptDir");

        QDir tempDir;
        bool temp=false;

        // Удаление файлов на хосте, если их нет на сервере
        QMultiMap<QString , QString> :: iterator it;
        QMultiMap<QString , QString> :: iterator it2;
        for ( it2 = DestDirFiles.begin(); it2 != DestDirFiles.end(); ++it2) {
            QString tempDestDir = it2.key();     // внутренняя папка
            QString tempDestFile = it2.value();  // голое имя файла

            // Проверка на исключения exceptDir
            for (int i=0;i<exceptDirs.count();i++) { if ( tempDestDir.left(exceptDirs.at(i).length()) == exceptDirs.at(i) ) temp=true; }
            if (temp==true) {temp=false; continue;}
            if (exceptFiles.contains(tempDestDir+"/"+tempDestFile)) {continue;}

            // Удаляет файл, если он отсутствует на сервере
            bool temp_file=false, temp_dir=false;
            for ( it = ServerDirFiles.begin(); it != ServerDirFiles.end(); ++it) {
                if (tempDestDir==it.key()) temp_dir=true;
                if (tempDestDir==it.key() && tempDestFile==it.value()) temp_file=true;
            }
            if(temp_file==false && tempDestFile!="") {
                QFile targetFile(DestDir+tempDestDir+"/"+tempDestFile);       // конечный файл (полный путь)
                if (!mode_updateCheck){
                    writeLog("- Удаление файла: "+targetFile.fileName());
                    targetFile.remove();
                } else {ifUpdateAvailable=true;}
            }
// Удаляет папку, если он отсутствует на сервере
            if(!temp_dir && tempDestFile=="") {
                if (!mode_updateCheck){
                    writeLog("- Удаление папки: "+DestDir+tempDestDir);
                    tempDir.rmpath(DestDir + tempDestDir);
                } else {ifUpdateAvailable=true;}
            }
        }
        DestDirFiles = ALocal::getFilesTree(DestDir, DestDir);
        for ( it = ServerDirFiles.begin(); it != ServerDirFiles.end(); ++it) {
            QString tempServerDir = it.key();     // внутренняя папка
            QString tempServerFile = it.value();  // голое имя файла

            for (int i=0;i<exceptDirs.count();i++) { if ( tempServerDir.left(exceptDirs.at(i).length()) == exceptDirs.at(i) ) temp=true; }
            if (temp==true) {temp=false; continue;}
            if (exceptFiles.contains(tempServerDir+"/"+tempServerFile)) {continue;}

            QString serverFileName = ServerDir+tempServerDir+"/"+tempServerFile;
            QFile serverFile(serverFileName);
            QString targetFileName = DestDir+tempServerDir+"/"+tempServerFile;
            QFile targetFile(targetFileName);

//            writeLog(serverFileName+" "+targetFileName+" !"+tempServerDir+" !"+tempServerFile);

            // Создает папку на Хосте, если ее нет
            if(!tempDir.exists(DestDir+"/"+tempServerDir)){
                if (!mode_updateCheck){
                    writeLog("- Новая папка: "+DestDir+tempServerDir);
                    tempDir.mkdir(DestDir+"/"+tempServerDir);
                } else {ifUpdateAvailable=true;}
            }
            if (tempServerFile=="." || tempServerFile==".." || tempServerFile=="") continue;

            // Копирует файл в хост, если его нет
            if(!DestDirFiles.contains(tempServerDir, tempServerFile)){
                if (!mode_updateCheck){
                    writeLog("- Новый файл: "+targetFileName);
                    serverFile.copy(targetFile.fileName());
//                    ftp->downloadFile(serverFileName, targetFileName);
                    progressBar->setValue(progressBar->value()+it.value().size());
                } else {ifUpdateAvailable=true; updateSize += it.value().size(); }
            }
            else if (tempServerFile!="") {
            // Заменяет файл на хосте, если не совпадают даты
//                QFileInfo servF_inf(serverFile);
                QFileInfo hostF_inf(targetFile);
//                QDateTime serverFileModified = it.value().lastModified();//servF_inf.lastModified();
//                QDateTime targetFileModified = hostF_inf.lastModified();
                int serverFileSize = serverFile.size(); //serverFile.size();
                int targetFileSize = targetFile.size();
                if (serverFileSize != targetFileSize){
                    if (!mode_updateCheck){
                        writeLog("- Обновление файла: "+targetFileName);
//                        ftp->downloadFile(serverFileName, targetFileName);
                        targetFile.remove();
                        serverFile.copy(targetFile.fileName());
                        progressBar->setValue(progressBar->value()+it.value().size());
                    } else {ifUpdateAvailable=true; updateSize += it.value().size(); }
                }
            }
        }

}

void AUpdater::beginUpdate() {
    writeLog("Обновление началось");
    pbUpdate->setEnabled(false);
    pbExit->setEnabled(false);
    progressBar->setValue(0);
    progressBar2->setValue(0);
    if (!ifUpdateAvailable) {checkUpdates();}
    if (ifUpdateAvailable) {
        if (updateSize==0) {updateSize=1;}
        progressBar->setMinimum(0);
        //writeLog("updateSize"+QString::number(updateSize));
        progressBar->setMaximum(updateSize);
        bool FtpUpdate = false;
        if (cbFtp->checkState()==Qt::Checked) {FtpUpdate = true;}

        if (FtpUpdate) {
            // Обновление из FTP папки
//            pteLog->clear();
            if (ftp->connectionstate==false) {ftp->setTransferMode(QFtp::Active); ftp->connectServer(leFtpConnection->text()); }

            updateFtp();
            writeLog("Обновление завершено!");
        } else {
            // Обновление из сетевой папки
            updateFolder();
        }
        progressBar->setValue(updateSize);
    }
    ifUpdateAvailable = false;
    pbUpdate->setEnabled(true);
    pbExit->setEnabled(true);
    writeLog("------------------------------------------");
}

void AUpdater::checkUpdates() {
    writeLog("Проверка обновлений.");
    bool FtpUpdate = false;
    if (cbFtp->checkState()==Qt::Checked) {FtpUpdate = true;}
    if (FtpUpdate) {
        // Обновление из FTP папки
        if (ftp->connectionstate==false) {ftp->connectServer(leFtpConnection->text());}

        mode_updateCheck = true;
        updateSize = 0;
        ifUpdateAvailable = false;
        updateFtp();
        mode_updateCheck = false;
    } else {
        // Обновление из сетевой папки
        updateFolder();
    }
    if (ifUpdateAvailable) {
        writeLog("Доступны обновления.");
        writeLog("Размер обновлений: "+QString::number(updateSize));
    } else {
        writeLog("Новых обновлений нет.");
    }
}

QMultiMap<QString, QString>  AUpdater::readIniFile(QString filename) {
    QMultiMap<QString, QString>  result;
    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        writeLog("Warning (readIniFile): Can't open "+filename+". "+file.errorString());
        return result;
    }
    QString line, K, T;
    QTextStream accIn(&file), lineIn(&line);
    while (!accIn.atEnd()) {
        line = accIn.readLine();
        lineIn.flush();
                int pos = line.indexOf("=",0);
                if (pos==0) continue;
                K = line.mid(0, pos).trimmed();
                T = line.mid(pos+1, line.length()-pos).trimmed();
                if (T.right(1)=="/") {T.chop(1);}
                if (K=="exceptDir" && T.left(1)!="/") {T.insert(0, "/");}
                if (K=="exceptFile" && T.left(1)!="/") {T.insert(0, "/");}
                if (K.left(2)!="//" && K!="") {
                    result.insert(K, T);
                }
    }
    file.close();
    return result;
}

void AUpdater::writeLog(QString text) {
    pteLog->appendPlainText(text);
    qDebug() << text;
}

void AUpdater::exitApp() {
    ftp->close();
    qApp->exit();
}
