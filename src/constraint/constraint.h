#include "priority.h"

#include <functional>

/*
 * Здесь пока что находятся наброски Method и Constraint. Они не доработаны.
 */
class Method {
public:
  Method(std::function<void()> func) : action_(func) {
  }

private:
  void Execute() {
    action_();
  };

  std::function<void()> action_;
};

class Constraint {
public:
  Constraint(Priopity priority, std::vector<Method> methods)
      : priority_(priority), methods_(methods) {};

  void PushBackMethod(Method method) {
    methods_.push_back(method);
  }

private:
  int selected_method = -1;
  Priopity priority_;
  std::vector<Method> methods_{};
};