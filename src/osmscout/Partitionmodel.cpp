#include <osmscout/Partitionmodel.h>
#include <QDebug>
#include <QVariant>


namespace osmscout {

PartitionModel::PartitionModel(){
    this->db=QSqlDatabase::addDatabase("QSQLITE");
}


bool PartitionModel::open(QString name){
    db.setDatabaseName(name);
    if(!db.open()){
        qDebug()<<"[SQLiteDatabase::open]"<<db.lastError();
        return false;
    }
    return true;
}


bool PartitionModel::createTables()
{
    bool ret = false;
    if (db.isOpen())
    {
        QSqlQuery query;
        QString strQuery =
                "CREATE TABLE [boundaryEdges] ("
                "[node1] INTEGER  NOT NULL,"
                "[node2] INTEGER  NOT NULL,"
                "[wayId] INTEGER  NOT NULL,"
                "[prio_car] REAL DEFAULT '1' NOT NULL"
                ");"
                "CREATE TABLE [innerWays] ("
                "[id] INTEGER  UNIQUE NULL,"
                "[prio_car] REAL DEFAULT '1' NOT NULL"
                ");"
                "CREATE TABLE [nodes] ("
                "[id] INTEGER  UNIQUE NULL,"
                "[lon] REAL  NULL,"
                "[lat] REAL  NULL,"
                "[cell] INTEGER  NULL"
                ");"
                "CREATE TABLE [restrictions] ("
                "[node1] INTEGER  NOT NULL,"
                "[node2] INTEGER  NOT NULL"
                ");"
                "CREATE TABLE [routingEdges] ("
                "[node1] InTEGER  NOT NULL,"
                "[node2] INTEGER  NOT NULL,"
                "[cost_car] REAL DEFAULT '9999' NOT NULL,"
                "[lastWayId] INTEGER  NOT NULL,"
                "[lastNodeId] INTEGER  NOT NULL"
                ");"
                "CREATE TABLE [ways_nodes] ("
                "[way] INTEGER  NULL,"
                "[node] INTEGER  NULL,"
                "[num] INTEGER  NULL"
                ");";

        foreach(QString q, strQuery.split(';'))
        {
            ret = query.exec(q);
        }

    }
    return ret;
}



bool PartitionModel::exportToDatabase(Partitioning::DatabasePartition &dbpart){
    //std::vector< PartNode > nodes;
    //std::vector< PartWay > innerWays;
    //std::vector< BoundaryEdge > boundaryEdges;
    //std::vector< RouteEdge > routeEdges;


    //Add nodes
    for(unsigned int i=0;i<dbpart.nodes.size();i++){
        Partitioning::PartNode node=dbpart.nodes.at(i);
        queriesBuffer.append(QString("INSERT INTO nodes VALUES("+QString::number(node.id)+", "+QString::number(node.lon)+", "+QString::number(node.lat)+", "+QString::number(node.cell)+", "+QString::number(node.type)+");\n "));
        dbInsert(false);
    }

    //Add ways
    for(unsigned int i=0;i<dbpart.innerWays.size();i++){
        Partitioning::PartWay way=dbpart.innerWays.at(i);
        queriesBuffer.append(QString("INSERT INTO ways ( id, prio_car ) VALUES("
                                     +QString::number(way.id)+", "
                                     +QString::number(way.priority)+");\n "));
        for(int j=0;j<way.nodes.size();j++){            
            queriesBuffer.append(QString("INSERT INTO ways_nodes VALUES("
                                         +QString::number(way.id)+", "
                                         +QString::number(way.nodes.at(j))+", "
                                         +QString::number(j)+");\n "));
            dbInsert(false);
        }
    }
    //Add Boundary Edges
    for(unsigned int i=0;i<dbpart.boundaryEdges.size();i++){
        Partitioning::BoundaryEdge boundary=dbpart.boundaryEdges.at(i);
        queriesBuffer.append(QString("INSERT INTO boundaryEdges (node1,node2,wayId,prio_car) VALUES("
                                     +QString::number(boundary.nodeA)+", "
                                     +QString::number(boundary.nodeB)+", "
                                     +QString::number(boundary.wayId)+", "
                                     +QString::number(boundary.priority)+");\n "));
        dbInsert(false);
    }

    //Add Boundary Edges
    for(unsigned int i=0;i<dbpart.routeEdges.size();i++){
        Partitioning::RouteEdge routeEdge=dbpart.routeEdges.at(i);
        queriesBuffer.append(QString("INSERT INTO routeEdges (node1,node2,cost_car,lastWayId,lastNodeId) VALUES("
                                     +QString::number(routeEdge.nodeA)+", "
                                     +QString::number(routeEdge.nodeB)+", "
                                     +QString::number(routeEdge.cost)+", "
                                     +QString::number(routeEdge.lastWayId)+", "
                                     +QString::number(routeEdge.lastNodeId)+
                                     ");\n "));
        dbInsert(false);
    }

    dbInsert(true);

}

bool PartitionModel::dbInsert(bool forceInsert)
{
    bool ret = false;
    if ((queriesBuffer.size() >= MAX_TRANSACTION_QUERIES || forceInsert) && db.isOpen())
    {
        QSqlQuery query;
        query.exec("BEGIN TRANSACTION;");
        foreach( QString q, queriesBuffer)
        {
            query.exec(q);
            if(query.lastError().type()!=QSqlError::NoError)
                qDebug()<<query.lastError();
        }
        ret = query.exec("END TRANSACTION;");
        queriesBuffer.clear();
    }
    return ret;
}

std::vector<Partitioning::PartWay> PartitionModel::getAllWays(){
    // TODO: Needs to return ways not nodes...
    //return getNodesByQuery("SELECT way,node FROM way_nodes ORDER BY way,num;");
}

Partitioning::PartWay PartitionModel::getWay( long WayId ){
    std::vector<Partitioning::PartWay> n=getInnerWaysByQuery("SELECT way,node FROM way_nodes WHERE way ="+QString::number(WayId)+" ORDER BY way,num;");
    if(n.size()>0)
        return n[0];

    Partitioning::PartWay empty;
    return empty;
}

std::vector< Partitioning::PartWay > PartitionModel::getWays( std::vector< long > WaysIds ){
    QString ways;
    for(int i=0;i<WaysIds.size();i++){
        if(i!=0)
            ways+=", ";
        ways+=QString::number(WaysIds[i]);
    }
    return getInnerWaysByQuery("SELECT way,node FROM way_nodes WHERE way IN ("+ways+") ORDER BY way,num;");
}

std::vector<Partitioning::PartWay> PartitionModel::getInnerWaysWithNode( long NodeId ){
    return getInnerWaysByQuery("SELECT way,node FROM way_nodes WHERE way IN (SELECT way FROM ways_nodes WHERE node = "+QString::number(NodeId)+" ) ORDER BY way,num;");
}

std::vector<Partitioning::BoundaryEdge> PartitionModel::getBoundaryEdgesWithNode( long NodeId )
{
    // TODO: Implementation
}

std::vector<Partitioning::RouteEdge> PartitionModel::getRouteEdgesWithNode( long NodeId )
{
    // TODO: Implementation
}

std::vector<Partitioning::PartNode> PartitionModel::getAllNodes(){
    return getNodesByQuery("SELECT * FROM nodes");
}

Partitioning::PartNode PartitionModel::getNode( long NodeId ){
    std::vector<Partitioning::PartNode> n=getNodesByQuery("SELECT DISTINCT * FROM nodes WHERE id IN (SELECT node FROM ways_nodes WHERE id = "+QString::number(NodeId)+" ) ");
    if(n.size()>0)
        return n[0];

    Partitioning::PartNode empty;
    return empty;
}

std::vector< Partitioning::PartNode > PartitionModel::getNodes( std::vector< long > NodesIds ){
    QString nodes;
    for(int i=0;i<NodesIds.size();i++){
        if(i!=0)
            nodes+=", ";
        nodes+=QString::number(NodesIds[i]);
    }

    return getNodesByQuery("SELECT * FROM nodes WHERE id IN( "+nodes+" )");
}

std::vector<Partitioning::PartNode> PartitionModel::getNodesInWay( long WayId ){
    return getNodesByQuery("SELECT DISTINCT * FROM nodes WHERE id IN (SELECT node FROM ways_nodes WHERE way = "+QString::number(WayId)+" ) ");
}

std::vector<Partitioning::PartNode> PartitionModel::getNodesByQuery( QString queryStr ){
    std::vector<Partitioning::PartNode> nodes;

    QSqlQuery query(db);
    query.prepare(queryStr);
    query.exec();

    while (query.next()) {
        Partitioning::PartNode node;
        node.id=query.value(0).toLongLong();
        node.lon=query.value(1).toDouble();
        node.lat=query.value(2).toDouble();
        //node.type=(query.value(3).toInt()==0)?Partitioning::BOUNDARY:Partitioning::INTERNAL;
        node.cell=query.value(3).toInt();
        nodes.push_back(node);
    }

    return nodes;
}


std::vector<Partitioning::PartWay> PartitionModel::getInnerWaysByQuery( QString queryStr ){
    std::vector<Partitioning::PartWay> ways;

    QSqlQuery query(db);
    query.prepare(queryStr);
    query.exec();

    Partitioning::PartWay wayL;
    wayL.id=-1;
    while (query.next()) {
        long way=query.value(0).toLongLong();
        if(wayL.id==-1){
            wayL.id=way;
        }
        else if(way!=wayL.id){
            ways.push_back(wayL);
            wayL.nodes.clear();
            wayL.id=way;
        }
        wayL.nodes.push_back(query.value(1).toLongLong());
    }
    ways.push_back(wayL);

    return ways;
}

}
