#include "osmscout/Searching.h"

#include <QList>

#include <osmscout/Database.h>
#include <osmscout/StyleConfigLoader.h>
#include <osmscout/util/Projection.h>
#include <osmscout/MapPainter.h>
#include <osmscout/AdminRegion.h>

namespace osmscout {

Searching::Searching()
{
    database = new Database(databaseParameter);

    QString map = "/home/bartek/osmscout-map/3poland/";

    if (!database->Open((const char*)map.toAscii())) {
        std::cerr << "Cannot open database" << std::endl;
    } else {
        std::cerr << "Database is open!" << std::endl;
    }

//    searchAllRegions();
}

void Searching::searchAllRegions()
{
    const QString searchName = "";
    size_t size = 10000000;
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
    size_t size = 10000000;
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
    size_t max_size = 1000;
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
}

void Searching::searchWay(const int id, WayRef &wayRef)
{
    database->GetWay(id, wayRef);
}

}
