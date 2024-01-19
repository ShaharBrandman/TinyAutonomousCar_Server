#include <iostream>

#include <dlib/revision.h>
#include <opencv2/core.hpp>

#include <dlib/dnn.h>

int main() {
    std::cout << "Dlib version: " << DLIB_MAJOR_VERSION << "." << DLIB_MINOR_VERSION << "." << DLIB_PATCH_VERSION << std::endl;
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;

    return 0;
}

