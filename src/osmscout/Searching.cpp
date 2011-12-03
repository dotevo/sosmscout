#include "osmscout/Searching.h"

#include <QList>
#include <QDebug>
#include <QMatrix>
#include <QPointF>

#include "settings.h"
#include "qmath.h"

#include <osmscout/Database.h>
#include <osmscout/StyleConfigLoader.h>
#include <osmscout/util/Projection.h>
#include <osmscout/MapPainter.h>
#include <osmscout/AdminRegion.h>
#include <osmscout/StyleConfig.h>

namespace osmscout {

Searching::Intersection::Intersection()
{
    myPos = QPointF(0, 0);
    cross = QPointF(0, 0);
    way = QPointF(0,0);
    //ways = QList<QPointF>();
    //ways.append(QPointF(0,0));
}

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
    // lon -> x
    // lat ->y
    double PI = 3.141592653589793;
    double R = 6371;    // km
    double dLat = (dy - sy) * PI / 180;
    double dLon = (dx - sx) * PI / 180;
    double lat1 = sy * PI / 180;
    double lat2 = dy * PI / 180;

    double a = sin(dLat / 2) * sin(dLat / 2) + sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double d = R * c;

// 1,528331795
    //return d;
    return d;

    //return sqrt(pow(sx - dx, 2) + pow(sy - dy, 2));
}

bool Searching::IsBetweenNodes(osmscout::Routing::Step actualPoint,
                               osmscout::Routing::Step firstPoint,
                               osmscout::Routing::Step secondPoint,
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

QPointF Searching::CorrectPosition(osmscout::Routing::Step firstNode,
                                   osmscout::Routing::Step secondNode,
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
            // TODO: destination coordinates
            //emit leaveRoute(pLon, pLat, 0, 0);
            return QPointF(pLon, pLat);
        } else {
            return QPointF(oLon, oLat);
        }
    }
}

osmscout::Searching::Intersection Searching::SimulateNextCrossing(osmscout::Routing::Step lastNode,
                                                                  osmscout::Routing::Step crossingNode,
                                                                  QList<osmscout::Routing::Step> waysNodes)
{
    int panelWidth = 109;
    int panelHeight = 122;

    int scale = 100000;
    long double minX, minY;

    minX = qMin(lastNode.lon, crossingNode.lon);
    minX = qMin(minX, waysNodes[0].lon);

    minY = qMin(lastNode.lat, crossingNode.lat);
    minY = qMin(minY, waysNodes[0].lat);

    QPointF last, cross;
    QList<QPointF> ways;
    last.setX(scale * (lastNode.lon - minX));
    last.setY(scale * (lastNode.lat - minY));
    cross.setX(scale * (crossingNode.lon - minX));
    cross.setY(scale * (crossingNode.lat - minY));
    for (int i = 0; i < waysNodes.length(); i++) {
        ways.append(QPointF(scale * (waysNodes.at(i).lon - minX), scale * (waysNodes.at(i).lat - minY)));
    }

    //qDebug() << "MIN: " << minX << " " << minY;
    //qDebug() << "LAST: " << QString::number(lastNode.lon, 'f', 10) << " " << QString::number(lastNode.lat, 'f', 10) << " | " << last.x() << " " << last.y();
    //qDebug() << "CROSS: " << QString::number(crossingNode.lon, 'f', 10) << " " << QString::number(crossingNode.lat, 'f', 10) << " | " << cross.x() << " " << cross.y();
    //qDebug() << "TARGET: " << QString::number(targetNode.lon, 'f', 10) << " " << QString::number(targetNode.lat, 'f', 10) << " | " << target.x() << " " << target.y();

    QMatrix moveToStart;
    moveToStart.translate(-last.x(), -last.y());

    last = moveToStart.map(last);
    cross = moveToStart.map(cross);
    for (int i = 0; i < ways.length(); i++) {
        ways[i] = moveToStart.map(ways[i]);
    }

    double yDist = qAbs(cross.y() - last.y());
    double toCrossDist = qSqrt(qPow(cross.x() - last.x(), 2) + qPow(cross.y() - last.y(), 2));
    double cosVal = yDist / toCrossDist;
    double angle = - acos(cosVal) * 180 / M_PI;   // angle in deegres

    QMatrix rotateToVert;
    rotateToVert.rotate(angle);

    last = rotateToVert.map(last);
    cross = rotateToVert.map(cross);
    for (int i = 0; i < ways.length(); i++) {
        ways[i] = rotateToVert.map(ways[i]);
    }

    cross.setX(0);

    QMatrix moveToCenter;
    moveToCenter.translate(panelWidth / 2, panelHeight / 2 - cross.y());

    last = moveToCenter.map(last);
    cross = moveToCenter.map(cross);
    for (int i = 0; i < ways.length(); i++) {
        ways[i] = moveToCenter.map(ways[i]);
    }

    double a = (ways[0].y() - cross.y()) / (ways[0].x() - cross.x());
    double b = ways[0].y() - a * ways[0].x();

    last.setY(panelHeight - last.y());
    cross.setY(panelHeight - cross.y());
    for (int i = 0; i < ways.size(); i++) {
        ways[i].setY(panelHeight - ways[i].y());
    }

    Intersection intersection;
    intersection.myPos = QPointF(last);
    intersection.cross = QPointF(cross);
    intersection.ways = ways;
    intersection.way = ways.at(0);

    //qDebug() << "LAST: " << QString::number(lastNode.lon, 'f', 10) << " " << QString::number(lastNode.lat, 'f', 10) << " | " << last.x() << " " << last.y();
    //qDebug() << "CROSS: " << QString::number(crossingNode.lon, 'f', 10) << " " << QString::number(crossingNode.lat, 'f', 10) << " | " << cross.x() << " " << cross.y();
    //qDebug() << "TARGET: " << QString::number(waysNodes[0].lon, 'f', 10) << " " << QString::number(waysNodes[0].lat, 'f', 10) << " | " << ways[0].x() << " " << ways[0].y();

    qDebug() << intersection.ways.at(0).isNull();
    return intersection;

}

}
