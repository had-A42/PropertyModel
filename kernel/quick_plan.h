#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <queue>
#include <stack>
#include <unordered_set>
#include <vector>

#include "../src/priority.h"

namespace NSPropertyModel {
using IndexType = int;
using StepType = int;

struct Constraint;
struct Variable;
struct Method;

using ConstraintPtrs = std::vector<Constraint*>;
using VariablePtrs = std::vector<Variable*>;

using ConstraintPtrsSet = std::unordered_set<Constraint*>;
using VariablePtrsSet = std::unordered_set<Variable*>;

using Methods = std::vector<std::unique_ptr<Method>>;

struct Variable {
  void SetDeterminedBy(Constraint* constraint) {
    determined_by = constraint;
  }

  void SetDeterminedByNull() {
    determined_by = nullptr;
  }

  StepType last_propagation = StepType{0};

  Constraint* determined_by = nullptr;
  ConstraintPtrsSet involved_in;

  IndexType num_constraint;

  Priority priority;
};

struct Method {
  bool HasFreeOutput() {
    return std::all_of(out.begin(), out.end(), [](Variable* variable) {
      return variable->num_constraint == 1;
    });
  }

  std::function<void()> action;

  std::vector<Variable*> in;
  std::vector<Variable*> out;
};

struct Constraint {

  bool IsApplied() {
    return state == State::Applied;
  };

  bool IsDisable() {
    return state == State::Disabled;
  };

  bool IsUnused() {
    return state == State::Unused;
  };

  bool IsRequired() {
    return priority.strength == 0 &&
           priority.status == Priority::Status::Regular;
  }

  void MarkApplied() {
    state = State::Applied;
  }

  void MarkUnused() {
    assert(selected_method == nullptr);
    state = State::Unused;
  }

  void MarkDisabled() {
    assert(selected_method == nullptr);
    state = State::Disabled;
  }

  Method* TryToFindFreeOutputMethod() {
    Method* result = nullptr;
    IndexType min_output_size = variables.size() + 1;
    for (auto& method : methods) {
      if (method->HasFreeOutput() && min_output_size > method->out.size()) {
        result = method.get();
      }
    }

    return result;
  }

  enum class State { Applied, Unused, Disabled };
  State state = State::Disabled;
  Method* selected_method = nullptr;
  Priority priority;
  StepType last_execution = StepType{0};
  Methods methods;
  VariablePtrs variables;
};

struct ConstraintGraph {
  ConstraintPtrs constraints;
  VariablePtrs variables;
};

class QuickPlan {
public:
  void AddConstraint(ConstraintGraph& c_graph, Constraint* new_constraint,
                     StepType& propagation_counter) {
    if (!new_constraint->IsDisable()) {
      std::cout << "Constraint already added\n";
      return;
    }

    new_constraint->MarkUnused();
    for (auto variable : new_constraint->variables) {
      variable->involved_in.insert(new_constraint);
    }

    Method* free_output_method = new_constraint->TryToFindFreeOutputMethod();
    if (free_output_method != nullptr) {
      new_constraint->selected_method = free_output_method;
      for (auto variable : free_output_method->out) {
        variable->determined_by = new_constraint;
      }
    } else {
      unused_queue_ = PriorityConstraintQueueMax();
      unused_queue_.push(new_constraint);

      ConstraintHierarchySolver(c_graph, propagation_counter);

      if (new_constraint->selected_method == nullptr &&
          new_constraint->IsRequired()) {
        std::cout << "ALARM: required constraint could not be applied!";
      }
    }
  }

  void RemoveConstraint(ConstraintGraph& c_graph,
                        Constraint* constraint_to_remove,
                        StepType& propagation_counter) {
    unused_queue_ = PriorityConstraintQueueMax();
    Priority priority = constraint_to_remove->priority;
    for (auto variable : constraint_to_remove->variables) {
      variable->involved_in.erase(constraint_to_remove);
      if (variable->determined_by == constraint_to_remove) {
        CollectDownstreamUnusedConstreaints(variable);
      }
    }
    ConstraintHierarchySolver(c_graph, propagation_counter);
  }

private:
  //        void MultiOutputPlanner() {
  //            while ()
  //        }

  void ConstraintHierarchySolver(ConstraintGraph& c_graph,
                                 StepType& propagation_counter) {
    while (unused_queue_.empty()) {
      Constraint* constraint_to_enforce = unused_queue_.top();
      unused_queue_.pop();

      ++propagation_counter;
      StepType visited_mark = propagation_counter;
      ++propagation_counter;
      StepType search_mark = propagation_counter;

      Priority strongest_retracted_priority =
          Priority{Priority::Status::Stay, Priority::Strength{-1}};
      VariablePtrsSet potential_undetermined_variables;
      retractable_queue_ = PriorityConstraintQueueMin();

      CollectUpstreamConstraint(c_graph, visited_mark, constraint_to_enforce,
                                constraint_to_enforce);

      for (auto variable : c_graph.variables) {
        if (variable->num_constraint > 1)
          free_variable_set_.erase(variable);
      }

      ConstraintHierarchyPlanner(constraint_to_enforce->priority);

      if (constraint_to_enforce->selected_method == nullptr) {
        while (!undo_stack_.empty()) {
          auto&& [undo_constraint, restored_method] = undo_stack_.top();
          undo_stack_.pop();

          for (auto variable : undo_constraint->variables) {
            variable->SetDeterminedByNull();
          }
          for (auto variable : restored_method->out) {
            variable->SetDeterminedBy(undo_constraint);
          }

          undo_constraint->selected_method = restored_method;
        }
      } else if (strongest_retracted_priority >
                 Priority{Priority::Status::Stay, Priority::Strength{-1}}
                 // && TODO!
      ) {
        VariablePtrsSet undetermined_variables;

        for (auto variable : potential_undetermined_variables) {
          if (variable->determined_by == nullptr)
            undetermined_variables.insert(variable);
        }

        for (auto variable : free_variable_set_) {
          if (variable->determined_by == nullptr && variable->mark)
        }
      }
    }
  }

  void CollectUpstreamConstraint(ConstraintGraph& c_graph,
                                 StepType visited_mark,
                                 Constraint* constraint_to_enforce,
                                 Constraint* constraint) {
    constraint->last_execution = visited_mark;
    if (constraint->priority < constraint_to_enforce->priority) {
      retractable_queue_.push(constraint);
    }

    for (auto variable : constraint->variables) {
      if (variable->last_propagation == visited_mark) {
        ++variable->num_constraint;
      } else {
        variable->last_propagation = visited_mark;
        variable->num_constraint = 1;
      }

      Constraint* previous = variable->determined_by;
      if (previous != nullptr && previous->last_execution != visited_mark) {
        CollectUpstreamConstraint(c_graph, visited_mark, constraint_to_enforce,
                                  previous);
      } else if (variable->num_constraint == 1) {
        free_variable_set_.insert(variable);
      }
    }
  }

  struct MaxPriorityComparator {
    bool operator()(const Constraint* lhs, const Constraint* rhs) const {
      return lhs->priority < rhs->priority;
    }
  };

  struct MinPriorityComparator {
    bool operator()(const Constraint* lhs, const Constraint* rhs) const {
      return lhs->priority > rhs->priority;
    }
  };

  struct UndoNode {
    Constraint* constraint;
    Method* restored_method;
  };

  using PriorityConstraintQueueMin =
      std::priority_queue<Constraint*, std::vector<Constraint*>,
                          MinPriorityComparator>;
  using PriorityConstraintQueueMax =
      std::priority_queue<Constraint*, std::vector<Constraint*>,
                          MaxPriorityComparator>;
  using UndoStack = std::stack<UndoNode>;

  PriorityConstraintQueueMin retractable_queue_;
  PriorityConstraintQueueMax unused_queue_;

  VariablePtrsSet free_variable_set_;
  UndoStack undo_stack_;
};
} // namespace NSPropertyModel
