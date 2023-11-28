#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

using namespace fs;
using namespace std;

class FaceDetector {
    cv::CascadeClassifier faceCascade;
    string outputFolderPath;

    void drawRectangle(
        cv::Mat image,
        const auto& face,
        const string &personName
    ) {
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

    void proccessImage(
        const auto& entry,
        const string &personName,
        const string &personOutputFolder
    ) {
        // Read the image
        cv::Mat image = cv::imread(entry.path(), cv::IMREAD_COLOR);

        // Check if the image is loaded successfully
        if (image.empty()) {
            throw runtime_error("Error loading image: " + entry.path().string());
        }

        // Convert the image to grayscale for face detection
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

        // Detect faces in the image
        vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.1, 4);

        // Draw rectangles around the detected faces
        for (const auto& face : faces) {
            drawRectangle(image, face ,personName);
        }

        // Save the modified image to the person's output folder
        cv::imwrite(
            personOutputFolder + "/" + personName + "_" + entry.path().filename().string(),
            image
        );

    }
    
    public:
        FaceDetector(const string &preTrainedModelPath, const string &outputFolderPath) {
            if (!faceCascade.load(preTrainedModelPath)) {
                throw runtime_error("Error loading face cascade");
            }

            this->outputFolderPath = outputFolderPath;

            // Create the output folder if it doesn't exist
            create_directory(outputFolderPath);
        }

        void Train(const string &trainingFolderPath) {
            for (const auto& personFolder : directory_iterator(trainingFolderPath)) {
                if (!personFolder.is_directory()) {
                    continue;
                }

                // Get the person's name from the folder name
                string personName = personFolder.path().filename().string();

                // Create a folder for the person in the output folder
                string personOutputFolder = outputFolderPath + "/" + personName;
                create_directory(personOutputFolder);

                // Iterate over files in the person's folder
                for (const auto& entry : directory_iterator(personFolder.path())) {
                    proccessImage(entry, personName, personOutputFolder);
                }
            }
        }
};

int main() {
    // Path to the folder containing labeled JPEG images for training
    string trainingFolderPath = "TrainingData";
    // Path to the folder containing labeled JPEG images for testing
    string testingFolderPath = "TestingData";
    // Base folder to save the output images
    string outputFolderPath = "OutputData";

    FaceDetector faceDetector("haarcascade_frontalface_default.xml", outputFolderPath);
    
    faceDetector.Train(trainingFolderPath);

    return 0;
}
