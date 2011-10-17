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

namespace osmscout {
    Partitioning::Partitioning(QString mapDir, QString style)
    {
        this->mapDir = mapDir;
        this->style = style;

        Init();
    }

    void Partitioning::Init()
    {
        MapData data;
        AreaSearchParameter parameter;

        double lonMin, latMin, lonMax, latMax, magnification;
        lonMin = 19.142;
        latMin = 42.2725;
        lonMax = 19.178;
        latMax = 42.2875;
        magnification = 10000;

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

        // create partition ways
        int idx = 0;
        int maxCell = 0;
        bool alreadyAdded;
        for (std::vector<WayRef>::const_iterator w = data.ways.begin(); w != data.ways.end(); ++w) {
            const WayRef& way= *w;
            //qDebug() << "way " << way->GetId();
            PartWay partWay;
            for(std::vector<Point>::const_iterator n = way->nodes.begin(); n != way->nodes.end(); ++n) {
                const Point point = *n;
                alreadyAdded = false;
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
            partition.ways.push_back(partWay);
        }

        UpdatePartitionData();
    }

    void Partitioning::UpdatePartitionData()
    {
        qDebug() << "Updating partition...";
        // counts cells
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
            partition.cellsBoundaryNodesCount[i] = 0;
            partition.cellsNodesCount[i] = 0;
            partition.cellsEdgesCount[i] = 0;
            partition.cellsRouteEdgesCount[i] = 0;
        }

        // counts edges and adds boundary edges to list
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
                    partition.cellsEdgesCount[cell]++;
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
        for (unsigned int i=0; i<partition.nodesCount; ++i) {
            PartNode node = partition.nodes[i];
            //qDebug() << "partN " << nodeRef->GetId() << " cell " << node.cell;
            partition.cellsNodesCount[node.cell]++;
            if(node.type == BOUNDARY) {
                partition.cellsBoundaryNodesCount[node.cell]++;
            }
        }
        for(unsigned int i=0; i<partition.cellsCount; ++i) {
            partition.boundaryNodesCount += partition.cellsBoundaryNodesCount[i];
        }

        // calculates route edges in cells
        for(unsigned int i=0; i<partition.cellsBoundaryNodesCount.size(); i++) {
            partition.cellsRouteEdgesCount[i] = partition.cellsBoundaryNodesCount[i] * (partition.cellsBoundaryNodesCount[i] - 1);
        }

        // prints values
        qDebug() << "ways: " << partition.ways.size();
        qDebug() << "nodesCount: " << partition.nodesCount;
        qDebug() << "boundaryNodesCount: " << partition.boundaryNodesCount;
        qDebug() << "boundaryEdgesCount: " << partition.boundaryEdgesCount;
        qDebug() << "cellsCount: " << partition.cellsCount;
        qDebug() << "cellsBoundaryNodesCount[0]: " << partition.cellsBoundaryNodesCount[0];
        qDebug() << "cellsRouteEdgesCount[0]: " << partition.cellsRouteEdgesCount[0];
    }

    double Partitioning::CalculateQuality()
    {
        //qDebug() << "Calculating quality...";
        double alpha;

        alpha = 0.5;
        double sumA;
        double sumB;

        for(unsigned int i=0; i<partition.cellsCount; i++) {
            sumA += (partition.cellsNodesCount[i]/partition.nodesCount)*(2 - (partition.cellsNodesCount[i]/partition.nodesCount))*partition.cellsEdgesCount[i];
            sumB += pow(1 - (partition.cellsNodesCount[i]/partition.nodesCount), 2) * partition.cellsRouteEdgesCount[i];
        }

        double result = sumA + (alpha * sumB) + partition.boundaryEdgesCount;

        qDebug() << "Quality = " << result;
        return result;
    }

    void Partitioning::TestAlgorithm()
    {
        qDebug() << "Testing algorithm...";
        double priority;
        double quality;
        quality = CalculateQuality();
        qDebug() << "";
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
            qDebug() << "";
        }

    }

    void Partitioning::FindPartition()
    {
        qDebug() << "Finding partition...";
        double priority;
        double maxPriority;
        double quality;
        double bestQuality;
        quality = bestQuality = CalculateQuality();
        unsigned int cellI;
        unsigned int cellJ;
        while(partition.cellsCount > 0) {
            maxPriority = 0;
            for(unsigned int i=0; i<partition.cellsCount-1; ++i) {
                for(unsigned int j=i+1; j<partition.cellsCount; ++j) {
                    priority = CalculatePriority(i, j);
                    if(priority > maxPriority) {
                        cellI = i;
                        cellJ = j;
                        maxPriority = priority;
                    }
                }
            }
            MergeCells(cellI, cellJ);

            quality = CalculateQuality();
            if(quality < bestQuality) {
                bestQuality = quality;
            }
        }
    }

    double Partitioning::CalculatePriority(unsigned int i, unsigned int j)
    {
        //qDebug() << "Calculating priority for " << i << " and " << j << "...";
        // counts boundary edges and nodes of cell after merging
        int boundaryEdgesIJ = 0;
        std::vector<unsigned int> boundaryNodesIJ;
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
        double result = ((double)numerator/(double)denominator) * (1+(qrand()/32767./100));

        if(boundaryEdgesIJ != 0) {
            qDebug() << "Cell " << i
                << " and  " << j
                << " with priority = " << result;
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
        qDebug() << "Merging cells " << i << " and " << j << "...";
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
