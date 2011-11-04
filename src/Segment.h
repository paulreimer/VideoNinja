#pragma once
#include "json.h"

class Segment
{
public:
  Segment();
  Segment(const std::string _name,
          const double _startPos,
          const double _endPos);

  Segment(const Segment& _segment);

  Json::Value toJSON();
  void fromJSON(Json::Value json);

  void clear();

  std::string name;  
  double startPos;
  double endPos;
};

