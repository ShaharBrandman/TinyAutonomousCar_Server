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
        const string &personName,
        const double confidence
    ) {
        cv::rectangle(
            image,
            face,
            cv::Scalar(0, 255, 0),
            2
        );

        //draw the personName at the top of the rectangle
        cv::putText(
            image,
            personName,
            cv::Point(face.x, face.y - 5),
            cv::FONT_HERSHEY_SIMPLEX,
            2,
            cv::Scalar(255, 0, 0),
            2
        );

        cv::putText(
            image,
            to_string(static_cast<double>(confidence)) + "%",
            cv::Point(face.x, face.y + face.height + 30),
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
        cv::Mat image = cv::imread(entry.path(), cv::IMREAD_COLOR);

        if (image.empty()) {
            throw runtime_error("Error loading image: " + entry.path().string());
        }

        //convert the image to grayscale for face detection
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

        //detect faces in the image
        vector<cv::Rect> faces;
        vector<int> rejectLevels;
        faceCascade.detectMultiScale(gray, faces, rejectLevels, 1.1, 4);

        //draw rectangles around the detected faces
        for (size_t i = 0; i < faces.size(); ++i) {
            //obtain confidence from levelWeights
            double confidence = 100 - rejectLevels[i] * 100;

            drawRectangle(image, faces[i], personName, confidence);
        }

        //save the modified image to the person's output folder
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

        //create the output folder if it doesn't exists
        create_directory(outputFolderPath);
    }

    void Train(const string &trainingFolderPath) {
        for (const auto& personFolder : directory_iterator(trainingFolderPath)) {
            if (!personFolder.is_directory()) {
                continue;
            }

            //get the person name from the name of the folder
            string personName = personFolder.path().filename().string();

            //create the output folder for the person if doesn't exists
            string personOutputFolder = outputFolderPath + "/" + personName;
            create_directory(personOutputFolder);

            //proccess each image in the person folder
            for (const auto& entry : directory_iterator(personFolder.path())) {
                proccessImage(entry, personName, personOutputFolder);
            }
        }
    }
};

int main() {
    string trainingFolderPath = "TrainingData";

    string testingFolderPath = "TestingData";
    
    string outputFolderPath = "OutputData";

    FaceDetector faceDetector("Models/haarcascade_frontalface_default.xml", outputFolderPath);
    
    faceDetector.Train(trainingFolderPath);

    return 0;
}