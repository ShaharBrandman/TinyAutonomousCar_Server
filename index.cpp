#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

using namespace fs;
using namespace std;

int main() {
    // Load the local face detection cascade
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
        cerr << "Error loading face cascade." << endl;
        return -1;
    }

    // Path to the folder containing labeled JPEG images
    string inputFolderPath = "TrainingData";
    string outputFolderPath = "OutputData"; // Base folder to save the output images

    // Create the output folder if it doesn't exist
    create_directory(outputFolderPath);

    // Iterate over subfolders (labeled with person names) in the input folder
    for (const auto& personFolder : directory_iterator(inputFolderPath)) {
        if (!personFolder.is_directory()) {
            continue; // Skip non-directory entries
        }

        // Get the person's name from the folder name
        string personName = personFolder.path().filename().string();

        // Create a folder for the person in the output folder
        string personOutputFolder = outputFolderPath + "/" + personName;
        create_directory(personOutputFolder);

        // Iterate over files in the person's folder
        for (const auto& entry : directory_iterator(personFolder.path())) {
            // Read the image
            cv::Mat image = cv::imread(entry.path(), cv::IMREAD_COLOR);

            // Check if the image is loaded successfully
            if (image.empty()) {
                cerr << "Error loading image: " << entry.path() << endl;
                continue;
            }

            // Convert the image to grayscale for face detection
            cv::Mat gray;
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

            // Detect faces in the image
            vector<cv::Rect> faces;
            faceCascade.detectMultiScale(gray, faces, 1.1, 4);

            // Draw rectangles around the detected faces
            for (const auto& face : faces) {
                cv::rectangle(
                    image,
                    face,
                    cv::Scalar(0, 255, 0),
                    2
                );

                // Label the face with the person's name
                cv::putText(
                    image,
                    personName,
                    cv::Point(face.x, face.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX,
                    2,
                    cv::Scalar(255, 0, 0),
                    2
                );
            }

            // Save the modified image to the person's output folder
            string outputFilePath = personOutputFolder + "/" + personName + "_" + entry.path().filename().string();
            cv::imwrite(outputFilePath, image);
        }
    }

    return 0;
}