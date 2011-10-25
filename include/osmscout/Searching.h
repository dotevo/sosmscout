#ifndef SEARCHING_H
#define SEARCHING_H

#include <vector>

#include <QString>
#include <QList>

#include <osmscout/Node.h>
#include <osmscout/Way.h>
#include <osmscout/AdminRegion.h>
#include <osmscout/Database.h>

namespace osmscout {

/**
  Represents searching POIs and address operations.
  */
class Searching
{
private:
    osmscout::DatabaseParameter databaseParameter;
    osmscout::Database          *database;

    std::list<AdminRegion> regions;

public:
    Searching();
    void searchAllRegions();

    QList<AdminRegion> searchRegion(QString name);
    QList<Location> searchLocation(QString name, AdminRegion region);

    void searchNode(const int id, NodeRef &nodeRef);
    void searchWay(const int id, WayRef &wayRef);
};

}
#endif // SEARCHING_H
