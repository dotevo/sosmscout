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

#include <osmscout/Database.h>

#include <cassert>
#include <iostream>

#include <QDebug>

#include <osmscout/RoutingProfile.h>
#include <osmscout/TypeConfigLoader.h>

#include <osmscout/util/Geometry.h>
#include <osmscout/util/StopClock.h>

#include <osmscout/private/Math.h>

namespace osmscout {

  struct NodeUseCacheValueSizer : public Database::NodeUseCache::ValueSizer
  {
    unsigned long GetSize(const std::vector<Database::NodeUse>& value) const
    {
      unsigned long memory=0;

      for (size_t i=0; i<value.size(); i++) {
        memory+=sizeof(Database::NodeUse);
      }

      return memory;
    }
  };

  DatabaseParameter::DatabaseParameter()
  : areaAreaIndexCacheSize(500),
    areaNodeIndexCacheSize(500),
    nodeIndexCacheSize(500),
    nodeCacheSize(500),
    wayIndexCacheSize(500),
    wayCacheSize(500),
    relationIndexCacheSize(500),
    relationCacheSize(500),
    debugPerformance(false)
  {
    // no code
  }

  void DatabaseParameter::SetAreaAreaIndexCacheSize(unsigned long areaAreaIndexCacheSize)
  {
    this->areaAreaIndexCacheSize=areaAreaIndexCacheSize;
  }

  void DatabaseParameter::SetAreaNodeIndexCacheSize(unsigned long areaNodeIndexCacheSize)
  {
    this->areaNodeIndexCacheSize=areaNodeIndexCacheSize;
  }

  void DatabaseParameter::SetNodeIndexCacheSize(unsigned long nodeIndexCacheSize)
  {
    this->nodeIndexCacheSize=nodeIndexCacheSize;
  }

  void DatabaseParameter::SetNodeCacheSize(unsigned long nodeCacheSize)
  {
    this->nodeCacheSize=nodeCacheSize;
  }

  void DatabaseParameter::SetWayIndexCacheSize(unsigned long wayIndexCacheSize)
  {
    this->wayIndexCacheSize=wayIndexCacheSize;
  }

  void DatabaseParameter::SetWayCacheSize(unsigned long wayCacheSize)
  {
    this->wayCacheSize=wayCacheSize;
  }

  void DatabaseParameter::SetRelationIndexCacheSize(unsigned long relationIndexCacheSize)
  {
    this->relationIndexCacheSize=relationIndexCacheSize;
  }

  void DatabaseParameter::SetRelationCacheSize(unsigned long relationCacheSize)
  {
    this->relationCacheSize=relationCacheSize;
  }

  void DatabaseParameter::SetDebugPerformance(bool debug)
  {
    debugPerformance=debug;
  }

  unsigned long DatabaseParameter::GetAreaAreaIndexCacheSize() const
  {
    return areaAreaIndexCacheSize;
  }

  unsigned long DatabaseParameter::GetAreaNodeIndexCacheSize() const
  {
    return areaNodeIndexCacheSize;
  }

  unsigned long DatabaseParameter::GetNodeIndexCacheSize() const
  {
    return nodeIndexCacheSize;
  }

  unsigned long DatabaseParameter::GetNodeCacheSize() const
  {
    return nodeCacheSize;
  }

  unsigned long DatabaseParameter::GetWayIndexCacheSize() const
  {
    return wayIndexCacheSize;
  }

  unsigned long DatabaseParameter::GetWayCacheSize() const
  {
    return wayCacheSize;
  }

  unsigned long DatabaseParameter::GetRelationIndexCacheSize() const
  {
    return relationIndexCacheSize;
  }

  unsigned long DatabaseParameter::GetRelationCacheSize() const
  {
    return relationCacheSize;
  }

  bool DatabaseParameter::IsDebugPerformance() const
  {
    return debugPerformance;
  }

  AreaSearchParameter::AreaSearchParameter()
  : maxAreaLevel(4),
    maxNodes(2000),
    maxWays(10000),
    maxAreas(std::numeric_limits<unsigned long>::max())
  {
    // no code
  }

  void AreaSearchParameter::SetMaximumAreaLevel(unsigned long maxAreaLevel)
  {
    this->maxAreaLevel=maxAreaLevel;
  }

  void AreaSearchParameter::SetMaximumNodes(unsigned long maxNodes)
  {
    this->maxNodes=maxNodes;
  }

  void AreaSearchParameter::SetMaximumWays(unsigned long maxWays)
  {
    this->maxWays=maxWays;
  }

  void AreaSearchParameter::SetMaximumAreas(unsigned long maxAreas)
  {
    this->maxAreas=maxAreas;
  }

  unsigned long AreaSearchParameter::GetMaximumAreaLevel() const
  {
    return maxAreaLevel;
  }

  unsigned long AreaSearchParameter::GetMaximumNodes() const
  {
    return maxNodes;
  }

  unsigned long AreaSearchParameter::GetMaximumWays() const
  {
    return maxWays;
  }

  unsigned long AreaSearchParameter::GetMaximumAreas() const
  {
    return maxAreas;
  }

  Database::Database(const DatabaseParameter& parameter)
   : isOpen(false),
     debugPerformance(parameter.IsDebugPerformance()),
     areaAreaIndex(parameter.GetAreaAreaIndexCacheSize()),
     areaNodeIndex(parameter.GetAreaNodeIndexCacheSize()),
     areaWayIndex(),
     nodeDataFile("nodes.dat",
                  "node.idx",
                  parameter.GetNodeCacheSize(),
                  parameter.GetNodeIndexCacheSize()),
     relationDataFile("relations.dat",
                      "relation.idx",
                      parameter.GetRelationCacheSize(),
                      parameter.GetRelationIndexCacheSize()),
     wayDataFile("ways.dat",
                 "way.idx",
                  parameter.GetWayCacheSize(),
                  parameter.GetWayIndexCacheSize()),
     typeConfig(NULL),
     hashFunction(NULL)
  {
    // no code
  }

  Database::~Database()
  {
    delete typeConfig;
  }

  bool Database::Open(const QString& path,
                      std::string (*hashFunction) (std::string))
  {
	  assert(!path.isEmpty());
std::string l((const char*)path.toAscii());
	  this->path=l;
    this->hashFunction=hashFunction;

    typeConfig=new TypeConfig();

    if (!LoadTypeData(l,*typeConfig)) {
      std::cerr << "Cannot load 'types.dat'!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }

    FileScanner scanner;
    std::string file=AppendFileToDir(this->path,"bounding.dat");

    if (!scanner.Open(file)) {
      std::cerr << "Cannot open 'bounding.dat'" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }

    uint32_t minLonDat;
    uint32_t minLatDat;
    uint32_t maxLonDat;
    uint32_t maxLatDat;

    scanner.ReadNumber(minLatDat);
    scanner.ReadNumber(minLonDat);
    scanner.ReadNumber(maxLatDat);
    scanner.ReadNumber(maxLonDat);

    if (scanner.HasError() || !scanner.Close()) {
      std::cerr << "Error while reading/closing '" << file << "'" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }

    minLon=minLonDat/conversionFactor-180.0;
    minLat=minLatDat/conversionFactor-90.0;
    maxLon=maxLonDat/conversionFactor-180.0;
    maxLat=maxLatDat/conversionFactor-90.0;

//    std::cout << "Data bounding box: [" << minLat << "," << minLon << "] - [" << maxLat << "," << maxLon << "]" << std::endl;

//    std::cout << "Opening 'nodes.dat'..." << std::endl;
    if (!nodeDataFile.Open(l)) {
//      std::cerr << "Cannot open 'nodes.dat'!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Opening 'nodes.dat' done." << std::endl;

//    std::cout << "Opening 'ways.dat'..." << std::endl;
    if (!wayDataFile.Open(this->path)) {
//      std::cerr << "Cannot open 'ways.dat'!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Opening 'ways.dat' done." << std::endl;

//    std::cout << "Opening 'relations.dat'..." << std::endl;
    if (!relationDataFile.Open(this->path)) {
      std::cerr << "Cannot open 'relations.dat'!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Opening 'relations.dat' done." << std::endl;

//    std::cout << "Loading low zoom optimizations..." << std::endl;
    if (!optimizeLowZoom.Open(this->path)) {
//      std::cerr << "Cannot load low zoom optimizations!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Loading water index done." << std::endl;

//    std::cout << "Loading area area index..." << std::endl;
    if (!areaAreaIndex.Load(this->path)) {
      std::cerr << "Cannot load area area index!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Loading area index done." << std::endl;

//    std::cout << "Loading area node index..." << std::endl;
    if (!areaNodeIndex.LoadAreaNodeIndex(this->path)) {
//      std::cerr << "Cannot load area node index!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Loading area node index done." << std::endl;

//    std::cout << "Loading area way index..." << std::endl;
    if (!areaWayIndex.Load(this->path)) {
      std::cerr << "Cannot load area way index!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Loading area way index done." << std::endl;

//    std::cout << "Loading city street index..." << std::endl;
    if (!cityStreetIndex.Load(this->path, hashFunction)) {
      std::cerr << "Cannot load city street index!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Loading city street index done." << std::endl;

//    std::cout << "Loading water index..." << std::endl;
    if (!waterIndex.Load(this->path)) {
      std::cerr << "Cannot load water index!" << std::endl;
      delete typeConfig;
      typeConfig=NULL;
      return false;
    }
//    std::cout << "Loading water index done." << std::endl;

    isOpen=true;

    return true;
  }

  bool Database::IsOpen() const
  {
    return isOpen;
  }


  void Database::Close()
  {
    nodeDataFile.Close();
    wayDataFile.Close();

    isOpen=false;
  }

  void Database::FlushCache()
  {
    nodeDataFile.FlushCache();
    wayDataFile.FlushCache();
    relationDataFile.FlushCache();
  }

  TypeConfig* Database::GetTypeConfig() const
  {
    return typeConfig;
  }

  bool Database::GetBoundingBox(double& minLat,double& minLon,
                                double& maxLat,double& maxLon) const
  {
    if (!IsOpen()) {
      return false;
    }

    minLat=this->minLat;
    minLon=this->minLon;
    maxLat=this->maxLat;
    maxLon=this->maxLon;

    return true;
  }

  bool Database::GetObjects(const StyleConfig& styleConfig,
                            double lonMin, double latMin,
                            double lonMax, double latMax,
                            double magnification,
                            const AreaSearchParameter& parameter,
                            std::vector<NodeRef>& nodes,
                            std::vector<WayRef>& ways,
                            std::vector<WayRef>& areas,
                            std::vector<RelationRef>& relationWays,
                            std::vector<RelationRef>& relationAreas) const
  {
    if (!IsOpen()) {
      return false;
    }

    std::vector<TypeId>     wayTypes;
    TypeSet                 areaTypes;
    std::vector<TypeId>     nodeTypes;
    std::vector<FileOffset> nodeOffsets;
    std::vector<FileOffset> wayWayOffsets;
    std::vector<FileOffset> relationWayOffsets;
    std::vector<FileOffset> wayAreaOffsets;
    std::vector<FileOffset> relationAreaOffsets;
    double                  magLevel=log2(magnification);

    nodes.clear();
    ways.clear();
    areas.clear();
    relationWays.clear();
    relationAreas.clear();

    StopClock nodeIndexTimer;

    styleConfig.GetNodeTypesWithMag(magnification,
                                    nodeTypes);

    if (!areaNodeIndex.GetOffsets(styleConfig,
                                  lonMin,latMin,lonMax,latMax,
                                  nodeTypes,
                                  parameter.GetMaximumNodes(),
                                  nodeOffsets)) {
      std::cout << "Error getting nodes from area node index!" << std::endl;
      return false;
    }

    nodeIndexTimer.Stop();

    StopClock wayIndexTimer;

    styleConfig.GetWayTypesByPrioWithMag(magnification,
                                         wayTypes);

    if (optimizeLowZoom.HasOptimizations(magnification)) {
      optimizeLowZoom.GetWays(styleConfig,
                              lonMin,
                              latMin,
                              lonMax,
                              latMax,
                              parameter.GetMaximumWays(),
                              wayTypes,
                              ways);
    }

    if (!areaWayIndex.GetOffsets(styleConfig,
                                 lonMin,
                                 latMin,
                                 lonMax,
                                 latMax,
                                 wayTypes,
                                 parameter.GetMaximumWays(),
                                 wayWayOffsets,
                                 relationWayOffsets)) {
      std::cout << "Error getting ways and relations from area way index!" << std::endl;
    }

    wayIndexTimer.Stop();

    StopClock areaAreaIndexTimer;

    /*
    std::cout << "Ways for magnification: " << magLevel << std::endl;
    for (size_t i=0; i<wayTypes.size(); i++) {
      std::cout << "Drawing way of type: " << typeConfig->GetTypes()[wayTypes[i]].GetName() << " " << typeConfig->GetTypes()[wayTypes[i]].GetId() << std::endl;
    }*/

    wayTypes.clear();

    styleConfig.GetAreaTypesWithMag(magnification,
                                    areaTypes);

    if (!areaAreaIndex.GetOffsets(styleConfig,
                                  lonMin,
                                  latMin,
                                  lonMax,
                                  latMax,
                                  ((size_t)ceil(magLevel))+
                                  parameter.GetMaximumAreaLevel(),
                                  areaTypes,
                                  parameter.GetMaximumAreas(),
                                  wayAreaOffsets,
                                  relationAreaOffsets)) {
      std::cout << "Error getting ways and relations from area index!" << std::endl;
    }

    areaAreaIndexTimer.Stop();

    StopClock nodesTimer;

    if (!GetNodes(nodeOffsets,
                  nodes)) {
      std::cout << "Error reading nodes in area!" << std::endl;
      return false;
    }

    nodesTimer.Stop();

    StopClock waysTimer;

    if (!GetWays(wayWayOffsets,
                 ways)) {
      std::cout << "Error reading ways in area!" << std::endl;
      return false;
    }

    /*
    for (size_t i=0; i<ways.size(); i++) {
      std::cout << ways[i]->GetId() << " " << ways[i]->GetName() << " " << ways[i]->GetRefName() << std::endl;
    }*/

    waysTimer.Stop();

    StopClock areasTimer;

    if (!GetWays(wayAreaOffsets,
                 areas)) {
      std::cout << "Error reading areas in area!" << std::endl;
      return false;
    }

    areasTimer.Stop();

    StopClock relationWaysTimer;

    if (!GetRelations(relationWayOffsets,
                      relationWays)) {
      std::cout << "Error reading relation ways in area!" << std::endl;
      return false;
    }

    relationWaysTimer.Stop();

    StopClock relationAreasTimer;

    if (!GetRelations(relationAreaOffsets,
                      relationAreas)) {
      std::cerr << "Error reading relation areas in area!" << std::endl;
      return false;
    }

    relationAreasTimer.Stop();

    if (debugPerformance) {
      std::cout << "I/O: ";
      std::cout << "n " << nodeIndexTimer << " ";
      std::cout << "w " << wayIndexTimer << " ";
      std::cout << "a " << areaAreaIndexTimer;
      std::cout << " - ";
      std::cout << "n " << nodesTimer << " ";
      std::cout << "w " << waysTimer << "/" << relationWaysTimer << " ";
      std::cout << "a " << areasTimer << "/" << relationAreasTimer;
      std::cout << std::endl;
    }

    return true;
  }

  bool Database::GetGroundTiles(double lonMin, double latMin,
                                double lonMax, double latMax,
                                std::list<GroundTile>& tiles) const
  {
    if (!IsOpen()) {
      return false;
    }

    StopClock timer;

    if (!waterIndex.GetRegions(lonMin, latMin, lonMax, latMax, tiles)) {
      std::cerr << "Error reading ground tiles in area!" << std::endl;
      return false;
    }

    timer.Stop();

    //std::cout << "Found " << tiles.size() << " ground tiles " << timer << std::endl;

    return true;
  }

  bool Database::GetNode(const Id& id, NodeRef& node) const
  {
    if (!IsOpen()) {
      return false;
    }

    std::vector<Id>      ids;
    std::vector<NodeRef> nodes;

    ids.push_back(id);

    if (GetNodes(ids,nodes)) {
      if (!nodes.empty()) {
        node=*nodes.begin();
        return true;
      }
    }

    return false;
  }

  bool Database::GetNodes(const std::vector<FileOffset>& offsets,
                          std::vector<NodeRef>& nodes) const
  {
    if (!IsOpen()) {
      return false;
    }

    return nodeDataFile.Get(offsets,nodes);
  }

  bool Database::GetNodes(const std::vector<Id>& ids,
                          std::vector<NodeRef>& nodes) const
  {
    if (!IsOpen()) {
      return false;
    }

    return nodeDataFile.Get(ids,nodes);
  }

  bool Database::GetWays(const std::vector<FileOffset>& offsets,
                         std::vector<WayRef>& ways) const
  {
    if (!IsOpen()) {
      return false;
    }

    return wayDataFile.Get(offsets,ways);
  }

  bool Database::GetWays(const std::list<FileOffset>& offsets,
                         std::vector<WayRef>& ways) const
  {
    if (!IsOpen()) {
      return false;
    }

    return wayDataFile.Get(offsets,ways);
  }

  bool Database::GetRelations(const std::vector<FileOffset>& offsets,
                              std::vector<RelationRef>& relations) const
  {
    if (!IsOpen()) {
      return false;
    }

    return relationDataFile.Get(offsets,relations);
  }

  bool Database::GetRelations(const std::list<FileOffset>& offsets,
                              std::vector<RelationRef>& relations) const
  {
    if (!IsOpen()) {
      return false;
    }

    return relationDataFile.Get(offsets,relations);
  }

  bool Database::GetWay(const Id& id, WayRef& way) const
  {
    if (!IsOpen()) {
      return false;
    }

    std::vector<Id>     ids;
    std::vector<WayRef> ways;

    ids.push_back(id);

    if (GetWays(ids,ways)) {
      if (!ways.empty()) {
        way=*ways.begin();
        return true;
      }
    }

    return false;
  }

  bool Database::GetWays(const std::vector<Id>& ids,
                         std::vector<WayRef>& ways) const
  {
    if (!IsOpen()) {
      return false;
    }

    return wayDataFile.Get(ids,ways);
  }

  bool Database::GetWays(const std::set<Id>& ids,
                         std::vector<WayRef>& ways) const
  {
    if (!IsOpen()) {
      return false;
    }

    return wayDataFile.Get(ids,ways);
  }

  bool Database::GetRelation(const Id& id,
                             RelationRef& relation) const
  {
    if (!IsOpen()) {
      return false;
    }

    return relationDataFile.Get(id,relation);
  }

  bool Database::GetRelations(const std::vector<Id>& ids,
                              std::vector<RelationRef>& relations) const
  {
    if (!IsOpen()) {
      return false;
    }

    return relationDataFile.Get(ids,relations);
  }

  bool Database::GetMatchingAdminRegions(const std::string& name,
                                         std::list<AdminRegion>& regions,
                                         size_t limit,
                                         bool& limitReached,
                                         bool startWith) const
  {
    if (!IsOpen()) {
      return false;
    }

    return cityStreetIndex.GetMatchingAdminRegions(name,
                                                   regions,
                                                   limit,
                                                   limitReached,
                                                   startWith);
  }

  bool Database::GetMatchingLocations(const AdminRegion& region,
                                      const std::string& name,
                                      std::list<Location>& locations,
                                      size_t limit,
                                      bool& limitReached,
                                      bool startWith) const
  {
    if (!IsOpen()) {
      return false;
    }

    return cityStreetIndex.GetMatchingLocations(region,
                                                name,
                                                locations,
                                                limit,
                                                limitReached,
                                                startWith);
  }

  bool Database::GetJoints(NodeUseIndex& nodeUseIndex,
                           NodeUseCache& nodeUseCache,
                           const std::set<Id>& ids,
                           std::set<Id>& wayIds) const
  {
    if (!IsOpen()) {
      return false;
    }

    std::list<NodeUseIndexEntry> indexEntries;
    std::string                  file=path+"/"+"nodeuse.idx";

    wayIds.clear();

    nodeUseIndex.GetNodeIndexEntries(ids,indexEntries);

    if (indexEntries.empty()) {
      std::cout << "GetJoints(): Ids not found in index" << std::endl;
      return false;
    }

    if (!nodeUseScanner.IsOpen()) {
      if (!nodeUseScanner.Open(file)) {
        std::cerr << "Cannot open nodeuse.idx file!" << std::endl;
        return false;
      }
    }

    Cache<size_t,std::vector<NodeUse> >::CacheRef cacheRef;

    for (std::list<NodeUseIndexEntry>::const_iterator indexEntry=indexEntries.begin();
         indexEntry!=indexEntries.end();
         ++indexEntry) {
      if (!nodeUseCache.GetEntry(indexEntry->interval,cacheRef)) {
        if (!nodeUseScanner.SetPos(indexEntry->offset)) {
          std::cerr << "Cannot read nodeuse.idx page from file!" << std::endl;
          nodeUseScanner.Close();
          return false;
        }

        Cache<size_t, std::vector<NodeUse> >::CacheEntry cacheEntry(indexEntry->interval);
        cacheRef=nodeUseCache.SetEntry(cacheEntry);

        cacheRef->value.resize(indexEntry->count);

        for (size_t i=0; i<indexEntry->count; i++) {
          uint32_t count;

          nodeUseScanner.Read(cacheRef->value[i].id);
          nodeUseScanner.ReadNumber(count);

          if (nodeUseScanner.HasError()) {
            std::cerr << "Error while reading from nodeuse.idx file!" << std::endl;
            nodeUseScanner.Close();
            return false;
          }

          cacheRef->value[i].references.resize(count);

          for (size_t j=0; j<count; j++) {
            nodeUseScanner.Read(cacheRef->value[i].references[j]);
          }
        }
      }

      for (std::vector<NodeUse>::const_iterator w=cacheRef->value.begin();
           w!=cacheRef->value.end();
           ++w) {
        if (ids.find(w->id)!=ids.end()) {
          for (size_t i=0; i<w->references.size(); i++) {
            wayIds.insert(w->references[i]);
          }
        }
      }
    }

    return true;
  }

  bool Database::GetJoints(NodeUseIndex& nodeUseIndex,
                           NodeUseCache& nodeUseCache,
                           Id id,
                           std::set<Id>& wayIds) const
  {
    if (!IsOpen()) {
      return false;
    }

    std::set<Id> ids;

    ids.insert(id);

    return GetJoints(nodeUseIndex,nodeUseCache,ids,wayIds);
  }

  typedef const Way* WayPtr;

  static bool GetWays(const WayIndex& index,
                      const std::string& path,
                      std::map<Id,Way>& cache,
                      const std::set<Id>& ids,
                      std::vector<WayPtr>& refs)
  {
    bool result=true;

    refs.clear();
    refs.reserve(ids.size());

    std::vector<Id> remaining;

    for (std::set<Id>::const_iterator id=ids.begin();
         id!=ids.end();
         ++id) {
      std::map<Id,Way>::const_iterator ref=cache.find(*id);

      if (ref!=cache.end()) {
        refs.push_back(&ref->second);
      }
      else {
        remaining.push_back(*id);
      }
    }

    if (!remaining.empty()) {
      std::vector<FileOffset> offsets;
      static FileScanner      wayScanner;
      std::string             file=path+"/"+"ways.dat";

      if (!wayScanner.IsOpen()) {
        if (!wayScanner.Open(file)){
          std::cerr << "Error while opening ways.dat for routing file!" << std::endl;

          return false;
        }
      }

      if (!index.GetOffsets(remaining,offsets)) {
        return false;
      }

      for (std::vector<FileOffset>::const_iterator entry=offsets.begin();
           entry!=offsets.end();
           ++entry) {
        wayScanner.SetPos(*entry);

        Way way;

        way.Read(wayScanner);

        std::pair<std::map<Id,Way>::iterator,bool> result=cache.insert(std::pair<Id,Way>(way.GetId(),way));

        refs.push_back(&result.first->second);
      }

      result=!wayScanner.HasError()/* && wayScanner.Close()*/;
    }

    assert(ids.size()==refs.size());

    return result;
  }

  static bool GetWay(const WayIndex& index,
                     const std::string& path,
                     std::map<Id,Way>& cache,
                     Id id,
                     WayPtr& ref)
  {
    std::set<Id>        ids;
    std::vector<WayPtr> refs;

    ids.insert(id);

    if (!GetWays(index,path,cache,ids,refs)) {
      return false;
    }

    ref=refs[0];

    return true;
  }


  struct RNode
  {
    Id        id;
    double    lon;
    double    lat;
    double    currentCost;
    double    estimateCost;
    double    overallCost;
    ObjectRef ref;
    Id        prev;

    RNode()
     : id(0)
    {
    }

    RNode(Id id,
          double lon, double lat,
          const ObjectRef& reference,
          Id prev)
     : id(id),
       lon(lon),
       lat(lat),
       currentCost(0),
       estimateCost(0),
       overallCost(0),
       ref(reference),
       prev(prev)
    {
      // no code
    }

    bool operator==(const RNode& node)
    {
      return id==node.id;
    }

    bool operator<(const RNode& node) const
    {
      return id<node.id;
    }
  };

  struct RNodeCostCompare
  {
    bool operator()(const RNode& a, const RNode& b) const
    {
      return a.overallCost<b.overallCost;
    }
  };

  bool CanBeTurnedInto(const Way& way, Id via, Id to)
  {
    if (way.restrictions.empty()) {
      return true;
    }

    for (std::vector<Way::Restriction>::const_iterator iter=way.restrictions.begin();
         iter!=way.restrictions.end();
         ++iter) {
      if (iter->type==Way::rstrAllowTurn) {
        // If our "to" is restriction "to" and our via is in the list of restriction "vias"
        // we can turn, else not.
        // If our !"to" is not the "to" of our restriction we also cannot turn.
        if (iter->members[0]==to) {
          for (size_t i=1; i<iter->members.size(); i++) {
            if (iter->members[i]==via) {
              return true;
            }
          }

          return false;
        }
        else {
          return false;
        }
      }
      else if (iter->type==Way::rstrForbitTurn) {
        // If our "to" is the restriction "to" and our "via" is in the list of the restriction "vias"
        // we cannot turn.
        if (iter->members[0]==to) {
          for (size_t i=1; i<iter->members.size(); i++) {
            if (iter->members[i]==via) {
              return false;
            }
          }
        }
      }
    }

    return true;
  }

  struct Follower
  {
    std::set<Id>  ways;
  };

  typedef std::set<RNode,RNodeCostCompare> OpenList;
  typedef OpenList::iterator RNodeRef;

  struct RouteStep
  {
    Id wayId;
    Id nodeId;
  };

  bool Database::CalculateRoute(Id startWayId, Id startNodeId,
                                Id targetWayId, Id targetNodeId,
                                RouteData& route)
  {
      TypeId              type;
      std::map<Id,Way>    waysCache;
      std::map<Id,Follower> candidatesCache;
      std::vector<WayPtr> followWays;
      WayPtr              startWay;
      WayPtr              currentWay;
      double              startLon=0.0L,startLat=0.0L;
      WayPtr              targetWay;
      double              targetLon=0.0L,targetLat=0.0L;
      OpenList            openList;
      std::map<Id,RNodeRef> openMap;
      std::map<Id,RNode>  closeMap;
      std::set<Id>        loaded;
      std::vector<size_t> costs;
      RoutingProfile      profile;
      NodeUseCache        nodeUseCache(10);  //! Cache for node use data, seems like the cache is more expensive than direct loading!?
      NodeUseIndex        nodeUseIndex;
      WayIndex            waysIndexed("way.idx",100000);


      // ==============================INITIALIZATION=======================================

      std::cout << "Loading node use index..." << std::endl;
      if (!nodeUseIndex.LoadNodeUseIndex(path)) {
        std::cerr << "Cannot load node use index!" << std::endl;
        return false;
      }
      std::cout << "Loading node use index done." << std::endl;

      std::cout << "Loading way index..." << std::endl;
      if (!waysIndexed.Load(path)) {
        std::cerr << "Cannot load way index..." << std::endl;
      }
      std::cout << "Loading way index done." << std::endl;

      std::cout << startWayId << " " << startNodeId << " => " << targetWayId << " " << targetNodeId << std::endl;

      std::cout << "=========== Routing start =============" << std::endl;

      StopClock clock;

      route.Clear();

      // Setting cost values of specific road types and turns
      profile.SetTurnCostFactor(1/60/2); // 30 seconds

      type=typeConfig->GetWayTypeId("highway_motorway");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/110.0);

      type=typeConfig->GetWayTypeId("highway_motorway_link");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/60.0);

      type=typeConfig->GetWayTypeId("highway_trunk");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/70.0);

      type=typeConfig->GetWayTypeId("highway_trunk_link");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/70.0);

      type=typeConfig->GetWayTypeId("highway_primary");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/70.0);

      type=typeConfig->GetWayTypeId("highway_primary_link");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/60.0);

      type=typeConfig->GetWayTypeId("highway_secondary");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/60.0);

      type=typeConfig->GetWayTypeId("highway_secondary_link");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/50.0);

      type=typeConfig->GetWayTypeId("highway_tertiary");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/55.0);

      type=typeConfig->GetWayTypeId("highway_unclassified");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/50.0);

      type=typeConfig->GetWayTypeId("highway_road");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/50.0);

      type=typeConfig->GetWayTypeId("highway_residential");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/40.0);

      type=typeConfig->GetWayTypeId("highway_living_street");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/10.0);

      type=typeConfig->GetWayTypeId("highway_service");
      assert(type!=typeIgnore);
      profile.SetTypeCostFactor(type,1/30.0);

      // Get start way
      if (!osmscout::GetWay(waysIndexed,
                            path,
                            waysCache,
                            startWayId,
                            startWay)) {
        std::cerr << "Cannot get start way!" << std::endl;
        return false;
      }

      // Get end way
      if (!osmscout::GetWay(waysIndexed,
                            path,
                            waysCache,
                            targetWayId,
                            targetWay)) {
        std::cerr << "Cannot get end way!" << std::endl;
        return false;
      }

      // Get start node
      std::cout << "Searching for node " << startNodeId << " in way " << startWayId << "..." << std::endl;

      size_t index=0;
      while (index<startWay->nodes.size()) {
        if (startWay->nodes[index].id==startNodeId) {
          startLon=startWay->nodes[index].lon;
          startLat=startWay->nodes[index].lat;
          break;
        }

        index++;
      }

      assert(index<startWay->nodes.size());

      // Get end node
      std::cout << "Searching for node " << targetNodeId << " in way " << targetWayId << "..." << std::endl;

      index=0;
      while (index<targetWay->nodes.size()) {
        if (targetWay->nodes[index].id==targetNodeId) {
          targetLon=targetWay->nodes[index].lon;
          targetLat=targetWay->nodes[index].lat;
          break;
        }

        index++;
      }

      assert(index<targetWay->nodes.size());

      /*
      RNode(Id id,
            double lon, double lat,
            const ObjectRef& reference,
            Id prev)
       : id(id),
         lon(lon),
         lat(lat),
         currentCost(0),
         estimateCost(0),
         overallCost(0),
         ref(reference),
         prev(prev)
       */
      // RNode - routingNode (?)
      RNode node=RNode(startNodeId,
                       startLon,
                       startLat,
                       ObjectRef(startWayId,refWay),
                       0);

      node.currentCost=0.0;
      node.estimateCost=GetSphericalDistance(startLon,
                                             startLat,
                                             targetLon,
                                             targetLat);
      node.overallCost=node.currentCost+node.estimateCost;

      openList.insert(node); // List of available nodes (?)
      openMap[openList.begin()->id]=openList.begin();

      currentWay=startWay;

      std::vector<RNode> follower;

      follower.reserve(1000);

      bool cachedFollower=false;

      // ==============================/INITIALIZATION=======================================


      do {
        //
        // Take entry from open list with lowest cost
        //

        RNode currentNode=*openList.begin();

        /*
        std::cout << "S:   " << openList.size() << std::endl;
        std::cout << "ID:  " << current.id << std::endl;
        std::cout << "REF: " << current.ref.id << std::endl;
        std::cout << "PRV: " << current.prev << std::endl;
        std::cout << "CC:  " << current.currentCost << std::endl;
        std::cout << "EC:  " << current.estimateCost << std::endl;
        std::cout << "OC:  " << current.overallCost << std::endl;
          */

        openList.erase(openList.begin());
        openMap.erase(currentNode.id);

        //
        // Place all followers on list
        //

        follower.clear();

        // Get joint nodes in same way/area
        if (currentWay->GetId()!=currentNode.ref.id) {
          if (!osmscout::GetWay(waysIndexed,
                                path,
                                waysCache,
                                currentNode.ref.id,
                                currentWay)) {
            return false;
          }

          cachedFollower=false;
        }

        if (!profile.CanUse(currentWay->GetType())) {
          continue;
        }

        if (currentWay->IsArea()) {
          for (size_t i=0; i<currentWay->nodes.size(); ++i) {
            if (currentWay->nodes[i].id!=currentNode.id) {

              std::map<Id,RNode>::iterator closeEntry=closeMap.find(currentWay->nodes[i].id); // Already used nodes (all of them and each has previous node id, so in the end one can go back from the end node to start)

              if (closeEntry!=closeMap.end()) {
                continue;
              }

              follower.push_back(RNode(currentWay->nodes[i].id,
                                       currentWay->nodes[i].lon,
                                       currentWay->nodes[i].lat,
                                       ObjectRef(currentWay->GetId(),refWay),
                                       currentNode.id));
            }
          }
        }
        else {
          for (size_t i=0; i<currentWay->nodes.size(); ++i) {
            if (currentWay->nodes[i].id==currentNode.id) {
              if (i>0 && !currentWay->IsOneway()) {
                std::map<Id,RNode>::iterator closeEntry=closeMap.find(currentWay->nodes[i-1].id);

                if (closeEntry==closeMap.end()) {
                  follower.push_back(RNode(currentWay->nodes[i-1].id,
                                           currentWay->nodes[i-1].lon,
                                           currentWay->nodes[i-1].lat,
                                           ObjectRef(currentWay->GetId(),refWay),
                                           currentNode.id));
                }
              }

              if (i<currentWay->nodes.size()-1) {
                std::map<Id,RNode>::iterator closeEntry=closeMap.find(currentWay->nodes[i+1].id);

                if (closeEntry==closeMap.end()) {
                  follower.push_back(RNode(currentWay->nodes[i+1].id,
                                           currentWay->nodes[i+1].lon,
                                           currentWay->nodes[i+1].lat,
                                           ObjectRef(currentWay->GetId(),refWay),
                                           currentNode.id));
                }
              }

              break;
            }
          }
        }

        // Get joint ways and areas

        if (!cachedFollower) {
          std::map<Id,Follower>::const_iterator cacheEntry=candidatesCache.find(currentNode.ref.id);

          if (cacheEntry==candidatesCache.end()) {
            std::pair<std::map<Id,Follower >::iterator,bool> result;

            result=candidatesCache.insert(std::pair<Id,Follower>(currentNode.ref.id,Follower()));

            if (!GetJoints(nodeUseIndex,
                           nodeUseCache,
                           currentNode.ref.id,
                           result.first->second.ways)) {
              return false;
            }

            cacheEntry=result.first;
          }

          if (!osmscout::GetWays(waysIndexed,
                                 path,
                                 waysCache,
                                 cacheEntry->second.ways,
                                 followWays)) {
            return false;
          }

          cachedFollower=true;
        }

        // Get joint nodes in joint way/area

        for (std::vector<WayPtr>::const_iterator iter=followWays.begin();
             iter!=followWays.end();
             ++iter) {
          const Way* way=*iter;

          if (!profile.CanUse(way->GetType())) {
            continue;
          }

          if (way->IsArea()) {
            for (size_t i=0; i<way->nodes.size(); i++) {
              if (way->nodes[i].id!=currentNode.id) {
                std::map<Id,RNode>::iterator closeEntry=closeMap.find(way->nodes[i].id);

                if (closeEntry!=closeMap.end()) {
                  continue;
                }

                follower.push_back(RNode(way->nodes[i].id,
                                         way->nodes[i].lon,
                                         way->nodes[i].lat,
                                         ObjectRef(way->GetId(),refWay),
                                         currentNode.id));
              }
            }
          }
          else {
            for (size_t i=0; i<way->nodes.size(); ++i) {
              if (way->nodes[i].id==currentNode.id  &&
                  CanBeTurnedInto(*currentWay,way->nodes[i].id,way->GetId())) {

                if (i>0 && !way->IsOneway()) {
                  std::map<Id,RNode>::iterator closeEntry=closeMap.find(way->nodes[i-1].id);

                  if (closeEntry==closeMap.end()) {
                    follower.push_back(RNode(way->nodes[i-1].id,
                                             way->nodes[i-1].lon,
                                             way->nodes[i-1].lat,
                                             ObjectRef(way->GetId(),refWay),
                                             currentNode.id));
                  }
                }

                if (i<way->nodes.size()-1) {
                  std::map<Id,RNode>::iterator closeEntry=closeMap.find(way->nodes[i+1].id);

                  if (closeEntry==closeMap.end()) {
                    follower.push_back(RNode(way->nodes[i+1].id,
                                             way->nodes[i+1].lon,
                                             way->nodes[i+1].lat,
                                             ObjectRef(way->GetId(),refWay),
                                             currentNode.id));
                  }
                }

                break;
              }
            }
          }
        }

        for (std::vector<RNode>::iterator iter=follower.begin();
             iter!=follower.end();
             ++iter) {
          double currentCost=currentNode.currentCost+
                             profile.GetCostFactor(currentWay->GetType())*
                             GetSphericalDistance(currentNode.lon,
                                                  currentNode.lat,
                                                  iter->lon,
                                                  iter->lat);

          if (currentWay->GetId()!=iter->id) {
            currentCost+=profile.GetTurnCostFactor();
          }

          std::map<Id,RNodeRef>::iterator openEntry=openMap.find(iter->id);

          if (openEntry!=openMap.end() &&
              openEntry->second->currentCost<=currentCost) {
            continue;
          }

          double estimateCost=profile.GetMinCostFactor()*
                              GetSphericalDistance(iter->lon,iter->lat,targetLon,targetLat);
          double overallCost=currentCost+estimateCost;

          if (openEntry!=openMap.end()) {
            iter->prev=currentNode.id;
            iter->currentCost=currentCost;
            iter->estimateCost=estimateCost;
            iter->overallCost=overallCost;

            openList.erase(openEntry->second);
            std::pair<RNodeRef,bool> result=openList.insert(*iter);

            openEntry->second=result.first;
          }
          else {
            iter->currentCost=currentCost;
            iter->estimateCost=estimateCost;
            iter->overallCost=overallCost;

            std::pair<RNodeRef,bool> result=openList.insert(*iter);
            openMap[iter->id]=result.first;
          }
        }

        //
        // Added current node to close map
        //

        closeMap[currentNode.id]=currentNode;

        //
        // Check if finished
        //

        if (currentNode.id==targetNodeId) {
          /*
          std::cout << "Final Path:" << std::endl;
          std::cout << "-----------" << std::endl;
          std::cout << current.currentCost << "Km" << std::endl;
          std::cout << waysCache.size() << " ways cached" << std::endl;
          std::cout << std::endl;*/

          std::list<RouteStep> steps;

          while (currentNode.prev!=0) {
            RouteStep step;

            step.wayId=currentNode.ref.id;
            step.nodeId=currentNode.id;

            steps.push_back(step);
            currentNode=closeMap.find(currentNode.prev)->second;
          }

          route.AddEntry(startWayId,startNodeId);
          for (std::list<RouteStep>::reverse_iterator step=steps.rbegin();
               step!=steps.rend();
               ++step) {
            route.AddEntry(step->wayId,step->nodeId);

            /*
            std::cout << "node " << step->nodeId << "( way " << step->wayId << ")";

            Way way;

            GetWay(step->wayId,way);

            for (size_t i=0; i<way.tags.size(); i++) {
              if (way.tags[i].key==tagName) {
                std::cout << " " << way.tags[i].value;
              }
              else if (way.tags[i].key==tagRef) {
                std::cout << " " << way.tags[i].value;
              }
            }

            std::cout << std::endl;*/
          }

          clock.Stop();

          std::cout << "Time: " << clock << std::endl;

          std::cout << "=========== Routing end ==============" << std::endl;
          return true;
        }
      } while (!openList.empty());

      std::cout << "No route found!" << std::endl;
      std::cout << "=========== Routing end ==============" << std::endl;

      return false;
  }

  bool Database::TransformRouteDataToRouteDescription(const RouteData& data,
                                                      RouteDescription& description)
  {
    if (!IsOpen()) {
      return false;
    }

    WayRef                                           way,newWay;
    Id                                               node=0,newNode=0;
    std::list<RouteData::RouteEntry>::const_iterator iter;
    double                                           distance=0.0;
    double                                           lastDistance=0.0;

    description.Clear();

    if (data.Entries().empty()) {
      return true;
    }

    iter=data.Entries().begin();

    if (!GetWay(iter->GetWayId(),way)) {
      return false;
    }

    // Find the starting node
    for (size_t i=0; i<way->nodes.size(); i++) {
      if (way->nodes[i].id==iter->GetNodeId()) {
        node=i;
        break;
      }
    }

    // Lets start at the starting node (suprise, suprise ;-))
    description.AddStep(0.0,0.0,RouteDescription::start,way->GetName(),way->GetRefName());
    description.AddStep(0.0,0.0,RouteDescription::drive,way->GetName(),way->GetRefName());

    iter++;

    // For every step in the route...
    for ( /* no code */ ;iter!=data.Entries().end(); ++iter, way=newWay, node=newNode) {
      // Find the corresponding way (which may be the old way?)
      if (iter->GetWayId()!=way->GetId()) {
        if (!GetWay(iter->GetWayId(),newWay)) {
          return false;
        }
      }
      else {
        newWay=way;
      }

      // Find the current node in the new way and calculate the distance
      // between the old point and the new point
      for (size_t i=0; i<newWay->nodes.size(); i++) {
        if (newWay->nodes[i].id==iter->GetNodeId()) {
          distance+=GetEllipsoidalDistance(way->nodes[node].lon,way->nodes[node].lat,
                                           newWay->nodes[i].lon,newWay->nodes[i].lat);
          newNode=i;
        }
      }

      // We skip steps where street doe not have any names
      if (newWay->GetName().empty() &&
          newWay->GetRefName().empty()) {
        continue;
      }

      // We didn't change street name, so we do not create a new entry...
      if (!way->GetName().empty() &&
          way->GetName()==newWay->GetName()) {
        continue;
      }

      // We didn't change ref name, so we do not create a new entry...
      if (!way->GetRefName().empty()
          && way->GetRefName()==newWay->GetRefName()) {
        continue;
      }

      description.AddStep(distance,distance-lastDistance,RouteDescription::switchRoad,newWay->GetName(),newWay->GetRefName());
      description.AddStep(distance,distance-lastDistance,RouteDescription::drive,newWay->GetName(),newWay->GetRefName());
      lastDistance=distance;
    }

    // We reached the destination!
    description.AddStep(distance,distance-lastDistance,RouteDescription::reachTarget,newWay->GetName(),newWay->GetRefName());

    return true;
  }

  bool Database::TransformRouteDataToWay(const RouteData& data,
                                         Way& way)
  {
    TypeId routeType;
    Way    tmp;

    routeType=typeConfig->GetWayTypeId("_route");

    assert(routeType!=typeIgnore);

    way=tmp;

    way.SetId(0);
    way.SetType(routeType);
    way.nodes.reserve(data.Entries().size());

    if (data.Entries().empty()) {
      return true;
    }

    for (std::list<RouteData::RouteEntry>::const_iterator iter=data.Entries().begin();
         iter!=data.Entries().end();
         ++iter) {
      WayRef w;

      if (!GetWay(iter->GetWayId(),w)) {
        return false;
      }

      for (size_t i=0; i<w->nodes.size(); i++) {
        if (w->nodes[i].id==iter->GetNodeId()) {
          way.nodes.push_back(w->nodes[i]);
          break;
        }
      }
    }

    return true;
  }
  void Database::DumpStatistics()
  {
    nodeDataFile.DumpStatistics();
    wayDataFile.DumpStatistics();
    relationDataFile.DumpStatistics();

    areaAreaIndex.DumpStatistics();
    areaNodeIndex.DumpStatistics();
    areaWayIndex.DumpStatistics();
    cityStreetIndex.DumpStatistics();
    waterIndex.DumpStatistics();
  }
}
