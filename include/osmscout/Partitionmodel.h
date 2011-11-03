#ifndef PARTITIONMODEL_H
#define PARTITIONMODEL_H

#include <QtSql/QSqlDatabase>
#include <osmscout/Partitioning.h>
#include <QStringList>

namespace osmscout {

/**
 * @brief Class to load partition data
 *
 * @class PartitionModel Partitionmodel.h
 */
class PartitionModel
{
public:
/**
 * @brief
 *
 * @fn PartitionModel
 */
    PartitionModel();
    /**
     * @brief
     *
     * @fn open
     * @param name
     */
    bool open(QString name);
    /**
     * @brief
     *
     * @fn getWays
     * @param NodeId id node in database
     * @return vector of PartWays with nodes id in.
     */
    std::vector<Partitioning::PartWay> getWays( long NodeId );
    /**
     * @brief
     *
     * @fn getNodes
     * @param WayId id way in database
     * @return vector of PartNodes in Way
     */
    std::vector<Partitioning::PartNode> getNodes( long WayId );
    /**
     * @brief
     *
     * @fn exportToDatabase
     * @param ways with vector of nodesids to nodes index
     * @param nodes
     * @param boundaryEdges
     */
    bool exportToDatabase(std::vector<Partitioning::PartWay> &ways,std::vector<Partitioning::PartNode> &nodes,std::vector< Partitioning::BoundaryEdge > &boundaryEdges);
private:
    const static int MAX_TRANSACTION_QUERIES = 100000; /**< TODO */

    QSqlDatabase db; /**< TODO */
    /**
     * @brief
     *
     * @fn dbInsert
     * @param forceInsert ignore MAX_TRANSACTION_QUERIES
     */
    bool dbInsert(bool forceInsert);
    QStringList queriesBuffer; /**< TODO */
};

}

#endif // PARTITIONMODEL_H
