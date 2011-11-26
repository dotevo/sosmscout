#include "osmscout/Routing.h"
#include "osmscout/Partitionmodel.h"
#include "osmscout/Database.h"
#include "../../../Pilocik/settings.h"

#include "math.h"

#include <QDebug>
#include <QString>

#include <../../PiLibocik/include/pilibocik/partition/partitionfile.h>
#include <../../PiLibocik/include/pilibocik/partition/boundaryedge.h>
#include <../../PiLibocik/include/pilibocik/partition/edge.h>
#include <../../PiLibocik/include/pilibocik/partition/routenode.h>
#include <../../PiLibocik/include/pilibocik/partition/node.h>
#include <../../PiLibocik/include/pilibocik/partition/way.h>

namespace osmscout {
    Routing::Routing()
    {
    }

    QList< Routing::Step > Routing::CalculateRoute(PiLibocik::Position startPosition, PiLibocik::Position endPosition)
    {
        //
        // initialization
        //
        PiLibocik::Partition::PartitionFile partitionFile("test","car",QIODevice::ReadOnly,1);

        PiLibocik::Partition::Node startNode = partitionFile.getNearestNode(startPosition); // TODO: implementation of this method
        PiLibocik::Partition::Node endNode = partitionFile.getNearestNode(endPosition);
        unsigned int startCell = startNode.getCell();
        unsigned int endCell = endNode.getCell();

        QList< Routing::Step > finalRoute;

        int prevWayId = -1;
        double rating = distance(startNode.getLon(), startNode.getLat(), endNode.getLon(), endNode.getLat());
        PiLibocik::Partition::RouteNode * currentRouteNode = new PiLibocik::Partition::RouteNode(startNode, prevWayId, rating);

        QMap< QString, PiLibocik::Partition::RouteNode * > availableMoves;
        QMap< QString, PiLibocik::Partition::RouteNode * > usedMoves;
        QString key = QString::number(currentRouteNode->getPrevNode().getId()) + "_" + QString::number(currentRouteNode->getId());
        usedMoves.insert(key, currentRouteNode);

        QVector< PiLibocik::Partition::Way > innerWays;
        QVector< PiLibocik::Partition::BoundaryEdge > boundaryEdges;
        QVector< PiLibocik::Partition::Edge > routeEdges;

        //
        // main body of algorithm
        //
        while(currentRouteNode->getId() != endNode.getId()) {
            //
            // adding available moves from current node
            //

            // getting ways with current node
            innerWays.clear();
            boundaryEdges.clear();
            routeEdges.clear();
            if(currentRouteNode->getCell() == startCell || currentRouteNode->getCell() == endCell) {
                innerWays = currentRouteNode->getWaysObj();
            } else {
                routeEdges = currentRouteNode->getRoutingEdges();
            }
            boundaryEdges = currentRouteNode->getBoundaryEdges();

            // getting neighbours of current node (in every of ways and edges), creating RouteNodes from them and adding to available moves
            for(int i=0; i<innerWays.size(); ++i) { // start of searching in innerWays
                PiLibocik::Partition::Way innerWay = innerWays.at(i);
                QVector<PiLibocik::Partition::Node> nodesInInnerWay = innerWay.getNodesObj();
                int innerWayOneway = innerWay.getOneway();

                for(int j=0; j<nodesInInnerWay.size(); ++j) {
                    PiLibocik::Partition::Node nodeInWay = nodesInInnerWay.at(j);
                    if(currentRouteNode->getId()
                            == nodeInWay.getId()) {
                        if(j > 0 && innerWayOneway != 1) {
                            // TODO: taking oneway into account
                            PiLibocik::Partition::Node neighbourNode = nodesInInnerWay.at(j-1);

                            rating = (innerWay.getPrioritet() // priority of way on which one should go
                                            * distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat())) // distance from current node
                                        + distance(neighbourNode.getLon(), neighbourNode.getLat(), endNode.getLon(), endNode.getLat()); // distance to endNod

                            PiLibocik::Partition::RouteNode * newNode = new PiLibocik::Partition::RouteNode(neighbourNode, innerWay.getId(), rating, currentRouteNode);

                            key = QString::number(newNode->getPrevNode().getId()) + "_" + QString::number(newNode->getId());
                            availableMoves.insert(key, newNode);
                        }
                        if(j < nodesInInnerWay.size()-1 && innerWayOneway != -1) {
                            PiLibocik::Partition::Node neighbourNode = nodesInInnerWay.at(j+1);

                            rating = (innerWay.getPrioritet() // priority of way on which one should go
                                            * distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat())) // distance from current node
                                        + distance(neighbourNode.getLon(), neighbourNode.getLat(), endNode.getLon(), endNode.getLat()); // distance to endNod

                            PiLibocik::Partition::RouteNode * newNode = new PiLibocik::Partition::RouteNode(neighbourNode, innerWay.getId(), rating, currentRouteNode);

                            key = QString::number(newNode->getPrevNode().getId()) + "_" + QString::number(newNode->getId());
                            availableMoves.insert(key, newNode);
                        }
                    }
                }
            } // end of searching in innerWays

            for(int i=0; i<boundaryEdges.size(); ++i) { // start of searching in boundaryEdges
                PiLibocik::Partition::BoundaryEdge boundaryEdge = boundaryEdges.at(i);
                PiLibocik::Partition::Way boundaryWay = boundaryEdge.getWayObj();
                PiLibocik::Partition::Node neighbourNode = boundaryEdge.getPairObj();

                // checking if oneway
                bool valid = true;
                if(boundaryWay.getOneway() != 0) {
                    QVector<PiLibocik::Partition::Node> nodesInBoundaryWay = boundaryWay.getNodesObj();

                    for(int j=0; j<nodesInBoundaryWay.size(); ++j) {
                        PiLibocik::Partition::Node nodeInWay = nodesInBoundaryWay.at(j);

                        if(nodeInWay.getId() == currentRouteNode->getId()) {
                            if(boundaryWay.getOneway() == -1) {
                                valid = false;
                            }
                            break;
                        }
                        if(nodeInWay.getId() == neighbourNode.getId()) {
                            if(boundaryWay.getOneway() == 1) {
                                valid = false;
                            }
                            break;
                        }
                    }
                }

                rating = (boundaryEdge.getPrioritet() // priority of way on which one should go
                                * distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat())) // distance from current node
                            + distance(neighbourNode.getLon(), neighbourNode.getLat(), endNode.getLon(), endNode.getLat()); // distance to endNod
                PiLibocik::Partition::RouteNode * newNode = new PiLibocik::Partition::RouteNode(neighbourNode, boundaryWay.getId(), rating, currentRouteNode);

                key = QString::number(newNode->getPrevNode().getId()) + "_" + QString::number(newNode->getId());
                availableMoves.insert(key, newNode);
            } // end of searching in boundaryEdges

            for(int i=0; i<routeEdges.size(); ++i) { // start of searching in routeEdges
                PiLibocik::Partition::Edge routeEdge = routeEdges.at(i);

                PiLibocik::Partition::Node neighbourNode = routeEdge.getPairObj();

                double cost = 1.2 * distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat()); // estimated cost of going through route edge
                rating = cost + distance(neighbourNode.getLon(), neighbourNode.getLat(), endNode.getLon(), endNode.getLat()); // distance to endNod

                PiLibocik::Partition::RouteNode * newNode = new PiLibocik::Partition::RouteNode(neighbourNode, -1, rating, currentRouteNode, true);

                key = QString::number(newNode->getPrevNode().getId()) + "_" + QString::number(newNode->getId());
                availableMoves.insert(key, newNode);
            } // end of searching in routeEdges

            //
            // getting best move from available moves
            //
            double bestRating = 99999.99;
            QString bestKey("");
            QMapIterator< QString, PiLibocik::Partition::RouteNode * > it(availableMoves);
            while (it.hasNext()) {
                it.next();
                if(it.value()->getRating() < bestRating) {
                    bestRating = it.value()->getRating();
                    bestKey = it.key();
                }
            }

            currentRouteNode = availableMoves.take(bestKey);
            usedMoves.insert(bestKey, currentRouteNode);
            availableMoves.remove(bestKey);
        }

        //
        // currendNode = endNode, so reproduce path...
        //
        QList< Step > partialRoute;
        Step currentStep;
        unsigned int previousId = -1;
        unsigned int currentId = -1;
        Id wayId = -1;
        osmscout::WayRef way;

        // connectiong to database
        osmscout::DatabaseParameter databaseParameter;
        osmscout::Database database(databaseParameter);
        QString map("D:\\pilocik\\map\\");
        //QString map = Settings::getInstance()->getMapPath();
        if (map.size() != 0 && !database.Open((const char*)map.toAscii())) {
            std::cerr << "Fatal error: Cannot open database" << std::endl;
        }

        // creating route
        while(currentRouteNode->getId() != startNode.getId()) {
            // adding current node (crossing)
            currentStep.id = currentRouteNode->getId();
            currentStep.lon = currentRouteNode->getLon();
            currentStep.lat = currentRouteNode->getLat();
            wayId = currentRouteNode->getPrevWay();
            currentStep.wayId = wayId;
            currentStep.routing = currentRouteNode->getRouting();
            currentStep.crossing = true;

            finalRoute.push_front(currentStep);

            if(!currentStep.routing) {
                // creating route from previous node to current
                partialRoute.clear();
                previousId = currentRouteNode->getPrevNode().getId();
                currentId = currentRouteNode->getId();

                // getting way
                database.GetWay(wayId, way);
                for(unsigned int i=0; i<way->nodes.size(); ++i) {
                    if(way->nodes.at(i).id == previousId) {
                        // creating partial route (starting with next)
                        for(int j=i+1; way->nodes.at(j).id == currentId; ++j) {
                            currentStep.id = way->nodes.at(j).id;
                            currentStep.lon = way->nodes.at(j).lon;
                            currentStep.lat = way->nodes.at(j).lat;
                            currentStep.wayId = wayId;
                            currentStep.routing = false;
                            currentStep.crossing = false;

                            partialRoute.push_back(currentStep);
                        }
                        break;
                    }
                    if(way->nodes.at(i).id == currentId) {
                        // creating partial route (starting with next)
                        for(int j=i+1; way->nodes.at(j).id == previousId; ++j) {
                            currentStep.id = way->nodes.at(j).id;
                            currentStep.lon = way->nodes.at(j).lon;
                            currentStep.lat = way->nodes.at(j).lat;
                            currentStep.wayId = wayId;
                            currentStep.routing = false;
                            currentStep.crossing = false;

                            partialRoute.push_front(currentStep);
                        }
                        break;
                    }
                }

                // adding created route to final route
                partialRoute.append(finalRoute);
                finalRoute = partialRoute;
            }

            // find previous node and set it to current
            currentRouteNode = &(currentRouteNode->getPrevNode());
        }

        // adding first step
        currentStep.id = currentRouteNode->getId();
        currentStep.lon = currentRouteNode->getLon();
        currentStep.lat = currentRouteNode->getLat();
        currentStep.routing = currentRouteNode->getRouting();
        currentStep.crossing = true;

        finalRoute.push_front(currentStep);

        // deleting all route nodes
        QMapIterator< QString, PiLibocik::Partition::RouteNode * > itA(availableMoves);
        while (itA.hasNext()) {
            itA.next();
            delete itA.value();
        }
        QMapIterator< QString, PiLibocik::Partition::RouteNode * > itB(usedMoves);
        while (itB.hasNext()) {
            itB.next();
            delete itB.value();
        }

        return finalRoute;
    }

    std::vector< Routing::RouteNode > Routing::CalculateRouteFromDatabase(Id startId, Id endId)
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
        std::list< RouteNode > simplifiedRoute;
        Step currentStep;
        while(currentNode.id != startId) {
            currentStep.id = currentNode.id;
            currentStep.lon = currentNode.lon;
            currentStep.lat = currentNode.lat;
            currentStep.routing = currentNode.routing;
            currentStep.crossing = true;
            //route.push_front(currentStep);
            simplifiedRoute.push_front(currentNode);

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

<<<<<<< HEAD
        std::vector<RouteNode> route;

        // TODO: Reproduce detailed path
        /*
=======
        std::vector<RouteNode> route2;
/*
>>>>>>> 96135bedc6f510d74ccc97fd74076e4f11304770
        // printing results for test
        for(std::list< RouteNode >::const_iterator it = simplifiedRoute.begin(); it != simplifiedRoute.end(); ++it) {
            qDebug() <<  "Node " << it->id << " from " << it->prevNodeId;

            route.push_back(*(it));
        }
<<<<<<< HEAD
        // TODO: Reproduce detailed path*/

        return route;
=======
        // TODO: Reproduce detailed path
*/
        return route2;
>>>>>>> 96135bedc6f510d74ccc97fd74076e4f11304770
    }

    double Routing::distance(double lonA, double latA, double lonB, double latB)
    {
        return sqrt(pow(lonA - lonB, 2) + pow(latA - latB, 2));
    }
}
