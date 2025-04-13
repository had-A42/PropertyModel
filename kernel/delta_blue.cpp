#include "delta_blue.h"

namespace NSPropertyModel {
void DeltaBlue::CreateInitialSolution(ConstraintGraph& c_graph,
                                      StepType& propagation_counter) {

  /* добавление ограничений разделено на два цикла для
   * последовательного удовлетворения сначала stay-ограничений, затем
   * всех осатвшихся. Это необходимо, так как для добавления Regular-ограничений
   * требуется инвариант корректного текущего графа решения
   */
  for (const auto& constraint : c_graph.AllConstraints()) {
    if (constraint->IsStay()) {
      constraint->SelectMethodByIndex(0);
      constraint->MarkApplied();
      constraint->GetSelectedMethodOut()->SetDeterminedBy(constraint.get());
      constraint->GetSelectedMethodOut()->UpdatePriority();
    }
  }

  for (const auto& constraint : c_graph.AllConstraints()) {
    if (!constraint->IsStay()) {
      AddConstraint(c_graph, constraint.get(), propagation_counter);
    }
  }
}

void DeltaBlue::AddConstraint(ConstraintGraph& c_graph,
                              Constraint* new_constraint,
                              StepType& propagation_counter) {
  assert(new_constraint != nullptr);

  if (!new_constraint->IsDisable()) {
    std::cout << "Constraint already added\n";
    return;
  }

  new_constraint->MarkUnused();
  if (!new_constraint->IsBlocked()) {
    if (new_constraint->IsRequired()) {
      std::cout << "ALARM: failed to fulfil the required constraint!!!\n";
    }
    return;
  }

  Method* method_candidate = new_constraint->OutputMinPriorityMethod();
  Variable* output_candidate = method_candidate->GetOut();
  ReversePath(output_candidate);

  output_candidate->determined_by = new_constraint;
  new_constraint->selected_method = method_candidate;

  assert(method_candidate->GetOut() == output_candidate);
  new_constraint->MarkApplied();

  UpdatingPropagation(output_candidate, propagation_counter);
}

void DeltaBlue::AddConstraintByIndex(ConstraintGraph& c_graph, IndexType index,
                                     StepType& propagation_counter) {
  Constraint* new_constraint = c_graph.ConstraintByIndex(index);
  AddConstraint(c_graph, new_constraint, propagation_counter);
}

void DeltaBlue::RemoveConstraint(ConstraintGraph& c_graph,
                                 Constraint* constraint_to_remove,
                                 StepType& propagation_counter) {
  assert(constraint_to_remove != nullptr);
  if (constraint_to_remove->IsStay()) {
    std::cout << "Not allowed to delete Stay!!!\n";
    return;
  }

  if (constraint_to_remove->IsDisable() || constraint_to_remove->IsUnused()) {
    assert(constraint_to_remove->selected_method == nullptr);
    constraint_to_remove->MarkDisabled();
    return;
  }

  Variable* output = constraint_to_remove->GetSelectedMethodOut();
  output->SetDeterminedByNull();
  constraint_to_remove->SetSelectedMethodNull();
  constraint_to_remove->MarkDisabled();

  Constraint* output_stay = output->GetStay();
  output->SetDeterminedBy(output_stay);
  output_stay->SelectMethodByIndex(0);
  output_stay->MarkApplied();

  UpdatingPropagation(output, propagation_counter);

  Constraint* candidate = c_graph.FindLowestPriorityBlockedConstraint();
  if (candidate != nullptr) {
    AddConstraint(c_graph, candidate, propagation_counter);
  }
}

void DeltaBlue::RemoveConstraintByIndex(ConstraintGraph& c_graph,
                                        IndexType index,
                                        StepType& propagation_counter) {
  Constraint* constraint_to_remove = c_graph.ConstraintByIndex(index);
  RemoveConstraint(c_graph, constraint_to_remove, propagation_counter);
}

void DeltaBlue::UpdateStayPriority(ConstraintGraph& c_graph, Constraint* stay,
                                   Priority priority,
                                   StepType& propagation_counter) {
  assert(stay != nullptr);
  assert(stay->IsStay());
  assert(priority.status == Priority::Status::Stay);

  stay->priority = priority;
  if (stay->IsApplied()) {
    Variable* output = stay->GetSelectedMethodOut();
    output->SetDeterminedByNull();

    stay->SetSelectedMethodNull();
  }
  stay->MarkDisabled();
  AddConstraint(c_graph, stay, propagation_counter);
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

  variable->UpdatePriority();
  variable->UpdateStep(propagation_counter);

  for (const auto& constraint : variable->involved_as_potential_output) {
    if (!constraint->IsApplied() ||
        constraint->GetSelectedMethodOut() == variable)
      continue;

    Variable* next_variable = constraint->GetSelectedMethodOut();

    if (next_variable->IsProcessing(propagation_counter)) {
      std::cout << "ALARM: Cycle!!! Property Model is dying!!!\n";
      exit(1); // TODO
    } else if (!next_variable->IsUpdatedInCurrentStep(propagation_counter)) {
      UpdatingPropagationImpl(next_variable, propagation_counter);
      variable->UpdateStep(propagation_counter + 1);
    }
  }

  variable->UpdateStep(propagation_counter + 1);
}

void DeltaBlue::ReversePath(Variable* variable) {
  assert(variable != nullptr);
  Constraint* current_constraint = variable->determined_by;
  if (current_constraint == nullptr) {
    return;
  }

  if (current_constraint->IsReversiblePathSource()) {
    variable->SetDeterminedByNull();
    current_constraint->SetSelectedMethodNull();
    current_constraint->MarkUnused();
  } else {
    Method* next_determining_method =
        current_constraint->PotentialOutputsMinMethod(variable);
    ReversePath(next_determining_method->GetOut());
    next_determining_method->GetOut()->SetDeterminedBy(current_constraint);
    current_constraint->SelectMethod(next_determining_method);
  }
}
} // namespace NSPropertyModel
