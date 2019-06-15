#pragma once

#include <list>
#include <map>
#include <string>

template <class Item>
class Layer
{
public:
  //
  static constexpr const char* DefaultLayer = "default";

protected:
  std::string current_layer = DefaultLayer;
  using Ptr                 = std::shared_ptr<Item>;
  using ItemList            = std::list<Ptr>;
  std::map<std::string, ItemList> item_list;

public:
  // レイヤー切り替え
  bool bind(std::string layer)
  {
    if (current_layer != layer)
    {
      current_layer = layer;
      return true;
    }
    return false;
  }

  //
  void clear(std::string layer)
  {
    auto& target = item_list[layer];
    target.clear();
  }

  //
  void appen(Ptr i)
  {
    auto& target = item_list[current_layer];
    target.push_back(i);
  }

  //
  bool erase(Ptr id)
  {
    auto& target = item_list[current_layer];
    for (auto p = target.begin(); p != target.end(); p++)
    {
      if (*p == id)
      {
        target.erase(p);
        return true;
      }
    }
    return false;
  }

  //
  ItemList& getCurrent() { return item_list[current_layer]; }
};

//
