#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

#include "geometry.h"

void render() {
    const int width  = 1024;
    const int height = 768;
    
    // 创建一维的帧缓冲区
    std::vector<Vec3f> framebuffer(width * height);

    // 给每个像素上色
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            // 正确的一维索引方式
            framebuffer[i + j * width] = Vec3f(float(j) / height, float(i) / width, 0.0f);
        }
    }

    // 保存为 PPM 图片
    std::ofstream ofs;
   ofs.open("./out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    
    for (size_t i = 0; i < height * width; ++i) {
        // 把 Vec3f 的 x, y, z 转成 0-255 的字符输出去
        ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i].x)));
        ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i].y)));
        ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i].z)));
    }
    ofs.close();
}

int main() {
    render();
    return 0;
}