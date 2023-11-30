#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

using namespace fs;
using namespace std;

class FaceDetector {
    cv::dnn::Net net;
    string outputFolderPath;

    const double confidenceThreshold = 0.2;

    void drawRectangle(
        cv::Mat image,
        const auto& face,
        const string &personName,
        double confidence
    ) {
        cout << "Detected: " << personName << endl;
        cv::rectangle(
            image,
            face,
            cv::Scalar(0, 255, 0),
            2
        );

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
            to_string(static_cast<int>(confidence)) + "%",
            cv::Point(face.x, face.y + 30),
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
        //load the image
        cv::Mat image = cv::imread(entry.path(), cv::IMREAD_COLOR);

        if (image.empty()) {
            throw runtime_error("Error loading image: " + entry.path().string());
        }

        //resize the image to match the input size of the SSD model
        cv::resize(image, image, cv::Size(300, 300));

        //preprocess the image for the SSD model
        cv::Mat inputBlob = cv::dnn::blobFromImage(image, 0.007843, cv::Size(300, 300), 127.5);

        //set the input for the DNN model
        net.setInput(inputBlob);

        //run forward pass to get predictions
        cv::Mat detection = net.forward();

        //extract the faces from the detection results
        vector<cv::Rect> faces;
        vector<double> confidences;
        for (int i = 0; i < detection.rows; ++i) {
            float confidence = detection.at<float>(i, 2);
            if (confidence > confidenceThreshold) {
                int x = static_cast<int>(detection.at<float>(i, 3) * image.cols);
                int y = static_cast<int>(detection.at<float>(i, 4) * image.rows);
                int width = static_cast<int>(detection.at<float>(i, 5) * image.cols - x);
                int height = static_cast<int>(detection.at<float>(i, 6) * image.rows - y);

                faces.emplace_back(x, y, width, height);
                confidences.push_back(confidence);
            }
        }

        //draw rectangles around the detected faces
        for (size_t i = 0; i < faces.size(); ++i) {
            drawRectangle(image, faces[i], personName, confidences[i] * 100);
        }

        cv::imwrite(
            personOutputFolder + "/" + personName + "_" + entry.path().filename().string(),
            image
        );
    }

public:
    FaceDetector(const string &protoTxtPath, const string &caffeModelPath, const string &outputFolderPath) {
        net = cv::dnn::readNetFromCaffe(protoTxtPath, caffeModelPath);
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        this->outputFolderPath = outputFolderPath;

        create_directory(outputFolderPath);
    }

    void Train(const string &trainingFolderPath) {
        for (const auto& personFolder : directory_iterator(trainingFolderPath)) {
            if (!personFolder.is_directory()) {
                continue;
            }

            string personName = personFolder.path().filename().string();

            string personOutputFolder = outputFolderPath + "/" + personName;
            create_directory(personOutputFolder);

            for (const auto& entry : directory_iterator(personFolder.path())) {
                proccessImage(entry, personName, personOutputFolder);
            }
        }
    }
};

int main() {
    const string trainingFolderPath = "TrainingData";

    const string outputFolderPath = "OutputData";

    //const string protoTxtPath = "Models/deploy.prototxt";
    //const string caffeModelPath = "Models/res10_300x300_ssd_iter_140000.caffemodel";

    const string protoTxtPath = "Models/opencv_face_detector.pbtxt";
    const string caffeModelPath = "Models/opencv_face_detector_uint8.pb";

    FaceDetector faceDetector(protoTxtPath, caffeModelPath, outputFolderPath);

    faceDetector.Train(trainingFolderPath);

    return 0;
}
