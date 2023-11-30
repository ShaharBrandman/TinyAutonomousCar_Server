#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

using namespace fs;
using namespace std;

int main() {
    // Load the local face detection cascade
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("Models/haarcascade_frontalface_default.xml")) {
        cerr << "Error loading face cascade." << endl;
        return -1;
    }

    // Path to the folder containing labeled JPEG images for training
    string trainingFolderPath = "TrainingData";
    // Path to the folder containing labeled JPEG images for testing
    string testingFolderPath = "TestingData";
    // Base folder to save the output images
    string outputFolderPath = "OutputData";

    // Create the output folder if it doesn't exist
    create_directory(outputFolderPath);

    // ------------ Training Phase ------------

    // Iterate over subfolders (labeled with person names) in the training folder
    for (const auto& personFolder : directory_iterator(trainingFolderPath)) {
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
            std::string outputFilePath = personOutputFolder + "/" + personName + "_" + entry.path().filename().string();
            cv::imwrite(outputFilePath, image);
        }
    }

    // ------------ Testing Phase ------------

    // Iterate over files in the testing folder
    for (const auto& entry : directory_iterator(testingFolderPath)) {
        // Read the test image
        cv::Mat testImage = cv::imread(entry.path(), cv::IMREAD_COLOR);

        // Check if the image is loaded successfully
        if (testImage.empty()) {
            cerr << "Error loading test image: " << entry.path() << endl;
            continue;
        }

        // Convert the test image to grayscale for face detection
        cv::Mat testGray;
        cv::cvtColor(testImage, testGray, cv::COLOR_BGR2GRAY);

        // Detect faces in the test image
        vector<cv::Rect> testFaces;
        faceCascade.detectMultiScale(testGray, testFaces, 1.1, 4);

        // Process each detected face in the test image
        for (const auto& testFace : testFaces) {
            // Label the detected face with the person's name (if known)
            bool faceLabeled = false;

            // Iterate over subfolders (labeled with person names) in the training folder
            // for (const auto& personFolder : directory_iterator(trainingFolderPath)) {
                for (const auto& personFolder : directory_iterator(outputFolderPath)) {
                if (!personFolder.is_directory()) {
                    continue; // Skip non-directory entries
                }

                // Get the person's name from the folder name
                string personName = personFolder.path().filename().string();
                // Load the labeled image of the person for comparison
                string labeledImagePath = personFolder.path().string() + "/" + personName + "_1.jpeg"; // Assuming only one labeled image per person
                cv::Mat labeledImage = cv::imread(labeledImagePath, cv::IMREAD_GRAYSCALE);

                // Check if the labeled image is loaded successfully
                if (labeledImage.empty()) {
                    cerr << "Error loading labeled image: " << labeledImagePath << endl;
                    continue;
                }

                // Resize the labeled image to match the detected face size
                cv::resize(labeledImage, labeledImage, testFace.size());

                // Compare the labeled image with the detected face
                cv::Mat diff;
                cv::absdiff(labeledImage, testGray(testFace), diff);
                double mismatchPercentage = (double)cv::countNonZero(diff) / (testFace.width * testFace.height) * 100.0;

                // If the mismatch percentage is below a threshold, label the face
                if (mismatchPercentage < 20.0) {
                    cv::rectangle(
                        testImage,
                        testFace,
                        cv::Scalar(0, 255, 0),
                        2
                    );

                    // Label the face with the person's name
                    cv::putText(
                        testImage,
                        personName,
                        cv::Point(testFace.x, testFace.y - 5),
                        cv::FONT_HERSHEY_SIMPLEX,
                        2,
                        cv::Scalar(255, 0, 0),
                        2
                    );

                    faceLabeled = true;
                    break; // Stop searching for the face in other labeled images
                }
            }

            // If the face is not labeled, mark it as "Unknown"
            if (!faceLabeled) {
                cv::rectangle(
                    testImage,
                    testFace,
                    cv::Scalar(0, 255, 0),
                    2
                );

                // Label the face as "Unknown"
                cv::putText(
                    testImage,
                    "Unknown",
                    cv::Point(testFace.x, testFace.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX,
                    2,
                    cv::Scalar(255, 0, 0),
                    2
                );
            }
        }

        // Save the modified test image to the output folder
        std::string outputTestFilePath = outputFolderPath + "/test_" + entry.path().filename().string();
        cv::imwrite(outputTestFilePath, testImage);
    }

    return 0;
}
