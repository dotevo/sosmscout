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
            Id id;
            double lon;
            double lat;
            PARTITION_NODE_TYPE type;
            unsigned int cell;
        };
        /**
         * @brief Simple structure that represents boundary edge (which cells it connects).
         *
         */
        struct BoundaryEdge
        {
            unsigned int nodeA;
            unsigned int nodeB;
        };
        /**
         * @brief Simple structure that represents ways in graph (set of edges).
         *
         */
        struct PartWay
        {
            Id id;
            std::vector<unsigned int> nodes;
        };
        /**
         * @brief Simple structure that holds all nodes, ways and calculated values needed during partitioning process.
         *
         */
    private:
        struct Partition
        {
            std::vector< PartNode > nodes;
            std::vector< PartWay > ways;
            std::vector< BoundaryEdge > boundaryEdges;
            unsigned int nodesCount;
            unsigned int cellsCount;
            std::vector< unsigned int > cellsNodesCount;
            std::vector< unsigned int > cellsEdgesCount;
            std::vector< unsigned int > cellsBoundaryNodesCount;
            std::vector< unsigned int > cellsRouteEdgesCount;
            unsigned int boundaryEdgesCount;
            unsigned int boundaryNodesCount;
            std::vector< std::vector< double > > priorities;
        };

        QString mapDir;
        QString style;
        Partition partition;
        Partition bestPartition;
        double alpha;

        /**
         * @brief Initializes all objects and fields.
         *
         */
        void Init();
        /**
         * @brief Calculates all values in partition after some change in it (ie. merging cells).
         *
         */
        void UpdatePartitionData();
        /**
         * @brief Updates priorities table after merging two cells.
         *
         * @param i index of the first cell
         * @param j index of the seccond cell
         */
        void UpdatePriorities(unsigned int i, unsigned int j);
        /**
         * @brief Calculates the value of priority for merging two cells.
         *
         * @param i index of the first cell
         * @param j index of the seccond cell
         *
         * @return value of priority for merging cells i and j
         */
        double CalculatePriority(unsigned int i, unsigned int j);
        /**
         * @brief Merges two cells into one.
         *
         * @param i index of the first cell to merge
         * @param j index of the second cell to merge
         */
        void MergeCells(unsigned int i, unsigned int j);
        /**
         * @brief Calculates quality of current partition.
         *
         * @return value of quality function.
         */
        double CalculateQuality();

    public:
        struct DatabasePartition {
            std::vector< PartNode > nodes;
            std::vector< PartWay > innerWays;
            std::vector< PartWay > boundaryWays;
            //std::vector< PartWay > routingWays;
        };

        /**
         * @brief Initializes all data and stuff.
         *
         * @param mapDir directory for map files
         * @param style directory for style file
         */
        Partitioning(QString mapDir, QString style);
        /**
         * @brief Finds the best (not really, but close enough) partition for graph.
         *
         */
        void FindPartition();
        /**
         * @brief Simple temporary method for testing the algorithm.
         *
         */
        void TestAlgorithm();
        /**
         * @brief Save data by PartitionModel
         *
         */
        void saveToDatabase(QString name, DatabasePartition databasePartition);
    };
}

#endif // OSMSCOUT_PARTITIONING_H

