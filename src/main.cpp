#include "GxIAPI.h"
#include "DxImageProc.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <csignal>
#include <chrono>
#include <thread>
#include <string>

GX_DEV_HANDLE gDevice = nullptr;
bool running = true;

void cleanup() {
    if (gDevice) {
        GXStreamOff(gDevice);
        GXCloseDevice(gDevice);
    }
    GXCloseLib();
    std::cout << "Camera stopped and SDK closed." << std::endl;
}

void signalHandler(int) {
    running = false;
}

int main() {
    signal(SIGINT, signalHandler);  // Handle Ctrl+C
    GX_STATUS status = GXInitLib();
    if (status != GX_STATUS_SUCCESS) {
        std::cerr << "Failed to initialize SDK." << std::endl;
        return -1;
    }

    // Enumerate and open camera
    uint32_t device_num = 0;
    GXUpdateDeviceList(&device_num, 1000);
    if (device_num == 0) {
        std::cerr << "No Daheng cameras found." << std::endl;
        GXCloseLib();
        return -1;
    }

    status = GXOpenDeviceByIndex(1, &gDevice);
    if (status != GX_STATUS_SUCCESS) {
        std::cerr << "Failed to open camera." << std::endl;
        GXCloseLib();
        return -1;
    }

    // Setup camera
    int64_t width, height, payload_size;
    GXGetInt(gDevice, GX_INT_WIDTH, &width);
    GXGetInt(gDevice, GX_INT_HEIGHT, &height);
    GXGetInt(gDevice, GX_INT_PAYLOAD_SIZE, &payload_size);

    GXSetEnum(gDevice, GX_ENUM_PIXEL_FORMAT, GX_PIXEL_FORMAT_BAYER_RG8);

    GXStreamOn(gDevice);

    std::vector<unsigned char> rgb_buffer(width * height * 3);
    PGX_FRAME_BUFFER frame = nullptr;

    std::cout << "Capturing frames... Press Ctrl+C to stop.\n";
    int frame_id = 0;

    while (running) {
        status = GXDQBuf(gDevice, &frame, 1000);
        if (status != GX_STATUS_SUCCESS || frame->nStatus != GX_FRAME_STATUS_SUCCESS) {
            continue;
        }

        // Convert Bayer to RGB
        DxRaw8toRGB24((unsigned char*)frame->pImgBuf, rgb_buffer.data(),
                      width, height, RAW2RGB_NEIGHBOUR, BAYERRG, false);

        // Create OpenCV Mat
        cv::Mat img(height, width, CV_8UC3, rgb_buffer.data());

        // Write "Frame N" on the image
        std::string label = "Frame " + std::to_string(frame_id);
        cv::putText(img, label, {50, 50}, cv::FONT_HERSHEY_SIMPLEX, 1.0, {255, 255, 255}, 2);

        // Save to JPG
        std::string filename = "frame_" + std::to_string(frame_id++) + ".jpg";
        cv::imwrite(filename, img);
        std::cout << "Saved: " << filename << std::endl;

        GXQBuf(gDevice, frame);

        // Wait 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    cleanup();
    return 0;
}
