#include <string>
#include <InfluxDbCloud.h>
#include <InfluxDbClient.h>
using f_int_t = int (*)();

class Sensor
{
private:
  String _name;
  String _unit;
  f_int_t _cb;
  Point _point = Point("");

public:
  Sensor(String name, String unit, f_int_t targetCb)
  {
    _name = name;
    _unit = unit;
    _cb = targetCb;
    _point = Point(name);
  }

  void addTag(String key, String value)
  {
    _point.addTag(key, value);
  }

  void addField(String key, int value)
  {
    _point.addField(key, value);
  }

  void clearFields()
  {
    _point.clearFields();
  }

  String getName()
  {
    return _name;
  }

  Point getPoint()
  {
    return _point;
  }

  Point& getPointRef()
  {
    Point& ref = _point;
    return ref;
  }

  String getUnit()
  {
    return _unit;
  }

  int getValue()
  {
    return _cb();
  }
};
