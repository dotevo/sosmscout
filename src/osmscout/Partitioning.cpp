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

#include <osmscout/Database.h>
#include <osmscout/StyleConfigLoader.h>
#include <osmscout/Util.h>
#include <osmscout/MapPainter.h>
#include <osmscout/Partitionmodel.h>

namespace osmscout {
    Partitioning::Partitioning(QString mapDir, QString style)
    {
        this->mapDir = mapDir;
        this->style = style;

        Init();
    }

    void Partitioning::Init()
    {
        qDebug() << "Reading data...";

        // setting alpha factor for quality function
        alpha = 0.3;

        // getting data from database
        MapData data;
        AreaSearchParameter parameter;

        double lonMin, latMin, lonMax, latMax, magnification;
        lonMin = -20;
        latMin = -20;
        lonMax = 99;
        latMax = 99;
        magnification = 300000;

        osmscout::DatabaseParameter databaseParameter;
        osmscout::Database          database(databaseParameter);

        if (!database.Open((const char*)mapDir.toAscii())) {
            std::cerr << "Cannot open database" << std::endl;
        }

        osmscout::StyleConfig styleConfig(database.GetTypeConfig());

        if (!osmscout::LoadStyleConfig((const char*)style.toAscii(),styleConfig)) {
            std::cerr << "Cannot open style" << std::endl;
        }

        database.GetObjects(styleConfig,
                    lonMin,
                    latMin,
                    lonMax,
                    latMax,
                    magnification,
                    parameter,
                    data.nodes,
                    data.ways,
                    data.areas,
                    data.relationWays,
                    data.relationAreas);

        qDebug() << "Initializing data...";
        qDebug() << "ways " << data.ways.size();

        // create partition ways
        int idx = 0;
        int maxCell = 0;
        int wayCntr = 0;
        for (std::vector< WayRef >::const_iterator w = data.ways.begin(); w != data.ways.end(); ++w) {
            const WayRef& way= *w;
            //qDebug() << "way " << wayCntr << " nodes " << partition.nodes.size();
            wayCntr++;
            PartWay partWay;
            for(std::vector< Point >::const_iterator p = way->nodes.begin(); p != way->nodes.end(); ++p) {
                const Point point = *p;
                // checks if this node is in any other way
                int waysContainingNodeCount = 0;
                if(p != way->nodes.begin() && p !=  way->nodes.end() - 1) {
                    //qDebug() << "   point " << point.GetId();
                    for (std::vector< WayRef >::const_iterator tmpW = data.ways.begin(); tmpW != data.ways.end(); ++tmpW) {
                        const WayRef& tmpWay= *tmpW;
                        for(std::vector< Point >::const_iterator tmpP = tmpWay->nodes.begin(); tmpP != tmpWay->nodes.end(); ++tmpP) {
                            const Point tmpPoint = *tmpP;
                            if((tmpPoint.GetLon() == point.GetLon()) && (tmpPoint.GetLat() == point.GetLat())) {
                                waysContainingNodeCount++;
                                // break; // is it posible that one node is twice in the same way?
                            }
                        }
                        if(waysContainingNodeCount > 1)
                            break;
                    }
                } else {
                    waysContainingNodeCount = 2; // just becouse way needs starting and end node
                }

                if(waysContainingNodeCount > 1) {

                    // checks if this node is already added to the nodes list
                    bool alreadyAdded = false;
                    for(unsigned int i=0; i<partition.nodes.size(); ++i) {
                        const PartNode pNode= partition.nodes[i];
                        if(pNode.lon == point.GetLon()&& pNode.lat == point.GetLat()) {
                            alreadyAdded = true;
                            idx = i;
                            break;
                        }
                    }
                    if(!alreadyAdded) {
                        PartNode partNode;
                        partNode.id = point.GetId();
                        partNode.lon = point.GetLon();
                        partNode.lat = point.GetLat();
                        partNode.cell = maxCell;
                        partNode.type = INTERNAL;
                        partition.nodes.push_back(partNode);
                        idx = maxCell;
                        maxCell++;
                    }
                    partWay.nodes.push_back(idx);
                }
            }
            partWay.id = way->GetId();
            partition.ways.push_back(partWay);
        }

        UpdatePartitionData();

        // calculates initial priorities
        qDebug() << "Reserving space for priorities...";
        partition.priorities.reserve(partition.cellsCount * partition.cellsCount);
        for(unsigned int i=0; i<partition.cellsCount; ++i) {
            //qDebug() << i;
            for(unsigned int j=0; j<partition.cellsCount; ++j) {
                partition.priorities[i].push_back(0);
                //partition.priorities[i].reserve(partition.cellsCount);
            }
        }
        qDebug() << "Calculating priorities...";
        for(unsigned int i=0; i<partition.cellsCount; ++i) {
            for(unsigned int j=i+1; j<partition.cellsCount; ++j) {
                partition.priorities[i][j] = CalculatePriority(i, j);
            }
        }

        /*qDebug() << "Writing priorities to the file...";

        QFile file("priorities.txt");
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return;
        }
        QTextStream out(&file);
        out << "xxx";*/

        // TODO: writing to file
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
        for (unsigned int i=0; i < partition.nodesCount; ++i) {
            PartNode node = partition.nodes[i];
            //qDebug() << " node " << node.ref->GetLon() << " " << node.ref->GetLat();
            if(node.cell >= partition.cellsCount) { // ATTENTION!! It's important to take care that there's no missing cells number, ie. 1, 2, 4, 5, 8 <- WRONG!
                partition.cellsCount = node.cell + 1; // cells are numbered starting from 0
            }
            node.type = INTERNAL;
        }

        partition.cellsBoundaryNodesCount.clear();
        partition.cellsNodesCount.clear();
        partition.cellsEdgesCount.clear();
        partition.cellsRouteEdgesCount.clear();
        partition.boundaryEdges.clear();

        partition.cellsBoundaryNodesCount.reserve(partition.cellsCount);
        partition.cellsNodesCount.reserve(partition.cellsCount);
        partition.cellsEdgesCount.reserve(partition.cellsCount);
        partition.cellsRouteEdgesCount.reserve(partition.cellsCount);
        partition.boundaryEdges.reserve(partition.cellsCount);

        for(unsigned int i=0; i<partition.cellsCount; ++i) {
            partition.cellsBoundaryNodesCount.push_back(0);
            partition.cellsNodesCount.push_back(0);
            partition.cellsEdgesCount.push_back(0);
            partition.cellsRouteEdgesCount.push_back(0);
        }

        // counts edges and adds boundary edges to list
        //qDebug() << "Counting edges...";
        for (unsigned int i=0; i < partition.ways.size(); ++i) {
            const PartWay way = partition.ways[i];
            //qDebug() << "w " << way->GetId() << "\n";
            PartNode prevNode = partition.nodes[way.nodes[0]];
            PartNode node;
            int prevCell = prevNode.cell;
            int cell;
            for(unsigned int j=1; j < way.nodes.size(); ++j) {
                node = partition.nodes[way.nodes[j]];
                cell = node.cell;
                if(cell == prevCell) {
                    partition.cellsEdgesCount[cell] += 1;
                } else {
                    partition.nodes[way.nodes[j-1]].type = BOUNDARY;
                    partition.nodes[way.nodes[j]].type = BOUNDARY;

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
            //qDebug() << "partN " << nodeRef->GetId() << " cell " << node.cell;
            partition.cellsNodesCount[node.cell] += 1;
            if(node.type == BOUNDARY) {
                partition.cellsBoundaryNodesCount[node.cell] += 1;
            }
        }
        for(unsigned int i=0; i<partition.cellsCount; ++i) {
            partition.boundaryNodesCount += partition.cellsBoundaryNodesCount[i];
        }

        // calculates route edges in cells
        //qDebug() << "Counting route edges...";
        for(unsigned int i=0; i<partition.cellsBoundaryNodesCount.size(); i++) {
            partition.cellsRouteEdgesCount[i] = partition.cellsBoundaryNodesCount[i] * (partition.cellsBoundaryNodesCount[i] - 1);
        }

        // prints values
        //qDebug() << "CELLS: " << partition.cellsCount;
        /*qDebug() << "ways: " << partition.ways.size();
        qDebug() << "nodesCount: " << partition.nodesCount;
        qDebug() << "boundaryNodesCount: " << partition.boundaryNodesCount;
        qDebug() << "boundaryEdgesCount: " << partition.boundaryEdgesCount;
        qDebug() << "cellsCount: " << partition.cellsCount;
        qDebug() << "cellsBoundaryNodesCount[0]: " << partition.cellsBoundaryNodesCount[0];
        qDebug() << "cellsRouteEdgesCount[0]: " << partition.cellsRouteEdgesCount[0];*/
    }

    void Partitioning::UpdatePriorities(unsigned int i, unsigned int j)
    {
        //qDebug() << "Updating priorities...";
        // updates lines for merged cell
        for(unsigned int k=0; k<i; ++k) {
            partition.priorities[k][i] = CalculatePriority(k, i);
        }
        for(unsigned int l=i+1; l<partition.cellsCount; ++l) {
            partition.priorities[i][l] = CalculatePriority(i, l);
        }

        // moves lines to delete those for deleted cell
        for(unsigned int k = j; k<partition.cellsCount; ++k) {
            partition.priorities[k] = partition.priorities[k+1];
        }
        for(unsigned int k = 0; k<partition.cellsCount; ++k) {
            for(unsigned int l=j; l<partition.cellsCount; ++l) {
                partition.priorities[k][l] = partition.priorities[k][l+1];
            }
        }

        // resets last rows
        for(unsigned int k = 0; k<partition.cellsCount; ++k) {
            partition.priorities[k][partition.cellsCount] = 0;
        }
        for(unsigned int l = 0; l<partition.cellsCount+1; ++l) {
            partition.priorities[partition.cellsCount][l] = 0;
        }
        /*
        // moves lines to delete those for deleted cell
        for(unsigned int k = j; k<(partition.cellsCount-1); ++k) {
            partition.priorities[k] = partition.priorities[k+1];
        }
        for(unsigned int k = 0; k<partition.cellsCount; ++k) {
            for(unsigned int l=j; l<(partition.cellsCount-1); ++l) {
                partition.priorities[k][l] = partition.priorities[k][l+1];
            }
        }*/
    }

    double Partitioning::CalculateQuality()
    {
        //qDebug() << "Calculating quality of partition...";
        double sumA = 0;
        double sumB = 0;

        for(unsigned int i=0; i<partition.cellsCount; i++) {
            double fraction = (double)partition.cellsNodesCount[i]/(double)partition.nodesCount;
            sumA += (fraction) * (2 - fraction) * (double)partition.cellsEdgesCount[i];
            sumB += pow(1 - fraction, 2) * (double)partition.cellsRouteEdgesCount[i];
        }

        double result = sumA
                + alpha * (sumB + (double)partition.boundaryEdgesCount);

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

    void Partitioning::saveToDatabase(QString name, DatabasePartition& databasePartition){

        osmscout::PartitionModel pm;
        pm.open(name);
        pm.createTables();
        pm.exportToDatabase(databasePartition);
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

        quality = CalculateQuality();
        bestQuality = quality;
        bestCellsCount = partition.cellsCount;

        while(partition.cellsCount > 1) {
            maxPriority = 0;
            for(unsigned int i=0; i<partition.cellsCount-1; ++i) {
                for(unsigned int j=i+1; j<partition.cellsCount; ++j) {
                    priority = partition.priorities[i][j] * (1+(qrand()/32767./100));
                    if(priority > maxPriority) {
                        cellI = i;
                        cellJ = j;
                        maxPriority = partition.priorities[i][j];
                    }
                }
            }

            if(maxPriority > 0) {
                MergeCells(cellI, cellJ);
                UpdatePriorities(cellI, cellJ);

                quality = CalculateQuality();
                if(quality < bestQuality) {
                    bestQuality = quality;
                    bestCellsCount = partition.cellsCount;

                    bestPartition.nodes.clear();
                    for(unsigned int i=0; i<partition.nodes.size(); ++i) {
                        bestPartition.nodes[i] = partition.nodes[i];
                    }
                    bestPartition.ways.clear();
                    for(unsigned int i=0; i<partition.ways.size(); ++i) {
                        bestPartition.ways[i] = partition.ways[i];
                    }
                }
            } else {
                qDebug() << "best quality: " << bestQuality;
                qDebug() << "best cells count: " << bestCellsCount;

                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                // Next +/-50 lines needs to be put into another method (maybe even savaToDatabase) and then it needs adding routing edges.
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                // preparing format for writing to database
                DatabasePartition databasePartition;
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
                // TODO: adding routing edges to database partition

                // saving to database
                return databasePartition;

                break;
            }
            qDebug() << "";
        }
        DatabasePartition empty;
        return empty;
    }

    double Partitioning::CalculatePriority(unsigned int i, unsigned int j)
    {
        //qDebug() << "Calculating priority for merging cells " << i << " and " << j << "...";
        // counts boundary edges and nodes of cell after merging
        int boundaryEdgesIJ = 0;
        std::vector< unsigned int > boundaryNodesIJ;
        for(unsigned int k=0; k<partition.boundaryEdges.size(); ++k) {
            BoundaryEdge edge = partition.boundaryEdges[k];

            // looks for edge thet connects cell i and j
            if((partition.nodes[edge.nodeA].cell == i && partition.nodes[edge.nodeB].cell == j)
                    || (partition.nodes[edge.nodeA].cell == j && partition.nodes[edge.nodeB].cell == i)) {
                boundaryEdgesIJ++;

                // checks if any of nodes in found edge connects to another cell
                for(unsigned int l=0; l<partition.boundaryEdges.size(); ++l) {
                    BoundaryEdge tEdge = partition.boundaryEdges[l];
                    if((tEdge.nodeA == edge.nodeA && partition.nodes[tEdge.nodeB].cell != partition.nodes[edge.nodeB].cell)
                            || (tEdge.nodeB == edge.nodeA && partition.nodes[tEdge.nodeA].cell != partition.nodes[edge.nodeB].cell)) {

                        // checks if this node is already added to the list
                        bool alreadyAdded = false;
                        for(unsigned int m=0; m<boundaryNodesIJ.size(); ++m) {
                            if(boundaryNodesIJ[m] == edge.nodeA)
                                alreadyAdded = true;
                        }
                        if(!alreadyAdded)
                            boundaryNodesIJ.push_back(edge.nodeA);
                    }
                    if((tEdge.nodeA == edge.nodeB && partition.nodes[tEdge.nodeB].cell != partition.nodes[edge.nodeA].cell)
                            || (tEdge.nodeB == edge.nodeB && partition.nodes[tEdge.nodeA].cell != partition.nodes[edge.nodeA].cell)) {

                        // checks if this node is already added to the list
                        bool alreadyAdded = false;
                        for(unsigned int m=0; m<boundaryNodesIJ.size(); ++m) {
                            if(boundaryNodesIJ[m] == edge.nodeB)
                                alreadyAdded = true;
                        }
                        if(!alreadyAdded)
                            boundaryNodesIJ.push_back(edge.nodeB);
                    }
                }
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
        //qDebug() << "Cells " << i << " and " << j << "merged";
    }
}
