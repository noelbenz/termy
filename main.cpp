
// C++ Standard
#include <iostream>

// Termy
#include "windows.hpp"

using std::cout;
using std::endl;

int main() {

    cout << "Hello windows!" << endl;

    try {
        Termy *termy = new Termy(GetModuleHandleW(NULL));
        termy->start();
    } catch (char* msg) {
        cout << msg << endl;
        return -1;
    }

    return 0;
}
