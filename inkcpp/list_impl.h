#pragma once

#include "list.h"

namespace ink::runtime::internal {
  class list_table;
  class value;
  class list_impl : public list_interface {
  public:
    list_impl(list_table& table, int lid) : list_interface(table, lid) {}
    int get_lid() const { return _list; }
    
    bool contains(const char* flag_name) const override final;
    void add(const char* flag_name) override final; 
    void remove(const char* flag_name) override final;

    list_interface::iterator begin() const override final {
      return ++new_iterator(nullptr, 0);
    }

    list_interface::iterator end() const override final {
      return new_iterator(nullptr, -1);
    }

  private:
    friend ink::runtime::internal::value;
    
    /// @todo wrong iteration order, first lists then flags
    void next(const char*& flag_name, const char*& list_name, int& i) const override final;
    
  };
}
