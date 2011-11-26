#ifndef OSMSCOUT_PARTITIONING_H
#define OSMSCOUT_PARTITIONING_H

#include <QString>
#include <QThread>

#include <osmscout/util/Geometry.h>
#include <osmscout/Node.h>
#include <osmscout/Way.h>



namespace osmscout {
    /**
     * @brief
     *
     */
    class Partitioning : public QThread
    {
        Q_OBJECT

    signals:
        /**
         * @brief Signal emited when status changed in data initialiyation.
         *
         * @param status - emited status
         */
        void initDataStatusChanged(QString status);
        /**
         * @brief Signal emited when partial progress of data initialization has been made.
         *
         * @param progress - number from 0 to 100 indicating progress
         */
        void initDataPartProgress(int progress);
        /**
         * @brief Signal emited when overall progress of data initialization has been made.
         *
         * @param progress - number from 0 to 100 indicating progress
         */
        void initDataOverallProgress(int progress);
        /**
         * @brief Signal emited when data initialiyation ends.
         *
         */
        void initDataFinished();
        /**
         * @brief Signal emited when status changed in priorities calculation.
         *
         * @param status - emited status
         */
        void prioCalcStatusChanged(QString status);
        /**
         * @brief Signal emited when progress of priorities calculation has been made.
         *
         * @param progress - number from 0 to 100 indicating progress
         */
        void prioCalcProgress(int progress);
        /**
         * @brief Signal emited when priorities calculation ends.
         *
         */
        void prioCalcFinished();
        /**
         * @brief Signal emited when status changed in partitions calculation.
         *
         * @param status - emited status
         */
        void partCalcStatusChanged(QString status);
        /**
         * @brief Signal emited when progress of partition calculation has been made.
         *
         * @param progress - number from 0 to 100 indicating progress
         */
        void partCalcProgress(int progress);
        /**
         * @brief Signal emited when partitions calculation ends.
         *
         */
        void partCalcFinished();

    public:
        /**
         * @brief Default constructor.
         *
         */
        Partitioning();

        void run();

        /**
         * @brief Current stage of partitioning
         *
         */
        enum PARTITIONING_STAGE
        {
            DATA_INITIALIZATION = 0,
            PRIORITIES_CALCULATION = 1,
            PARTITIONS_CALCULATION = 2
        };
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
            PARTITION_NODE_TYPE type; // ignore in writing to database
            unsigned int cell;
        };
        /**
         * @brief Simple structure that represents boundary edge.
         *
         */
        struct BoundaryEdge
        {
            Id wayId;
            unsigned int nodeA;
            unsigned int nodeB;
            double priority;
        };
        /**
         * @brief Simple structure that represents route edge.
         *
         */
        struct RouteEdge
        {
            Id lastWayId;
            Id lastNodeId;
            unsigned int nodeA;
            unsigned int nodeB;
            double cost;
        };
        /**
         * @brief Simple structure that represents ways in graph (set of edges).
         *
         */
        struct PartWay
        {
            Id id;
            std::vector<unsigned int> nodes;
            double priority;
            QString type;
            int oneway;
        };
        /**
         * @brief Simple structure that helps during saving to the file.
         *
         */
        struct DatabasePartition {
            std::vector< PartNode > nodes;
            std::vector< PartWay > innerWays;
            std::vector< BoundaryEdge > boundaryEdges;
            std::vector< RouteEdge > routingEdges;
        };
    private:
        /**
         * @brief Simple structure that holds all nodes, ways and calculated values needed during partitioning process.
         *
         */
        struct Partition
        {
            std::vector< PartNode > nodes;
            std::vector< PartWay > ways;
            std::vector< BoundaryEdge > boundaryEdges;
            unsigned int nodesCount;
            unsigned int waysCount;
            unsigned int cellsCount;
            std::vector< unsigned int > cellsNodesCount;
            std::vector< unsigned int > cellsEdgesCount;
            std::vector< unsigned int > cellsBoundaryNodesCount;
            std::vector< unsigned int > cellsRouteEdgesCount;
            unsigned int boundaryEdgesCount;
            unsigned int boundaryNodesCount;
            //std::vector< std::vector< double > * > priorities;
            std::vector< std::vector< unsigned int > * > cellsBoundaryNodes;
            std::vector< std::map< unsigned int, double > * > priorities;
            std::vector< std::vector< unsigned int > * > cellsConnections;
        };

        PARTITIONING_STAGE stage;
        QString databasePath;
        QString simpleDataPath;
        QString prioritiesDataPath;
        QString finalDataPath;
        Partition partition;
        Partition bestPartition;
        double alpha;
        double beta;

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
        /**
         * @brief Converts bestPartition to DatabasePartition object for saving.
         *
         */
        DatabasePartition getDatabasePartition();

    public:
        /**
         * @brief Initializes all objects and fields.
         *
         */
        void InitData();
        /**
         * @brief Saves partition to simple txt file.
         *
         * @param path the path to file for data should to be saved in
         */
        void SaveData();
        /**
         * @brief Loads partition from simple txt file.
         *
         * @param path the path to file for data should to be loaded from
         */
        void LoadData();
        /**
         * @brief Finds the best (not really, but close enough) partition for graph.
         *
         */
        DatabasePartition FindPartition();
        /**
         * @brief Simple temporary method for testing the algorithm.
         *
         */
        void TestAlgorithm();
#ifdef PiLibocik_WRITE_MODE
        /**
         * @brief Save data by PartitionModel.
         *
         * @param databasePartition - object with partition to save
         */
        void saveToDatabase(DatabasePartition& databasePartition);
#endif
        /**
         * @brief Calculates all priorities.
         *
         */
        void CalculatePriorities();
        /**
         * @brief Saves all priorities to the file.
         *
         * @param path the path to file for data should to be saved in
         */
        void SavePriorities();
        /**
         * @brief Loads priorities from file.
         *
         * @param path the path to file for data should to be loaded from
         */
        void LoadPriorities();
        /**
         * @brief Deletes all data.
         *
         */
        void Delete();
        /**
         * @brief Sets stage that will be done.
         *
         * @param newStage - stage to set
         */
        void setStage(PARTITIONING_STAGE newStage);
        /**
         * @brief Sets path to database with map (db file).
         *
         * @param path
         */
        void setDatabasePath(QString path);
        /**
         * @brief Sets path to graph data (txt file).
         *
         * @param path
         */
        void setSimpleDataPath(QString path);
        /**
         * @brief Sets path to priorities data (txt file).
         *
         * @param path
         */
        void setPrioritiesDataPath(QString path);
        /**
         * @brief Sets path to final partition data (folder).
         *
         * @param path
         */
        void setFinalDataPath(QString path);
    };
}

#endif // OSMSCOUT_PARTITIONING_H

