#include "Segment.h"

//--------------------------------------------------------------
Segment::Segment()
{ clear(); }

//--------------------------------------------------------------
Segment::Segment(const std::string _name,
                 const double _startPos,
                 const double _endPos)
: name(_name)
, startPos(_startPos)
, endPos(_endPos)
{}

//--------------------------------------------------------------
Segment::Segment(const Segment& _segment)
: name(_segment.name)
, startPos(_segment.startPos)
, endPos(_segment.endPos)
{}

//--------------------------------------------------------------
void
Segment::clear()
{
  name = "";
  startPos = -1;
  endPos = -1;
}

//--------------------------------------------------------------
Json::Value
Segment::toJSON()
{
  Json::Value json;
  
  if (!name.empty())
    json["name"] = name;
  
  json["startPos"] = startPos;
  json["endPos"] = endPos;
  
  return json;
}

//--------------------------------------------------------------
void
Segment::fromJSON(Json::Value json)
{
  Json::Value val;
  
  val = json.get("name", "");
  if (!val.isNull())
    name = val.asString();
  
  val = json.get("startPos", -1);
  if (!val.isNull())
    startPos = val.asDouble();
  
  val = json.get("endPos", -1);
  if (!val.isNull())
    endPos = val.asDouble();
}
