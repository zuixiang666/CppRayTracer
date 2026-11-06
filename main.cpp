#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include<cstdlib>
#include "geometry.h"

// 1. 材质升级：albedo 现在需要 3 个参数（漫反射、高光、反射率）
struct Material {
    Material(const Vec3f &a, const Vec3f &color, const float &spec) : albedo(a), diffuse_color(color), specular_exponent(spec) {}
    Material() : albedo(1, 0, 0), diffuse_color(), specular_exponent() {}
    Vec3f albedo;             // 现在用 Vec3f 存 3 个比例
    Vec3f diffuse_color;
    float specular_exponent;
};

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

struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N * 2.0f * (I * N);
}

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < spheres.size(); i++) {
        float dist_i;
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir * dist_i;
            N = (hit - spheres[i].center).normalize();
            material = spheres[i].material;
        }
    }
    return spheres_dist < 1000;
}

// 2. 核心改动：cast_ray 增加了一个递归深度 depth 参数，默认值为 0
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, size_t depth = 0) {
    Vec3f point, N;
    Material material;

    if (depth > 3 || !scene_intersect(orig, dir, spheres, point, N, material)) {
        return Vec3f(0.2, 0.7, 0.8); // 背景色
    }

    float diffuse_light_intensity = 0, specular_light_intensity = 0;

    for (size_t i = 0; i < lights.size(); i++) {
        Vec3f light_dir = (lights[i].position - point).normalize();
        float light_distance = (lights[i].position - point).norm();

        Vec3f shadow_orig = light_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3; 
        Vec3f shadow_pt, shadow_N;
        Material tmpmaterial;

        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance) {
            continue;
        }

        diffuse_light_intensity += lights[i].intensity * std::max(0.0f, light_dir * N);
        specular_light_intensity += powf(std::max(0.0f, -reflect(-light_dir, N) * dir), material.specular_exponent) * lights[i].intensity;
    }

    // 3. 核心新增：计算反射方向，并递归调用 cast_ray 去找反射的颜色
    Vec3f reflect_dir = reflect(dir, N).normalize();
    Vec3f reflect_orig = reflect_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3; // 同样需要偏移防伪影
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);

    // 4. 颜色混合：漫反射色 + 高光色 + 反射色
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + 
           Vec3f(1., 1., 1.) * specular_light_intensity * material.albedo[1] +
           reflect_color * material.albedo[2];
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int width  = 1024;
    const int height = 768;
    const float aspect_ratio = (float)width / (float)height;
    const float PI = 3.14159265358979323846f;
    
    std::vector<Vec3f> framebuffer(width * height);

        // 循环遍历每个像素
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            
            Vec3f final_color(0, 0, 0); 
            int samples = 64; // ⚠️ 采样次数：4意味着 2x2=4 条光线（想要更平滑可以改成 16，但会变慢）
            int sqrt_samples = (int)sqrt(samples);

            // 在一个像素内部，再分成几份去采样
            for (int m = 0; m < sqrt_samples; m++) {
                for (int n = 0; n < sqrt_samples; n++) {
                    // 计算偏移量，不再是死死盯住中心 0.5，而是在像素内部随机游走
                    float dx = (m + (float)rand() / RAND_MAX) / sqrt_samples;
                    float dy = (n + (float)rand() / RAND_MAX) / sqrt_samples;

                    float x = (2 * (i + dx) / (float)width - 1) * aspect_ratio * tan(PI / 6.0);
                    float y = -(2 * (j + dy) / (float)height - 1) * aspect_ratio * tan(PI / 6.0);
                    Vec3f dir = Vec3f(x, y, -1).normalize(); 
                    
                    // 把这 4 条光线的颜色加起来
                    final_color = final_color + cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
                }
            }
            
            // 取平均值，存入画板
            framebuffer[i + j * width] = final_color * (1.0f / samples);
        }
    }

    std::ofstream ofs;
    ofs.open("./out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height * width; ++i) {
        Vec3f &c = framebuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1) c = c * (1. / max);
        for (size_t j = 0; j < 3; ++j) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}

int main() {
    // 5. 材质升级：albedo 传入 3 个数，最后一个数字越大，反射（像镜子）越强
    Material ivory(Vec3f(0.6, 0.3, 0.1), Vec3f(0.4, 0.4, 0.3), 50.);
    Material red_rubber(Vec3f(0.9, 0.1, 0.0), Vec3f(0.3, 0.1, 0.1), 10.); // 红球不反射
    Material mirror(Vec3f(0.0, 10.0, 0.8), Vec3f(0.5, 0.5, 0.5), 1425.); // ⚠️ 新增一个镜子材质

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-0.5, -0.5, -12), 2, red_rubber));
    // 把蓝球换成一面“镜子”
    spheres.push_back(Sphere(Vec3f( 1.5, 2.0, -18), 3, mirror));

    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
    lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f(30, 20, 30), 1.7));

    render(spheres, lights);
    return 0;
}