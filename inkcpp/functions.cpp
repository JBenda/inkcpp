#include "functions.h"

namespace ink::runtime::internal
{
functions::functions()
    : _list(nullptr)
    , _last(nullptr)
{
}

functions::~functions()
{
	// clean list
	while (_list) {
		entry* toDelete = _list;
		_list           = _list->next;

		// delete both value and entry
		delete toDelete->value;
		delete toDelete;
	}
	_list = _last = nullptr;
}

void functions::add(hash_t name, function_base* func)
{
	entry* current = new entry;
	current->name  = name;
	current->value = func;
	current->next  = nullptr;

	if (_list == nullptr) {
		_list = _last = current;
	} else {
		_last->next = current;
		_last       = current;
	}
}

function_base* functions::find(hash_t name)
{
	// find entry
	entry* iter = _list;
	while (iter != nullptr && iter->name != name)
		iter = iter->next;
	return iter == nullptr ? nullptr : iter->value;
}
} // namespace ink::runtime::internal
