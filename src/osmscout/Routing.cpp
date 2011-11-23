#include "osmscout/Routing.h"
#include "osmscout/Partitionmodel.h"

#include "math.h"

#include <QDebug>

namespace osmscout {
    Routing::Routing()
    {
    }

    std::vector< Routing::RouteNode > Routing::CalculateRoute(Id startId, Id endId)
    {
        //
        // initialization
        //
        PartitionModel *partitionModel = new PartitionModel();
        partitionModel->open("C:\\pilocik\\map\\partition.db");

        Partitioning::PartNode startNode = partitionModel->getNode(startId);
        Partitioning::PartNode endNode = partitionModel->getNode(endId);
        unsigned int startCell = startNode.cell;
        unsigned int endCell = endNode.cell;

        RouteNode currentNode;
        currentNode.id = startNode.id;
        currentNode.prevNodeId = -1;
        currentNode.lon = startNode.lon;
        currentNode.lat = startNode.lat;
        // TODO: way of the first node
        currentNode.cell = startNode.cell;
        currentNode.rating = distance(currentNode.lon, currentNode.lat, endNode.lon, endNode.lat);

        std::map< int, RouteNode > availableMoves;
        std::map< int, RouteNode > usedMoves;
        usedMoves[usedMoves.size()] = currentNode;

        std::vector< Partitioning::PartWay > innerWays;
        std::vector< Partitioning::BoundaryEdge > boundaryEdges;
        std::vector< Partitioning::RouteEdge > routeEdges;

        //
        // main body of algorithm
        //
        while(currentNode.id != endId) {
            //
            // adding available moves from current node
            //

            // getting ways with current node
            innerWays.clear();
            boundaryEdges.clear();
            routeEdges.clear();
            if(currentNode.cell == startCell || currentNode.cell == endCell) {
                innerWays = partitionModel->getInnerWaysWithNode(currentNode.id);
            } else {
                routeEdges = partitionModel->getRouteEdgesWithNode(currentNode.id);
            }
            boundaryEdges = partitionModel->getBoundaryEdgesWithNode(currentNode.id);

            // getting neighbours of current node (in every of ways and edges), creating RouteNodes from them and adding to available moves
            for(unsigned int i=0; i<innerWays.size(); ++i) { // start of searching in innerWays
                for(unsigned int j=0; j<innerWays[i].nodes.size(); ++j) {
                    if(innerWays[i].nodes[j] == currentNode.id) {
                        if(j > 0) {
                            Partitioning::PartNode pNode = partitionModel->getNode(innerWays[i].nodes[j-1]);

                            RouteNode newNode;
                            newNode.id = pNode.id;
                            newNode.prevNodeId = currentNode.id;
                            newNode.lon = pNode.lon;
                            newNode.lat = pNode.lat;
                            newNode.wayId = innerWays[i].id;
                            newNode.cell = pNode.cell;
                            newNode.routing = false;
                            // TODO: getting combined priorities
                            newNode.rating = (innerWays[i].priority // priority of way on which one should go
                                                  * distance(pNode.lon, pNode.lat, currentNode.lon, currentNode.lat)) // distance from current node
                                                + distance(pNode.lon, pNode.lat, endNode.lon, endNode.lat); // distance to endNode

                            availableMoves[availableMoves.size()] = newNode;
                        }
                        if(j < innerWays[i].nodes.size()-1) {
                            Partitioning::PartNode pNode = partitionModel->getNode(innerWays[i].nodes[j+1]);

                            RouteNode newNode;
                            newNode.id = pNode.id;
                            newNode.prevNodeId = currentNode.id;
                            newNode.lon = pNode.lon;
                            newNode.lat = pNode.lat;
                            newNode.wayId = innerWays[i].id;
                            newNode.cell = pNode.cell;
                            newNode.routing = false;
                            // TODO: getting combined priorities
                            newNode.rating = (innerWays[i].priority // priority of way on which one should go
                                                  * distance(pNode.lon, pNode.lat, currentNode.lon, currentNode.lat)) // distance from current node
                                                + distance(pNode.lon, pNode.lat, endNode.lon, endNode.lat); // distance to endNode

                            availableMoves[availableMoves.size()] = newNode;
                        }
                    }
                }
            } // end of searching in innerWays

            for(unsigned int i=0; i<boundaryEdges.size(); ++i) { // start of searching in boundaryEdges
                Partitioning::PartNode pNode;
                if(boundaryEdges[i].nodeA == currentNode.id) { // there's only two nodes in boundaryEdge, so only one of them needs to be added to available moves
                    pNode = partitionModel->getNode(boundaryEdges[i].nodeB);
                } else {
                    pNode = partitionModel->getNode(boundaryEdges[i].nodeA);
                }
                RouteNode newNode;
                newNode.id = pNode.id;
                newNode.prevNodeId = currentNode.id;
                newNode.lon = pNode.lon;
                newNode.lat = pNode.lat;
                newNode.wayId = boundaryEdges[i].wayId;
                newNode.cell = pNode.cell;
                newNode.routing = false;
                // TODO: getting combined priorities
                newNode.rating = (innerWays[i].priority // priority of way on which one should go
                                      * distance(pNode.lon, pNode.lat, currentNode.lon, currentNode.lat)) // distance from current node
                                    + distance(pNode.lon, pNode.lat, endNode.lon, endNode.lat); // distance to endNode

                availableMoves[availableMoves.size()] = newNode;
            } // end of searching in boundaryEdges

            for(unsigned int i=0; i<routeEdges.size(); ++i) { // start of searching in routeEdges
                Partitioning::PartNode pNode;
                if(routeEdges[i].nodeA == currentNode.id) { // there's only two nodes in routeEdge, so only one of them needs to be added to available moves
                    pNode = partitionModel->getNode(routeEdges[i].nodeB);
                } else {
                    pNode = partitionModel->getNode(routeEdges[i].nodeA);
                }
                RouteNode newNode;
                newNode.id = pNode.id;
                newNode.prevNodeId = currentNode.id;
                newNode.lon = pNode.lon;
                newNode.lat = pNode.lat;
                newNode.wayId = routeEdges[i].lastWayId;
                newNode.cell = pNode.cell;
                newNode.routing = true;
                newNode.lastRoutingNodeId = routeEdges[i].lastNodeId;
                // TODO: getting combined priorities
                newNode.rating = routeEdges[i].cost // overall cost of getting through route edge
                                    + distance(pNode.lon, pNode.lat, endNode.lon, endNode.lat); // distance to endNode

                availableMoves[availableMoves.size()] = newNode;
            } // end of searching in routeEdges

            //
            // getting best move from available moves
            //
            double bestRating = 99999;
            int bestKey = -1;
            for(std::map< int , RouteNode >::const_iterator it = availableMoves.begin(); it != availableMoves.end(); ++it) {
                if(it->second.rating < bestRating) {
                    bestRating = it->second.rating;
                    bestKey = it->first;
                }
            }

            currentNode.prevNodeId = currentNode.id;
            currentNode.id = availableMoves[bestKey].id;
            currentNode.lon = availableMoves[bestKey].lon;
            currentNode.lat = availableMoves[bestKey].lat;
            currentNode.wayId = availableMoves[bestKey].wayId;
            currentNode.cell = availableMoves[bestKey].cell;
            currentNode.rating = availableMoves[bestKey].rating;
            usedMoves[usedMoves.size()] = currentNode;
            availableMoves.erase(bestKey);
        }

        //
        // currendNode = endNode, so reproduce path...
        //
        QList< Step > route;
        std::list< RouteNode > simplifiedRoute;
        Step currentStep;
        while(currentNode.id != startId) {
            currentStep.id = currentNode.id;
            currentStep.lon = currentNode.lon;
            currentStep.lat = currentNode.lat;
            currentStep.routing = currentNode.routing;
            currentStep.crossing = true;
            route.push_front(currentStep);
            simplifiedRoute.push_front(currentNode);

            Partitioning::PartWay way = partitionModel->getWay(currentNode.wayId);

            for(unsigned int i=0; i<way.nodes.size(); ++i) {
                if(way.nodes[i] == currentNode.id) {
                    unsigned int prevId = currentNode.prevNodeId;
                    while(currentNode.id != prevId) {
                        //osodir;
                    }
                } else if(way.nodes[i] == currentNode.prevNodeId) {
                    //y;
                }
            }

            // find previous node and set it to current
            for(std::map< int, RouteNode >::const_iterator it = usedMoves.begin(); it != usedMoves.end(); ++it) {
                if(it->second.id == currentNode.prevNodeId) {
                    currentNode.id = it->second.id;
                    currentNode.prevNodeId = it->second.prevNodeId;
                    currentNode.wayId = it->second.wayId;
                    break;
                }
            }
        }

        std::vector<RouteNode> route;

        // printing results for test
        for(std::list< RouteNode >::const_iterator it = simplifiedRoute.begin(); it != simplifiedRoute.end(); ++it) {
            qDebug() <<  "Node " << it->id << " from " << it->prevNodeId;

            route.push_back(*(it));
        }
        // TODO: Reproduce detailed path

        return route;
    }

    double Routing::distance(double lonA, double latA, double lonB, double latB)
    {
        return sqrt(pow(lonA - lonB, 2) + pow(latA - latB, 2));
    }
}
