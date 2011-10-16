#ifndef OSMSCOUT_PARTITIONING_H
#define OSMSCOUT_PARTITIONING_H

#include <QString>

#include <osmscout/util/Geometry.h>
#include <osmscout/Node.h>
#include <osmscout/Way.h>



namespace osmscout {
    /**
     * @brief
     *
     */
    class Partitioning
    {
    public:
        /**
         * @brief
         *
         * @param mapDir directory for map files
         * @param style directory for style file
         */
        Partitioning(QString mapDir, QString style);
        /**
         * @brief Calculates quality of current partition.
         *
         * @return value of quality function.
         */
        double PartitionQuality();
        /**
         * @brief Finds the best (not really, but close enough) partition for graph.
         *
         */
        void findPartition();
    private:
        /**
         * @brief Type of node in graph. Can be on the border of the cell (BOUNDARY), or inside the cell (INTERNAL)
         *
         */
        enum PARTITION_NODE_TYPE
        {
            BOUNDARY = 0,
            INTERNAL = 1
        };
        /**
         * @brief Simple structure that represents node in graph (has field that shows to which cell it belongs).
         *
         */
        struct PartNode
        {
            int lon;
            int lat;
            PARTITION_NODE_TYPE type;
            unsigned int cell;
        };
        /**
         * @brief Simple structure that represents boundary edge (which cells it connects).
         *
         */
        struct BoundaryEdge
        {
            unsigned int cellI;
            unsigned int cellJ;
        };
        /**
         * @brief Simple structure that represents ways in graph (set of edges).
         *
         */
        struct PartWay
        {
            std::vector<unsigned int> nodes;
        };
        /**
         * @brief Simple structure that holds all nodes, ways and calculated values needed during partitioning process.
         *
         */
        struct Partition
        {
            std::vector<PartNode> nodes;
            std::vector<PartWay> ways;
            std::vector<BoundaryEdge> boundaryEdges;
            unsigned int nodesCount;
            unsigned int cellsCount;
            std::vector<unsigned int> cellsNodesCount;
            std::vector<unsigned int> cellsEdgesCount;
            std::vector<unsigned int> cellsBoundaryNodesCount;
            std::vector<unsigned int> cellsRouteEdgesCount;
            unsigned int boundaryEdgesCount;
            unsigned int boundaryNodesCount;
        };

        QString mapDir;
        QString style;
        Partition partition;

        /**
         * @brief Initializes all objects and fields.
         *
         */
        void init();
        /**
         * @brief Calculates all values in partition after some change in it (ie. merging cells).
         *
         */
        void updatePartition();
    };
}

#endif // OSMSCOUT_PARTITIONING_H
