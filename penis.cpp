#include <iostream>
#include <dlib/revision.h>

int main() {
    std::cout << DLIB_MAJOR_VERSION << "." << DLIB_MINOR_VERSION << "." << DLIB_PATCH_VERSION << std::endl;
    // Rest of your code
    return 0;
}