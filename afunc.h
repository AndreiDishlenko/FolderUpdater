#ifndef AFUNC_H
#define AFUNC_H

#include <QObject>
#include <QString>

class afunc : public QObject
{
    Q_OBJECT
public:
    explicit afunc(QObject *parent = 0);
    static QString procFolderName(QString name);
    static QString procFileName(QString name);
    static QString procPath(QString name);

    
signals:
    
public slots:
    
};

#endif // AFUNC_H
