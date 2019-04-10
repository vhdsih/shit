//
// Created by idz on 2019/4/2.
//

#include <zconf.h>
#include <opencv2/opencv.hpp>
#include "Server.h"
#include "../utils/utils.h"

bool Server::init() {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        std::cout << "create sockets fail!" << std::endl;
        return false;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    int ret = bind(fd, (struct sockaddr*)&server, sizeof(server));
    if (ret < 0) {
        std::cout << "sockets bind fail!" << std::endl;
        return false;
    }
    std::cout << "sockets yes!" << std::endl;
    return true;
}


void Server::recv_photo() {
    u_char mode, where;
    int recv_len = -1, copy_len;
    // get photo info
    std::cout << "SERVER::Photo Getting Info ..." << std::endl;
    if (recv_into_buff(recv_len)) {
        std::cout << "SERVER::Photo Getting Info ..." << std::endl;
        if (buffer[0] != 0x55) {
            std::cout << "not an image info segment" << std::endl;
            return;
        }
    }
    memcpy(&mode, &buffer[4], 1);
    get_height_width(mode, height, width);
    copy_len = width * 3 / 2;
    // get data
    std::cout << "SERVER::Photo Getting content ..." << std::endl;
    cv::Mat photo(height, width, CV_8UC3, cv::Scalar::all(0));
    u_char op, id;
    short nline = 0;
    while (recv_into_buff(recv_len)) {
        if (buffer[0] != 0xAA) {
            std::cout << buffer[0] << "not an image line" << std::endl;
            return;
        }
        memcpy(&id, &buffer[10], sizeof(char));
        memcpy(&nline, &buffer[12], sizeof(short));
        if (mode == 0x00) {
            memcpy(&where, &buffer[16], 1);
            if (where == 0x55) {
                memcpy(&photo.data[nline * width * 3], &buffer[18], copy_len);
            } else {
                memcpy(&photo.data[nline * width * 3 + copy_len], &buffer[18], copy_len);
                ++nline;
            }
        } else {
            memcpy(&photo.data[nline * width * 3], &buffer[18], width * sizeof(char) * 3);
            ++nline;
        }
        memcpy(&op, &buffer[17], 1);
        if (op == 0xAA)
            break;
        std::cout << nline << std::endl;
    }
    std::cout << "SERVER::Photo Stat: y" << std::endl;
    cv::imwrite(std::to_string(photo_code++) + ".png", photo);
}

bool Server::recv_into_buff(int &recv_len) {
    memset(buffer, 0, BUFFER_SIZE);
    recv_len = recvfrom(fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&peer, &len);
    if (recv_len == -1) {
        std::cout << "receive data fail!" << std::endl;
        return false;
    }
    return true;
}