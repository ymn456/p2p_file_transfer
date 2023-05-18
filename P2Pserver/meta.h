#ifndef META_H
#define META_H


#include <QString>
#include <QVector>
struct meta{
    meta(){}
    meta(QString);

    QString filename;
    qint64 size;
    QString owner;
    QString ip;
    qint64 port;
    QString filepath;
    QString online;

    bool status;

    QString toString();
};



#endif // META_H
