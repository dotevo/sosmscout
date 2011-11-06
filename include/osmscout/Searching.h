#ifndef SEARCHING_H
#define SEARCHING_H

#include <vector>

#include <QString>
#include <QList>
#include <QMap>

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
    osmscout::StyleConfig       *styleConfig;

    std::list<AdminRegion> regions;
    QMap<QString, QString> poi;

    void insertPoiTypes();

public:
    Searching();

    static double calculateDistance(double sx, double sy, double dx, double dy);

    void searchAllRegions();

    QMap<QString, QString> getPOIS();

    QList<AdminRegion> searchRegion(QString name);
    QList<Location> searchLocation(QString name, AdminRegion region);

    void searchNode(const int id, NodeRef &nodeRef);
    void searchWay(const int id, WayRef &wayRef);
    void searchRelation(const int id, RelationRef &relationRef);
    void searchObjects(double lonMin, double latMin, double lonMax, double latMax,
                       std::vector<osmscout::NodeRef> &nodes, std::vector<osmscout::WayRef> &ways,
                       std::vector<osmscout::WayRef> &areas, std::vector<osmscout::RelationRef>& relationWays,
                       std::vector<osmscout::RelationRef>& relationAreas);
    void searchPoi(double x, double y, double distance, QString type, QVector<NodeRef> &poiRef);
};

}
#endif // SEARCHING_H
