#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>

#include "psnr.hpp"
#include "ctpl_stl.h"  // ThreadPool
#include "jpeg_decoder.hpp"
#include "jpeg_encoder.hpp"


typedef struct {
    int x, y;
    const uint8_t* pixels;
} ppm;

ppm* read_ppm(std::string filename) {
    ppm* img;
    FILE* fp;
    char buffer[16];
    int c, rgb;
    //open PPM file for reading
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        fprintf(stderr, "Kan bestand niet openen: '%s'\n", filename.c_str());
        exit(1);
    }

    //alloc memory for ppm struct
    img = (ppm*) malloc(sizeof(ppm));
    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read image format
    if (!fgets(buffer, sizeof(buffer), fp)) {
        perror(filename.c_str());
        exit(1);
    }

    //check the image format
    if (strcmp(buffer, "P6") == 0) {
        fprintf(stderr, "Invalid image format\n");
        exit(1);
    }


    //check for comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n');
        c = getc(fp);
    }
    ungetc(c, fp);

    //read image size information
    fscanf(fp, "%d %d", &img->x, &img->y);

    //read rgb component
    fscanf(fp, "%d", &rgb);

    // Skip lege lijnen
    while (fgetc(fp) != '\n');

    //memory allocation for pixel data
    img->pixels = (uint8_t*) malloc(img->x * img->y * 3 * sizeof(uint8_t));
    if (!img->pixels) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }


    //read pixel data from file
    fread((void*) img->pixels, 3 * img->x, img->y, fp);

    fclose(fp);
    return img;
}

struct compression_task {
    // Task description
    std::string input_file; ///< PPM file
    int quality{0};
    bool chroma_subsampling{false};
    std::string label;

    // Compresion Result
    bool success{false};
    std::string output_file; ///< JPEG file
    uint64_t uncompressed_filesize{0};
    uint64_t jpeg_filesize{0};
    double psnr{0.0}; ///< PSNR in dB
    double rate{0.0}; ///< Bits per pixels (bpp)
};

void compress(compression_task* task) {
    std::cout << task->output_file << "\n";
    ppm* img = read_ppm(task->input_file);

    FILE* output_file = fopen(task->output_file.c_str(), "wb");
    auto myCallback = [&output_file](uint8_t oneByte) { fputc(oneByte, output_file); };
    task->success = JpegEncoder::writeJpeg(myCallback, img->pixels, img->x, img->y, true, task->quality,
                                           task->chroma_subsampling);
    fclose(output_file);

    //psnr
    if (task->success) {


        FILE* FileContent;
        FileContent = fopen(task->output_file.c_str(), "rb");
        fseek(FileContent, 0, SEEK_END);
        long length = ftell(FileContent);
        fseek(FileContent, 0, 0);
        char* bytes = (char*) malloc(length);
        fread(bytes, length, 1, FileContent);
        fclose(FileContent);


        JpegDecoder::Decoder JPGData((const unsigned char*) bytes, (size_t) length);

        task->jpeg_filesize = length;
        task->psnr = PSNR::PSNR((uint8_t*) img->pixels, JPGData.GetImage(),
                                img->x * img->y * 3);
        task->rate = 8 * (((double) length / ((double) (img->x * img->y))));

        std::ofstream myfile;
        myfile.open("../results.csv", std::ios_base::app);
        myfile << task->label << ", "
               << (task->chroma_subsampling ? "true" : "false") << ", "
               << task->psnr << ", "
               << task->rate << "\n";
        myfile.close();
        free(bytes);

    }
    free(img);
}

void opgave2() {
    //4.2
    std::vector<std::string> names = {"test_bw", "test_circle", "test_colorlines", "test_freq", "test_noise","test_noise_bin", "test_star", "big_tree", "kodim01", "kodim05", "kodim11","artificial", "flower_foveon", "leaves_iso_200", "leaves_iso_1600","nightshot_iso_100", "nightshot_iso_1600"};

    //std::vector<std::string> names = {"big_tree"};

    ctpl::thread_pool pool(std::thread::hardware_concurrency());

    for (auto it = names.begin(); it != names.end(); ++it) {
        std::string fileName = *it;
        for (int i = 5; i <= 100; i += 5) {
            // Met chroma
            pool.push([fileName, i](int thread_id) {
                compression_task task;
                task.input_file = std::string("../afbeeldingen/") + fileName + std::string(".ppm");
                task.output_file = std::string("../out/") + fileName + std::to_string(i) + std::string("chroma.jpeg");
                task.quality = i;
                task.chroma_subsampling = true;
                task.label = fileName;
                compress(&task);
            });

            // Zonder chroma
            pool.push([fileName, i](int thread_id) {
                compression_task task;
                task.input_file = std::string("../afbeeldingen/") + fileName + std::string(".ppm");
                task.output_file = std::string("../out/") + fileName + std::to_string(i) + std::string(".jpeg");
                task.quality = i;
                task.label = fileName;
                compress(&task);
            });
        }
    }

    pool.stop(true);
}

void opgave3() {
}

int main() {
    opgave2();

    return 0;
}

