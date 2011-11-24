#ifndef ROUTING_H
#define ROUTING_H

#include <osmscout/util/Geometry.h>

#include <../../PiLibocik/include/pilibocik/position.h>

namespace osmscout {
    class Routing
    {
    public:
        /**
         * @brief Structure that represets one step of calculated route.
         *
         */
        struct Step {
            Id id;
            double lon;
            double lat;
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
         * @brief Calculates distance from given node to end node.
         *
         * @param node node from which calculate distance
         * @param endNode node to which calculate distance
         *
         * @return distance from node to endNode
         */
        double distance(double lonA, double latA, double lonB, double latB);

    //public:
        /**
         * @brief Default constructor.
         *
         */
        Routing();
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
         * @brief Old method that uses sql database.
         *
         * @param startId id of start node
         * @param endId id of end node
         */
        std::vector< RouteNode > CalculateRouteFromDatabase(Id startId, Id endId);
    };
}

#endif // ROUTING_H
