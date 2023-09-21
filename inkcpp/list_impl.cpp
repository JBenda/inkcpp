#include "list_impl.h"
#include "list.h"
#include "list_table.h"

namespace ink::runtime::internal {
  bool list_impl::contains(const char* flag_name) const {
    auto flag = _list_table->toFlag(flag_name);
    inkAssert(flag.has_value(), "No flag with name found! '%s'", flag_name);
    return _list_table->has(list_table::list{_list}, *flag);
  }

  void list_impl::add(const char* flag_name) {
    auto flag = _list_table->toFlag(flag_name);
    inkAssert(flag.has_value(), "No flag with name found to add! '%s'", flag_name);
    _list = _list_table->add(list_table::list{_list}, *flag).lid;
  }

  void list_impl::remove(const char* flag_name) {
    auto flag = _list_table->toFlag(flag_name);
    inkAssert(flag.has_value(), "No flag with name found to remove! '%s'", flag_name);
    _list = _list_table->sub(list_table::list{_list}, *flag).lid;
  }

  void list_impl::next(const char*& flag_name, const char*& list_name, int& i) const {
    if (i == -1) { return; }

    list_flag flag{.list_id = static_cast<int16_t>(i >> 16), .flag = static_cast<int16_t>(i & 0xFF)};
    if(flag_name != nullptr) {
      ++flag.flag;
    }
    if (flag.flag >= _list_table->_list_end[flag.list_id]) {
      next_list:
      flag.flag = 0;
      do {
        ++flag.list_id;
        if(static_cast<size_t>(flag.list_id) >= _list_table->_list_end.size()) {
          i = -1;
          return;
        }
      } while(!_list_table->hasList(_list_table->getPtr(_list), flag.list_id));
    }
    while(!_list_table->has(list_table::list{_list}, flag)) {
      ++flag.flag;
      if(flag.flag >= _list_table->_list_end[flag.list_id] - _list_table->listBegin(flag.list_id)) {
        goto next_list;
      }
    }
    flag_name = _list_table->_flag_names[_list_table->toFid(flag)];
    list_name = _list_table->_list_names[flag.list_id];

    i = (flag.list_id << 16) | flag.flag;
  }

  list_interface::iterator list_impl::begin(const char* list_name) const {
          size_t list_id = _list_table->get_list_id(list_name).list_id;
      return ++new_iterator(nullptr, list_id<<16);
  }
}
