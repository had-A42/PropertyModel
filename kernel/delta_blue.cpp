#include "delta_blue.h"

namespace NSPropertyModel {
ConstraintGraph
DeltaBlue::CreateInitialSolution(ConstraintGraph&& c_graph,
                                 StepType& propagation_counter) {

  /* добавление ограничений разделено на два цикла для
   * последовательного удовлетворения сначала stay-ограничений, затем
   * всех осатвшихся. Это необходимо, так как для добавления Regular-ограничений
   * требуется инвариант корректного текущего графа решения
   */
  for (const auto& constraint : c_graph.AllConstraints()) {
    Constraint* constraint_ptr = constraint.get();
    if (IsStay(constraint_ptr))
      ConstraintGraph::InsertStayToSolution(constraint_ptr);
  }

  for (const auto& constraint : c_graph.AllConstraints()) {
    Constraint* constraint_ptr = constraint.get();
    if (!IsStay(constraint_ptr)) {
      AddConstraint(constraint_ptr, propagation_counter);
    }
  }

  return std::move(c_graph);
}

void DeltaBlue::AddConstraint(Constraint* new_constraint,
                              StepType& propagation_counter) {
  assert(new_constraint != nullptr);

  if (!IsDisable(new_constraint)) {
    std::cout << "Constraint already added\n";
    return;
  }

  assert(new_constraint->selected_method == nullptr);
  MarkUnused(new_constraint);
  if (!IsBlocked(new_constraint)) {
    if (IsRequired(new_constraint)) {
      std::cout << "ALARM: failed to fulfil the required constraint!!!\n";
    }
    return;
  }

  Method* method_candidate = OutputMinPriorityMethod(new_constraint);
  Variable* output_candidate = GetOut(method_candidate);

  ReversePath(output_candidate);

  output_candidate->determined_by = new_constraint;
  new_constraint->selected_method = method_candidate;

  assert(GetOut(method_candidate) == output_candidate);
  MarkApplied(new_constraint);

  UpdatingPropagation(output_candidate, propagation_counter);
}

void DeltaBlue::AddConstraintByIndex(ConstraintGraph& c_graph, IndexType index,
                                     StepType& propagation_counter) {
  Constraint* new_constraint = c_graph.ConstraintByIndex(index);
  AddConstraint(new_constraint, propagation_counter);
}

void DeltaBlue::RemoveConstraint(ConstraintGraph& c_graph,
                                 Constraint* constraint_to_remove,
                                 StepType& propagation_counter) {
  assert(constraint_to_remove != nullptr);
  if (IsStay(constraint_to_remove)) {
    std::cout << "Not allowed to delete Stay!!!\n";
    return;
  }

  if (IsDisable(constraint_to_remove) || IsUnused(constraint_to_remove)) {
    assert(constraint_to_remove->selected_method == nullptr);
    MarkDisabled(constraint_to_remove);
    return;
  }

  Variable* output = GetSelectedMethodOut(constraint_to_remove);
  SetDeterminedByNull(output);
  SetSelectedMethodNull(constraint_to_remove);
  assert(constraint_to_remove->selected_method == nullptr);
  MarkDisabled(constraint_to_remove);

  Constraint* output_stay = GetStay(output);
  SetDeterminedBy(output, output_stay);
  SelectMethodByIndex(output_stay, 0);
  MarkApplied(output_stay);

  UpdatingPropagation(output, propagation_counter);

  Constraint* candidate = c_graph.FindLowestPriorityBlockedConstraint();
  if (candidate != nullptr) {
    AddConstraint(candidate, propagation_counter);
  }
}

void DeltaBlue::RemoveConstraintByIndex(ConstraintGraph& c_graph,
                                        IndexType index,
                                        StepType& propagation_counter) {
  Constraint* constraint_to_remove = c_graph.ConstraintByIndex(index);
  RemoveConstraint(c_graph, constraint_to_remove, propagation_counter);
}

void DeltaBlue::UpdateStayPriority(Constraint* stay, Priority priority,
                                   StepType& propagation_counter) {
  assert(stay != nullptr);
  assert(IsStay(stay));
  assert(priority.status == Priority::Status::Stay);

  stay->priority = priority;
  if (IsApplied(stay)) {
    Variable* output = GetSelectedMethodOut(stay);
    SetDeterminedByNull(output);

    SetSelectedMethodNull(stay);
  }
  assert(stay->selected_method == nullptr);
  MarkDisabled(stay);
  AddConstraint(stay, propagation_counter);
}

void DeltaBlue::UpdatingPropagation(Variable* variable,
                                    StepType& propagation_counter) {
  assert(variable != nullptr);
  ++propagation_counter;
  UpdatingPropagationImpl(variable, propagation_counter);
  ++propagation_counter;
}

void DeltaBlue::UpdatingPropagationImpl(Variable* variable,
                                        StepType& propagation_counter) {
  assert(variable != nullptr);

  UpdatePriority(variable);
  UpdateStep(variable, propagation_counter);

  for (const auto& constraint : variable->involved_as_potential_output) {
    if (!IsApplied(constraint) || GetSelectedMethodOut(constraint) == variable)
      continue;

    Variable* next_variable = GetSelectedMethodOut(constraint);

    if (IsProcessing(next_variable, propagation_counter)) {
      std::cout << "ALARM: Cycle!!! Property Model is dying!!!\n";
      exit(1); // TODO
    } else if (!IsUpdatedInCurrentStep(next_variable, propagation_counter)) {
      UpdatingPropagationImpl(next_variable, propagation_counter);
      UpdateStep(variable, propagation_counter + 1);
    }
  }

  UpdateStep(variable, propagation_counter + 1);
}

void DeltaBlue::ReversePath(Variable* variable) {
  assert(variable != nullptr);
  Constraint* current_constraint = variable->determined_by;
  if (current_constraint == nullptr) {
    return;
  }

  if (IsReversiblePathSource(current_constraint)) {
    SetDeterminedByNull(variable);
    SetSelectedMethodNull(current_constraint);
    MarkUnused(current_constraint);
  } else {
    Method* next_determining_method =
        PotentialOutputsMinMethod(current_constraint, variable);
    ReversePath(GetOut(next_determining_method));
    SetDeterminedBy(GetOut(next_determining_method), current_constraint);
    SelectMethod(current_constraint, next_determining_method);
  }
}
} // namespace NSPropertyModel
