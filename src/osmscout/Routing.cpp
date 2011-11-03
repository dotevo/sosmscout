#include "osmscout/Routing.h"
#include "osmscout/Partitionmodel.h"

namespace osmscout {
    Routing::Routing()
    {
    }
    std::vector< Routing::RouteNode > Routing::CalculateRoute(Id startId, Id endId)
    {
        // initialization
        PartitionModel *pm = new PartitionModel();
        pm->open("C:\\pilocik\\map\\partition.db");

        //Partitioning::PartNode startNode = pm->getNode(startId);
        //Partitioning::PartNode endNode = pm->getNode(endId);
        //unsigned int startCell = startNode.cell;
        //unsigned int endCell = endNode.cell;
        //Partitioning::PartNode currentNode = startNode;

/*
        long startID=10020;
        std::vector<PartWay> ways=pm->getWaysID(id);
        for(int i=0;i<way[0];i++){
          qDebug()<<way[0].nodes[0].id;
        }
*/
    }
}
