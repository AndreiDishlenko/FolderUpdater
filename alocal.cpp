#include "alocal.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QDateTime>

ALocal::ALocal(QObject *parent) : QObject(parent) {
}

QMultiMap<QString , QString> ALocal::getFilesTree(QString PathDir, QString startDir) {
    QDir dir(PathDir);
    QString filter = "*";
    QString tempString;
    QStringList listFiles = dir.entryList(filter.split(" "), QDir::Files);
    QMultiMap<QString , QString> MMlistFiles;
    for(int i=0;i<listFiles.count();i++) {
        tempString = PathDir;
        tempString.replace(startDir, "");
        MMlistFiles.insert(tempString, listFiles.at(i));
    }
    QStringList listDir = dir.entryList(QDir::AllDirs);
    for (int i=0;i<listDir.count();i++) {
        QString subdir = listDir.at(i);
        if (subdir!="." && subdir!="..") {
            tempString = PathDir;
            tempString.replace(startDir, "");
            MMlistFiles.insert(tempString+"/"+subdir, "");
            MMlistFiles = MMlistFiles + ALocal::getFilesTree(PathDir+"/"+subdir, startDir);
        }
    }
    return MMlistFiles;
}

QStringList ALocal::getConfigList(QMultiMap<QString, QString> configMap, QString varname) {
    QStringList result;
    QMultiMap<QString, QString>::iterator iter = configMap.begin();
    while (iter!=configMap.end()) {
        if (iter.key()==varname) {result << iter.value();}
        iter++;
    }
    return result;
}

bool ALocal::Sync_Dir(QString ServerDir, QString DestDir, QMultiMap<QString, QString> configMap){
//    qDebug() << "ALocal::Sync_Dir" << ServerDir << DestDir;
    bool result = true;

    QMultiMap<QString , QString> ServerDirFiles = ALocal::getFilesTree(ServerDir, ServerDir/*, Start_DirSr*/);
    QMultiMap<QString , QString> DestDirFiles = ALocal::getFilesTree(DestDir, DestDir/*, Start_DirHt*/);

    QStringList exceptFiles = ALocal::getConfigList(configMap, "exceptFile");
    QStringList exceptDirs = ALocal::getConfigList(configMap, "exceptDir");

    QDir tempDir;
    bool temp=false;

    QMultiMap<QString , QString> :: iterator it;
    for ( it = ServerDirFiles.begin(); it != ServerDirFiles.end(); ++it){
        QString tempServerDir = it.key();     // внутренняя папка
        QString tempServerFile = it.value();  // голое имя файла

        for (int i=0;i<exceptDirs.count();i++) { if ( tempServerDir.left(exceptDirs.at(i).length()) == exceptDirs.at(i) ) temp=true; }
        if (temp==true) {temp=false; continue;}
        if (exceptFiles.contains(tempServerDir+"/"+tempServerFile)) {continue;}



        QFile serverFile(ServerDir+tempServerDir+"/"+tempServerFile);       // исходный файл
        QFile targetFile(DestDir + tempServerDir+"/"+tempServerFile);       // конечный файл (полный путь)

        // Проверка на исключения exceptDir

        // Создает папку на Хосте, если ее нет
        if(!tempDir.exists(DestDir+"/"+tempServerDir)){
            qDebug() << "Create folder" << DestDir+tempServerDir << tempServerDir;
            tempDir.mkdir(DestDir+"/"+tempServerDir);
        }
        // Копирует файл в хост, если его нет
        if(!DestDirFiles.contains(tempServerDir, tempServerFile)){
            qDebug() << "Copy new file" << targetFile.fileName();
            serverFile.copy(targetFile.fileName());
        } else if (tempServerFile!="") {
        // Заменяет файл на хосте, если не совпадают даты
            QFileInfo servF_inf(serverFile);
            QFileInfo hostF_inf(targetFile);
            QDateTime serverFileModified = servF_inf.lastModified();
            QDateTime targetFileModified = hostF_inf.lastModified();
            int serverFileSize = serverFile.size();
            int targetFileSize = targetFile.size();
            if((serverFileSize != targetFileSize) | (targetFileModified != serverFileModified && !exceptFiles.contains(tempServerDir + "/" + tempServerFile))){
                qDebug() << "Replace exists file" << targetFile.fileName();
                targetFile.remove();
                serverFile.copy(targetFile.fileName());
            }
        }
    }

    // Удаление файлов на хосте, если их нет на сервере
    for ( it = DestDirFiles.begin(); it != DestDirFiles.end(); ++it){
        QString tempDestDir = it.key();     // внутренняя папка
        QString tempDestFile = it.value();  // голое имя файла

        // Проверка на исключения exceptDir
        for (int i=0;i<exceptDirs.count();i++) { if ( tempDestDir.left(exceptDirs.at(i).length()) == exceptDirs.at(i) ) temp=true; }
        if (temp==true) {temp=false; continue;}
        if (exceptFiles.contains(tempDestDir+"/"+tempDestFile)) {continue;}

        // Удаляет файл, если он отсутствует на сервере
        if(!ServerDirFiles.contains(tempDestDir, tempDestFile) && tempDestFile!="" && !exceptFiles.contains(tempDestDir+"/"+tempDestFile)) {
            QFile targetFile(DestDir+tempDestDir+"/"+tempDestFile);       // конечный файл (полный путь)
            qDebug() << "Remove deleted file" << targetFile.fileName();
            targetFile.remove();
        }
        if(!tempDir.exists(ServerDir+tempDestDir) && tempDestFile=="") {
            qDebug() << "Remove deleted folder" << DestDir + tempDestDir;
            tempDir.rmpath(DestDir + tempDestDir);
        }

    }
    return result;
}
