#include <osmscout/Partitioning.h>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

#include <QFileInfo>
#include <QDir>

#include <QMouseEvent>
#include <QLineF>

#include <QDebug>

#include <QDomDocument>

#include <osmscout/Util.h>
#include <osmscout/Partitionmodel.h>

#include <../../PiLibocik/include/pilibocik/geohash.h>
#include <../../PiLibocik/include/pilibocik/partition/boundaryedge.h>
#include <../../PiLibocik/include/pilibocik/partition/partitionfile.h>
#include <../../PiLibocik/include/pilibocik/partition/node.h>
#include <../../PiLibocik/include/pilibocik/partition/way.h>

namespace osmscout {
Partitioning::Partitioning()
{
    // setting alpha and beta factors for quality function
    alpha = 0.99;
    beta = 0.005;
    //alpha = 0.95;
    //beta = 1;

    databasePath = QString("");
    simpleDataPath = QString("");
    prioritiesDataPath = QString("");
    finalDataPath = QString("");
}

void Partitioning::run()
{
    switch(stage) {
    case DATA_INITIALIZATION:
        InitData();
        SaveData();
        break;
    case PRIORITIES_CALCULATION:
        LoadData();
        CalculatePriorities();
        SavePriorities();
        break;
    case PARTITIONS_CALCULATION:
#ifdef PiLibocik_WRITE_MODE
        LoadData();
        LoadPriorities();
        osmscout::Partitioning::DatabasePartition dbPart = FindPartition();
        saveToDatabase(dbPart);
#endif
        break;
    }
    //Delete();
}

void Partitioning::InitData()
{
    emit initDataStatusChanged("Opening database...");
    qDebug() << "Opening database...";
    emit initDataPartProgress(0);
    emit initDataOverallProgress(0);

    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databasePath);
    if(!db.open()){
        qDebug() << "[SQLiteDatabase::open]" << db.lastError();
        return;
    }

    double lonMin, latMin, lonMax, latMax;
    //, magnification;
    latMin = 51.1;
    lonMin = 17.0;
    latMax = 51.15;
    lonMax = 17.3;
    //Wroc³aw 51.118552&lon=17.057824

    latMin = 51.08;
    lonMin = 16.98;
    latMax = 51.14;
    lonMax = 17.09;
    emit initDataPartProgress(100);

    //
    // getting nodes
    //
    emit initDataStatusChanged("Getting nodes...");
    qDebug() << "Getting nodes...";
    emit initDataPartProgress(0);
    emit initDataOverallProgress(5);
    QSqlQuery qQuery(db);
    QString query = "SELECT count(*) FROM nodes WHERE lon>" + QString::number(lonMin)
            + " AND lat>" + QString::number(latMin)
            + " AND lon<" + QString::number(lonMax)
            + " AND lat<" + QString::number(latMax);
    qQuery.prepare(query);
    qQuery.exec();

    unsigned int querySize;
    if(qQuery.next()) {
        querySize = qQuery.value(0).toInt();
    }

    query = "SELECT id,lon,lat FROM nodes WHERE lon>" + QString::number(lonMin)
            + " AND lat>" + QString::number(latMin)
            + " AND lon<" + QString::number(lonMax)
            + " AND lat<" + QString::number(latMax);
    qQuery.clear();
    qQuery.prepare(query);
    qQuery.exec();

    std::vector< PartNode > tmpNodes;
    tmpNodes.clear();
    unsigned int cell = 0;
    while(qQuery.next()) {
        if(cell%100 == 0) {
            emit initDataPartProgress((int)((double)cell/(double)querySize * 100));
        }

        PartNode newNode;
        newNode.id = qQuery.value(0).toInt();
        newNode.lon = qQuery.value(1).toDouble();
        newNode.lat = qQuery.value(2).toDouble();
        newNode.cell = cell;
        cell++;
        newNode.type = INTERNAL;
        tmpNodes.push_back(newNode);
    }
    emit initDataPartProgress(100);

    //
    // getting ways
    //
    emit initDataStatusChanged("Getting ways...");
    qDebug() << "Getting ways...";
    emit initDataPartProgress(0);
    emit initDataOverallProgress(15);
    query = "SELECT count(*) FROM ways,way_nodes WHERE way_nodes.way=ways.id AND ways.lon1>" + QString::number(lonMin)
            + " AND ways.lat1>" + QString::number(latMin)
            + " AND ways.lon2<" + QString::number(lonMax)
            + " AND ways.lat2<" + QString::number(latMax)
            + " AND way IN(SELECT ref FROM way_tags"
            + " WHERE tag IN("
            + " SELECT id FROM tags"
            + " WHERE key == 'highway' "
            + " AND(value == 'primary' "
            + " OR value == 'primary_link' "
            + " OR value == 'secondary' "
            + " OR value == 'secondary_link' "
            + " OR value == 'residential' "
            + " OR value == 'residential_link' "
            + " OR value == 'minor' "
            + " OR value == 'track' "
            + " OR value == 'tertiary' "
            + " OR value == 'tertiary_link' "
            + " OR value == 'trunk' "
            + " OR value == 'trunk_link' "
            + " OR value == 'road' "
            + " OR value == 'motorway' "
            + " OR value == 'living_street')))"
            + " ORDER BY way,num";
    qQuery.clear();
    qQuery.prepare(query);
    qQuery.exec();

    if(qQuery.next()) {
        querySize = qQuery.value(0).toInt();
    }

    query = "SELECT way,num,node FROM ways,way_nodes WHERE way_nodes.way=ways.id AND ways.lon1>" + QString::number(lonMin)
            + " AND ways.lat1>" + QString::number(latMin)
            + " AND ways.lon2<" + QString::number(lonMax)
            + " AND ways.lat2<" + QString::number(latMax)
            + " AND way IN(SELECT ref FROM way_tags"
            + " WHERE tag IN("
            + " SELECT id FROM tags"
            + " WHERE key == 'highway' "
            + " AND(value == 'primary' "
            + " OR value == 'primary_link' "
            + " OR value == 'secondary' "
            + " OR value == 'secondary_link' "
            + " OR value == 'residential' "
            + " OR value == 'residential_link' "
            + " OR value == 'minor' "
            + " OR value == 'track' "
            + " OR value == 'tertiary' "
            + " OR value == 'tertiary_link' "
            + " OR value == 'trunk' "
            + " OR value == 'trunk_link' "
            + " OR value == 'road' "
            + " OR value == 'motorway' "
            + " OR value == 'living_street')))"
            + " ORDER BY way,num";
    qQuery.clear();
    qQuery.prepare(query);
    qQuery.exec();

    std::vector< PartWay > tmpWays;
    PartWay newWay;
    newWay.id = 0;
    cell = 0; // just for measuring progress
    while (qQuery.next()) {
        if(cell%100 == 0) {
            emit initDataPartProgress((int)((double)cell/(double)querySize * 100));
        }
        cell++;

        unsigned int id = qQuery.value(0).toLongLong();
        if(newWay.id == 0) {
            newWay.id = id; // first way
        }

        if(newWay.id != id) {
            tmpWays.push_back(newWay);
            newWay.id = id;
            newWay.nodes.clear();
        }

        unsigned int nodeId = qQuery.value(2).toInt();
        int idx = -1;
        for(unsigned int i=0; i<tmpNodes.size(); ++i) {
            if(tmpNodes[i].id == nodeId) {
                idx = i;
                break;
            }
        }
        if(idx != -1) {
            newWay.nodes.push_back(idx);
        }
    }
    tmpWays.push_back(newWay);
    emit initDataPartProgress(100);

    //
    // deleting all unnecessary nodes
    //
    emit initDataStatusChanged("Deleting all unnecessary nodes...");
    qDebug() << "Deleting all unnecessary nodes...";
    emit initDataPartProgress(0);
    emit initDataOverallProgress(25);
    for(unsigned int i=0; i<tmpNodes.size(); ++i) {
        if(i % 100 == 0) {
            emit initDataPartProgress((int)((double)i/(double)tmpNodes.size() * 100));
        }

        //check if exists in more than one way
        int waysContainingNodeCount = 0;
        for(unsigned int j=0; j<tmpWays.size(); ++j) {
            for(unsigned int k=0; k<tmpWays[j].nodes.size(); ++ k) {
                if(i == tmpWays[j].nodes[k]) {
                    waysContainingNodeCount++;
                    if(k == 0 || k == tmpWays[j].nodes.size()-1) {
                        waysContainingNodeCount++;
                    }
                    break;
                }
            }
            if(waysContainingNodeCount > 1) {
                break;
            }
        }
        if(waysContainingNodeCount > 1) {
            tmpNodes[i].cell = partition.nodes.size();
            partition.nodes.push_back(tmpNodes[i]);
        } else {
            tmpNodes[i].id = 0;
            tmpNodes[i].lon = -1;
            tmpNodes[i].lat = -1;
        }
    }
    emit initDataPartProgress(100);

    //
    // deleting all unnecessary nodes from ways
    //
    emit initDataStatusChanged("Deleting all unnecessary nodes from ways...");
    qDebug() << "Deleting all unnecessary nodes from ways...";
    emit initDataPartProgress(0);
    emit initDataOverallProgress(65);
    for(unsigned int i=0; i<tmpWays.size(); ++i) {
        if(i % 100 == 0) {
            emit initDataPartProgress((int)((double)i/(double)tmpWays.size() * 100));
        }

        PartWay newWay;
        newWay.id = tmpWays[i].id;
        newWay.priority = 1; // TODO: calculating priority due to road type
        newWay.oneway = 0;\
        QString tmpString = "primary";
        newWay.type = tmpString;
        newWay.nodes.clear();

        //check if each node exists in nodes vector
        for(unsigned int j=0; j<tmpWays[i].nodes.size(); ++j) {
            // if this node is not deleted
            if(!(tmpNodes[tmpWays[i].nodes[j]].id == 0
                 && tmpNodes[tmpWays[i].nodes[j]].lon == -1
                 && tmpNodes[tmpWays[i].nodes[j]].lat == -1)) {
                unsigned int id = tmpNodes[tmpWays[i].nodes[j]].id;
                for(unsigned int k=0; k<partition.nodes.size(); ++k) {
                    if(partition.nodes[k].id == id) {
                        newWay.nodes.push_back(k);
                        break;
                    }
                }
            }
        }

        // finally add way to graph
        partition.ways.push_back(newWay);
    }
    emit initDataPartProgress(100);

    //
    // checking for oneways
    //
    emit initDataStatusChanged("Checking for oneways...");
    qDebug() << "Checking for oneways...";
    emit initDataPartProgress(0);
    emit initDataOverallProgress(85);
    query = "SELECT ref, value FROM way_tags LEFT JOIN tags ON way_tags.tag = tags.id"
            " WHERE tag IN("
            " SELECT id FROM tags"
            " WHERE key='oneway')"
            " ORDER BY ref";
    qQuery.clear();
    qQuery.prepare(query);
    qQuery.exec();

    QMap< unsigned int, QString > oneways;
    while(qQuery.next()) {
        oneways.insert(qQuery.value(0).toInt(), qQuery.value(1).toString());
    }

    for(unsigned int i=0; i<partition.ways.size(); ++i) {
        if(i % 100 == 0) {
            emit initDataPartProgress((int)((double)i/(double)partition.ways.size() * 100));
        }

        PartWay *way = &partition.ways[i];
        QMap< unsigned int, QString >::iterator it = oneways.find(way->id);

        if(it != oneways.end()) {
            QString onewayString = it.value();

            if(onewayString.compare("1") == 0
                    || onewayString.compare("yes") == 0
                    || onewayString.compare("true") == 0
                    || onewayString.compare("1;yes") == 0) {
                way->oneway = 1;
            } else if(onewayString.compare("-1") == 0
                      //|| onewayString.compare("no") == 0
                      //|| onewayString.compare("false") == 0
                      || onewayString.compare("-1;yes") == 0) {
                way->oneway = -1;
            }
        }
    }
    emit initDataPartProgress(100);

    //
    // checking for priorities
    //
    emit initDataStatusChanged("Checking for types...");
    qDebug() << "Checking for types...";
    emit initDataPartProgress(0);
    emit initDataOverallProgress(90);
    query = "SELECT ref, value FROM way_tags LEFT JOIN tags ON way_tags.tag = tags.id"
            " WHERE tag IN("
            " SELECT id FROM tags"
            " WHERE key='highway')"
            " ORDER BY ref";
    qQuery.clear();
    qQuery.prepare(query);
    qQuery.exec();

    QMap< unsigned int, QString > types;
    while(qQuery.next()) {
        types.insert(qQuery.value(0).toInt(), qQuery.value(1).toString());
    }

    for(unsigned int i=0; i<partition.ways.size(); ++i) {
        if(i % 100 == 0) {
            emit initDataPartProgress((int)((double)i/(double)partition.ways.size() * 100));
        }

        PartWay *way = &partition.ways[i];
        QMap< unsigned int, QString >::iterator it = types.find(way->id);

        if(it != types.end()) {
            QString highwayType = it.value();
            partition.ways[i].type = highwayType;
        }
    }
    emit initDataPartProgress(100);

    emit initDataStatusChanged("Updating partition data...");
    emit initDataPartProgress(0);
    UpdatePartitionData();
    emit initDataPartProgress(100);

    db.close();
    emit initDataOverallProgress(95);
}

void Partitioning::SaveData()
{
    emit initDataStatusChanged("Writing partition to the file...");
    qDebug() << "Writing partition to the file...";
    emit initDataPartProgress(0);
    QFile file(simpleDataPath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << partition.nodes.size() << "\n";
    out << partition.ways.size() << "\n";
    out << partition.cellsCount << "\n";
    out << "\n";
    for(unsigned int i=0; i<partition.nodes.size(); ++i) {
        out << partition.nodes[i].id << " " << partition.nodes[i].lon << " " << partition.nodes[i].lat << " " << partition.nodes[i].cell << "\n";
    }
    out << "\n";

    emit initDataPartProgress(50);

    for(unsigned int i=0; i<partition.ways.size(); ++i) {
        PartWay way = partition.ways[i];
        out << way.id << " " << way.type << " " << way.oneway << " " << way.nodes.size() << " ";
        for(unsigned int j=0; j<way.nodes.size(); ++j) {
            out << way.nodes[j] << " ";
            //out << partition.nodes[way.nodes[j]].id << " ";
        }
        out << "\n";
    }
    out << "\n";

    file.close();

    emit initDataPartProgress(100);
    emit initDataOverallProgress(100);
    emit initDataStatusChanged("Finished.");
    emit initDataFinished();
}

void Partitioning::LoadData()
{
    switch(stage) {
    case PRIORITIES_CALCULATION:
        emit prioCalcStatusChanged("Reading partition from file...");
        break;
    case PARTITIONS_CALCULATION:
        emit partCalcStatusChanged("Reading partition from file...");
        break;
    }
    qDebug() << "Reading partition from file...";
    switch(stage) {
    case PRIORITIES_CALCULATION:
        emit prioCalcProgress(0);
        break;
    case PARTITIONS_CALCULATION:
        emit partCalcProgress(0);
        break;
    }

    QFile file(simpleDataPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QTextStream in(&file);
    QString line = in.readLine();
    unsigned int nodesCount = line.toInt();
    line = in.readLine();
    unsigned int waysCount = line.toInt();
    line = in.readLine();
    //unsigned int cellsCount = line.toInt();
    line = in.readLine();

    partition.nodes.clear();
    for(unsigned int i=0; i<nodesCount; ++i) {
        line = in.readLine();
        QStringList stringList = line.split(" ");

        PartNode newNode;
        newNode.id = stringList.at(0).toDouble();
        newNode.lon = stringList.at(1).toDouble();
        newNode.lat = stringList.at(2).toDouble();
        newNode.cell = stringList.at(3).toDouble();
        newNode.type = INTERNAL;

        partition.nodes.push_back(newNode);
    }
    line = in.readLine();

    for(unsigned int i=0; i<waysCount; ++i) {
        PartWay newWay;
        line = in.readLine();
        QStringList stringList = line.split(" ");
        newWay.id = stringList.at(0).toInt();
        newWay.type = stringList.at(1);
        newWay.oneway = stringList.at(2).toInt();
        unsigned int wayNodesCount = stringList.at(3).toInt();
        for(unsigned int j=0; j<wayNodesCount; ++j) {
            newWay.nodes.push_back(stringList.at(4+j).toInt());
        }

        partition.ways.push_back(newWay);
    }

    file.close();

    UpdatePartitionData();

    switch(stage) {
    case PRIORITIES_CALCULATION:
        emit prioCalcProgress(1);
        break;
    case PARTITIONS_CALCULATION:
        emit partCalcProgress(1);
        break;
    }
}

void Partitioning::UpdatePartitionData()
{
    //qDebug() << "Updating partition data...";

    // counts cells
    //qDebug() << "Counting cells...";
    partition.cellsCount = 0;
    partition.boundaryEdgesCount = 0;
    partition.boundaryNodesCount = 0;
    partition.nodesCount = partition.nodes.size();
    partition.waysCount = partition.ways.size();
    for (unsigned int i=0; i < partition.nodesCount; ++i) {
        PartNode node = partition.nodes[i];
        if(node.cell >= partition.cellsCount) { // ATTENTION!! It's important to take care that there's no missing cells number, ie. 1, 2, 4, 5, 8 <- WRONG!
            partition.cellsCount = node.cell + 1; // cells are numbered starting from 0
        }
        node.type = INTERNAL;
    }

    partition.cellsBoundaryNodesCount.clear();
    partition.cellsNodesCount.clear();
    partition.cellsEdgesCount.clear();
    partition.cellsRouteEdgesCount.clear();
    for (unsigned int i=0; i < partition.cellsBoundaryNodes.size(); ++i) {
        delete partition.cellsBoundaryNodes[i];
    }
    partition.cellsBoundaryNodes.clear();
    for (unsigned int i=0; i < partition.cellsConnections.size(); ++i) {
        delete partition.cellsConnections[i];
    }
    partition.cellsConnections.clear();
    partition.boundaryEdges.clear();

    partition.cellsBoundaryNodesCount.reserve(partition.cellsCount);
    partition.cellsNodesCount.reserve(partition.cellsCount);
    partition.cellsEdgesCount.reserve(partition.cellsCount);
    partition.cellsRouteEdgesCount.reserve(partition.cellsCount);
    partition.cellsBoundaryNodes.reserve(partition.cellsCount);
    partition.cellsConnections.reserve(partition.cellsCount);
    partition.boundaryEdges.reserve(partition.cellsCount);

    for(unsigned int i=0; i<partition.cellsCount; ++i) {
        partition.cellsBoundaryNodesCount.push_back(0);
        partition.cellsNodesCount.push_back(0);
        partition.cellsEdgesCount.push_back(0);
        partition.cellsRouteEdgesCount.push_back(0);
        std::vector< unsigned int > * tmpVector = new std::vector< unsigned int >;
        partition.cellsBoundaryNodes.push_back(tmpVector);
        tmpVector = new std::vector< unsigned int >;
        partition.cellsConnections.push_back(tmpVector);
    }

    // counts edges and adds boundary edges to list
    //qDebug() << "Counting edges...";
    for (unsigned int i=0; i < partition.ways.size(); ++i) {
        const PartWay way = partition.ways[i];
        //qDebug() << "w " << way->GetId() << "\n";
        PartNode prevNode = partition.nodes[way.nodes[0]];
        PartNode node;
        unsigned int prevCell = prevNode.cell;
        unsigned int cell;
        for(unsigned int j=1; j < way.nodes.size(); ++j) {
            node = partition.nodes[way.nodes[j]];
            cell = node.cell;
            if(cell == prevCell) {
                partition.cellsEdgesCount[cell] += 1;
            } else {
                // adding connection between two cells
                unsigned int cellA, cellB;
                if(cell < prevCell) {
                    cellA = cell;
                    cellB = prevCell;
                } else {
                    cellB = cell;
                    cellA = prevCell;
                }
                bool connectionExists = false;
                for(unsigned int k=0; k<partition.cellsConnections[cellA]->size(); ++k) {
                    if(partition.cellsConnections[cellA]->at(k) == cellB) {
                        connectionExists = true;
                        break;
                    }
                }
                if(!connectionExists) {
                    partition.cellsConnections[cellA]->push_back(cellB);
                }

                // setting nodes connecting two cells to boundary
                partition.nodes[way.nodes[j-1]].type = BOUNDARY;
                partition.nodes[way.nodes[j]].type = BOUNDARY;

                // adding boundary edge
                BoundaryEdge edge;
                edge.nodeA = way.nodes[j-1];
                edge.nodeB = way.nodes[j];
                partition.boundaryEdges.push_back(edge);
            }
            prevNode = node;
            prevCell = cell;
        }
    }

    partition.boundaryEdgesCount = partition.boundaryEdges.size();

    // counts nodes in cells and boundary nodes
    //qDebug() << "Counting nodes...";
    for (unsigned int i=0; i<partition.nodesCount; ++i) {
        PartNode node = partition.nodes[i];
        partition.cellsNodesCount[node.cell] += 1;
        if(node.type == BOUNDARY) {
            partition.cellsBoundaryNodesCount[node.cell] += 1;
            partition.cellsBoundaryNodes[node.cell]->push_back(i);
        }
    }
    for(unsigned int i=0; i<partition.cellsCount; ++i) {
        partition.boundaryNodesCount += partition.cellsBoundaryNodesCount[i];
    }

    // calculates route edges in cells
    for(unsigned int i=0; i<partition.cellsBoundaryNodesCount.size(); i++) {
        partition.cellsRouteEdgesCount[i] = partition.cellsBoundaryNodesCount[i] * (partition.cellsBoundaryNodesCount[i] - 1);
    }
}

void Partitioning::UpdatePriorities(unsigned int i, unsigned int j)
{
    //qDebug() << "Updating priorities...";
    // updates lines for merged cell
    // for cells before i
    for(unsigned int k=0; k<i; ++k) {
        std::map< unsigned int, double > * oldMap = partition.priorities[k];
        std::map< unsigned int, double > * newMap = new std::map< unsigned int, double >;
        for(std::map< unsigned int, double >::iterator it = oldMap->begin(); it!=oldMap->end(); ++it) {
            unsigned int key = it->first;
            double value = it->second;
            if(key == j) {
                bool added = false;
                for(std::map< unsigned int, double >::iterator newIt = newMap->begin(); newIt!=newMap->end(); ++newIt) {
                    if(newIt->first == i) {
                        added = true;
                    }
                }
                if(!added) {
                    key = i;
                    value = CalculatePriority(k, key);
                } else {
                    continue;
                }
            } else if(key > j) {
                key--;
            } else if(key == i) {
                value = CalculatePriority(k, key);
            }
            newMap->insert(std::pair< unsigned int, double > (key, value));
        }
        partition.priorities[k] = newMap;
        delete oldMap;
    }
    // for new cell i
    {
        std::map< unsigned int, double > * newMap = new std::map< unsigned int, double >;
        for(unsigned int k = 0; k<partition.cellsConnections[i]->size(); ++k) {
            unsigned int tmp = partition.cellsConnections[i]->at(k);
            newMap->insert(std::pair< unsigned int, double > (tmp, CalculatePriority(i, tmp)));
        }
        partition.priorities[i] = newMap;
    }
    // for cells between i and j
    for(unsigned int k=i+1; k<j && k<partition.cellsCount; ++k) {
        std::map< unsigned int, double > * oldMap = partition.priorities[k];
        std::map< unsigned int, double > * newMap = new std::map< unsigned int, double >;
        for(std::map< unsigned int, double >::iterator it = oldMap->begin(); it!=oldMap->end(); ++it) {
            unsigned int key = it->first;
            double value = it->second;
            if(key == j) {
                continue;
            } else if(key > j) {
                key--;
            }
            newMap->insert(std::pair< unsigned int, double > (key, value));
        }
        partition.priorities[k] = newMap;
        delete oldMap;
    }
    // delete maps for cell j
    std::map< unsigned int, double > * mapJ = partition.priorities[j];
    delete mapJ;
    // for cells after j
    for(unsigned int k=j; k<partition.cellsCount; ++k) {
        std::map< unsigned int, double > * oldMap = partition.priorities[k+1];
        std::map< unsigned int, double > * newMap = new std::map< unsigned int, double >;
        for(std::map< unsigned int, double >::iterator it = oldMap->begin(); it!=oldMap->end(); ++it) {
            unsigned int key = it->first - 1;
            double value = it->second;
            newMap->insert(std::pair< unsigned int, double > (key, value));
        }
        partition.priorities[k] = newMap;
        delete oldMap;
    }
    // delete last row
    partition.priorities.pop_back();
}

double Partitioning::CalculateQuality()
{
    //qDebug() << "Calculating quality of partition...";
    double sumA = 0;
    double sumB = 0;

    for(unsigned int i=0; i<partition.cellsCount; i++) {
        double fraction = (double)partition.cellsNodesCount[i]/(double)partition.nodesCount;
        sumA += (fraction * (2 - fraction) * (double)partition.cellsEdgesCount[i]) + ((1.5 - fraction) * (double)partition.cellsRouteEdgesCount[i]);
        sumB += pow(1 - fraction, 2);
        //sumA += (fraction) * (2 - fraction) * (double)partition.cellsEdgesCount[i];
        //sumB += pow(1 - fraction, 2) * (double)partition.cellsRouteEdgesCount[i];
    }

    double result = beta * sumA + alpha * (sumB + (double)partition.boundaryEdgesCount);

    //qDebug() << "Q: " << result;
    qDebug() << result;
    return result;
}

void Partitioning::TestAlgorithm()
{
    qDebug() << "Testing algorithm...";
    double priority;
    double quality;
    quality = CalculateQuality();
    unsigned int cellI;
    unsigned int cellJ;
    for(int x=0; x<3; ++x) {
        bool flag = false;
        for(unsigned int i=0; i<partition.cellsCount-1; ++i) {
            for(unsigned int j=i+1; j<partition.cellsCount; ++j) {
                priority = CalculatePriority(i, j);
                if(priority > 0) {
                    cellI = i;
                    cellJ = j;
                    flag = true;
                    break;
                }
            }
            if(flag)
                break;
        }
        MergeCells(cellI, cellJ);

        quality = CalculateQuality();
    }
}

#ifdef PiLibocik_WRITE_MODE
void Partitioning::saveToDatabase(DatabasePartition& databasePartition)
{
    //
    // saving to sql database
    //
    /*osmscout::PartitionModel pm;
    pm.open(finalDataPath + "\part.db");
    pm.createTables();
    pm.exportToDatabase(databasePartition);*/

    //
    // saving to binary file
    //
    emit partCalcStatusChanged("Saving files...");
    emit partCalcProgress(100);
    PiLibocik::Partition::PartitionFile partitionFile(finalDataPath, "car", QIODevice::WriteOnly, 1);
    QList< PiLibocik::Partition::Way > fileWays;
    QList< PiLibocik::Partition::Node > fileNodes;
    std::vector< PiLibocik::Partition::Way > ways;
    std::vector< PiLibocik::Partition::Node > nodes;

    // adding nodes
    for(unsigned int i=0; i<databasePartition.nodes.size(); ++i) {
        Partitioning::PartNode node = databasePartition.nodes[i];
        PiLibocik::Partition::Node fileNode(node.id, node.cell, node.lon, node.lat);

        nodes.push_back(fileNode);
    }

    // adding ways
    for(unsigned int i=0; i<databasePartition.innerWays.size(); ++i) {
        Partitioning::PartWay way = databasePartition.innerWays[i];
        PiLibocik::Partition::Way fileWay(way.id, way.priority, way.oneway);

        // adding nodes to way and way to nodes
        for(unsigned int j=0; j<way.nodes.size(); ++j) {
            //PiLibocik::Partition::Node *fileNode = &nodes[way.nodes[j]];

            fileWay.addNode(way.nodes[j]); // ATTENTION! zmienic na indeksy nodów w liœcie, a nie ich id (w FindPartition na koñcu przy tworzeniu DatabasePartition)
            nodes[way.nodes[j]].addWay(i);
        }

        ways.push_back(fileWay);
    }
/*
    // adding boundary edges
    for(unsigned int i=0; i<databasePartition.boundaryEdges.size(); ++i) {
        Partitioning::BoundaryEdge edge = databasePartition.boundaryEdges[i];
        PiLibocik::Partition::BoundaryEdge fileEdgeAB(edge.nodeB, edge.wayId, edge.priority); // edge from A to B
        PiLibocik::Partition::BoundaryEdge fileEdgeBA(edge.nodeA, edge.wayId, edge.priority); // edge from B to A
        //PiLibocik::Partition::Node *fileNodeA = &nodes[edge.nodeA];
        //PiLibocik::Partition::Node *fileNodeB = &nodes[edge.nodeB];

        nodes[edge.nodeA].addBoundaryEdge(fileEdgeAB);
        nodes[edge.nodeB].addBoundaryEdge(fileEdgeBA);
    }*/

    // adding routing edges
    for(unsigned int i=0; i<databasePartition.routingEdges.size(); ++i) {
        Partitioning::RouteEdge edge = databasePartition.routingEdges[i];
        PiLibocik::Partition::Edge fileEdgeAB(edge.nodeB, 1); // edge from A to B
        PiLibocik::Partition::Edge fileEdgeBA(edge.nodeA, 1); // edge from B to A
        //PiLibocik::Partition::Node *fileNodeA = &nodes[edge.nodeA];
        //PiLibocik::Partition::Node *fileNodeB = &nodes[edge.nodeB];

        nodes[edge.nodeA].addRoutingEdge(fileEdgeAB);
        nodes[edge.nodeB].addRoutingEdge(fileEdgeBA);
    }

    // creating list of nodes
    for(unsigned int i=0; i<nodes.size(); ++i) {
        fileNodes.append(nodes[i]);
    }

    // creating list of ways
    for(unsigned int i=0; i<ways.size(); ++i) {
        fileWays.append(ways[i]);
    }

    partitionFile.savePartition(fileWays, fileNodes, 4);

    emit partCalcStatusChanged("Finished.");
    emit partCalcProgress(100);
    emit partCalcFinished();
}
#endif

Partitioning::DatabasePartition  Partitioning::FindPartition()
{
    emit partCalcStatusChanged("Finding partition...");
    qDebug() << "Finding partition...";
    emit partCalcProgress(2);

    double priority;
    double maxPriority;
    double quality;
    double bestQuality;
    unsigned int bestCellsCount;
    unsigned int cellI;
    unsigned int cellJ;

    quality = 99999;
    bestQuality = quality;
    bestCellsCount = partition.cellsCount;

    while(partition.cellsCount > 1) {
        if(partition.cellsCount % 100 == 0) {
            emit partCalcProgress((int)((1.0-((double)partition.cellsCount/(double)partition.nodesCount)) * 97)+2);
        }
        cellI = 0;
        cellJ = partition.cellsCount-1;
        maxPriority = 0;
        for(unsigned int i=0; i<partition.cellsCount-1; ++i) {
            for(std::map< unsigned int, double >::iterator it = partition.priorities[i]->begin(); it!=partition.priorities[i]->end(); ++it) {
                priority = it->second * (1+(qrand()/32767./100));
                if(priority > maxPriority) {
                    cellI = i;
                    cellJ = it->first;
                    maxPriority = priority;
                }
            }
        }

        if(maxPriority > 0) {
            //if(partition.cellsCount != 10) {
            MergeCells(cellI, cellJ);
            UpdatePriorities(cellI, cellJ);

            quality = CalculateQuality();
            if(quality < bestQuality) {
                //if(partition.cellsCount == 6) {
                bestQuality = quality;
                bestCellsCount = partition.cellsCount;

                bestPartition.nodes.clear();
                for(unsigned int i=0; i<partition.nodes.size(); ++i) {
                    bestPartition.nodes.push_back(partition.nodes[i]);
                }

                bestPartition.ways.clear();
                for(unsigned int i=0; i<partition.ways.size(); ++i) {
                    bestPartition.ways.push_back(partition.ways[i]);
                }

                for(unsigned int i=0; i<bestPartition.cellsBoundaryNodes.size(); ++i) {
                    delete bestPartition.cellsBoundaryNodes[i];
                }
                bestPartition.cellsBoundaryNodes.clear();
                for(unsigned int i=0; i<partition.cellsBoundaryNodes.size(); ++i) {
                    std::vector< unsigned int > * tmpVector = new std::vector< unsigned int >;
                    for(unsigned int j=0; j<partition.cellsBoundaryNodes[i]->size(); ++j) {
                        tmpVector->push_back(partition.cellsBoundaryNodes[i]->at(j));
                    }
                    bestPartition.cellsBoundaryNodes.push_back(tmpVector);
                }
            }
        } else {
            break;
        }
    }

    qDebug() << "best quality: " << bestQuality;
    qDebug() << "best cells count: " << bestCellsCount;

    return getDatabasePartition();
}

Partitioning::DatabasePartition Partitioning::getDatabasePartition()
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Next +/-50 lines needs to be put into another method (maybe even savaToDatabase).
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // preparing format for writing to database
    DatabasePartition databasePartition;
    qDebug()<<"FOUND PARTITION N:"<<bestPartition.nodes.size()<<" W:"<<bestPartition.ways.size();
    databasePartition.nodes = bestPartition.nodes;

    // reading priorities according to types from xml file
    QMap< QString, double > prioritiesMap;

    QDomDocument *prioXML = new QDomDocument("PrioritiesXML");
    QFile file("priorities.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        DatabasePartition DBPart;
        //return DBPart;
    }
    prioXML->setContent(&file);
    file.close();

    QDomElement typeNode = prioXML->firstChildElement("priorities").firstChildElement("category").firstChildElement("type");
    while(!typeNode.isNull())
    {
        QString typeName = typeNode.attributeNode("name").value();
        double typePriority = typeNode.attributeNode("priority").value().toDouble();

        prioritiesMap.insert(typeName, typePriority);

        typeNode = typeNode.nextSiblingElement("type");
    }
    delete prioXML;

    // adding inner ways and boundary edges
    QMap< QString, double >::iterator it;
    for (unsigned int i=0; i < bestPartition.ways.size(); ++i) {
        const PartWay way = bestPartition.ways[i];

        PartWay databaseWay;
        databaseWay.id = way.id;
        it = prioritiesMap.find(way.type);
        if(it != prioritiesMap.end()) {
            databaseWay.priority = it.value();
        } else {
            databaseWay.priority = 1;
        }
        databaseWay.nodes.push_back(way.nodes[0]);
        PartNode node = bestPartition.nodes[way.nodes[0]];
        int prevCell = node.cell;
        int cell;
        for(unsigned int j=1; j < way.nodes.size(); ++j) {
            node = bestPartition.nodes[way.nodes[j]];
            cell = node.cell;
            //if(cell == prevCell) {
                // if still in the same cell then continue building inner way
                databaseWay.nodes.push_back(way.nodes[j]);
            //} else {
            if(cell != prevCell) {
                // if went to another cell than push inner way created so far to database partition, push boundary edge and start new inner way
                /*if(databaseWay.nodes.size() > 1) {
                    // if it's not just the start of way than push inner way to database partition
                    databasePartition.innerWays.push_back(databaseWay);
                }*/
                BoundaryEdge bEdge;
                bEdge.wayId = i;
                bEdge.nodeA = way.nodes[j-1];
                bEdge.nodeB = way.nodes[j];
                bEdge.priority = databaseWay.priority;
                databasePartition.boundaryEdges.push_back(bEdge);
                /*databaseWay.nodes.clear();
                databaseWay.nodes.push_back(way.nodes[j]);*/
            }
            prevCell = cell;
        }
        /*if(databaseWay.nodes.size()>1) {
            // if database way contains only one node than it means that last step detected boundary way and we should not push inner way to database
            databasePartition.innerWays.push_back(databaseWay);
        }*/
        databasePartition.innerWays.push_back(databaseWay);
    }

    // adding routing edges
    for(unsigned int i=0; i<bestPartition.cellsBoundaryNodes.size(); ++i) {
        for(unsigned int j=0; j<bestPartition.cellsBoundaryNodes[i]->size(); ++j) {
            for(unsigned int k=0; k<bestPartition.cellsBoundaryNodes[i]->size(); ++k) {
                if(k != j) {
                    RouteEdge rEdge;
                    rEdge.nodeA = bestPartition.cellsBoundaryNodes[i]->at(j);
                    rEdge.nodeB = bestPartition.cellsBoundaryNodes[i]->at(k);
                    databasePartition.routingEdges.push_back(rEdge);
                }
            }
        }
    }

    return databasePartition;
}

double Partitioning::CalculatePriority(unsigned int i, unsigned int j)
{
    //qDebug() << "Calculating priority for merging cells " << i << " and " << j << "...";
    // counts boundary edges and nodes of cell after merging
    int boundaryEdgesIJ = 0;
    std::vector< unsigned int > boundaryNodesIJ;
    for(unsigned int k=0; k<partition.boundaryEdges.size(); ++k) {
        BoundaryEdge edge = partition.boundaryEdges[k];

        // looks for edge that connects cell i and j
        if((partition.nodes[edge.nodeA].cell == i && partition.nodes[edge.nodeB].cell == j)
                || (partition.nodes[edge.nodeA].cell == j && partition.nodes[edge.nodeB].cell == i)) {
            boundaryEdgesIJ++;
        }

        // checks if any of nodes in edge is boundary node of cell i or j
        if((partition.nodes[edge.nodeA].cell == i)
                || (partition.nodes[edge.nodeA].cell == j)) {

            // checks if this node is already added to the list
            bool alreadyAdded = false;
            for(unsigned int m=0; m<boundaryNodesIJ.size(); ++m) {
                if(boundaryNodesIJ[m] == edge.nodeA) {
                    alreadyAdded = true;
                    break;
                }
            }
            if(!alreadyAdded)
                boundaryNodesIJ.push_back(edge.nodeA);
        }
        if((partition.nodes[edge.nodeB].cell == i)
                || (partition.nodes[edge.nodeB].cell == j)) {

            // checks if this node is already added to the list
            bool alreadyAdded = false;
            for(unsigned int m=0; m<boundaryNodesIJ.size(); ++m) {
                if(boundaryNodesIJ[m] == edge.nodeB) {
                    alreadyAdded = true;
                    break;
                }
            }
            if(!alreadyAdded)
                boundaryNodesIJ.push_back(edge.nodeB);
        }
    }

    // calculates the value of priority
    double numerator = boundaryEdgesIJ * (1 + partition.cellsBoundaryNodesCount[i] + partition.cellsBoundaryNodesCount[j] - boundaryNodesIJ.size());
    double denominator = partition.cellsNodesCount[i] * partition.cellsNodesCount[j];
    double result = (double)numerator/(double)denominator;

    if(boundaryEdgesIJ != 0) {
        //qDebug() << "Cell " << i << " and  " << j << " with priority = " << result;
        /*qDebug() << "boundaryEdgesIJ = " << boundaryEdgesIJ
                << " BoundaryNodesCount[i] = " << partition.cellsBoundaryNodesCount[i]
                << " BoundaryNodesCount[j] = " << partition.cellsBoundaryNodesCount[j]
                << " boundaryNodesIJ = " << boundaryNodesIJ.size()
                << " NodesCount[i] = " << partition.cellsNodesCount[i]
                << " NodesCount[j] = " << partition.cellsNodesCount[j]
                << "priority = " << result;*/
    }
    return result;
}

void Partitioning::MergeCells(unsigned int i, unsigned int j)
{
    //qDebug() << "Merging cells " << i << " and " << j << " with priority = " << partition.priorities[i][j] << " ...";
    for(unsigned int k=0; k<partition.nodes.size(); ++k) {
        if(partition.nodes[k].cell == j) {
            partition.nodes[k].cell = i;
        } else if(partition.nodes[k].cell > j) {
            partition.nodes[k].cell--;
        }
    }

    UpdatePartitionData();
}

void Partitioning::CalculatePriorities()
{
    // calculates initial priorities
    emit prioCalcStatusChanged("Calculating priorities...");
    qDebug() << "Calculating priorities...";
    emit prioCalcProgress(0);
    for(unsigned int i=0; i<partition.cellsCount; ++i) {
        if(i % 100 == 0) {
            emit prioCalcProgress((int)((double)i/(double)partition.cellsCount * 100) - 1);
        }

        std::map< unsigned int, double > * tmpMap = new std::map< unsigned int, double >;
        for(unsigned int j=0; j<partition.cellsConnections[i]->size(); ++j) {
            double tmp = partition.cellsConnections[i]->at(j);
            tmpMap->insert(std::pair< unsigned int, double > (tmp, CalculatePriority(i, tmp)));
        }
        partition.priorities.push_back(tmpMap);
    }
    emit prioCalcProgress(99);
}

void Partitioning::SavePriorities()
{
    emit prioCalcStatusChanged("Writing priorities to the file...");
    qDebug() << "Writing priorities to the file...";
    QFile file(prioritiesDataPath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);

    out << partition.cellsCount << "\n";
    for(unsigned int i=0; i<partition.cellsCount; ++i) {
        out << partition.priorities[i]->size() << " ";
        for(std::map< unsigned int, double >::iterator it = partition.priorities[i]->begin(); it!=partition.priorities[i]->end(); ++it) {
            unsigned int key = it->first;
            double value = it->second;
            out << key << ":" << value << " ";
        }
        out << "\n";
    }

    file.close();

    emit prioCalcStatusChanged("Finished.");
    emit prioCalcProgress(100);
    emit prioCalcFinished();
}

void Partitioning::LoadPriorities()
{
    emit partCalcStatusChanged("Reading priorities from file...");
    qDebug() << "Reading priorities from file...";
    emit partCalcProgress(1);

    QFile file(prioritiesDataPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    unsigned int cellsCount = line.toInt();
    for(unsigned int i=0; i<cellsCount; ++i) {
        std::map< unsigned int, double > * tmpMap = new std::map< unsigned int, double >;
        line = in.readLine();
        QStringList stringList = line.split(" ");
        unsigned int connectionsCount = stringList.at(0).toInt();
        for(unsigned int j=1; j<=connectionsCount; ++j) {
            QStringList tmpPair = stringList.at(j).split(":");
            tmpMap->insert(std::pair< unsigned int, double > (tmpPair.at(0).toInt(), tmpPair.at(1).toDouble()));
        }
        partition.priorities.push_back(tmpMap);
    }

    file.close();

    emit partCalcProgress(2);
}

void Partitioning::setStage(PARTITIONING_STAGE newStage)
{
    stage = newStage;
}

void Partitioning::setDatabasePath(QString path)
{
    databasePath = QString(path);
}

void Partitioning::setSimpleDataPath(QString path)
{
    simpleDataPath = QString(path);
}

void Partitioning::setPrioritiesDataPath(QString path)
{
    prioritiesDataPath = QString(path);
}

void Partitioning::setFinalDataPath(QString path)
{
    finalDataPath = QString(path);
}

void Partitioning::Delete()
{
    for(unsigned int i=0; i<partition.priorities.size(); ++i) {
        delete partition.priorities[i];
    }
}
}
