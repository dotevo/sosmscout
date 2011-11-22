#include "osmscout/Searching.h"

#include <QList>
#include <QDebug>

#include "settings.h"
#include "qmath.h"

#include <osmscout/Database.h>
#include <osmscout/StyleConfigLoader.h>
#include <osmscout/util/Projection.h>
#include <osmscout/MapPainter.h>
#include <osmscout/AdminRegion.h>
#include <osmscout/StyleConfig.h>

namespace osmscout {

Searching::Searching()
{
    database = new Database(databaseParameter);

    QString map = Settings::getInstance()->getMapPath();
    QString style = Settings::getInstance()->getMapStylePath();

    if (!database->Open((const char*)map.toAscii())) {
        std::cerr << "Cannot open database" << std::endl;
    } else {
    //    std::cerr << "Database is open!" << std::endl;
    }

    styleConfig = new osmscout::StyleConfig(database->GetTypeConfig());

    if (!osmscout::LoadStyleConfig((const char*)style.toAscii(),*(styleConfig))) {
        std::cerr << "Cannot open style" << std::endl;
    }

    insertPoiTypes();
//    searchAllRegions();
}

void Searching::insertPoiTypes()
{
    poi.setInsertInOrder(true);

    poi.insert("-choose", "--choose--");
    poi.insert("bus_stop", "Bus stop");
}

QMap<QString, QString> Searching::getPOIS()
{
    return poi;
}

void Searching::searchAllRegions()
{
    const QString searchName = "";
    size_t size = 1000000;
    bool reached;

    if (!database->GetMatchingAdminRegions(searchName, regions, size, reached, false)) {
        std::cerr << "Cannot load admin regions" << std::endl;
    } else {
        std::cerr << "Admin regions are loaded!" << std::endl;
    }

    for (std::list<osmscout::AdminRegion>::const_iterator r = regions.begin(); r!=regions.end(); ++r) {
            const osmscout::AdminRegion region = *r;

            //std::cout << region.name.toStdString() << " " << region.reference.GetTypeName() << std::endl;

        }
}

QList<AdminRegion> Searching::searchRegion(QString name)
{
    size_t size = 100;
    std::list<osmscout::AdminRegion> regions;
    bool reached;

    database->GetMatchingAdminRegions(name, regions, size, reached, false);

    QList<AdminRegion> qRegions;
	foreach(AdminRegion el, regions)
		qRegions.append(el);
    return qRegions;

    /*
    for (std::list<osmscout::AdminRegion>::const_iterator r = regions.begin(); r!=regions.end(); ++r) {
        const osmscout::AdminRegion region = *r;

        std::cout << region.name.toStdString() << " " << region.reference.GetTypeName() << std::endl;

    }
    std::cerr << "Regions size: " << regions.size() << std::endl;
    */
}

QList<Location> Searching::searchLocation(QString name, AdminRegion region)
{
    std::list<Location> locations;
    size_t max_size = 100;
    bool reached;

    database->GetMatchingLocations(region, name, locations, max_size, reached, false);

    QList<Location> qLocations;
	foreach(Location el, locations)
		qLocations.append(el);

    return qLocations;

    /*
    for (std::list<osmscout::Location>::const_iterator l = locations.begin(); l!=locations.end(); ++l) {
        const osmscout::Location location = *l;

        std::cout << location.name.toStdString() << std::endl;
    }
    std::cerr << "Locations size: " << locations.size() << std::endl;
    */
}

void Searching::searchNode(const int id, NodeRef &node)
{
    database->GetNode(id, node);
}

void Searching::searchWay(const int id, WayRef &wayRef)
{
    database->GetWay(id, wayRef);
}

void Searching::searchRelation(const int id, RelationRef &relationRef)
{
    database->GetRelation(id, relationRef);
}

void Searching::searchObjects(double lonMin, double latMin, double lonMax, double latMax,
                              std::vector<osmscout::NodeRef> &nodes, std::vector<osmscout::WayRef> &ways,
                              std::vector<osmscout::WayRef> &areas, std::vector<osmscout::RelationRef> &relationWays,
                              std::vector<osmscout::RelationRef> &relationAreas)
{
    osmscout::AreaSearchParameter areaSearchParameter;

    database->GetObjects(*(styleConfig), lonMin, latMin, lonMax, latMax, osmscout::magVeryClose, areaSearchParameter,
                         nodes, ways, areas, relationWays, relationAreas);
}

void Searching::searchPoi(double x, double y, double distance, QString type, QVector<NodeRef> &poiRef)
{
    std::vector<osmscout::NodeRef> nodes;
    std::vector<osmscout::WayRef> ways;
    std::vector<osmscout::WayRef> areas;
    std::vector<osmscout::RelationRef> relationWays;
    std::vector<osmscout::RelationRef> relationAreas;

    osmscout::AreaSearchParameter areaSearchParameter;

    database->GetObjects(*(styleConfig), x - 0.01, y - 0.01, x + 0.01, y + 0.01, osmscout::magVeryClose, areaSearchParameter,
                         nodes, ways, areas, relationWays, relationAreas);

    for (int i = 0; i < ways.size(); i++)
    {
        osmscout::WayRef way;

        searchWay(ways.at(i).Get()->GetId(), way);

        if (way.Valid()) {
            std::vector<Point> points = way.Get()->nodes;

            for (int j = 0; j < points.size(); j++) {
                NodeRef node;

                searchNode(points.at(j).GetId(), node);

                if (node.Valid()) {
                    for (int k = 0; k < node.Get()->GetTagCount(); k++) {
                    //    qDebug() << k << "/" << node.Get()->GetTagCount() << QString::fromStdString(node.Get()->GetTagValue(k));
                        if (QString(node.Get()->GetTagValue(k).c_str()).compare(type) == 0) {
                            poiRef.append(node);
                        //    qDebug() << type << " = " << QString::fromStdString(node.Get()->GetTagValue(k));
                        }
                    }
                }
            }
        }

    }
}

double Searching::CalculateDistance(double sx, double sy, double dx, double dy)
{
    double R = 6371;    // km
    double dLat = (dy - sy) * M_PI / 180;
    double dLon = (dx - sx) * M_PI / 180;
    double sx1 = sx * M_PI / 180;
    double sx2 = sx * M_PI / 180;

    double a = sin(dLat / 2) * sin(dLat / 2) + sin(dLon / 2) * sin(dLon / 2) * cos(sx1) * cos(sx2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double d = R * c;

    return d;
}

bool Searching::IsBetweenNodes(osmscout::Routing::RouteNode actualPoint,
                               osmscout::Routing::RouteNode firstPoint,
                               osmscout::Routing::RouteNode secondPoint,
                               int precision)
{
    double partDist1 = CalculateDistance(actualPoint.lat, actualPoint.lon, firstPoint.lat, firstPoint.lon);
    double partDist2 = CalculateDistance(actualPoint.lat, actualPoint.lon, secondPoint.lat, secondPoint.lon);
    double fullDist = floor(1000 * CalculateDistance(firstPoint.lat, firstPoint.lon, secondPoint.lat, secondPoint.lon));
    double partDistSum = floor(1000 * (partDist1 + partDist2));

    if ( (fullDist - precision) < partDistSum && (fullDist + precision) > partDistSum) {
        //qDebug() << "FULL: " << fullDist << " |partDistSum: " << partDistSum;
        return true;
    } else {
        return false;
    }


}

QPointF Searching::CorrectPosition(osmscout::Routing::RouteNode firstNode,
                                   osmscout::Routing::RouteNode secondNode,
                                   QPointF position,
                                   const osmscout::Projection &projection,
                                   double tolerance,
                                   double changeRouteTolerance)
{
    double fx, fy, sx, sy;
    projection.GeoToPixel(firstNode.lon, firstNode.lat, fx, fy);
    projection.GeoToPixel(secondNode.lon, secondNode.lat, sx, sy);

    double u = ((position.x() - fx) * (sx - fx) + (position.y() - fy) * (sy - fy)) / ((qPow(fx-sx, 2)) + (qPow(fy - sy, 2)));

    double ox = fx + (sx - fx) * u;
    double oy = fy + (sy - fy) * u;

    double oLon, oLat, pLon, pLat;
    projection.PixelToGeo(ox, projection.GetHeight() - oy, oLon, oLat);
    projection.PixelToGeo(position.x(), projection.GetHeight() - position.y(), pLon, pLat);

    //qDebug() << oLon << " " << oLat << " | " << pLon << " " << pLat;

    double distance = floor(1000 * CalculateDistance(pLon, pLat, oLon, oLat));

    //qDebug() << "\t" << distance;

    if (distance < tolerance) {
        return QPointF(pLon, pLat);
    }
    else {
        if (distance > changeRouteTolerance) {

            return QPointF(pLon, pLat);
        } else {
            return QPointF(oLon, oLat);
        }
    }
}

}
