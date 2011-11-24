#include <osmscout/Partitioning.h>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>

#include <QFileInfo>
#include <QDir>

#include <QMouseEvent>
#include <QLineF>

//#include <QtGui/QPixmap>
//#include <QtGui/QApplication>

#include <QDebug>

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
        alpha = 0.98;
        beta = 0.010;
        //alpha = 0.95;
        //beta = 1;
    }

    void Partitioning::InitData()
    {
        qDebug() << "Reading data...";

        QSqlDatabase db;
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("D:\\pilocik\\map\\poland.db");
        if(!db.open()){
            qDebug() << "[SQLiteDatabase::open]" << db.lastError();
            return;
        }

        double lonMin, latMin, lonMax, latMax, magnification;
        latMin = 50.1;
        lonMin = 14.85;
        latMax = 51.5;
        lonMax = 17.5;

        latMin = 50.2;
        lonMin = 16.4;
        latMax = 51.3;
        lonMax = 16.5;

        //
        // getting nodes
        //
        qDebug() << "Getting nodes...";
        QSqlQuery query(db);
        QString tmp = "SELECT id,lon,lat FROM nodes WHERE lon>" + QString::number(lonMin)
                + " AND lat>" + QString::number(latMin)
                + " AND lon<" + QString::number(lonMax)
                + " AND lat<" + QString::number(latMax);
        query.prepare(tmp);
        query.exec();

        std::vector< PartNode > tmpNodes;
        tmpNodes.clear();
        unsigned int cell = 0;
        while (query.next()) {
            PartNode newNode;
            newNode.id = query.value(0).toInt();
            newNode.lon = query.value(1).toDouble();
            newNode.lat = query.value(2).toDouble();
            newNode.cell = cell;
            cell++;
            newNode.type = INTERNAL;
            tmpNodes.push_back(newNode);
        }

        //
        // getting ways
        //
        qDebug() << "Getting ways...";
        tmp = "SELECT way,num,node FROM ways,way_nodes WHERE way_nodes.way=ways.id AND ways.lon1>" + QString::number(lonMin)
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

        query.clear();
        query.prepare(tmp);
        query.exec();

        std::vector< PartWay > tmpWays;
        PartWay newWay;
        newWay.id = 0;
        while (query.next()) {
            unsigned int id = query.value(0).toLongLong();
            if(newWay.id == 0) {
                newWay.id = id; // first way
            }

            if(newWay.id != id) {
                tmpWays.push_back(newWay);
                newWay.id = id;
                newWay.nodes.clear();
            }

            unsigned int nodeId = query.value(2).toInt();
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

        //
        // deleting all unnecessary nodes
        //
        qDebug() << "Deleting all unnecessary nodes...";
        for(unsigned int i=0; i<tmpNodes.size(); ++i) {
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
                tmpNodes[i].id = -1;
                tmpNodes[i].lon = -1;
                tmpNodes[i].lat = -1;
            }
        }

        //
        // deleting all unnecessary nodes from ways
        //
        qDebug() << "Deleting all unnecessary nodes from ways...";
        for(unsigned int i=0; i<tmpWays.size(); ++i) {
            PartWay newWay;
            newWay.id = tmpWays[i].id;
            newWay.priority = 1; // TODO: calculating priority due to road type
            newWay.oneway = 0;\
            QString tmpString = "";
            newWay.type = tmpString;
            newWay.nodes.clear();

            //check if each node exists in nodes vector
            for(unsigned int j=0; j<tmpWays[i].nodes.size(); ++j) {
                // if this node is not deleted
                if(!(tmpNodes[tmpWays[i].nodes[j]].id == -1
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

        //
        // checking for oneways
        //
        // for optimalization - get it all to list and then check
        qDebug() << "Checking for oneways...";
        for(int i=0; i<partition.ways.size(); ++i) {
            tmp = "SELECT value FROM way_tags LEFT JOIN tags ON way_tags.tag = tags.id"
                    " WHERE tag IN("
                        " SELECT id FROM tags"
                        " WHERE key='oneway')"
                    " AND ref == '" + QString::number(partition.ways[i].id) + "'";
            query.clear();
            query.prepare(tmp);
            query.exec();

            if(query.next()) {
                QString onewayString = query.value(0).toString();

                int x = onewayString.compare("1");

                if(onewayString.compare("1") == 0
                            || onewayString.compare("yes") == 0
                            || onewayString.compare("true") == 0
                            || onewayString.compare("1, yes") == 0) {
                    partition.ways[i].oneway = 1;
                } else if(onewayString.compare("-1") == 0
                            || onewayString.compare("no") == 0
                            || onewayString.compare("false") == 0
                            || onewayString.compare("-1;yes") == 0) {
                    partition.ways[i].oneway = -1;
                }
            }
        }

        //
        // checking for priorities
        //
        qDebug() << "Checking for priorities...";
        for(int i=0; i<partition.ways.size(); ++i) {
            tmp = "SELECT value FROM way_tags LEFT JOIN tags ON way_tags.tag = tags.id"
                    " WHERE tag IN("
                        " SELECT id FROM tags"
                        " WHERE key='highway')"
                    " AND ref == '" + QString::number(partition.ways[i].id) + "'";
            query.clear();
            query.prepare(tmp);
            query.exec();

            if(query.next()) {
                QString highwayType = query.value(0).toString();
                partition.ways[i].type = highwayType;
            }
        }

        UpdatePartitionData();
    }

    void Partitioning::SaveData(QString path)
    {
        qDebug() << "Writing partition to the file...";
        QFile file(path);
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
    }

    void Partitioning::LoadData(QString path)
    {
        qDebug() << "Reading partition from file...";

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        QTextStream in(&file);
        QString line = in.readLine();
        unsigned int nodesCount = line.toInt();
        line = in.readLine();
        unsigned int waysCount = line.toInt();
        line = in.readLine();
        unsigned int cellsCount = line.toInt();
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
            unsigned int wayNodesCount = stringList.at(1).toInt();
            for(unsigned int j=0; j<wayNodesCount; ++j) {
                newWay.nodes.push_back(stringList.at(2+j).toInt());
            }

            partition.ways.push_back(newWay);
        }

        file.close();

        UpdatePartitionData();
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
                partition.cellsBoundaryNodes[node.cell]->push_back(node.id);
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


        // old way
        /*// updates lines for merged cell
        for(unsigned int k=0; k<i; ++k) {
            for(unsigned l=0; l<partition.cellsConnections[k]->size(); ++l) { // calculates priority only if connection exists between cell k and i
                if(partition.cellsConnections[k]->at(l) == i) {
                    partition.priorities[k]->at(i) = CalculatePriority(k, i);
                    break;
                }
            }
        }
        for(unsigned int k=i+1; k<partition.cellsCount; ++k) {
            for(unsigned l=0; l<partition.cellsConnections[k]->size(); ++l) { // calculates priority only if connection exists between cell i and k
                if(partition.cellsConnections[i]->at(l) == k) {
                    partition.priorities[i]->at(k) = CalculatePriority(i, k);
                    break;
                }
            }
        }

        // moves lines to delete those for deleted cell
        for(unsigned int k = j; k<partition.cellsCount; ++k) {
            partition.priorities[k] = partition.priorities[k+1];
        }
        for(unsigned int k = 0; k<partition.cellsCount; ++k) {
            for(unsigned int l=j; l<partition.cellsCount; ++l) {
                partition.priorities[k]->at(l) = partition.priorities[k]->at(l+1);
            }
        }

        // resets last rows
        for(unsigned int k = 0; k<partition.cellsCount; ++k) {
            partition.priorities[k]->at(partition.cellsCount) = 0;
        }
        for(unsigned int k = 0; k<partition.cellsCount+1; ++k) {
            partition.priorities[partition.cellsCount]->at(k) = 0;
        }*/
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

    void Partitioning::saveToDatabase(QString name, DatabasePartition& databasePartition)
    {
        //
        // saving to sql database
        //
        osmscout::PartitionModel pm;
        pm.open(name);
        pm.createTables();
        pm.exportToDatabase(databasePartition);

        //
        // saving to binary file
        //
        PiLibocik::Partition::PartitionFile partitionFile("test", "car", QIODevice::WriteOnly, 1);
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
                PiLibocik::Partition::Node nodeInWay = nodes[way.nodes[j]];

                fileWay.addNode(way.nodes[j]); // ATTENTION! zmienic na indeksy nodów w liœcie, a nie ich id (w FindPartition na koñcu przy tworzeniu DatabasePartition)
                nodeInWay.addWay(i);
            }

            ways.push_back(fileWay);
        }

        // adding boundary edges
        for(unsigned int i=0; i<databasePartition.boundaryEdges.size(); ++i) {
            Partitioning::BoundaryEdge edge = databasePartition.boundaryEdges[i];
            PiLibocik::Partition::BoundaryEdge fileEdgeAB(edge.nodeB, edge.wayId, edge.priority); // edge from A to B
            PiLibocik::Partition::BoundaryEdge fileEdgeBA(edge.nodeA, edge.wayId, edge.priority); // edge from B to A
            PiLibocik::Partition::Node fileNodeA = nodes[edge.nodeA];
            PiLibocik::Partition::Node fileNodeB = nodes[edge.nodeB];

            fileNodeA.addBoundaryEdge(fileEdgeAB);
            fileNodeB.addBoundaryEdge(fileEdgeBA);
        }

        // adding routing edges
        for(unsigned int i=0; i<databasePartition.routingEdges.size(); ++i) {
            Partitioning::RouteEdge edge = databasePartition.routingEdges[i];
            PiLibocik::Partition::Edge fileEdgeAB(edge.nodeB, 1); // edge from A to B
            PiLibocik::Partition::Edge fileEdgeBA(edge.nodeA, 1); // edge from B to A
            PiLibocik::Partition::Node fileNodeA = nodes[edge.nodeA];
            PiLibocik::Partition::Node fileNodeB = nodes[edge.nodeB];

            fileNodeA.addRoutingEdge(fileEdgeAB);
            fileNodeB.addRoutingEdge(fileEdgeBA);
        }

        // creating list of nodes
        for(unsigned int i=0; i<nodes.size(); ++i) {
            fileNodes.append(nodes[i]);
        }

        // creating list of ways
        for(unsigned int i=0; i<ways.size(); ++i) {
            fileWays.append(ways[i]);
        }

        partitionFile.savePartition(fileWays, fileNodes, 2);
    }

    Partitioning::DatabasePartition  Partitioning::FindPartition()
    {
        qDebug() << "Finding partition...";

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
                // old way
                /*for(unsigned int j=0; j<partition.cellsCount-i-1; ++j) {
                    bool connected = false;
                    for(unsigned k=0; k<partition.cellsConnections[i]->size(); ++k) { // calculates priority only if connection exists between cell i and j
                        if(partition.cellsConnections[i]->at(k) == j) {
                            connected = true;
                            break;
                        }
                    }
                    priority = ((!connected) ? 0 : partition.priorities[i]->at(j)) * (1+(qrand()/32767./100));

                    if(priority > maxPriority) {
                        cellI = i;
                        cellJ = j;
                        maxPriority = priority;
                    }
                }*/
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

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // Next +/-50 lines needs to be put into another method (maybe even savaToDatabase).
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // preparing format for writing to database
        DatabasePartition databasePartition;
        qDebug()<<"FOUND PARTITION N:"<<bestPartition.nodes.size()<<" W:"<<bestPartition.ways.size();
        databasePartition.nodes = bestPartition.nodes;

        // adding inner ways and boundary edges
        for (unsigned int i=0; i < bestPartition.ways.size(); ++i) {
            const PartWay way = bestPartition.ways[i];

            PartWay databaseWay;
            databaseWay.id = way.id;
            databaseWay.priority = 1; // TODO: calculating priority somehow
            databaseWay.nodes.push_back(bestPartition.nodes[way.nodes[0]].id);
            PartNode node = bestPartition.nodes[way.nodes[0]];
            int prevCell = node.cell;
            int cell;
            for(unsigned int j=1; j < way.nodes.size(); ++j) {
                node = bestPartition.nodes[way.nodes[j]];
                cell = node.cell;
                if(cell == prevCell) {
                    // if still in the same cell then continue building inner way
                    databaseWay.nodes.push_back(bestPartition.nodes[way.nodes[j]].id);
                } else {
                    // if went to another cell than push inner way created so far to database partition, push boundary edge and start new inner way
                    if(databaseWay.nodes.size() > 1) {
                        // if it's not just the start of way than push inner way to database partition
                        databasePartition.innerWays.push_back(databaseWay);
                    }
                    BoundaryEdge bEdge;
                    bEdge.wayId = way.id;
                    bEdge.nodeA = bestPartition.nodes[way.nodes[j-1]].id;
                    bEdge.nodeB = bestPartition.nodes[way.nodes[j]].id;
                    bEdge.priority = databaseWay.priority;
                    databasePartition.boundaryEdges.push_back(bEdge);
                    databaseWay.nodes.clear();
                    databaseWay.nodes.push_back(bestPartition.nodes[way.nodes[j]].id);
                }
                prevCell = cell;
            }
            if(databaseWay.nodes.size()>1) {
                // if database way contains only one node than it means that last step detected boundary way and we should not push inner way to database
                databasePartition.innerWays.push_back(databaseWay);
            }
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
        qDebug() << "Calculating priorities...";
        for(unsigned int i=0; i<partition.cellsCount; ++i) {
            std::map< unsigned int, double > * tmpMap = new std::map< unsigned int, double >;
            for(unsigned int j=0; j<partition.cellsConnections[i]->size(); ++j) {
                double tmp = partition.cellsConnections[i]->at(j);
                tmpMap->insert(std::pair< unsigned int, double > (tmp, CalculatePriority(i, tmp)));
            }
            // old way
            /*std::vector< double > * tmpVector = new std::vector< double >;
            for(unsigned int j=i+1; j<partition.cellsCount; ++j) {
                bool connected = false;
                for(unsigned k=0; k<partition.cellsConnections[i]->size(); ++k) { // calculates priority only if connection exists between cell i and j
                    if(partition.cellsConnections[i]->at(k) == j) {
                        connected = true;
                        break;
                    }
                }
                tmpVector->push_back((!connected) ? 0 : CalculatePriority(i, j));
            }*/
            partition.priorities.push_back(tmpMap);
        }
    }

    void Partitioning::SavePriorities(QString path)
    {
        qDebug() << "Writing priorities to the file...";
        QFile file(path);
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
        // old way
        /*for(unsigned int i=0; i<partition.cellsCount; ++i) {
            for(unsigned int j=0; j<partition.cellsCount-i-1; ++j) {
                out << partition.priorities[i]->at(j) << " ";
            }
            out << "\n";
        }*/

        file.close();
    }

    void Partitioning::LoadPriorities(QString path)
    {
        qDebug() << "Reading priorities from file...";
        QFile file(path);
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

        // old way
        /*for(unsigned int i=0; i<partition.cellsCount; ++i) {
            std::vector< double > * tmpVector = new std::vector< double >;
            QString line = in.readLine();
            QStringList stringList = line.split(" ");
            for(unsigned int j=0; j<partition.cellsCount-i-1; ++j) {
                tmpVector->push_back(stringList.at((j)).toDouble());
            }
            partition.priorities.push_back(tmpVector);
        }*/

        file.close();
    }

    void Partitioning::Delete()
    {
        for(unsigned int i=0; i<partition.priorities.size(); ++i) {
            delete partition.priorities[i];
        }
    }
}
