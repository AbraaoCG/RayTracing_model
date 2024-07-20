#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <tuple>
#include <memory>
#include <algorithm>
#include <limits>
#include <iterator>
#include <omp.h> // Include OpenMP header
#include <chrono> // Include chrono for timing

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // Inclua o arquivo stb_image_write.h

// Definir a estrutura Vec3 e Sphere
struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float t) const { return Vec3(x * t, y * t, z * t); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }

    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalize() const { float len = length(); return Vec3(x / len, y / len, z / len); }

    static float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    // Overload para multiplicar float por Vec3
    friend Vec3 operator*(float t, const Vec3& v) { return Vec3(t * v.x, t * v.y, t * v.z); }
};

struct Sphere {
    Vec3 center;
    float radius;
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float shininess;
    float reflection;
};

// Função de normalização
Vec3 normalize(const Vec3& v) {
    float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return Vec3(v.x / length, v.y / length, v.z / length);
}

// Função de reflexão
Vec3 reflected(const Vec3& v, const Vec3& axis) {
    return v - axis * 2.0f * Vec3::dot(v, axis);
}

// Função de interseção de esfera
float sphere_intersect(const Vec3& center, float radius, const Vec3& ray_origin, const Vec3& ray_direction) {
    Vec3 oc = ray_origin - center;
    float a = Vec3::dot(ray_direction, ray_direction);
    float b = 2.0f * Vec3::dot(ray_direction, oc);
    float c = Vec3::dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return std::numeric_limits<float>::infinity();
    } else {
        float sqrt_discriminant = std::sqrt(discriminant);
        float t1 = (-b - sqrt_discriminant) / (2.0f * a);
        float t2 = (-b + sqrt_discriminant) / (2.0f * a);
        if (t1 > 0 && t2 > 0) {
            return std::min(t1, t2);
        }
        return std::numeric_limits<float>::infinity();
    }
}


// Função para encontrar o objeto mais próximo
std::tuple<std::shared_ptr<Sphere>, float> nearest_intersected_object(const std::vector<std::shared_ptr<Sphere>>& objects, const Vec3& ray_origin, const Vec3& ray_direction) {
    float min_distance = std::numeric_limits<float>::infinity();
    std::shared_ptr<Sphere> nearest_object = nullptr;
    for (const auto& sphere : objects) {
        float distance = sphere_intersect(sphere->center, sphere->radius, ray_origin, ray_direction);
        if (distance < min_distance) {
            min_distance = distance;
            nearest_object = sphere;
        }
    }
    return std::make_tuple(nearest_object, min_distance);
}

// Função principal
int main(int argc, char* argv[]) {

    int num_threads = 1; // Valor padrão de threads

    // Processar argumentos da linha de comando
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-t" && i + 1 < argc) {
            num_threads = std::stoi(argv[++i]);
        }
    }

    // Configurar o OpenMP para usar o número de threads fornecido
    omp_set_num_threads(num_threads);

    // const int width = 300;
    // const int height = 200;
    const int width = 1920;
    const int height = 1080;
    const int max_depth = 3;

    Vec3 camera(0, 0, 1);
    float ratio = float(width) / height;
    std::vector<float> screen = { -1, 1 / ratio, 1, -1 / ratio };

    Vec3 light_position(5, 5, 5);
    Vec3 light_ambient(1, 1, 1);
    Vec3 light_diffuse(1, 1, 1);
    Vec3 light_specular(1, 1, 1);

    std::vector<std::shared_ptr<Sphere>> objects = {
        std::make_shared<Sphere>(Sphere{ Vec3(-1, 0, -1), 0.7, Vec3(0.1, 0, 0), Vec3(0.7, 0, 0), Vec3(1, 1, 1), 100, 0.5 }),
        std::make_shared<Sphere>(Sphere{ Vec3(1, 0, -1), 0.7, Vec3(0.1, 0, 0), Vec3(0.7, 0, 0), Vec3(1, 1, 1), 100, 0.5 }),
        std::make_shared<Sphere>(Sphere{ Vec3(0.1, -0.4, -1.08), 0.2, Vec3(0.1, 0, 0.1), Vec3(0.7, 0, 0.7), Vec3(1, 1, 1), 100, 0.5 }),
        std::make_shared<Sphere>(Sphere{ Vec3(-0.3, 0, 0), 0.15, Vec3(0, 0.1, 0), Vec3(0, 0.6, 0), Vec3(1, 1, 1), 100, 0.5 }),
        
        // esferas refletidas atrás da câmera
        std::make_shared<Sphere>(Sphere{ Vec3(-2, 0.7, 1), 0.2, Vec3(0.1, 0.5, 0.5), Vec3(0.5, 0.5, 0.5), Vec3(1, 1, 1), 50, 0.2 }),
        std::make_shared<Sphere>(Sphere{ Vec3(-1, 0, 1), 0.2, Vec3(0.1, 0.5, 0.5), Vec3(0.5, 0.5, 0.5), Vec3(1, 1, 1), 50, 0.2 }),
        std::make_shared<Sphere>(Sphere{ Vec3(-0.5, 0.5, 1), 0.2, Vec3(0.1, 0.5, 0.5), Vec3(0.5, 0.5, 0.5), Vec3(1, 1, 1), 50, 0.2 }),

        // Esfera vertical para criar "chão"
        std::make_shared<Sphere>(Sphere{ Vec3(0, -9000, 0), 9000 - 0.7, Vec3(0.1, 0.1, 0.1), Vec3(0.6, 0.6, 0.6), Vec3(1, 1, 1), 100, 0.5 })
    
    };

    // // Adicionar N esferas para teste de paralelismo.
    // for (int i = 0; i < 200; ++i) {
    //     float x = (rand() % 2000 - 1000) / 1000.0f;
    //     float y = (rand() % 2000 - 1000) / 1000.0f;
    //     float z = -1 - (rand() % 1000) / 1000.0f;
    //     float radius = 0.05f + (rand() % 100) / 1000.0f;
    //     Vec3 color(rand() % 100 / 100.0f, rand() % 100 / 100.0f, rand() % 100 / 100.0f);

    //     objects.push_back(std::make_shared<Sphere>(Sphere{ Vec3(x, y, z), radius, color * 0.2f, color * 0.7f, Vec3(1, 1, 1), 100, 0.5 }));
    // }

    // Iniciar temporizador
    auto start_time = std::chrono::high_resolution_clock::now();

    // Renderização da imagem
    std::vector<std::vector<Vec3>> image(height, std::vector<Vec3>(width));

    #pragma omp parallel for
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            Vec3 pixel(screen[0] + (screen[2] - screen[0]) * (j + 0.5f) / width,
                       screen[1] + (screen[3] - screen[1]) * (i + 0.5f) / height,
                       0);
            Vec3 origin = camera;
            Vec3 direction = normalize(pixel - origin);

            Vec3 color(0, 0, 0);
            float reflection = 1;

            for (int k = 0; k < max_depth; ++k) {
                auto [nearest_object, min_distance] = nearest_intersected_object(objects, origin, direction);
                if (!nearest_object) {
                    break;
                }

                Vec3 intersection = origin + direction * min_distance;
                Vec3 normal_to_surface = normalize(intersection - nearest_object->center);
                Vec3 shifted_point = intersection + normal_to_surface * 1e-5f;
                Vec3 intersection_to_light = normalize(light_position - shifted_point);

                auto [shadow_obj, shadow_distance] = nearest_intersected_object(objects, shifted_point, intersection_to_light);
                float intersection_to_light_distance = (light_position - intersection).length();
                bool is_shadowed = shadow_obj && shadow_distance < intersection_to_light_distance;

                if (is_shadowed) {
                    break;
                }

                Vec3 illumination(0, 0, 0);
                illumination += nearest_object->ambient * light_ambient;
                illumination += nearest_object->diffuse * light_diffuse * std::max(0.0f, Vec3::dot(intersection_to_light, normal_to_surface));

                Vec3 intersection_to_camera = normalize(camera - intersection);
                Vec3 H = normalize(intersection_to_light + intersection_to_camera);
                illumination += nearest_object->specular * light_specular * std::pow(std::max(0.0f, Vec3::dot(normal_to_surface, H)), nearest_object->shininess / 4);

                color = color + reflection * illumination;
                reflection *= nearest_object->reflection;

                origin = shifted_point;
                direction = reflected(direction, normal_to_surface);

                // Debug
                // std::cout << "Pixel (" << i << ", " << j << ") Iter " << k << ": "
                //           << "Illumination = (" << illumination.x << ", " << illumination.y << ", " << illumination.z << ") "
                //           << "Color = (" << color.x << ", " << color.y << ", " << color.z << ") "
                //           << "Reflection = " << reflection << "\n";

            }
            image[i][j] = Vec3(std::min(1.0f, color.x), std::min(1.0f, color.y), std::min(1.0f, color.z));
        }
    }

    // Parar temporizador e calcular tempo de execução
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    // Imprimir tempo de renderização
    std::cout << "Renderização concluída em " << duration.count() << " segundos.\n";


    // // Salvar imagem no formato PPM
    // std::ofstream ofs("image.ppm", std::ios::out | std::ios::binary);
    // ofs << "P6\n" << width << " " << height << "\n255\n";
    // for (const auto& row : image) {
    //     for (const auto& pixel : row) {
    //         ofs << static_cast<unsigned char>(pixel.x * 255)
    //             << static_cast<unsigned char>(pixel.y * 255)
    //             << static_cast<unsigned char>(pixel.z * 255);
    //     }
    // }
    // ofs.close();

    // Salvar imagem no formato PNG usando stb_image_write
    std::vector<unsigned char> png_image(width * height * 3); // RGB format

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            Vec3 pixel = image[i][j];
            png_image[(i * width + j) * 3 + 0] = static_cast<unsigned char>(pixel.x * 255);
            png_image[(i * width + j) * 3 + 1] = static_cast<unsigned char>(pixel.y * 255);
            png_image[(i * width + j) * 3 + 2] = static_cast<unsigned char>(pixel.z * 255);
        }
    }

    stbi_write_png("image.png", width, height, 3, png_image.data(), width * 3);

    return 0;
}
