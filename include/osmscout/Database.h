#ifndef OSMSCOUT_DATABASE_H
#define OSMSCOUT_DATABASE_H

/*
  This source is part of the libosmscout library
  Copyright (C) 2009  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <list>
#include <set>

// Type and style sheet configuration
#include <osmscout/StyleConfig.h>
#include <osmscout/TypeConfig.h>

// Datafiles
#include <osmscout/NodeDataFile.h>
#include <osmscout/RelationDataFile.h>
#include <osmscout/WayDataFile.h>

#include <osmscout/OptimizeLowZoom.h>

// Fileoffset by Id index
#include <osmscout/NodeIndex.h>
#include <osmscout/WayIndex.h>

// In area index
#include <osmscout/AreaAreaIndex.h>
#include <osmscout/AreaNodeIndex.h>
#include <osmscout/AreaWayIndex.h>

// Location index
#include <osmscout/CityStreetIndex.h>

// Reverse index
#include <osmscout/NodeUseIndex.h>

// Water index
#include <osmscout/WaterIndex.h>

#include <osmscout/Route.h>

#include <osmscout/util/Cache.h>

#include <QString>

namespace osmscout {

  /**
    Database instance initialisation parameter to influence the behaviour of the database
    instance.

    The following groups attributes are currently available:
    * cache sizes.
    */
  class DatabaseParameter
  {
  private:
    unsigned long areaAreaIndexCacheSize;
    unsigned long areaNodeIndexCacheSize;

    unsigned long nodeIndexCacheSize;
    unsigned long nodeCacheSize;

    unsigned long wayIndexCacheSize;
    unsigned long wayCacheSize;

    unsigned long relationIndexCacheSize;
    unsigned long relationCacheSize;

    bool          debugPerformance;

  public:
    DatabaseParameter();

    void SetAreaAreaIndexCacheSize(unsigned long areaAreaIndexCacheSize);
    void SetAreaNodeIndexCacheSize(unsigned long areaNodeIndexCacheSize);

    void SetNodeIndexCacheSize(unsigned long nodeIndexCacheSize);
    void SetNodeCacheSize(unsigned long nodeCacheSize);

    void SetWayIndexCacheSize(unsigned long wayIndexCacheSize);
    void SetWayCacheSize(unsigned long wayCacheSize);

    void SetRelationIndexCacheSize(unsigned long relationIndexCacheSize);
    void SetRelationCacheSize(unsigned long relationCacheSize);

    void SetDebugPerformance(bool debug);

    unsigned long GetAreaAreaIndexCacheSize() const;
    unsigned long GetAreaNodeIndexCacheSize() const;

    unsigned long GetNodeIndexCacheSize() const;
    unsigned long GetNodeCacheSize() const;

    unsigned long GetWayIndexCacheSize() const;
    unsigned long GetWayCacheSize() const;

    unsigned long GetRelationIndexCacheSize() const;
    unsigned long GetRelationCacheSize() const;

    bool IsDebugPerformance() const;
  };

  /**
    Parameter to influence the search result for searching for (drawable)
    objects in a given area.
    */
  class AreaSearchParameter
  {
  private:
    unsigned long maxAreaLevel;
    unsigned long maxNodes;
    unsigned long maxWays;
    unsigned long maxAreas;

  public:
    AreaSearchParameter();

    void SetMaximumAreaLevel(unsigned long maxAreaLevel);

    void SetMaximumNodes(unsigned long maxNodes);
    void SetMaximumWays(unsigned long maxWays);
    void SetMaximumAreas(unsigned long maxAreas);

    unsigned long GetMaximumAreaLevel() const;

    unsigned long GetMaximumNodes() const;
    unsigned long GetMaximumWays() const;
    unsigned long GetMaximumAreas() const;
  };

  class Database
  {
  public: // Fix this
    struct NodeUse
    {
      Id              id;
      std::vector<Id> references;
    };

    typedef Cache<size_t,std::vector<NodeUse> > NodeUseCache;

  private:
    bool                  isOpen;          //! true, if opened
    bool                  debugPerformance;

    double                minLon;          //! bounding box of data
    double                minLat;          //! bounding box of data
    double                maxLon;          //! bounding box of data
    double                maxLat;          //! bounding box of data

    AreaAreaIndex         areaAreaIndex;
    AreaNodeIndex         areaNodeIndex;
    AreaWayIndex          areaWayIndex;

    CityStreetIndex       cityStreetIndex;

    WaterIndex            waterIndex;

    std::string           path;             //! Path to the directory containing all files

    NodeDataFile          nodeDataFile;     //! Cached access to the 'nodes.dat' file
    RelationDataFile      relationDataFile; //! Cached access to the 'relations.dat' file
    WayDataFile           wayDataFile;      //! Cached access to the 'ways.dat' file

    OptimizeLowZoom       optimizeLowZoom;  //! Optimized data for low zoom situations

    mutable FileScanner   nodeUseScanner;   //! File stream to the nodeuse.idx file

    TypeConfig            *typeConfig;      //! Type config for the currently opened map

    std::string           (*hashFunction) (std::string);

  private:
    /*
    size_t GetMaximumPriority(const StyleConfig& styleConfig,
                              double minlon, double minlat,
                              double maxlon, double maxlat,
                              double magnification,
                              size_t maxNodes) const;*/

    bool GetNodes(const std::vector<FileOffset>& offsets,
                  std::vector<NodeRef>& nodes) const;

    bool GetWays(const std::vector<FileOffset>& offsets,
                 std::vector<WayRef>& ways) const;

    bool GetWays(const std::list<FileOffset>& offsets,
                 std::vector<WayRef>& ways) const;

    bool GetRelations(const std::vector<FileOffset>& offsets,
                      std::vector<RelationRef>& relations) const;

    bool GetRelations(const std::list<FileOffset>& offsets,
                      std::vector<RelationRef>& relations) const;

    /*
    bool GetNodes(const StyleConfig& styleConfig,
                  double lonMin, double latMin,
                  double lonMax, double latMax,
                  double magnification,
                  size_t maxPriority,
                  std::vector<Node>& nodes) const;*/

    bool GetJoints(NodeUseIndex& nodeUseIndex,
                   NodeUseCache& nodeUseCache,
                   Id id,
                   std::set<Id>& wayIds) const;
    bool GetJoints(NodeUseIndex& nodeUseIndex,
                   NodeUseCache& nodeUseCache,
                   const std::set<Id>& ids,
                   std::set<Id>& wayIds) const;

  public:
    Database(const DatabaseParameter& parameter);
    virtual ~Database();

    bool Open(const QString& path,
              std::string (*hashFunction) (std::string) = NULL);
    bool IsOpen() const;
    void Close();

    void FlushCache();

    TypeConfig* GetTypeConfig() const;

    bool GetBoundingBox(double& minLat,double& minLon,
                        double& maxLat,double& maxLon) const;

    bool GetObjects(const StyleConfig& styleConfig,
                    double lonMin, double latMin,
                    double lonMax, double latMax,
                    double magnification,
                    const AreaSearchParameter& parameter,
                    std::vector<NodeRef>& nodes,
                    std::vector<WayRef>& ways,
                    std::vector<WayRef>& areas,
                    std::vector<RelationRef>& relationWays,
                    std::vector<RelationRef>& realtionAreas) const;

    bool GetGroundTiles(double lonMin, double latMin,
                        double lonMax, double latMax,
                        std::list<GroundTile>& tiles) const;

    bool GetNode(const Id& id,
                 NodeRef& node) const;
    bool GetNodes(const std::vector<Id>& ids,
                  std::vector<NodeRef>& nodes) const;

    bool GetWay(const Id& id,
                WayRef& way) const;
    bool GetWays(const std::vector<Id>& ids,
                 std::vector<WayRef>& ways) const;
    bool GetWays(const std::set<Id>& ids,
                 std::vector<WayRef>& ways) const;

    bool GetRelation(const Id& id,
                     RelationRef& relation) const;
    bool GetRelations(const std::vector<Id>& ids,
                      std::vector<RelationRef>& relations) const;

    bool GetMatchingAdminRegions(const std::string& name,
                                 std::list<AdminRegion>& regions,
                                 size_t limit,
                                 bool& limitReached,
                                 bool startWith) const;

    bool GetMatchingLocations(const AdminRegion& region,
                              const std::string& name,
                              std::list<Location>& locations,
                              size_t limit,
                              bool& limitReached,
                              bool startWith) const;

    bool CalculateRoute(Id startWayId, Id startNodeId,
                        Id targetWayId, Id targetNodeId,
                        RouteData& route);

    bool TransformRouteDataToRouteDescription(const RouteData& data,
                                              RouteDescription& description);
    bool TransformRouteDataToWay(const RouteData& data,
                                 Way& way);

    void DumpStatistics();
  };
}

#endif
