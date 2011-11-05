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
     * @brief Finds all ways that are in database.
     *
     * @return vector of all PartWays in database
     */
    std::vector<Partitioning::PartWay> getAllWays(); // TODO
    /**
     * @brief Finds way with given id.
     *
     * @param WayId id of way to find
     *
     * @return one PartWay
     */
    Partitioning::PartWay getWay( long WayId ); // TODO
    /**
     * @brief Finds ways with given ids.
     *
     * @param WayIds list of id of ways to find
     *
     * @return list of PartWays
     */
    std::vector< Partitioning::PartWay > getWays( std::vector< long > WaysIds ); // TODO
    /**
     * @brief Finds all inner ways that contains given node.
     *
     * @fn getWaysWithNode
     * @param NodeId id node in database
     * @return vector of PartWays with nodes id in.
     */
    std::vector<Partitioning::PartWay> getInnerWaysWithNode( long NodeId );
    /**
     * @brief Finds all boundary edges that contains given node.
     *
     * @fn getBoundaryEdgesWithNode
     * @param NodeId id node in database
     * @return vector of BoundaryEdges with nodes id in.
     */
    std::vector<Partitioning::BoundaryEdge> getBoundaryEdgesWithNode( long NodeId ); // TODO
    /**
     * @brief Finds all route edges that contains given node.
     *
     * @fn getRouteEdgesWithNode
     * @param NodeId id node in database
     * @return vector of RouteEdges with nodes id in.
     */
    std::vector<Partitioning::RouteEdge> getRouteEdgesWithNode( long NodeId ); // TODO
    /**
     * @brief Finds all nodes that are in database.
     *
     * @return vector of all PartNodes in database
     */
    std::vector<Partitioning::PartNode> getAllNodes(); // TODO
    /**
     * @brief Finds node with given id.
     *
     * @param NodeId id of node to find

     * @return one PartNode
     */
    Partitioning::PartNode getNode( long NodeId ); // TODO
    /**
     * @brief Finds nodes with given ids.
     *
     * @param NodeIds list of id of nodes to find

     * @return list of PartNodes
     */
    std::vector< Partitioning::PartNode > getNodes( std::vector< long > NodesIds ); // TODO
    /**
     * @brief Returns all nodes in given way.
     *
     * @fn getNodesInWay
     * @param WayId id way in database
     * @return vector of PartNodes in Way
     */
    std::vector<Partitioning::PartNode> getNodesInWay( long WayId );
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
    std::vector<Partitioning::PartNode> getNodesByQuery( QString queryStr );
    std::vector<Partitioning::PartWay> getInnerWaysByQuery( QString queryStr );
};

}

#endif // PARTITIONMODEL_H
