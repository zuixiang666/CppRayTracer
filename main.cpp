#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include "geometry.h"

// 1. 定义材质结构
struct Material {
    Vec3f diffuse_color; // 漫反射颜色
    Material(const Vec3f &color) : diffuse_color(color) {}
    Material() : diffuse_color() {}
};

// 2. 定义球体结构 (现在球有了自己的材质)
struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        Vec3f L = center - orig;
        float tca = L * dir;
        float d2 = L * L - tca * tca;
        if (d2 > radius * radius) return false;
        float thc = sqrtf(radius * radius - d2);
        t0 = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};

// 3. 定义光源结构
struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

// 4. 核心渲染函数：计算光线打中球后的颜色
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    float closest_t = std::numeric_limits<float>::max();
    int hit_index = -1;

    // 找最近的球
    for (size_t k = 0; k < spheres.size(); k++) {
        float t0;
        if (spheres[k].ray_intersect(orig, dir, t0)) {
            if (t0 < closest_t) {
                closest_t = t0;
                hit_index = k;
            }
        }
    }

    // 如果没打中任何球，返回背景色
    if (hit_index == -1) {
        return Vec3f(0.2, 0.2, 0.2); 
    }

    // 计算交点坐标
    Vec3f point = orig + dir * closest_t;
    // 计算交点处的法向量 (球心指向交点)
    Vec3f N = (point - spheres[hit_index].center).normalize();

    // 计算光照强度（核心逻辑）
    float diffuse_light_intensity = 0;
    for (size_t i = 0; i < lights.size(); i++) {
        // 从交点指向光源的方向
        Vec3f light_dir = (lights[i].position - point).normalize();
        // 点乘：夹角越小越亮。用 max 防止出现负数（背光面）
        diffuse_light_intensity += lights[i].intensity * std::max(0.0f, light_dir * N);
    }

    // 最终颜色 = 材质颜色 * 光照强度
    return spheres[hit_index].material.diffuse_color * diffuse_light_intensity;
}

void render() {
    const int width  = 1024;
    const int height = 768;
    const float aspect_ratio = (float)width / (float)height;
    const float PI = 3.14159265358979323846f;
    
    std::vector<Vec3f> framebuffer(width * height);

    // 创建材质和球体
    Material ivory(Vec3f(0.4, 0.4, 0.3)); // 象牙白材质
    Material red_rubber(Vec3f(0.3, 0.1, 0.1)); // 红色橡胶材质
    Material blue(Vec3f(0.1, 0.2, 0.8)); // 蓝色材质

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber));
    spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, blue));

    // 创建一个光源：位置在 (-20, 20, 20)，强度为 1.5
    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20,  20), 1.5));

    // 遍历每个像素
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            float x = (2 * (i + 0.5) / (float)width - 1) * aspect_ratio * tan(PI / 6.0);
            float y = -(2 * (j + 0.5) / (float)height - 1) * tan(PI / 6.0);
            Vec3f dir = Vec3f(x, y, -1).normalize(); 

            // 发射光线并获取颜色
            framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
        }
    }

    // 保存文件
    std::ofstream ofs;
    ofs.open("./out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height * width; ++i) {
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