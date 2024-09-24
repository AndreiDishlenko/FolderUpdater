#ifndef SYNCDIR_H
#define SYNCDIR_H

#include <QObject>
#include <QMultiMap>
//#include <QStringList>

class ALocal : public QObject
{
    Q_OBJECT
    public: ALocal(QObject *parent);
    static QMultiMap<QString, QString> getFilesTree(QString PathDir, QString StartDir);
    static QStringList getConfigList(QMultiMap<QString, QString> configMap, QString varname);
    static bool Sync_Dir(QString SDir_Server, QString SDir_Host, QMultiMap<QString, QString> configMap);
};

#endif // SYNCDIR_H
