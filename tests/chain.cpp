#include <cmath>
#include <cassert>
#include <format>

#include "pm.h"

/*
 * (c_0) ---- d_0 -- (c_1) -- d_1 -- ... -- d_8 -- (c_9) -- d_9 ---- (c_10)
 *   2                 1                             1                 3    - приоритет
 *   будем многократно удалять и добавлять c_0
 *
 *   если c_0 добавлен:
 * (c_0) ----> d_0 -->(c_1) --> d_1 --> ... --> d_8 --> (c_9) --> d_9      (c_10)
 *   2                  1                                 1                   3  - приоритет
 *
 *   если c_0 удален:
 * (c_0)      d_0 <--(c_1) <-- d_1 <-- ... <-- d_8 <-- (c_9) <-- d_9 <---- (c_10)
 *   2                 1                                 1                   3  - приоритет

 */

namespace {
    int Assign(int in) {
        return in;
    }

    int AssignOne() {
        return 1;
    }

    int AssignTwo() {
        return 2;
    }

    using namespace NSPropertyModel;
    class Application {
        using DataVars = Data<int, int, int, int, int, int, int, int, int, int>;
        using ValueVars = Value<>;
        using OutVars = Out<>;
        using PropertyModel = PropertyModel<DataVars, ValueVars, OutVars>;
        using Builder = PropertyModel::Builder;
        using IndexType = Templates::IndexType;

        static constexpr IndexType chain_length = 10;
    public:
        template<IndexType Index>
        struct AddMiddleConstraint {
            void operator()(Builder& builder) {
                builder.AddNewConstraint(Priority::Strength{1});
                builder.AddMethod<Templates::Data<Index>, Templates::Data<Index + 1>>(Assign);
                builder.AddMethod<Templates::Data<Index + 1>, Templates::Data<Index>>(Assign);
            }
        };

        Application() {
            assert(chain_length == Templates::Size<DataVars>);
            Builder b(0,0,0,0,0,0,0,0,0,0);

            b.AddNewConstraint(Priority::Strength{2});
            b.AddMethod<Templates::Data<0>>(AssignOne);

            Templates::For<0, chain_length - 1, 1>::template Do<AddMiddleConstraint>(b);

            b.AddNewConstraint(Priority::Strength{3});
            b.AddMethod<Templates::Data<9>>(AssignTwo);

            pm = b.ExtractPM();

            pm->Print();
        }
        void run() {
            std::string query;
            PrintIntroductoryCommand();
            while (std::cin >> query) {
                Query(query);
                PrintIntroductoryCommand();
            }
        }

        void stress() {
            for (int i = 0; i < 1000000; ++i) {
                pm->RemoveConstraint(0);
                pm->AddConstraint(0);
                if (i % 10000 == 0) {
                    std::cout << i << "\n";
                }
            }
        }

    private:
        static void PrintIntroductoryCommand() {
            std::cout
                    << "_____________________________________________________________\n";
            std::cout
                    << "Enter command (quit / update - update value / add - add "
                       "constraint / remove - remove constraint / print - print system "
                       "state): ";
        }

        template<IndexType Index>
        struct ValueUpdating {
            void operator()(PropertyModel& pm, IndexType target_index) {
                if (target_index == Index) {
                    std::cout << "Enter the value: ";
                    int value;
                    std::cin >> value;
                    pm->Set<Templates::Data<Index>>(value);
                }
            }
        };

        void Query(std::string& query) {
            IndexType index;
            if (query == "quit") {
                std::cout << "The End.\n";
                exit(0);
            } else if (query == "update") {
                char meta_marker;
                std::cout << std::format("Enter index (from 0 to {}): ", chain_length);

                std::cin >> index;
                if (index < 0 || index > 9) {
                    std::cout << "Index out of range!\n";
                } else {
                    Templates::For<0, chain_length, 1>::template Do<ValueUpdating>(pm, index);
                }

            } else if (query == "remove") {
                std::cout << "Enter the constraint index: ";
                std::cin >> index;
                pm->RemoveConstraint(index);
            } else if (query == "add") {
                std::cout << "Enter the constraint index: ";
                std::cin >> index;
                pm->AddConstraint(index);
            } else if (query == "print") {
                pm->Print();
            } else {
                std::cout << "Invalid command!\n";
            }
        }
        PropertyModel pm;
    };
} // namespace

int main() {
    try {
        Application app;
        std::string command;
        std::cout << "run / stress: ";
        std::cin >> command;
        if (command == "run") {
            app.run();
        } else if (command == "stress"){
            app.stress();
        }

    } catch(...) {
        NSExcept::react();
    }
    return 0;
}
