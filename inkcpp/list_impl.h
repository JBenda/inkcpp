#pragma once

#include "list.h"

namespace ink::runtime::internal {
  class list_table;
  class value;
  class list_impl final : public list_interface {
  public:
    list_impl(list_table& table, int lid) : list_interface(table, lid) {}
    int get_lid() const { return _list; }
    
    bool contains(const char* flag_name) const override;
    void add(const char* flag_name) override; 
    void remove(const char* flag_name) override;

    list_interface::iterator begin() const override {
      return ++new_iterator(nullptr, 0);
    }

    list_interface::iterator begin(const char* list_name) const override;
    list_interface::iterator end() const override {
      return new_iterator(nullptr, -1);
    }

  private:
    friend ink::runtime::internal::value;
    
    /// @todo wrong iteration order, first lists then flags
    void next(const char*& flag_name, const char*& list_name, int& i) const override;
    
  };
}
