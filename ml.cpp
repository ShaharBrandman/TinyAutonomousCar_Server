#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>

#include <json.hpp>

#define JSON_PRETTY_VALUE 4

#define STREAM_URL "http://10.9.28.71/stream"

namespace fs = boost::filesystem;

using json = nlohmann::json;

cv::Mat extractFeatures(const cv::Mat& image, const cv::Size& targetSize) {
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, targetSize);
    return resizedImage.reshape(1, 1);
}

cv::Mat fetchFrame(const std::string& streamURL) {
    cv::VideoCapture capture(streamURL, cv::CAP_ANY);
    cv::Mat frame;

    if (capture.isOpened()) {
        capture >> frame;
    }

    cv::imwrite("captured_frame.jpg", frame);

    return frame;
}

json train() {
    cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
    svm->setType(cv::ml::SVM::C_SVC);
    svm->setKernel(cv::ml::SVM::LINEAR);

    cv::Mat features, labels;

    int labelCounter = 1;

    json trainResult;

    std::unordered_map<std::string, int> labelMap;

    for (const auto& person_folder : {"Shahar", "Gingi"}) {
        for (const auto& entry : fs::directory_iterator("TrainingData/" + std::string(person_folder))) {
            cv::Mat img = cv::imread(entry.path().string());

            if (img.empty()) {
                trainResult["Error"][entry.path().string()] = "train(): Error loading image";
                continue;
            }

            cv::Mat featureVector = extractFeatures(img, cv::Size(64, 64));
            featureVector.convertTo(featureVector, CV_32F);

            features.push_back(featureVector);

            if (labelMap.find(person_folder) == labelMap.end()) {
                labelMap[person_folder] = labelCounter++;
            }

            int numericLabel = labelMap[person_folder];
            labels.push_back(numericLabel);
        }
    }

    labels.convertTo(labels, CV_32S);

    svm->train(features, cv::ml::ROW_SAMPLE, labels);
    svm->save("object_model.xml");

    std::ofstream labelMapFile("labelMap.json");
    labelMapFile << json(labelMap).dump(JSON_PRETTY_VALUE);
    labelMapFile.close();

    trainResult["Result"] = "Training has completed";
    trainResult["labelMap"] = json(labelMap);

    return trainResult;
}

json test() {
    json testResult;

    std::unordered_map<std::string, int> labelMap;

    testResult["TestPath"] = STREAM_URL;

    cv::Mat img = fetchFrame(testResult["TestPath"]);

    if (img.empty()) {
        testResult["Error"] = "test(): Error loading image";
        return testResult;
    }

    if (!fs::exists("object_model.xml")) {
        testResult["Error"] = "test(): object_model.xml doesn't exist!";
        return testResult;
    }

    if (!fs::exists("labelMap.json")) {
        testResult["Error"] = "test(): labelMap.json doesn't exist!";
        return testResult;
    }

    cv::Ptr<cv::ml::SVM> svm = cv::Algorithm::load<cv::ml::SVM>("object_model.xml");
    svm->setType(cv::ml::SVM::C_SVC);
    svm->setKernel(cv::ml::SVM::LINEAR);

    std::ifstream labelMapFile("labelMap.json");
    json labelMapJson;
    labelMapFile >> labelMapJson;
    labelMapFile.close();

    labelMap = labelMapJson.get<std::unordered_map<std::string, int>>();

    cv::Mat features, labels;

    cv::Mat featureVector = extractFeatures(img, cv::Size(64, 64));
    featureVector.convertTo(featureVector, CV_32F);

    float result = svm->predict(featureVector);

    std::unordered_map<int, std::string> reverseLabelMap;
    for (const auto& entry : labelMap) {
        reverseLabelMap[entry.second] = entry.first;
    }

    testResult["Prediction"] = "Unknown";

    if (result > 0 && reverseLabelMap.find(result) != reverseLabelMap.end()) {
        testResult["Prediction"] = reverseLabelMap[result];
    }

    return testResult;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "No Arguments were given" << std::endl;
    }

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--train") == 0) {
            json trainResult = train();
            std::cout << trainResult.dump(JSON_PRETTY_VALUE) << std::endl;
            return 1;
        } else if (strcmp(argv[i], "--test") == 0) {
            json testResult = test();
            std::cout << testResult.dump(JSON_PRETTY_VALUE) << std::endl;
            return 1;
        }
    }

    std::cout << "Invalid Arguments" << std::endl;
    return 0;
}
