#ifndef ROUTING_H
#define ROUTING_H

#include <osmscout/util/Geometry.h>

namespace osmscout {
    class Routing
    {
    private:
        /**
         * @brief
         *
         */
        struct RouteNode {
            Id id;
            Id prevNodeId;
            double lon;
            double lat;
            Id wayId; // by which one came to this node from prevNode
            unsigned int cell;
        };

        /**
         * @brief Struct that represents one fragment of the way with start and end node and no other.
         *
         */
        struct StepEdge {
            Id wayId;

        };

    public:
        /**
         * @brief
         *
         */
        Routing();
    };
}

#endif // ROUTING_H
