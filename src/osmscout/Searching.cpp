#include "osmscout/Searching.h"

#include <QList>
#include <QDebug>

#include "settings.h"

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
        std::cerr << "Database is open!" << std::endl;
    }

    styleConfig = new osmscout::StyleConfig(database->GetTypeConfig());

    if (!osmscout::LoadStyleConfig((const char*)style.toAscii(),*(styleConfig))) {
        std::cerr << "Cannot open style" << std::endl;
    }

//    searchAllRegions();
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

            std::cout << region.name.toStdString() << " " << region.reference.GetTypeName() << std::endl;

        }
}

QList<AdminRegion> Searching::searchRegion(QString name)
{
    size_t size = 100;
    std::list<osmscout::AdminRegion> regions;
    bool reached;

    database->GetMatchingAdminRegions(name, regions, size, reached, false);

    QList<AdminRegion> qRegions = QList<AdminRegion>::fromStdList(regions);

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

    QList<Location> qLocations = QList<Location>::fromStdList(locations);

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
   /* for (int i = 100000000; i < 1000000000; i++) {
        NodeRef noderef;
        //database->GetNode(id, node);
        database->GetNode(i, noderef);



        if (noderef.Valid())
            qDebug() << noderef.Get()->GetId();
    }*/
}

void Searching::searchWay(const int id, WayRef &wayRef)
{
    database->GetWay(id, wayRef);
}

void Searching::searchRelation(const int id, RelationRef &relationRef)
{
    database->GetRelation(id, relationRef);
}

void Searching::searchObjects(double lonMin, double latMin, double lonMax, double latMax, std::vector<osmscout::NodeRef> &nodes, std::vector<osmscout::WayRef> &ways, std::vector<osmscout::WayRef> &areas, std::vector<osmscout::RelationRef> &relationWays, std::vector<osmscout::RelationRef> &relationAreas)
{
    osmscout::AreaSearchParameter areaSearchParameter;

    database->GetObjects(*(styleConfig), lonMin, latMin, lonMax, latMax, osmscout::magVeryClose, areaSearchParameter,
                         nodes, ways, areas, relationWays, relationAreas);
}

}
