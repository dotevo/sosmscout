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

#include <osmscout/CityStreetIndex.h>

#include <QDebug>

#include <cassert>
#include <iostream>

#include <osmscout/Util.h>
#include <osmscout/util/StopClock.h>

namespace osmscout {

  CityStreetIndex::LocationVisitor::LocationVisitor(FileScanner& scanner)
  : scanner(scanner)
  {
    // no code
  }

  bool CityStreetIndex::LocationVisitor::Visit(const QString& locationName,
                                               const Loc &loc)
  {
    int pos;
    bool found;

    qDebug() << locationName;
    //std::string locationNameString = locationName.toUtf8();
    //std::cout << locationName.toUtf8();

    if (hashFunction!=NULL) {
      std::string hash = (*hashFunction)(locationName.toStdString());

      if (!hash.empty()) {
       // pos=hash.find(nameHash);
          pos = QString::fromStdString(hash).indexOf(QString::fromStdString(nameHash), 0, Qt::CaseInsensitive);
      }
      else {
        pos = locationName.indexOf(name, 0, Qt::CaseInsensitive);
      }
    }
    else {
      pos = locationName.indexOf(name, 0, Qt::CaseInsensitive);
    }

    if (startWith) {
      found=pos==0;
    }
    else {
      found=pos!=std::string::npos;
    }

    if (!found) {
      return true;
    }

    if (locations.size()>=limit) {
      limitReached=true;

      return false;
    }

    Location location;

    std::string locationNameStd = locationName.toStdString();
    //qDebug() << locationName;
    location.name = QString::fromUtf8(locationNameStd.c_str());

    //location.name=QString::fromUtf8(locationName.toStdString().c_str());

    for (std::list<Id>::const_iterator i=loc.nodes.begin();
         i!=loc.nodes.end();
         ++i) {
      location.references.push_back(ObjectRef(*i,refNode));
    }

    for (std::list<Id>::const_iterator i=loc.ways.begin();
         i!=loc.ways.end();
         ++i) {
      location.references.push_back(ObjectRef(*i,refWay));
    }

    // Build up path for each hit by following
    // the parent relation up to the top of the tree.

    FileOffset currentOffset;
    FileOffset regionOffset=loc.offset;

    if (!scanner.GetPos(currentOffset)) {
      return false;
    }

    while (!scanner.HasError() &&
           regionOffset!=0) {
      std::string nameStd;

      scanner.SetPos(regionOffset);
      scanner.Read(nameStd);
      scanner.ReadNumber(regionOffset);

      //std::cerr << nameStd << std::endl;

      QString name = QString::fromUtf8(nameStd.c_str());

      if (location.path.empty()) {
        // We dot not want something like "'Dortmund' in 'Dortmund'"!
        if (name!=locationName) {
          location.path.push_back(name);
        }
      }
      else {
        location.path.push_back(name);
      }
    }

    locations.push_back(location);
    //qDebug() << location.name;

    return scanner.SetPos(currentOffset);
  }

  CityStreetIndex::CityStreetIndex()
   : hashFunction(NULL)
  {
    // no code
  }

  CityStreetIndex::~CityStreetIndex()
  {
    // no code
  }

  bool CityStreetIndex::LoadRegion(FileScanner& scanner,
                                   LocationVisitor& visitor) const
  {
    std::string               name;
    FileOffset                offset;
    FileOffset                parentOffset;
    uint32_t                  childrenCount;
    uint32_t                  nodeCount;
    uint32_t                  wayCount;
    std::map<std::string,Loc> locations;

    if (!scanner.GetPos(offset) ||
      !scanner.Read(name) ||
      !scanner.ReadNumber(parentOffset)) {
      return false;
    }

    if (!scanner.ReadNumber(childrenCount)) {
      return false;
    }

    for (size_t i=0; i<childrenCount; i++) {
      if (!LoadRegion(scanner,visitor)) {
        return false;
      }
    }

    if (!scanner.ReadNumber(nodeCount)) {
      return false;
    }

    for (size_t i=0; i<nodeCount; i++) {
      std::string name;
      uint32_t    idCount;
      Id          lastId=0;

      if (!scanner.Read(name) ||
          !scanner.ReadNumber(idCount)) {
        return false;
      }

      locations[name].offset=offset;

      for (size_t j=0; j<idCount; j++) {
        Id id;

        if (!scanner.ReadNumber(id)) {
          return false;
        }

        locations[name].nodes.push_back(id+lastId);

        lastId=id;
      }
    }

    if (!scanner.ReadNumber(wayCount)) {
      return false;
    }

    for (size_t i=0; i<wayCount; i++) {
      std::string name;
      uint32_t    idCount;
      Id          lastId=0;

      if (!scanner.Read(name) ||
          !scanner.ReadNumber(idCount)) {
        return false;
      }

      locations[name].offset=offset;

      for (size_t j=0; j<idCount; j++) {
        Id id;

        if (!scanner.ReadNumber(id)) {
          return false;
        }

        locations[name].ways.push_back(id+lastId);

        lastId=id;
      }
    }

    for (std::map<std::string,Loc>::const_iterator l=locations.begin();
         l!=locations.end();
         ++l) {
        if (!visitor.Visit(QString::fromUtf8(l->first.c_str()), l->second)) {
        return true;
      }
    }

    return !scanner.HasError();
  }

  bool CityStreetIndex::LoadRegion(FileScanner& scanner,
                                   FileOffset offset,
                                   LocationVisitor& visitor) const
  {
    scanner.SetPos(offset);

    return LoadRegion(scanner,visitor);
  }

  bool CityStreetIndex::Load(const std::string& path,
                             std::string (*hashFunction) (std::string))
  {
    this->path=path;
    this->hashFunction=hashFunction;

    return true;
  }


  bool CityStreetIndex::GetMatchingAdminRegions(const QString& name,
                                                std::list<AdminRegion>& regions,
                                                size_t limit,
                                                bool& limitReached,
                                                bool startWith) const
  {
    std::string nameHash;

    limitReached=false;
    regions.clear();

    // if the user supplied a special hash function call it and use the result
    if (hashFunction!=NULL) {
      nameHash = (*hashFunction)(name.toStdString());
    }

    FileScanner   scanner;
    std::string   indexFile=AppendFileToDir(path,"nameregion.idx");

    if (!scanner.Open(indexFile)) {
      std::cerr << "Cannot open file '" << indexFile << "'!" << std::endl;
      return false;
    }

    uint32_t areaRefs;

    if (!scanner.ReadNumber(areaRefs)) {
      return false;
    }

    for (size_t i=0; i<areaRefs; i++) {
      QString regionName;
      std::string regionNameStd;
      uint32_t    entries;

      if (!scanner.Read(regionNameStd)) {
        return false;
      }

      if (!scanner.ReadNumber(entries)) {
        return false;
      }

      regionName = QString::fromUtf8(regionNameStd.c_str());

      for (size_t j=0; j<entries; j++) {
        Region   region;
        uint32_t type;

        region.name=regionName;

        if (!scanner.ReadNumber(type)) {
          return false;
        }

        region.reference.type=(RefType)type;

        if (!scanner.ReadNumber(region.reference.id)) {
          return false;
        }

        if (!scanner.ReadNumber(region.offset)) {
          return false;
        }


        bool                   found=false;
        std::string::size_type loc;

        // Calculate match

        if (hashFunction!=NULL) {
          std::string hash = (*hashFunction)(regionNameStd);

          if (!hash.empty()) {
            loc = QString::fromStdString(hash).indexOf(QString::fromStdString(nameHash), 0, Qt::CaseInsensitive);
          }
          else {
            loc = region.name.indexOf(name, 0, Qt::CaseInsensitive);
          }
        }
        else {
          loc = region.name.indexOf(name, 0, Qt::CaseInsensitive);
        }

        if (startWith) {
          found=loc==0;
        }
        else {
          found=loc!=std::string::npos;
        }

        // If match, Add to result

        if (found) {
          if (regions.size()>=limit) {
            limitReached=true;
          }
          else {
            AdminRegion adminRegion;

            adminRegion.reference=region.reference;
            adminRegion.offset=region.offset;
            adminRegion.name=region.name;

            regions.push_back(adminRegion);
          }
        }
      }
    }

    if (!scanner.Close()) {
      return false;
    }

    if (regions.empty()) {
      return true;
    }

    std::string regionFile=AppendFileToDir(path,"region.dat");

    if (!scanner.Open(regionFile)) {
      std::cerr << "Cannot open file '" << regionFile << "'!" << std::endl;
      return false;
    }

    // If there are results, build up path for each hit by following
    // the parent relation up to the top of the tree.

    for (std::list<AdminRegion>::iterator area=regions.begin();
         area!=regions.end();
         ++area) {
      FileOffset offset=area->offset;

      while (offset!=0) {
        QString name;
        std::string nameStd = name.toStdString();

        scanner.SetPos(offset);
        scanner.Read(nameStd);
        scanner.ReadNumber(offset);

        if (area->path.empty()) {
          if (name!=area->name) {
            area->path.push_back(nameStd);
          }
        }
        else {
          area->path.push_back(nameStd);
        }
      }
    }

    return !scanner.HasError() && scanner.Close();
  }

  bool CityStreetIndex::GetMatchingLocations(const AdminRegion& region,
                                             const QString& name,
                                             std::list<Location>& locations,
                                             size_t limit,
                                             bool& limitReached,
                                             bool startWith) const
  {
    FileScanner scanner;
    std::string file=AppendFileToDir(path,"region.dat");

    if (!scanner.Open(file)) {
      std::cerr << "Cannot open file '" << file << "'!" << std::endl;
      return false;
    }

    LocationVisitor locVisitor(scanner);

    locVisitor.name=name;
    locVisitor.startWith=startWith;
    locVisitor.limit=limit;
    locVisitor.limitReached=false;

    locVisitor.hashFunction=hashFunction;

    if (hashFunction!=NULL) {
      locVisitor.nameHash=(*hashFunction)(name.toStdString());
    }

    if (!LoadRegion(locVisitor.scanner,
                    region.offset,
                    locVisitor)) {
      return false;
    }

    locations=locVisitor.locations;
    limitReached=locVisitor.limitReached;

    return true;
  }

  void CityStreetIndex::DumpStatistics()
  {
    size_t memory=0;

    std::cout << "CityStreetIndex: Memory " << memory << std::endl;
  }
}
