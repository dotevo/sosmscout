#ifndef OSMSCOUT_RELATION_H
#define OSMSCOUT_RELATION_H

/*
  This source is part of the libosmscout library
  Copyright (C) 2010  Tim Teulings

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

#include <osmscout/Point.h>

#include <osmscout/SegmentAttributes.h>
#include <osmscout/TypeConfig.h>

#include <osmscout/util/FileScanner.h>
#include <osmscout/util/FileWriter.h>
#include <osmscout/util/Reference.h>

namespace osmscout {

  /**
    Representation of an relation
    */
  class Relation : public Referencable
  {
  public:
    class Role
    {
    public:
      SegmentAttributes  attributes;
      std::string        role;
      std::vector<Point> nodes;

    public:
      inline const SegmentAttributes& GetAttributes() const
      {
        return attributes;
      }

      inline TypeId GetType() const
      {
        return attributes.GetType();
      }

      inline uint16_t GetFlags() const
      {
        return attributes.GetFlags();
      }

      inline bool IsArea() const
      {
        return attributes.IsArea();
      }

      inline std::string GetName() const
      {
        return attributes.GetName();
      }

      inline std::string GetRefName() const
      {
        return attributes.GetRefName();
      }

      inline int8_t GetLayer() const
      {
        return attributes.GetLayer();
      }

      inline bool IsBridge() const
      {
        return attributes.IsBridge();
      }

      inline bool IsTunnel() const
      {
        return attributes.IsTunnel();
      }

      inline bool IsOneway() const
      {
        return attributes.IsOneway();
      }
    };

  private:
    Id                 id;

  public:
    SegmentAttributes  attributes;
    std::vector<Role>  roles;

  public:
    inline Relation()
    {
      // no code
    }

    inline Id GetId() const
    {
      return id;
    }

    inline const SegmentAttributes& GetAttributes() const
    {
      return attributes;
    }

    inline TypeId GetType() const
    {
      return attributes.GetType();
    }

    inline uint16_t GetFlags() const
    {
      return attributes.GetFlags();
    }

    inline bool IsArea() const
    {
      return attributes.IsArea();
    }

    inline std::string GetName() const
    {
      return attributes.GetName();
    }

    inline std::string GetRefName() const
    {
      return attributes.GetRefName();
    }

    inline int8_t GetLayer() const
    {
      return attributes.GetLayer();
    }

    inline bool IsBridge() const
    {
      return attributes.IsBridge();
    }

    inline bool IsTunnel() const
    {
      return attributes.IsTunnel();
    }

    inline bool IsOneway() const
    {
      return attributes.IsOneway();
    }

    inline const std::vector<Tag>& GetTags() const
    {
      return attributes.GetTags();
    }

    bool GetCenter(double& lat,
                   double& lon) const;
    void GetBoundingBox(double& minLon,
                        double& maxLon,
                        double& minLat,
                        double& maxLat) const;

    void SetId(Id id);
    void SetType(TypeId type);

    bool Read(FileScanner& scanner);
    bool Write(FileWriter& writer) const;
  };

  typedef Ref<Relation> RelationRef;
}

#endif
