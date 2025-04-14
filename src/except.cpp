#include <exception>
#include <iostream>

#include "except.h"

namespace NSExcept {
    void react() {
        try {
            throw;
        } catch(std::exception& e) {
            std::cout << e.what();
            exit(1);
        } catch(...) {
            std::cout << "Unknown error!\n";
            exit(1);
        }
    }
}