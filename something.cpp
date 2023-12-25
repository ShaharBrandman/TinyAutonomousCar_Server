#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/dnn.h>

using namespace std;
using namespace cv;

// Function to compute face descriptors for a given image
dlib::matrix<float, 0, 1> computeFaceDescriptor(const string& image_path, dlib::shape_predictor& sp, dlib::anet_type& net) {
    // Load the image
    Mat image = imread(image_path);

    // Convert OpenCV image to Dlib format
    dlib::cv_image<dlib::bgr_pixel> dlib_image(image);

    // Load pre-trained face detection model from Dlib
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

    // Detect faces in the image
    std::vector<dlib::rectangle> faces = detector(dlib_image);

    if (faces.empty()) {
        cerr << "No faces found in the image: " << image_path << endl;
        throw std::runtime_error("No faces found in the image.");
    }

    // Find facial landmarks
    dlib::full_object_detection shape = sp(dlib_image, faces[0]);

    // Compute the face descriptor using FaceNet
    dlib::matrix<float, 0, 1> face_descriptor = net(dlib_image, shape);

    return face_descriptor;
}

// Function to train the FaceNet model on a dataset
void trainFaceNetModel(const vector<string>& training_images, dlib::shape_predictor& sp, dlib::anet_type& net) {
    std::vector<dlib::matrix<float, 0, 1>> face_descriptors;
    std::vector<std::string> labels;

    // Compute face descriptors for each training image
    for (const auto& image_path : training_images) {
        try {
            dlib::matrix<float, 0, 1> face_descriptor = computeFaceDescriptor(image_path, sp, net);
            face_descriptors.push_back(face_descriptor);
            labels.push_back(image_path);
        } catch (const std::exception& e) {
            // Handle exceptions, e.g., if no face is found in the image
            cerr << "Error processing image: " << image_path << " - " << e.what() << endl;
        }
    }

    // Train your recognition model using the computed face descriptors and labels
    // You can use any machine learning library to train a classifier, e.g., SVM or k-NN
    // Here, we just print the labels and descriptors for demonstration purposes
    for (size_t i = 0; i < face_descriptors.size(); ++i) {
        cout << "Label: " << labels[i] << ", Descriptor: " << face_descriptors[i] << endl;
    }

    // Add your training logic here
}

// Function to test the FaceNet model on new images
void testFaceNetModel(const vector<string>& testing_images, dlib::shape_predictor& sp, dlib::anet_type& net) {
    // Test the FaceNet model on new images
    for (const auto& image_path : testing_images) {
        try {
            dlib::matrix<float, 0, 1> face_descriptor = computeFaceDescriptor(image_path, sp, net);

            // Implement your recognition logic here
            // Compare the computed descriptor with the trained descriptors
            // You can use distance metrics like Euclidean distance or cosine similarity
            // to find the closest match
            // Here, we just print the label and descriptor for demonstration purposes
            cout << "Testing Image: " << image_path << ", Descriptor: " << face_descriptor << endl;

        } catch (const std::exception& e) {
            // Handle exceptions, e.g., if no face is found in the image
            cerr << "Error processing image: " << image_path << " - " << e.what() << endl;
        }
    }
}

int main() {
    try {
        // Load pre-trained face detection model from Dlib
        dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

        // Load pre-trained shape predictor from Dlib
        dlib::shape_predictor sp;
        dlib::deserialize("shape_predictor_5_face_landmarks.dat") >> sp;

        // Load pre-trained FaceNet model from Dlib
        dlib::anet_type net;
        dlib::deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;

        // Example dataset for training and testing
        vector<string> training_images = {
            "TrainingData/Shahar/1.jpg",
            "TrainingData/Shahar/2.jpg"
        };

        vector<string> testing_images = {
            "TestingData/WhatsApp Image 2023-11-27 at 16.04.55 (2).jpeg",
            "TestingData/WhatsApp Image 2023-11-27 at 16.04.54.jpeg"
        };

        // Train the FaceNet model on the training dataset
        trainFaceNetModel(training_images, sp, net);

        // Test the FaceNet model on new images
        testFaceNetModel(testing_images, sp, net);

    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
