#include <osmscout/Partitioning.h>

#include <iostream>
#include <iomanip>

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
    Partitioning::Partitioning()
    {
    }

    bool Partitioning::PartitionQuality(QString map, QString style, double& result)
    {
        qDebug() << "part!";

        int nodesCount;
        std::vector<int> cellNodesCount;
        std::vector<int> cellEdgesCount;
        std::vector<int> cellBoundaryNodesCount;
        std::vector<int> cellRouteEdgesCount;
        int boundaryEdgesCount;
        int cellsCount;
        double alpha;

        MapData data;
        AreaSearchParameter parameter;

        double lonMin, latMin, lonMax, latMax, magnification;
        lonMin = -99;
        latMin = -99;
        lonMax = 99;
        latMax = 99;
        magnification = 10000;

        osmscout::DatabaseParameter databaseParameter;
        osmscout::Database          database(databaseParameter);

        if (!database.Open((const char*)map.toAscii())) {
            std::cerr << "Cannot open database" << std::endl;
            return 1;
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

        qDebug() << data.nodes.size();
        qDebug() << "nodes " << data.nodes.size();

        //int cell = 0;
        for (std::vector<WayRef>::const_iterator w = data.ways.begin(); w != data.ways.end(); ++w) {
            const WayRef& way=*w;
            qDebug() << "w " << way->GetId() << "\n";
            /*std::vector<NodeRef>::const_iterator n = way->nodes.begin();
            NodeRef& prevNode = *n;
            NodeRef& node;
            cell = prevNode->cell;
            for(; n != way->nodes.end(); ++n) {
                node = *n;
                if(prevNode->boundary && node->boundary) {
                    boundaryEdgesCount++;
                } else {
                    cellEdgesCount[node->cell]++;
                }
                prevNode = node;
            }*/
        }

        nodesCount = data.nodes.size();
        cellsCount = 0;
        for (std::vector<NodeRef>::const_iterator n = data.nodes.begin(); n != data.nodes.end(); ++n) {
            const NodeRef& node=*n;
            qDebug() << "n " << node->GetId() << "\n";
            /*cellNodesCount[node->cell]++;
            if(node->boundary) {
                cellBoundaryNodesCount[node->cell]++;
                boundaryNodes++;
            }
            if(node->cell > cellsCount) { // ATTENTION!! It's important to take care that there's no missing cells number, ie. 1, 2, 4, 5, 8 <- WRONG!
                cellsCount = node->cell;
            }*/
        }

        /*for(int i=0; i<cellBoundaryNodesCount.size(); i++) {
            cellRouteEdgesCount[i] = cellBoundaryNodesCount[i] * (cellBoundaryNodesCount[i] - 1);
        }

        alpha = 0.5;
        double sumA;
        double sumB;
        for(int i=0; i<cellsCount; i++) {
            sumA += (cellNodesCount[i]/nodesCount)*(2 - (cellNodesCount[i]/nodesCount))*cellEdgesCount[i];
            sumB += pow(1 - (cellNodesCount[i]/nodesCount), 2) * cellRouteEdgesCount[i];
        }

        result = sumA + (alpha * sumB) + boundaryEdgesCount;*/

        return true;
    }
}
