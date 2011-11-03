#include "osmscout/Routing.h"
#include "osmscout/Partitionmodel.h"

namespace osmscout {
    Routing::Routing()
    {
    }
    std::vector< Routing::RouteNode > Routing::CalculateRoute(Id startId, Id endId)
    {
        PartitionModel *pm = new PartitionModel();
        pm->open("C:\\pilocik\\map\\partition.db");

        pm->getNodesInWay(192);
/*
        long startID=10020;
        std::vector<PartWay> ways=pm->getWaysID(id);
        for(int i=0;i<way[0];i++){
          qDebug()<<way[0].nodes[0].id;
        }
*/
    }
}
