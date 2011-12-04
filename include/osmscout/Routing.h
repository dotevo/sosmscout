#ifndef OSMSCOUT_ROUTING_H
#define OSMSCOUT_ROUTING_H

#include <osmscout/util/Geometry.h>

#include <QThread>

#include <../../PiLibocik/include/pilibocik/position.h>
#include "../../PiLibocik/include/pilibocik/partition/partitionfile.h"

namespace osmscout {
    class Routing : public QObject
    {
        Q_OBJECT

    signals:
        /**
         * @brief Signal emited when progres is made.
         *
         * @param progress
         */
        void RoutingProgress(int progress);
        void Error(QString errorMessage);

    public:
        /**
         * @brief Structure that represets one step of calculated route.
         *
         */
        struct Step {
            Id id;
            long double lon;
            long double lat;
            Id wayId; // by which one came to this node from prevNode
            bool crossing;
            bool routing;
        };
        /**
         * @brief Structure that represets node with information from which node one came, so it represents step in graph.
         *
         */
        struct RouteNode {
            Id id;
            Id prevNodeId;
            double lon;
            double lat;
            Id wayId; // by which one came to this node from prevNode
            unsigned int cell;
            double rating;
            bool routing;
            bool crossing;  // defines is route node is crossing
            Id lastRoutingNodeId;
        };
        /**
         * @brief Default constructor.
         *
         */
        Routing(PiLibocik::Partition::PartitionFile *pFile);
        /**
         * @brief Default destructor.
         *
         */
        ~Routing();
        /**
         * @brief Calculates route from start node (point) to end node (point).
         *
         * @param startPosition - position of start node
         * @param endPosition - position of end node
         *
         * @return list of steps in route graph - it needs to be transformed into list of steps in map (lack of some nodes).
         */
        QList< Routing::Step > CalculateRoute(PiLibocik::Position startPosition, PiLibocik::Position endPosition);
        /**
         * @brief Transforms list of positions into list of steps.
         *
         * @param positions - list of positions to transform
         */
        QList< Routing::Step > positionsToSteps(QList< PiLibocik::Position > positions);
    private:
        PiLibocik::Partition::PartitionFile *partitionFile;
        /**
         * @brief Calculates distance between two positions.
         *
         * @param positionA - first position
         * @param positionB - second position
         *
         * @return distance from node to endNode
         */
        double distance(PiLibocik::Position positionA, PiLibocik::Position positionB);
        /**
         * @brief Calculates distance between two positions.
         *
         * @param lonA - lon of first position
         * @param latA - lat of first position
         * @param lonB - lon of second position
         * @param latB - lat of second position
         */
        double distance(double lonA, double latA, double lonB, double latB);
    };
}

#endif // OSMSCOUT_ROUTING_H
