/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
| Forked from boschmitt/synthewareQ
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "ast_node_kinds.hpp"
#include "detail/intrusive_list.hpp"

#include <array>
#include <memory>

namespace synthewareQ {
namespace qasm {

  class ast_context;

  // Base class for all QASM AST nodes
  class ast_node : detail::intrusive_list_node<ast_node> {
  public:
    ast_node(const ast_node&) = delete;
    ast_node& operator=(const ast_node&) = delete;

    virtual ~ast_node() = default;

    virtual ast_node* copy(ast_context* ctx) const = 0;

    ast_node_kinds kind() const
    {
      return do_get_kind();
    }

    ast_node const& parent() const
    {
      return *parent_;
    }

    uint32_t location() const
    {
      return location_;
    }

    // Sideways movement
    ast_node* next_sibling() const
    {
      return detail::intrusive_list_access<ast_node>::get_next(*this);
    }

    ast_node* prev_sibling() const
    {
      return detail::intrusive_list_access<ast_node>::get_prev(*this);
    }

  protected:
    ast_node(uint32_t location)
      : location_(location)
    {}

    uint32_t config_bits_ = 0;
    uint32_t location_;
    ast_node const* parent_;

  private:
    virtual ast_node_kinds do_get_kind() const = 0;

    void on_insert(const ast_node* parent)
    {
      parent_ = parent;
    }

    template<typename T>
    friend struct detail::intrusive_list_access;

    friend detail::intrusive_list_node<ast_node>;
  };

  // Type def for lists of nodes
  using ast_node_list = detail::intrusive_list<ast_node>;

  // Helper class for nodes that are containers. i.e not leafs
  template<typename Derived, typename T>
  class ast_node_container {
  public:
    using iterator = typename ast_node_list::iterator;
    using const_iterator = typename ast_node_list::const_iterator;

    iterator begin()
    {
      return children_.begin();
    }

    iterator end()
    {
      return children_.end();
    }

    const_iterator begin() const
    {
      return children_.begin();
    }

    const_iterator end() const
    {
      return children_.end();
    }
   
    size_t num_children() const
    {
      return children_.size();
    }

    void add_child(T* ptr)
    {
      children_.push_back(static_cast<Derived*>(this), ptr);
    }

    iterator insert_child(iterator it, ast_node* ptr)
    {
      return children_.insert(it, static_cast<Derived*>(this), ptr);
    }

    iterator insert_children(iterator it, ast_node_list& xs)
    {
      return children_.splice(it, static_cast<Derived*>(this), xs);
    }

    iterator set_child(iterator it, ast_node* ptr)
    {
      return children_.assign(it, static_cast<Derived*>(this), ptr);
    }

    iterator delete_child(iterator it)
    {
      return children_.erase(it);
    }

  protected:
    ~ast_node_container() = default;

  private:
	ast_node_list children_;
  };

} // namespace qasm
} // namespace synthewareQ