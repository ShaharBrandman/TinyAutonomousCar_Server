#include <iostream>
#include <fstream>

#include <vector>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>

#include <json.hpp>

namespace fs = boost::filesystem;

using json = nlohmann::json;

std::unordered_map<std::string, int> labelMap;

cv::Mat extractFeatures(const cv::Mat& image, const cv::Size& targetSize) {
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, targetSize);

    return resizedImage.reshape(1, 1);
}

json train() {
    cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
    svm->setType(cv::ml::SVM::C_SVC);
    svm->setKernel(cv::ml::SVM::LINEAR);

    cv::Mat features, labels;
    
    int labelCounter = 1;

    json trainResult;

    for (const auto& person_folder : {"Shahar", "Gingi"}) {
        for (const auto& entry : fs::directory_iterator("TrainingData/" + std::string(person_folder))) {
            cv::Mat img = cv::imread(entry.path().string());

            if (img.empty()) {
                //std::cout << "train(): Error loading image: " << entry.path().string() << std::endl;
                trainResult["Error"][entry.path().string()] =  "train(): Error loading image";
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

    trainResult["Result"] = "Training has complete";

    return trainResult;
}

json test(const std::string path) {
    //std::cout << "Testing on a new testing image" << std::endl;

    json testResult;

    testResult["TestPath"] = path;

    cv::Mat img = cv::imread(path);

    if (img.empty()) {
        testResult["Error"] = "test(): Error loading image";

        return testResult;
    }

    cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
    svm->setType(cv::ml::SVM::C_SVC);
    svm->setKernel(cv::ml::SVM::LINEAR);

    if (!fs::exists("object_model.xml")) {
        testResult["Error"] = "test() object_model.xml doesn't exists!";

        return testResult;
    }

    svm = cv::Algorithm::load<cv::ml::SVM>("object_model.xml");

    cv::Mat features, labels;

    cv::Mat featureVector = extractFeatures(img, cv::Size(64, 64));

    featureVector.convertTo(featureVector, CV_32F);

    float result = svm->predict(featureVector);

    std::unordered_map<int, std::string> reverseLabelMap;
    for (const auto& entry : labelMap) {
        reverseLabelMap[entry.second] = entry.first;
    }

    testResult["Prediction"] = "Unknown";

    //if result is above 0 and the result is known
    if (result > 0 && reverseLabelMap.find(result) != reverseLabelMap.end()) {
        std::cout << reverseLabelMap[result] << std::endl;
        testResult["Prediction"] = reverseLabelMap[result];
    }

    return testResult;
}

int main(int argc, char* argv[]) {

    if (argc == 1) {
        std::cout << "No Arguments were given" << std::endl;
    }

    for(int i=0; i<argc; i++) {
        if (strcmp(argv[i], "--train") == 0) {
            json trainResult = train();
            std::cout << trainResult << std::endl;
            test("TestingData/Gingi_1.jpeg");
        }
        else if(strcmp(argv[i], "--test") == 0) {
            if(i + 1 != argc) {
                json testResult = test(argv[i+1]);
                std::cout << testResult.dump() << std::endl;
            }
            else {
                json t;
                t["Error"] = "No Testing data was provided";
                std::cout << t.dump() << std::endl;
            }
        }
    }
}