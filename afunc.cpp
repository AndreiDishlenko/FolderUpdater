#include "afunc.h"


afunc::afunc(QObject *parent) :
    QObject(parent) {
}

QString afunc::procFolderName(QString name) {
    if (name=="") return name;
//    if (name.right(1)=="/") {name.remove(name.length()-1,1);}
    if (name.right(1)!="/") {name.append("/");}
}

QString afunc::procFileName(QString name) {
    if (name.left(1)=="/") {name.remove(0,1); }
    if (name.right(1)=="/") {name.remove(name.length()-1,1);}
}

QString afunc::procPath(QString name) {
    if (name.right(1)=="/") {name.remove(name.length()-1,1);}
}
