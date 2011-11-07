#ifndef ROUTING_H
#define ROUTING_H

#include <osmscout/util/Geometry.h>

namespace osmscout {
    class Routing
    {
    private:
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

    public:
        /**
         * @brief Default constructor.
         *
         */
        Routing();
        /**
         * @brief Calculates route from start node (point) to end node (point).
         *
         * @param startId id of start node
         * @param endId id of end node
         *
         * @return list of steps in route graph - it needs to be transformed into list of steps in map (lack of some nodes).
         */
        std::vector< RouteNode > CalculateRoute(Id startId, Id endId);
    };
}

#endif // ROUTING_H
