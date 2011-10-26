#include "MString.h"

#include <QChar>

namespace osmscout {

    MString::MString()
    {
        // no code
    }

    MString::~MString()
    {

    }

    QString MString::modifyCharacters(QString text)
    {
        QString tmp = text;

        tmp = tmp.replace('ą', 'a', Qt::CaseInsensitive);
        tmp = tmp.replace('ć', 'c', Qt::CaseInsensitive);
        tmp = tmp.replace('ę', 'e', Qt::CaseInsensitive);
        tmp = tmp.replace('ł', 'l', Qt::CaseInsensitive);
        tmp = tmp.replace('ń', 'n', Qt::CaseInsensitive);
        tmp = tmp.replace('ó', 'o', Qt::CaseInsensitive);
        tmp = tmp.replace('ś', 's', Qt::CaseInsensitive);
        tmp = tmp.replace('ź', 'z', Qt::CaseInsensitive);
        tmp = tmp.replace('ż', 'z', Qt::CaseInsensitive);

        return tmp;
}

}
