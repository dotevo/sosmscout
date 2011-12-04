#include "osmscout/Routing.h"
#include "osmscout/Database.h"
#include "../../../Pilocik/settings.h"

#include "math.h"

#include <QDebug>
#include <QString>

#include <../../PiLibocik/include/pilibocik/partition/boundaryedge.h>
#include <../../PiLibocik/include/pilibocik/partition/edge.h>
#include <../../PiLibocik/include/pilibocik/partition/routenode.h>
#include <../../PiLibocik/include/pilibocik/partition/node.h>
#include <../../PiLibocik/include/pilibocik/partition/way.h>

namespace osmscout {

    void Routing::Step::setCrossing(bool cross)
    {
        crossing = cross;
    }

    Routing::Routing(PiLibocik::Partition::PartitionFile *pFile)
    {
        partitionFile = pFile;
    }

    Routing::~Routing()
    {
        delete partitionFile;
    }

    QList< Routing::Step > Routing::CalculateRoute(PiLibocik::Position startPosition, PiLibocik::Position endPosition)
    {
        //
        // initialization
        //
        QList< Routing::Step > finalRoute;

        PiLibocik::Partition::Node startNode = partitionFile->getNearestNode(startPosition);
        PiLibocik::Partition::Node endNode = partitionFile->getNearestNode(endPosition);

        qDebug() << "Calculating from " << startNode.getId() << " to " << endNode.getId();

        if(startNode.getId() == endNode.getId()) {
            //emit Error(tr("Points are too close!"));
            Routing::Step oneAndOnlyStep;

            oneAndOnlyStep.id = startNode.getId();
            oneAndOnlyStep.lon = startNode.getLon();
            oneAndOnlyStep.lat = startNode.getLat();
            oneAndOnlyStep.routing = false;
            oneAndOnlyStep.crossing = true;

            finalRoute.push_front(oneAndOnlyStep);

            return finalRoute;
        }

        unsigned int startCell = startNode.getCell();
        unsigned int endCell = endNode.getCell();

        int prevWayId = -1;
        double rating = distance(startNode.getLon(), startNode.getLat(), endNode.getLon(), endNode.getLat());
        PiLibocik::Partition::RouteNode * currentRouteNode = new PiLibocik::Partition::RouteNode(startNode, prevWayId, rating);

        QMap< QString, PiLibocik::Partition::RouteNode * > availableMoves;
        QMap< QString, PiLibocik::Partition::RouteNode * > usedMoves;
        QString key = "-1_" + QString::number(currentRouteNode->getId());
        usedMoves.insert(key, currentRouteNode);

        QVector< PiLibocik::Partition::Way > innerWays;
        QVector< PiLibocik::Partition::BoundaryEdge > boundaryEdges;
        QVector< PiLibocik::Partition::Edge > routeEdges;

        //
        // main body of algorithm
        //
        double totalDistance = distance(startPosition, endPosition);
        int bestProgress = 0;
        while(currentRouteNode->getId() != endNode.getId()) {
            int currentProgress = (1.0 - (distance(currentRouteNode->getLon(), currentRouteNode->getLat(), endNode.getLon(), endNode.getLat())/totalDistance)) * 95 + 1;
            if(currentProgress > bestProgress) {
                bestProgress = currentProgress;
                emit RoutingProgress(bestProgress);
            }
            qDebug() << QString::number(currentProgress);

            //
            // adding available moves from current node
            //
            // getting ways with current node
            innerWays.clear();
            boundaryEdges.clear();
            routeEdges.clear();
            bool inStartOrEndCell = currentRouteNode->getCell() == startCell || currentRouteNode->getCell() == endCell;
            // old way
            /*if(inStartOrEndCell) {
                innerWays = currentRouteNode->getWaysObj();
            } else {
                routeEdges = currentRouteNode->getRoutingEdges();
            }
            boundaryEdges = currentRouteNode->getBoundaryEdges();*/
            // new way
            if(!inStartOrEndCell && startCell != endCell) {
                routeEdges = currentRouteNode->getRoutingEdges();
            }
            innerWays = currentRouteNode->getWaysObj();

            // getting neighbours of current node (in every of ways and edges), creating RouteNodes from them and adding to available moves
            for(int i=0; i<innerWays.size(); ++i) { // start of searching in innerWays
                PiLibocik::Partition::Way innerWay = innerWays.at(i);
                if(innerWay.getPrioritet() != 0) {
                    QVector<PiLibocik::Partition::Node> nodesInInnerWay = innerWay.getNodesObj();
                    int innerWayOneway = innerWay.getOneway();

                    for(int j=0; j<nodesInInnerWay.size(); ++j) {
                        PiLibocik::Partition::Node nodeInWay = nodesInInnerWay.at(j);
                        if(currentRouteNode->getId()
                                == nodeInWay.getId()) {
                            if(j > 0 && innerWayOneway != 1) {
                                // TODO: taking oneway into account
                                PiLibocik::Partition::Node neighbourNode = nodesInInnerWay.at(j-1);

                                if(inStartOrEndCell || neighbourNode.getCell() != currentRouteNode->getCell()) {
                                    double distanceFromCurrent = distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat());
                                    double distanceToEnd = distance(neighbourNode.getLon(), neighbourNode.getLat(), endNode.getLon(), endNode.getLat());
                                    if(distanceToEnd < 3*totalDistance) {
                                        rating = ((2-innerWay.getPrioritet()) // priority of way on which one should go
                                                        * (distanceFromCurrent // distance from current node
                                                    + distanceToEnd)) // distance to endNod
                                                    + currentRouteNode->getDistanceFromStart();  // distance from start node (real roads)

                                        PiLibocik::Partition::RouteNode * newNode = new PiLibocik::Partition::RouteNode(neighbourNode, innerWay.getId(), rating, currentRouteNode->getDistanceFromStart() + distanceFromCurrent, currentRouteNode);

                                        key = QString::number(newNode->getPrevNode()->getId()) + "_" + QString::number(newNode->getId());
                                        QMap< QString, PiLibocik::Partition::RouteNode * >::iterator it = usedMoves.find(key);
                                        if(it == usedMoves.end()) {
                                            availableMoves.insert(key, newNode);
                                        }
                                    }
                                }
                            }
                            if(j < nodesInInnerWay.size()-1 && innerWayOneway != -1) {
                                PiLibocik::Partition::Node neighbourNode = nodesInInnerWay.at(j+1);

                                if(inStartOrEndCell || neighbourNode.getCell() != currentRouteNode->getCell()) {
                                    double distanceFromCurrent = distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat());
                                    double distanceToEnd = distance(neighbourNode.getLon(), neighbourNode.getLat(), endNode.getLon(), endNode.getLat());
                                    if(distanceToEnd < 3*totalDistance) {
                                        rating = ((2-innerWay.getPrioritet()) // priority of way on which one should go
                                                        * (distanceFromCurrent // distance from current node
                                                    + distanceToEnd)) // distance to endNod
                                                    + currentRouteNode->getDistanceFromStart();  // distance from start node (real roads)

                                        PiLibocik::Partition::RouteNode * newNode = new PiLibocik::Partition::RouteNode(neighbourNode, innerWay.getId(), rating, currentRouteNode->getDistanceFromStart() + distanceFromCurrent, currentRouteNode);

                                        key = QString::number(newNode->getPrevNode()->getId()) + "_" + QString::number(newNode->getId());
                                        QMap< QString, PiLibocik::Partition::RouteNode * >::iterator it = usedMoves.find(key);
                                        if(it == usedMoves.end()) {
                                            availableMoves.insert(key, newNode);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } // end of searching in innerWays

            // old way
            /*for(int i=0; i<boundaryEdges.size(); ++i) { // start of searching in boundaryEdges
                PiLibocik::Partition::BoundaryEdge boundaryEdge = boundaryEdges.at(i);
                PiLibocik::Partition::Way boundaryWay = boundaryEdge.getWayObj();
                if(boundaryWay.getPrioritet() != 0) {
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

                    double distanceFromCurrent = distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat());
                    double distanceToEnd = distance(neighbourNode.getLon(), neighbourNode.getLat(), endNode.getLon(), endNode.getLat());
                    rating = ((2-boundaryEdge.getPrioritet()) // priority of way on which one should go
                                  * (distanceFromCurrent // distance from current node
                              + distanceToEnd)) // distance to endNod
                              + currentRouteNode->getDistanceFromStart();  // distance from start node (real roads)
                    PiLibocik::Partition::RouteNode * newNode = new PiLibocik::Partition::RouteNode(neighbourNode, boundaryWay.getId(), rating, currentRouteNode->getDistanceFromStart() + distanceFromCurrent, currentRouteNode);

                    key = QString::number(newNode->getPrevNode().getId()) + "_" + QString::number(newNode->getId());
                    QMap< QString, PiLibocik::Partition::RouteNode * >::iterator it = usedMoves.find(key);
                    if(it == usedMoves.end()) {
                        availableMoves.insert(key, newNode);
                    }
                }
            }*/ // end of searching in boundaryEdges

            for(int i=0; i<routeEdges.size(); ++i) { // start of searching in routeEdges
                PiLibocik::Partition::Edge routeEdge = routeEdges.at(i);

                PiLibocik::Partition::Node neighbourNode = routeEdge.getPairObj();

                double distanceFromCurrent = distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat());
                double distanceToEnd = distance(neighbourNode.getLon(), neighbourNode.getLat(), endNode.getLon(), endNode.getLat());
                if(distanceToEnd < 3*totalDistance) {
                    double cost = 1.5 * distance(neighbourNode.getLon(), neighbourNode.getLat(), currentRouteNode->getLon(), currentRouteNode->getLat()); // estimated cost of going through route edge
                    rating = cost
                            + distanceToEnd; // distance to endNod

                    PiLibocik::Partition::RouteNode * newNode = new PiLibocik::Partition::RouteNode(neighbourNode, -1, rating, currentRouteNode->getDistanceFromStart() + distanceFromCurrent, currentRouteNode, true);

                    key = QString::number(newNode->getPrevNode()->getId()) + "_" + QString::number(newNode->getId());
                    QMap< QString, PiLibocik::Partition::RouteNode * >::iterator it = usedMoves.find(key);
                    if(it == usedMoves.end()) {
                        availableMoves.insert(key, newNode);
                    }
                }
            } // end of searching in routeEdges

            //
            // getting best move from available moves
            //
            if(availableMoves.size() == 0 || currentProgress < -150 || usedMoves.size() > 1500) {
                emit Error(tr("Cannot find route! \nRouting to nearest..."));

                double bestDistance = 99999.99;
                QString bestKey("");
                QMapIterator< QString, PiLibocik::Partition::RouteNode * > it(usedMoves);
                while (it.hasNext()) {
                    it.next();
                    double currentDistance = distance(it.value()->getLon(), it.value()->getLat(), endNode.getLon(), endNode.getLat());
                    if(currentDistance < bestDistance) {
                        bestDistance = currentDistance;
                        bestKey = it.key();
                    }
                }

                currentRouteNode = usedMoves.take(bestKey);

                break;
            }
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

            //qDebug() << QString::number(distance(currentRouteNode->getLon(), currentRouteNode->getLat(), endNode.getLon(), endNode.getLat()));
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

#ifndef PiLibocik_WRITE_MODE
        QString map = Settings::getInstance()->getMapPath();
#else
        QString map = "";
#endif
        if (map.size() != 0 && !database.Open((const char*)map.toAscii())) {
            emit Error(tr("Cannot open database!"));
            qDebug() << "Fatal error: Cannot open database";
            return finalRoute;
        }

        // creating route
        while(currentRouteNode->getId() != startNode.getId()) {
            // adding current node (crossing)
            //qDebug() << "Creating route before " << currentRouteNode->getId();
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
                previousId = currentRouteNode->getPrevNode()->getId();
                currentId = currentRouteNode->getId();

                // getting way

                if (!database.GetWay(wayId, way)) {
                    emit Error(tr("Cannot load way") + " \n(id=" + QString::number(wayId) + ")");
                    qDebug() << "Cannot load way \n(id=" << wayId <<")";
                    finalRoute.clear();
                    return finalRoute;
                }
                for(unsigned int i=0; i<way->nodes.size(); ++i) {
                    //qDebug() << "Looking for node CURR or PREV";
                    if(way->nodes.at(i).id == previousId) {
                        //qDebug() << "Creating subroute on way " << way->GetId() << " start PREV = " << previousId;
                        // creating partial route (starting with next)
                        for(unsigned int j=i+1; j<way->nodes.size() && way->nodes.at(j).id != currentId; ++j) {
                            //qDebug() << "Adding step";
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
                        //qDebug() << "Creating subroute on way " << way->GetId() << " start CURR = " << currentId;
                        for(unsigned int j=i+1; j<way->nodes.size() && way->nodes.at(j).id != previousId; ++j) {
                            //qDebug() << "Adding step";
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
                //qDebug() << "Adding subroute to final route";
                partialRoute.append(finalRoute);
                finalRoute = partialRoute;
            } else {
                //qDebug() << "Current step is routing";
            }

            // find previous node and set it to current
            currentRouteNode = currentRouteNode->getPrevNode();
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

        //delete currentRouteNode;

        emit RoutingProgress(100);

        return finalRoute;
    }

    QList< Routing::Step > Routing::positionsToSteps(QList< PiLibocik::Position > positions)
    {
        QList< Routing::Step > stepsList;
        QListIterator< PiLibocik::Position > positionsIterator(positions);
        PiLibocik::Partition::Node fileNode;

        positionsIterator.next();
        while(positionsIterator.hasNext()) {
            PiLibocik::Position position = positionsIterator.next();

            fileNode = partitionFile->getNearestNode(position);

            osmscout::Routing::Step step;
            step.crossing = true;
            step.id = fileNode.getId();
            step.lon = fileNode.getLon();
            step.lat = fileNode.getLat();
            step.routing = true;
            step.wayId = -1;

            stepsList.push_back(step);
        }

        return stepsList;
    }

    double Routing::distance(PiLibocik::Position positionA, PiLibocik::Position positionB)
    {
        return sqrt(pow(positionA.getLon() - positionB.getLon(), 2) + pow(positionA.getLat() - positionB.getLat(), 2));
    }

    double Routing::distance(double lonA, double latA, double lonB, double latB)
    {
        return sqrt(pow(lonA - lonB, 2) + pow(latA - latB, 2));
    }
}
