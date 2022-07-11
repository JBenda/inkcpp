///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief avl_array class
// This is an AVL tree implementation using an array as data structure.
// avl_array combines the insert/delete and find advantages (log n) of an AVL tree
// with a static allocated arrays and minimal storage overhead.
// If memory is critical the 'Fast' template parameter can be set to false which
// removes the parent member of every node. This saves sizeof(size_type) * Size bytes,
// but slowes down the insert and delete operation by factor 10 due to 'parent search'.
// The find opeartion is not affected cause finding doesn't need a parent.
//
// usage:
// #include "avl_array.h"
// avl_array<int, int, int, 1024> avl;
// avl.insert(1, 1);
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _AVL_ARRAY_H_
#define _AVL_ARRAY_H_

#include "system.h"

#include <cstdint>


/**
 * \param Key The key type. The type (class) must provide a 'less than' and 'equal to' operator
 * \param T The Data type
 * \param size_type Container size type
 * \param Size Container size
 * \param Fast If true every node stores an extra parent index. This increases memory but speed up insert/erase by factor 10
 */
template<typename Key, typename T, typename size_type, const size_type Size, const bool Fast = true>
class avl_array
{
  // child index pointer class
  typedef struct tag_child_type {
    size_type left;
    size_type right;
  } child_type;

  // node storage, due to possible structure packing effects, single arrays are used instead of a 'node' structure 
  Key         key_[Size];                 // node key
  T           val_[Size];                 // node value
  std::int8_t balance_[Size];             // subtree balance
  child_type  child_[Size];               // node childs
  size_type   size_;                      // actual size
  size_type   root_;                      // root node
  size_type   parent_[Fast ? Size : 1];   // node parent, use one element if not needed (zero sized array is not allowed)
 
  // invalid index (like 'nullptr' in a pointer implementation)
  static const size_type INVALID_IDX = Size;

  // iterator class
  template<bool Const>
  class tag_avl_array_iterator
  {
	template<bool Con, typename T1, typename T2>
	using if_t = ink::runtime::internal::if_t<Con, T1, T2>;
    if_t<Const, const avl_array*, avl_array*>
		instance_;    		  // array instance
    size_type   idx_;         // actual node

    friend avl_array;         // avl_array may access index pointer

  public:

    // ctor
    tag_avl_array_iterator(if_t<Const, const avl_array*,avl_array*> instance = nullptr, size_type idx = 0U)
      : instance_(instance)
      , idx_(idx)
    { }

  	template<bool C = Const, typename = ink::runtime::internal::enable_if_t<C>>
  	tag_avl_array_iterator(const tag_avl_array_iterator<false>& itr)
  	  : instance_(itr.instance_)
  	  , idx_(itr.idx_)
  	{}

    inline tag_avl_array_iterator& operator=(const tag_avl_array_iterator& other)
    {
      instance_ = other.instance_;
      idx_      = other.idx_;
      return *this;
    }

    inline bool operator==(const tag_avl_array_iterator& rhs) const
    { return idx_ == rhs.idx_; }

    inline bool operator!=(const tag_avl_array_iterator& rhs) const
    { return !(*this == rhs); }

    // dereference - access value
    inline if_t<Const, const T&, T&> operator*() const
    { return val(); }

    // access value
    inline if_t<Const, const T&, T&> val() const
    { return instance_->val_[idx_]; }

    // access key
    inline const Key& key() const
    { return instance_->key_[idx_]; }

	// returns unique number for each entry
	// the numbers are unique as long no operation are executed
	// on the avl
  	inline size_t temp_identifier() const { return instance_->size() - idx_ - 1; }

    // preincrement
    tag_avl_array_iterator& operator++()
    {
      // end reached?
      if (idx_ >= Size) {
        return *this;
      }
      // take left most child of right child, if not existent, take parent
      size_type i = instance_->child_[idx_].right;
      if (i != instance_->INVALID_IDX) {
        // successor is the furthest left node of right subtree
        for (; i != instance_->INVALID_IDX; i = instance_->child_[i].left) {
          idx_ = i;
        }
      }
      else {
        // have already processed the left subtree, and
        // there is no right subtree. move up the tree,
        // looking for a parent for which nodePtr is a left child,
        // stopping if the parent becomes NULL. a non-NULL parent
        // is the successor. if parent is NULL, the original node
        // was the last node inorder, and its successor
        // is the end of the list
        i = instance_->get_parent(idx_);
        while ((i != instance_->INVALID_IDX) && (idx_ == instance_->child_[i].right)) {
          idx_ = i;
          i = instance_->get_parent(idx_);
        }
        idx_ = i;
      }
      return *this;
    }

    // postincrement
    inline tag_avl_array_iterator operator++(int)
    {
      tag_avl_array_iterator _copy = *this;
      ++(*this);
      return _copy;
    }
  };


public:

  typedef T                   value_type;
  typedef T*                  pointer;
  typedef const T*            const_pointer;
  typedef T&                  reference;
  typedef const T&            const_reference;
  typedef Key                 key_type;
  typedef tag_avl_array_iterator<false> iterator;
  typedef tag_avl_array_iterator<true> const_iterator;


  // ctor
  avl_array()
    : size_(0U)
    , root_(Size)
  { }


  // iterators
  inline iterator begin()
  {
    size_type i = INVALID_IDX;
    if (root_ != INVALID_IDX) {
      // find smallest element, it's the farthest node left from root
      for (i = root_; child_[i].left != INVALID_IDX; i = child_[i].left);
    }
    return iterator(this, i);
  }

  inline const_iterator begin() const
  {
	return const_iterator(const_cast<avl_array&>(*this).begin());
  }

  inline iterator end()
  { return iterator(this, INVALID_IDX); }

  inline const_iterator end() const
  {
	return const_iterator(this, INVALID_IDX);
  }


  // capacity
  inline size_type size() const
  { return size_; }

  inline bool empty() const
  { return size_ == static_cast<size_type>(0); }

  inline size_type max_size() const
  { return Size; }


  /**
   * Clear the container
   */
  inline void clear()
  {
    size_ = 0U;
    root_ = INVALID_IDX;
  }


  /**
   * Insert or update an element
   * \param key The key to insert. If the key already exists, it is updated
   * \param val Value to insert or update
   * \return True if the key was successfully inserted or updated, false if container is full
   */
  bool insert(const key_type& key, const value_type& val)
  {
    if (root_ == INVALID_IDX) {
      key_[size_]     = key;
      val_[size_]     = val;
      balance_[size_] = 0;
      child_[size_]   = { INVALID_IDX, INVALID_IDX };
      set_parent(size_, INVALID_IDX);
      root_ = size_++;
      return true;
    }

    for (size_type i = root_; i != INVALID_IDX; i = (key < key_[i]) ? child_[i].left : child_[i].right) {     
      if (key < key_[i]) {
        if (child_[i].left == INVALID_IDX) {
          if (size_ >= max_size()) {
            // container is full
            return false;
          }
          key_[size_]     = key;
          val_[size_]     = val;
          balance_[size_] = 0;
          child_[size_]   = { INVALID_IDX, INVALID_IDX };
          set_parent(size_, i);
          child_[i].left  = size_++;
          insert_balance(i, 1);
          return true;
        }
      }
      else if (key_[i] == key) {
        // found same key, update node
        val_[i] = val;
        return true;
      }
      else {
        if (child_[i].right == INVALID_IDX) {
          if (size_ >= max_size()) {
            // container is full
            return false;
          }
          key_[size_]     = key;
          val_[size_]     = val;
          balance_[size_] = 0;
          child_[size_]   = { INVALID_IDX, INVALID_IDX };
          set_parent(size_, i);
          child_[i].right = size_++;
          insert_balance(i, -1);
          return true;
        }
      }
    }
    // node doesn't fit (should not happen) - discard it anyway
    return false;
  }


  /**
   * Find an element
   * \param key The key to find
   * \param val If key is found, the value of the element is set
   * \return True if key was found
   */
  inline bool find(const key_type& key, value_type& val) const
  {
    for (size_type i = root_; i != INVALID_IDX;) {
      if (key < key_[i]) {
        i = child_[i].left;
      }
      else if (key == key_[i]) {
        // found key
        val = val_[i];
        return true;
      }
      else {
        i = child_[i].right;
      }
    }
    // key not found
    return false;
  }


  /**
   * Find an element and return an iterator as result
   * \param key The key to find
   * \return Iterator if key was found, else end() is returned
   */
  inline iterator find(const key_type& key)
  {
    for (size_type i = root_; i != INVALID_IDX;) {
      if (key < key_[i]) {
        i = child_[i].left;
      } else if (key == key_[i]) {
        // found key
        return iterator(this, i);
      }
      else {
        i = child_[i].right;
      }
    }
    // key not found, return end() iterator
    return end();
  }

  inline const_iterator find(const key_type& key) const
  {
	  return const_iterator(const_cast<avl_array&>(*this).find(key));
  }


  /**
   * Count elements with a specific key
   * Searches the container for elements with a key equivalent to key and returns the number of matches.
   * Because all elements are unique, the function can only return 1 (if the element is found) or zero (otherwise).
   * \param key The key to find/count
   * \return 0 if key was not found, 1 if key was found
   */
  inline size_type count(const key_type& key)
  {
    return find(key) != end() ? 1U : 0U;
  }


  /**
   * Remove element by key
   * \param key The key of the element to remove
   * \return True if the element ws removed, false if key was not found
   */
  inline bool erase(const key_type& key)
  {
    return erase(find(key));
  }


  /**
   * Remove element by iterator position
   * THIS ERASE OPERATION INVALIDATES ALL ITERATORS!
   * \param position The iterator position of the element to remove
   * \return True if the element was successfully removed, false if error
   */
  bool erase(iterator position)
  {
    if (empty() || (position == end())) {
      return false;
    }

    const size_type node  = position.idx_;
    const size_type left  = child_[node].left;
    const size_type right = child_[node].right;

    if (left == INVALID_IDX) {
      if (right == INVALID_IDX) {
        if (node == root_) {
          root_ = INVALID_IDX;
        }
        else {
          const size_type parent = get_parent(node);
          if (child_[parent].left == node) {
            child_[parent].left = INVALID_IDX;
            delete_balance(parent, -1);
          }
          else {
            child_[parent].right = INVALID_IDX;
            delete_balance(parent, 1);
          }
        }
      }
      else {
        const size_type parent = get_parent(node);
        child_[parent].left == node ? child_[parent].left = right : child_[parent].right = right;

        set_parent(right, parent);

        delete_balance(right, 0);
      }
    }
    else if (right == INVALID_IDX) {
      const size_type parent = get_parent(node);
      child_[parent].left == node ? child_[parent].left = left : child_[parent].right = left;

      set_parent(left, parent);

      delete_balance(left, 0);
    }
    else {
      size_type successor = right;
      if (child_[successor].left == INVALID_IDX) {
        const size_type parent = get_parent(node);
        child_[successor].left = left;
        balance_[successor] = balance_[node];
        set_parent(successor, parent);
        set_parent(left, successor);

        if (node == root_) {
          root_ = successor;
        }
        else {
          if (child_[parent].left == node) {
            child_[parent].left = successor;
          }
          else {
            child_[parent].right = successor;
          }
        }
        delete_balance(successor, 1);
      }
      else {
        while (child_[successor].left != INVALID_IDX) {
          successor = child_[successor].left;
        }

        const size_type parent           = get_parent(node);
        const size_type successor_parent = get_parent(successor);
        const size_type successor_right  = child_[successor].right;

        if (child_[successor_parent].left == successor) {
          child_[successor_parent].left = successor_right;
        }
        else {
          child_[successor_parent].right = successor_right;
        }

        set_parent(successor_right, successor_parent);
        set_parent(successor, parent);
        set_parent(right, successor);
        set_parent(left, successor);
        child_[successor].left  = left;
        child_[successor].right = right;
        balance_[successor]     = balance_[node];

        if (node == root_) {
          root_ = successor;
        }
        else {
          if (child_[parent].left == node) {
            child_[parent].left = successor;
          }
          else {
            child_[parent].right = successor;
          }
        }
        delete_balance(successor_parent, -1);
      }
    }
    size_--;

    // relocate the node at the end to the deleted node, if it's not the deleted one
    if (node != size_) {
      if (root_ == size_) {
        root_ = node;
      }
      else {
        const size_type parent = get_parent(size_);
        if (parent != INVALID_IDX) {  // should never be invalid, but anyway for security
          child_[parent].left == size_ ? child_[parent].left = node : child_[parent].right = node;
        }
      }

      // correct childs parent
      set_parent(child_[size_].left,  node);
      set_parent(child_[size_].right, node);

      // move content
      replace(node, size_);
    }

    return true;
  }


  /**
   * Integrity (self) check
   * \return True if the tree intergity is correct, false if error (should not happen normally)
   */   
  bool check() const
  {
    // check root
    if (empty() && (root_ != INVALID_IDX)) {
      // invalid root
      return false;
    }
    if (size() && root_ >= size()) {
      // root out of bounds
      return false;
    }

    // check tree
    for (size_type i = 0U; i < size(); ++i)
    {
      if ((child_[i].left != INVALID_IDX) && (!(key_[child_[i].left] < key_[i]) || (key_[child_[i].left] == key_[i]))) {
        // wrong key order to the left
        return false;
      }
      if ((child_[i].right != INVALID_IDX) && ((key_[child_[i].right] < key_[i]) || (key_[child_[i].right] == key_[i]))) {
        // wrong key order to the right
        return false;
      }
      const size_type parent = get_parent(i);
      if ((i != root_) && (parent == INVALID_IDX)) {
        // no parent
        return false;
      }
      if ((i == root_) && (parent != INVALID_IDX)) {
        // invalid root parent
        return false;
      }
    }
    // check passed
    return true;
  }


  /////////////////////////////////////////////////////////////////////////////
  // Helper functions
private:

  // find parent element
  inline size_type get_parent(size_type node) const
  {
    if (Fast) {
      return parent_[node];
    }
    else {
      const Key key_node = key_[node];
      for (size_type i = root_; i != INVALID_IDX; i = (key_node < key_[i]) ? child_[i].left : child_[i].right) {
        if ((child_[i].left == node) || (child_[i].right == node)) {
          // found parent
          return i;
        }
      }
      // parent not found
      return INVALID_IDX;
    }
  }


  // set parent element (only in Fast version)
  inline void set_parent(size_type node, size_type parent)
  {
    if (Fast) {
      if (node != INVALID_IDX) {
        parent_[node] = parent;
      }
    }
  }


  inline void replace(size_type target, size_type source)
  {
    key_[target]     = key_[source];
    val_[target]     = val_[source];
    balance_[target] = balance_[source];
    child_[target]   = child_[source];
    set_parent(target, get_parent(source));
  }


  void insert_balance(size_type node, std::int8_t balance)
  {
    while (node != INVALID_IDX) {
      balance = (balance_[node] += balance);
     
      if (balance == 0) {
        return;
      }
      else if (balance == 2) {
        if (balance_[child_[node].left] == 1) {
          rotate_right(node);
        }
        else {
          rotate_left_right(node);
        }
        return;
      }
      else if (balance == -2) {
        if (balance_[child_[node].right] == -1) {
          rotate_left(node);
        }
        else {
          rotate_right_left(node);
        }
        return;
      }
     
      const size_type parent = get_parent(node);
      if (parent != INVALID_IDX) {
        balance = child_[parent].left == node ? 1 : -1;
      }
      node = parent;
    }
  }


  void delete_balance(size_type node, std::int8_t balance)
  {
    while (node != INVALID_IDX) {
      balance = (balance_[node] += balance);

      if (balance == -2) {
        if (balance_[child_[node].right] <= 0) {
          node = rotate_left(node);
          if (balance_[node] == 1) {
            return;
          }
        }
        else {
          node = rotate_right_left(node);
        }
      }
      else if (balance == 2) {
        if (balance_[child_[node].left] >= 0) {
          node = rotate_right(node);
          if (balance_[node] == -1) {
            return;
          }
        }
        else {
          node = rotate_left_right(node);
        }
      }
      else if (balance != 0) {
        return;
      }

      if (node != INVALID_IDX) {
        const size_type parent = get_parent(node);
        if (parent != INVALID_IDX) {
          balance = child_[parent].left == node ? -1 : 1;
        }
        node = parent;
      }
    }
  }


  size_type rotate_left(size_type node)
  {
    const size_type right      = child_[node].right;
    const size_type right_left = child_[right].left;
    const size_type parent     = get_parent(node);

    set_parent(right, parent);
    set_parent(node, right);
    set_parent(right_left, node);
    child_[right].left = node;
    child_[node].right = right_left;

    if (node == root_) {
      root_ = right;
    }
    else if (child_[parent].right == node) {
      child_[parent].right = right;
    }
    else {
      child_[parent].left = right;
    }

    balance_[right]++;
    balance_[node] = -balance_[right];

    return right;
  }


  size_type rotate_right(size_type node)
  {
    const size_type left       = child_[node].left;
    const size_type left_right = child_[left].right;
    const size_type parent     = get_parent(node);

    set_parent(left, parent);
    set_parent(node, left);
    set_parent(left_right, node);
    child_[left].right = node;
    child_[node].left  = left_right;

    if (node == root_) {
      root_ = left;
    }
    else if (child_[parent].left == node) {
      child_[parent].left = left;
    }
    else {
      child_[parent].right = left;
    }

    balance_[left]--;
    balance_[node] = -balance_[left];

    return left;
  }


  size_type rotate_left_right(size_type node)
  {
    const size_type left             = child_[node].left;
    const size_type left_right       = child_[left].right;
    const size_type left_right_right = child_[left_right].right;
    const size_type left_right_left  = child_[left_right].left;
    const size_type parent           = get_parent(node);

    set_parent(left_right, parent);
    set_parent(left, left_right);
    set_parent(node, left_right);
    set_parent(left_right_right, node);
    set_parent(left_right_left, left);
    child_[node].left        = left_right_right;
    child_[left].right       = left_right_left;
    child_[left_right].left  = left;
    child_[left_right].right = node;

    if (node == root_) {
      root_ = left_right;
    }
    else if (child_[parent].left == node) {
      child_[parent].left = left_right;
    }
    else {
      child_[parent].right = left_right;
    }

    if (balance_[left_right] == 0) {
      balance_[node] = 0;
      balance_[left] = 0;
    }
    else if (balance_[left_right] == -1) {
      balance_[node] = 0;
      balance_[left] = 1;
    }
    else {
      balance_[node] = -1;
      balance_[left] = 0;
    }
    balance_[left_right] = 0;

    return left_right;
  }


  size_type rotate_right_left(size_type node)
  {
    const size_type right            = child_[node].right;
    const size_type right_left       = child_[right].left;
    const size_type right_left_left  = child_[right_left].left;
    const size_type right_left_right = child_[right_left].right;
    const size_type parent           = get_parent(node);

    set_parent(right_left, parent);
    set_parent(right, right_left);
    set_parent(node, right_left);
    set_parent(right_left_left, node);
    set_parent(right_left_right, right);
    child_[node].right       = right_left_left;
    child_[right].left       = right_left_right;
    child_[right_left].right = right;
    child_[right_left].left  = node;

    if (node == root_) {
      root_ = right_left;
    }
    else if (child_[parent].right == node) {
      child_[parent].right = right_left;
    }
    else {
      child_[parent].left = right_left;
    }

    if (balance_[right_left] == 0) {
      balance_[node]  = 0;
      balance_[right] = 0;
    }
    else if (balance_[right_left] == 1) {
      balance_[node]  = 0;
      balance_[right] = -1;
    }
    else {
      balance_[node]  = 1;
      balance_[right] = 0;
    }
    balance_[right_left] = 0;

    return right_left;
  }
};

#endif  // _AVL_ARRAY_H_
