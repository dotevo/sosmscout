#ifndef ROUTING_H
#define ROUTING_H

#include <osmscout/Types.h>

namespace osmscout {
    class Routing
    {
    private:
        struct RouteNode {
            Id id;
            Id prevNodeId;
            double lon;
            double lat;
            Id wayId; // by which one came to this node from prevNode
            unsigned int cell;
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
