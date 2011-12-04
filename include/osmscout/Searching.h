#ifndef SEARCHING_H
#define SEARCHING_H

#include <vector>

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>

#include <osmscout/Node.h>
#include <osmscout/Way.h>
#include <osmscout/AdminRegion.h>
#include <osmscout/Database.h>
#include <osmscout/Routing.h>

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

    class Intersection {
    public:
        Intersection();
        Intersection(QPointF myPos, QPointF cross, QPointF way);
        Intersection(QPointF myPos, QPointF cross, QList<QPointF> ways);
        QPointF myPos;
        QPointF cross;
        QPointF way;
        QList<QPointF> *ways; // at '0' is our road
        //QPointF *ways;
        //int numOfWays;
    };

    Searching();

    /**
      @brief It calculates distance between two points and return results in kilometers.
      @param sx Lon coordinate of first node.
      @param sy Lat coordinate of first node.
      @param dx Lon coordinate of second node.
      @param dy Lat coordinate of second node.
      @return Distance between two nodes.
      */
    static double CalculateDistance(double sx, double sy, double dx, double dy);

    /**
      @brief It checks that actual point is between two other points.
      @param actualPoint Actual node.
      @param firstPoint First node.
      @param secondPoint Second node.
      @param precision Tolerance in meters.
      @return true if <i>actualPoint</i> is between nodes, false otherwise.
      */
    static bool IsBetweenNodes(osmscout::Routing::Step actualPoint,
                               osmscout::Routing::Step firstPoint,
                               osmscout::Routing::Step secondPoint,
                               int precision = 10);

    /**
      @brief Correcting position if signal from satelits is not exactly real.
      @param firstNode Last visited node.
      @param secondNode Next visit node.
      @param position Position received from satelits.
      @param p <i>Projection</i> class from library.
      @param tolerance Maximal tolerance for signal in meters.
      @param changeRouteTolerance Tolerance of accepting new position and left route.
      @return Coordinates of correct position.
      */
    static QPointF CorrectPosition(osmscout::Routing::Step firstNode,
                                    osmscout::Routing::Step secondNode,
                                    QPointF position,
                                    const osmscout::Projection &p,
                                    double tolerance = 3,
                                    double changeRouteTolerance = 20);

    /**
      @brief Simulating of next intersection.
      @param lasNode Previous visited node.
      @param crossingNode Intersection node.
      @param waysNodes Ways nodes.
      @return Intersection in points coordinates form.
      */
    static Intersection SimulateNextCrossing(osmscout::Routing::Step lastNode,
                                     osmscout::Routing::Step crossingNode,
                                     QList<osmscout::Routing::Step> *waysNodes);

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
