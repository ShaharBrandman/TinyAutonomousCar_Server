#include <iostream>
#include <fstream>

#include <vector>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>

#define STREAM_URL "http://192.168.1.1/stream"

namespace fs = boost::filesystem;

std::unordered_map<std::string, int> labelMap;

cv::Mat extractFeatures(const cv::Mat& image, const cv::Size& targetSize) {
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, targetSize);

    return resizedImage.reshape(1, 1);
}

void train() {
    cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
    svm->setType(cv::ml::SVM::C_SVC);
    svm->setKernel(cv::ml::SVM::LINEAR);

    cv::Mat features, labels;
    
    int labelCounter = 1;

    for (const auto& person_folder : {"Shahar", "Gingi"}) {
        for (const auto& entry : fs::directory_iterator("TrainingData/" + std::string(person_folder))) {
            cv::Mat img = cv::imread(entry.path().string());

            if (img.empty()) {
                std::cerr << "Error loading image: " << entry.path().string() << std::endl;
                continue;
            }

            cv::Mat featureVector = extractFeatures(img, cv::Size(64, 64));

            featureVector.convertTo(featureVector, CV_32F);

            //std::cout << "Feature vector dimensions: " << featureVector.size() << std::endl;
            //std::cout << "Features matrix dimensions before push_back: " << features.size() << std::endl;

            features.push_back(featureVector);
            
            if (labelMap.find(person_folder) == labelMap.end()) {
                labelMap[person_folder] = labelCounter++;
            }
            
            int numericLabel = labelMap[person_folder];

            labels.push_back(numericLabel);

            //std::cout << "Features matrix dimensions after push_back: " << features.size() << std::endl;
        }
    }

    labels.convertTo(labels, CV_32S);

    //train the SVM model
    svm->train(features, cv::ml::ROW_SAMPLE, labels);

    svm->save("object_model.xml");

    std::cout << "Training complete." << std::endl;
}

void test() {
    std::cout << "Testing on a new testing image" << std::endl;

    cv::Mat img = cv::imread(STREAM_URL);

    if (img.empty()) {
        std::cerr << "Error loading image: " << STREAM_URL << std::endl;
        return;
    }

    cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
    svm->setType(cv::ml::SVM::C_SVC);
    svm->setKernel(cv::ml::SVM::LINEAR);

    svm = cv::Algorithm::load<cv::ml::SVM>("object_model.xml");

    cv::Mat features, labels;

    cv::Mat featureVector = extractFeatures(img, cv::Size(64, 64));

    featureVector.convertTo(featureVector, CV_32F);

    float result = svm->predict(featureVector);

    std::unordered_map<int, std::string> reverseLabelMap;
    for (const auto& entry : labelMap) {
        reverseLabelMap[entry.second] = entry.first;
    }

    std::ofstream outputFile("OutputData/label.txt");
    if (outputFile.is_open()) {
        outputFile << "Prediction: ";

        if (result > 0 && reverseLabelMap.find(result) != reverseLabelMap.end()) {
            outputFile << reverseLabelMap[result] << std::endl;
        } else {
            outputFile << "Unknown" << std::endl;
        }

        outputFile.close();
    } else {
        std::cerr << "Error opening OutputData/label.txt for writing" << std::endl;
    }
}

int main() {
    train();

    test();
}