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

#include <osmscout/TypeConfig.h>

#include <cassert>
#include <iostream>
namespace osmscout {

  Condition::~Condition()
  {
    // no code
  }

  NotCondition::NotCondition(Condition* condition)
  : condition(condition)
  {
    // no code
  }

  bool NotCondition::Evaluate(const std::map<TagId,std::string>& tagMap) const
  {
    return !condition->Evaluate(tagMap);
  }

  AndCondition::AndCondition()
  {
    // no code
  }

  void AndCondition::AddCondition(Condition* condition)
  {
    conditions.push_back(condition);
  }

  bool AndCondition::Evaluate(const std::map<TagId,std::string>& tagMap) const
  {
    for (std::list<ConditionRef>::const_iterator condition=conditions.begin();
         condition!=conditions.end();
         ++condition) {
      if (!(*condition)->Evaluate(tagMap)) {
        return false;
      }
    }

    return true;
  }

  OrCondition::OrCondition()
  {
    // no code
  }

  void OrCondition::AddCondition(Condition* condition)
  {
    conditions.push_back(condition);
  }

  bool OrCondition::Evaluate(const std::map<TagId,std::string>& tagMap) const
  {
    for (std::list<ConditionRef>::const_iterator condition=conditions.begin();
         condition!=conditions.end();
         ++condition) {
      if ((*condition)->Evaluate(tagMap)) {
        return true;
      }
    }

    return false;
  }

  ExistsCondition::ExistsCondition(TagId tag)
  : tag(tag)
  {
    // no code
  }

  bool ExistsCondition::Evaluate(const std::map<TagId,std::string>& tagMap) const
  {
    return tagMap.find(tag)!=tagMap.end();
  }

  EqualsCondition::EqualsCondition(TagId tag,
                                   const std::string& tagValue)
  : tag(tag),
    tagValue(tagValue)
  {
    // no code
  }

  bool EqualsCondition::Evaluate(const std::map<TagId,std::string>& tagMap) const
  {
    std::map<TagId,std::string>::const_iterator t;

    t=tagMap.find(tag);

    if (t==tagMap.end()) {
      return false;
    }

    return t->second==tagValue;
  }

  NotEqualsCondition::NotEqualsCondition(TagId tag,
                                         const std::string& tagValue)
  : tag(tag),
    tagValue(tagValue)
  {
    // no code
  }

  bool NotEqualsCondition::Evaluate(const std::map<TagId,std::string>& tagMap) const
  {
    std::map<TagId,std::string>::const_iterator t;

    t=tagMap.find(tag);

    if (t==tagMap.end()) {
      return false;
    }

    return t->second==tagValue;
  }

  IsInCondition::IsInCondition(TagId tag)
  : tag(tag)
  {
    // no code
  }

  void IsInCondition::AddTagValue(const std::string& tagValue)
  {
    tagValues.insert(tagValue);
  }

  bool IsInCondition::Evaluate(const std::map<TagId,std::string>& tagMap) const
  {
    std::map<TagId,std::string>::const_iterator t;

    t=tagMap.find(tag);

    if (t==tagMap.end()) {
      return false;
    }

    return tagValues.find(t->second)!=tagValues.end();
  }

  TagInfo::TagInfo()
   : id(0)
  {
  }

  TagInfo::TagInfo(const std::string& name,
                   bool internalOnly)
   : id(0),
     name(name),
     internalOnly(internalOnly)
  {
    // no code
  }

  TagInfo& TagInfo::SetId(TagId id)
  {
    this->id=id;

    return *this;
  }

  TypeInfo::TypeInfo()
   : id(0),
     canBeNode(false),
     canBeWay(false),
     canBeArea(false),
     canBeRelation(false),
     canBeRoute(false),
     canBeIndexed(false),
     consumeChildren(false),
     optimizeLowZoom(false),
     multipolygon(false),
     ignore(false)
  {
    // no code
  }

  TypeInfo::~TypeInfo()
  {
    // no code
  }

  TypeInfo& TypeInfo::SetId(TypeId id)
  {
    this->id=id;

    return *this;
  }

  TypeInfo& TypeInfo::SetType(const std::string& name)
  {
    this->name=name;

    return *this;
  }

  TypeInfo& TypeInfo::AddCondition(unsigned char types,
                                   Condition* condition)
  {
    TypeCondition typeCondition;

    if (types & typeNode) {
      canBeNode=true;
    }

    if (types & typeWay) {
      canBeWay=true;
    }

    if (types & typeArea) {
      canBeArea=true;
    }

    if (types & typeRelation) {
      canBeRelation=true;
    }

    typeCondition.types=types;
    typeCondition.condition=condition;

    conditions.push_back(typeCondition);

    return *this;
  }

  TypeConfig::TypeConfig()
   : nextTagId(0),
     nextTypeId(0)
  {
    // Make sure, that this is always registered first.
    // It assures that id 0 is always reserved for tagIgnore
    RegisterTagForInternalUse("");

    RegisterTagForExternalUse("admin_level");
    RegisterTagForExternalUse("boundary");
    RegisterTagForExternalUse("bridge");
    RegisterTagForExternalUse("highway");
    RegisterTagForExternalUse("layer");
    RegisterTagForExternalUse("name");
    RegisterTagForExternalUse("natural");
    RegisterTagForExternalUse("oneway");
    RegisterTagForExternalUse("place");
    RegisterTagForExternalUse("place_name");
    RegisterTagForExternalUse("ref");
    RegisterTagForExternalUse("restriction");
    RegisterTagForExternalUse("tunnel");
    RegisterTagForExternalUse("type");
    RegisterTagForExternalUse("width");
    RegisterTagForExternalUse("area");
    RegisterTagForExternalUse("addr:housenumber");

    TypeInfo ignore;
    TypeInfo route;

    // Make sure, that this is always registered first.
    // It assures that id 0 is always reserved for typeIgnore
    ignore.SetType("");

    AddTypeInfo(ignore);

    route.SetType("_route")
         .CanBeWay(true);

    AddTypeInfo(route);

    tagAdminLevel=GetTagId("admin_level");
    tagBoundary=GetTagId("boundary");
    tagBridge=GetTagId("bridge");
    tagLayer=GetTagId("layer");
    tagName=GetTagId("name");
    tagOneway=GetTagId("oneway");
    tagPlace=GetTagId("place");
    tagPlaceName=GetTagId("place_name");
    tagRef=GetTagId("ref");
    tagTunnel=GetTagId("tunnel");
    tagType=GetTagId("type");
    tagWidth=GetTagId("width");
    tagArea=GetTagId("area");
    tagHouseNr=GetTagId("addr:housenumber");

    assert(tagAdminLevel!=tagIgnore);
    assert(tagBoundary!=tagIgnore);
    assert(tagBridge!=tagIgnore);
    assert(tagLayer!=tagIgnore);
    assert(tagName!=tagIgnore);
    assert(tagOneway!=tagIgnore);
    assert(tagPlace!=tagIgnore);
    assert(tagPlaceName!=tagIgnore);
    assert(tagRef!=tagIgnore);
    assert(tagTunnel!=tagIgnore);
    assert(tagType!=tagIgnore);
    assert(tagWidth!=tagIgnore);
    assert(tagArea!=tagIgnore);
    assert(tagHouseNr!=tagIgnore);
  }

  TypeConfig::~TypeConfig()
  {
    // no code
  }

  const std::vector<TagInfo>& TypeConfig::GetTags() const
  {
    return tags;
  }

  const std::vector<TypeInfo>& TypeConfig::GetTypes() const
  {
    return types;
  }

  TagId TypeConfig::RegisterTagForInternalUse(const std::string& tagName)
  {
    std::map<std::string,TagId>::const_iterator mapping=stringToTagMap.find(tagName);

    if (mapping!=stringToTagMap.end()) {
      return mapping->second;
    }

    TagInfo tagInfo(tagName,true);

    if (tagInfo.GetId()==0) {
      tagInfo.SetId(nextTagId);

      nextTagId++;
    }
    else {
      nextTagId=std::max(nextTagId,(TagId)(tagInfo.GetId()+1));
    }

    tags.push_back(tagInfo);
    stringToTagMap[tagInfo.GetName()]=tagInfo.GetId();

    return tagInfo.GetId();
  }

  TagId TypeConfig::RegisterTagForExternalUse(const std::string& tagName)
  {
    std::map<std::string,TagId>::const_iterator mapping=stringToTagMap.find(tagName);

    if (mapping!=stringToTagMap.end()) {
      // TODO: Set to externalUse!
      return mapping->second;
    }

    TagInfo tagInfo(tagName,false);

    if (tagInfo.GetId()==0) {
      tagInfo.SetId(nextTagId);

      nextTagId++;
    }
    else {
      nextTagId=std::max(nextTagId,(TagId)(tagInfo.GetId()+1));
    }

    tags.push_back(tagInfo);
    stringToTagMap[tagInfo.GetName()]=tagInfo.GetId();

    return tagInfo.GetId();
  }

  void TypeConfig::RestoreTagInfo(const TagInfo& tagInfo)
  {
    // We have same tags, that are already and always
    // registered in the constructor, we skip them here...
    if (stringToTagMap.find(tagInfo.GetName())!=stringToTagMap.end()) {
      return;
    }

    assert(stringToTagMap.find(tagInfo.GetName())==stringToTagMap.end());
    assert(tagInfo.GetId()!=0 ||
           (tagInfo.GetId()==0 && tagInfo.GetName().empty()));

    nextTagId=std::max(nextTagId,(TagId)(tagInfo.GetId()+1));

    if (tags.size()>=tagInfo.GetId()) {
      tags.resize(tagInfo.GetId()+1);
    }

    tags[tagInfo.GetId()]=tagInfo;
    stringToTagMap[tagInfo.GetName()]=tagInfo.GetId();
  }

  TypeConfig& TypeConfig::AddTypeInfo(TypeInfo& typeInfo)
  {
    if (nameToTypeMap.find(typeInfo.GetName())!=nameToTypeMap.end()) {
      return *this;
    }

    if (typeInfo.GetId()==0) {
      typeInfo.SetId(nextTypeId);

      nextTypeId++;
    }
    else {
      nextTypeId=std::max(nextTypeId,(TypeId)(typeInfo.GetId()+1));
    }

    //std::cout << "Type: " << typeInfo.GetId() << " " << typeInfo.GetName() << std::endl;

    types.push_back(typeInfo);
    nameToTypeMap[typeInfo.GetName()]=typeInfo;

    idToTypeMap[typeInfo.GetId()]=typeInfo;

    return *this;
  }

  TypeId TypeConfig::GetMaxTypeId() const
  {
    if (nextTypeId==0) {
      return 0;
    }
    else {
      return nextTypeId-1;
    }
  }

  TagId TypeConfig::GetTagId(const char* name) const
  {
    std::map<std::string,TagId>::const_iterator iter=stringToTagMap.find(name);

    if (iter!=stringToTagMap.end()) {
      return iter->second;
    }
    else {
      return tagIgnore;
    }
  }

  const TagInfo& TypeConfig::GetTagInfo(TagId id) const
  {
    assert(id<tags.size());

    return tags[id];
  }

  const TypeInfo& TypeConfig::GetTypeInfo(TypeId id) const
  {
    assert(id<types.size());

    return types[id];
  }

  void TypeConfig::ResolveTags(const std::map<TagId,std::string>& map,
                               std::vector<Tag>& tags) const
  {
    tags.clear();

    for (std::map<TagId,std::string>::const_iterator t=map.begin();
         t!=map.end();
         ++t) {
      if (GetTagInfo(t->first).IsInternalOnly()) {
        continue;
      }

      Tag tag;

      tag.key=t->first;
      tag.value=t->second;

      tags.push_back(tag);
    }
  }

  bool TypeConfig::GetNodeTypeId(const std::map<TagId,std::string>& tagMap,
                                 TypeId &typeId) const
  {
    typeId=typeIgnore;

    if (tagMap.empty()) {
      return false;
    }

    for (size_t i=0; i<types.size(); i++) {
      if (!types[i].HasConditions() ||
          !types[i].CanBeNode()) {
        continue;
      }

      for (std::list<TypeInfo::TypeCondition>::const_iterator cond=types[i].GetConditions().begin();
           cond!=types[i].GetConditions().end();
           ++cond) {
        if (!(cond->types & TypeInfo::typeNode)) {
          continue;
        }

        if (cond->condition->Evaluate(tagMap)) {
          typeId=types[i].GetId();
          return true;
        }
      }
    }

    return false;
  }

  bool TypeConfig::GetWayAreaTypeId(const std::map<TagId,std::string>& tagMap,
                                    TypeId &wayType,
                                    TypeId &areaType) const
  {
    wayType=typeIgnore;
    areaType=typeIgnore;

    if (tagMap.empty()) {
      return false;
    }

    for (size_t i=0; i<types.size(); i++) {
      if (!((types[i].CanBeWay() ||
             types[i].CanBeArea()) &&
             types[i].HasConditions())) {
        continue;
      }

      for (std::list<TypeInfo::TypeCondition>::const_iterator cond=types[i].GetConditions().begin();
           cond!=types[i].GetConditions().end();
           ++cond) {
        if (!((cond->types & TypeInfo::typeWay) || (cond->types & TypeInfo::typeArea))) {
          continue;
        }

        if (cond->condition->Evaluate(tagMap)) {
          if (wayType==typeIgnore &&
              (cond->types & TypeInfo::typeWay)) {
            wayType=types[i].GetId();
          }

          if (areaType==typeIgnore &&
              (cond->types & TypeInfo::typeArea)) {
            areaType=types[i].GetId();
          }

          if (wayType!=typeIgnore &&
              areaType!=typeIgnore) {
            return true;
          }
        }
      }
    }

    return wayType!=typeIgnore || areaType!=typeIgnore;
  }

  bool TypeConfig::GetRelationTypeId(const std::map<TagId,std::string>& tagMap,
                                     TypeId &typeId) const
  {
    typeId=typeIgnore;

    if (tagMap.empty()) {
      return false;
    }

    std::map<TagId,std::string>::const_iterator relationType=tagMap.find(tagType);

    if (relationType!=tagMap.end() &&
        relationType->second=="multipolygon") {
      for (size_t i=0; i<types.size(); i++) {
        if (!types[i].HasConditions() ||
            !types[i].CanBeArea()) {
          continue;
        }

        for (std::list<TypeInfo::TypeCondition>::const_iterator cond=types[i].GetConditions().begin();
             cond!=types[i].GetConditions().end();
             ++cond) {
          if (!(cond->types & TypeInfo::typeArea)) {
            continue;
          }

          if (cond->condition->Evaluate(tagMap)) {
            typeId=types[i].GetId();
            return true;
          }
        }
      }
    }
    else {
      for (size_t i=0; i<types.size(); i++) {
        if (!types[i].HasConditions() ||
            !types[i].CanBeRelation()) {
          continue;
        }

        for (std::list<TypeInfo::TypeCondition>::const_iterator cond=types[i].GetConditions().begin();
             cond!=types[i].GetConditions().end();
             ++cond) {
          if (!(cond->types & TypeInfo::typeRelation)) {
            continue;
          }

          if (cond->condition->Evaluate(tagMap)) {
            typeId=types[i].GetId();
            return true;
          }
        }
      }
    }

    return false;
  }

  TypeId TypeConfig::GetNodeTypeId(const std::string& name) const
  {
    std::map<std::string,TypeInfo>::const_iterator iter=nameToTypeMap.find(name);

    if (iter!=nameToTypeMap.end() &&
        iter->second.CanBeNode()) {
      return iter->second.GetId();
    }

    return typeIgnore;
  }

  TypeId TypeConfig::GetWayTypeId(const std::string& name) const
  {
    std::map<std::string,TypeInfo>::const_iterator iter=nameToTypeMap.find(name);

    if (iter!=nameToTypeMap.end() &&
        iter->second.CanBeWay()) {
      return iter->second.GetId();
    }

    return typeIgnore;
  }

  TypeId TypeConfig::GetAreaTypeId(const std::string& name) const
  {
    std::map<std::string,TypeInfo>::const_iterator iter=nameToTypeMap.find(name);

    if (iter!=nameToTypeMap.end() &&
        iter->second.CanBeArea()) {
      return iter->second.GetId();
    }

    return typeIgnore;
  }

  TypeId TypeConfig::GetRelationTypeId(const std::string& name) const
  {
    std::map<std::string,TypeInfo>::const_iterator iter=nameToTypeMap.find(name);

    if (iter!=nameToTypeMap.end() &&
        iter->second.CanBeRelation()) {
      return iter->second.GetId();
    }

    return typeIgnore;
  }

  void TypeConfig::GetRoutables(std::set<TypeId>& types) const
  {
    types.clear();

    for (std::vector<TypeInfo>::const_iterator type=this->types.begin();
         type!=this->types.end();
         ++type) {
      if (type->CanBeRoute()) {
        types.insert(type->GetId());
      }
    }
  }

  void TypeConfig::GetIndexables(std::set<TypeId>& types) const
  {
    types.clear();

    for (std::vector<TypeInfo>::const_iterator type=this->types.begin();
         type!=this->types.end();
         ++type) {
      if (type->CanBeIndexed()) {
        types.insert(type->GetId());
      }
    }
  }
}
