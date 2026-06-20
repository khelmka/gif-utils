#include <argh.h>
#include <gifdec.h>
#include <gifenc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

int main(int argc, char** argv) {
  using namespace std::literals::string_literals;

  std::string_view programPath(argv[0]);
  programPath.remove_suffix(std::max<std::size_t>(0, programPath.size() - programPath.find_last_of('/')));
  const std::filesystem::path root(programPath.begin(), programPath.end());

  argh::parser cmdl;

  // input: dir/*.png or file.gif
  cmdl.add_param({"-i", "--input"});

  // output: dir or file.gif
  cmdl.add_param({"-o", "--output"});

  // take a gif and compose into images
  cmdl.add_param({"-d", "--decompose"});

  // take a set of images and copose into a gif
  cmdl.add_param({"-c", "--compose"});

  // image output type
  cmdl.add_param({"-f"});

  cmdl.parse(argc, argv);

  if (cmdl[{"-d", "--decompose"}]) {
    // 1. Input filename
    std::string input;
    if (!(cmdl({"-i", "--input"}) >> input)) {
      std ::cerr << "please specify a filepath using -i or --input";
      return EXIT_FAILURE;
    };

    if (auto inputFilepath(root / input); !std::filesystem::exists(inputFilepath)) {
      std::cerr << inputFilepath << " does not exist" << std::endl;
      return EXIT_FAILURE;
    }

    auto inputFilepath = std::filesystem::canonical(root / input);
    if (!std::filesystem::is_regular_file(inputFilepath)) {
      std::cerr << inputFilepath << " is not a file" << std::endl;
      return EXIT_FAILURE;
    }

    // 2. Output directory
    std::string output;
    if (!(cmdl({"-o", "--output"}) >> output)) {
      std::cerr << "please specify a directory using -o or --output" << std::endl;
      return EXIT_FAILURE;
    };

    if (auto outputDirectory(root / output); !std::filesystem::exists(outputDirectory)) {
      std::cerr << outputDirectory << " does not exist" << std::endl;
      return EXIT_FAILURE;
    }

    auto outputDirectory = std::filesystem::canonical(root / output);
    if (!std::filesystem::is_directory(outputDirectory)) {
      std::cerr << outputDirectory << " is not a directory" << std::endl;
      return EXIT_FAILURE;
    }

    // 3. Output file type
    enum class ImageType {
      kPng,
      kJpg,
      kBmp,
    } imageType{};

    const std::unordered_map<std::string, ImageType> allowedImageTypes = {
        {"jpeg"s, ImageType::kJpg},
        {"jpg"s, ImageType::kJpg},
        {"png"s, ImageType::kPng},
        {"bmp"s, ImageType::kBmp},
    };

    std::string imageTypeStr;
    if (!(cmdl({"-f"}) >> imageTypeStr)) {
      std::cerr << "please provide an output file type using -f" << std::endl;
      return EXIT_FAILURE;
    }

    std::for_each(imageTypeStr.begin(), imageTypeStr.end(), [](char& c) -> void { c = (char)std::tolower((int)c); });

    if (auto it = allowedImageTypes.find(imageTypeStr); it != allowedImageTypes.end()) {
      imageType = it->second;
    } else {
      std::cerr << imageTypeStr << "is not a supported output image type" << std::endl;
      std::cerr << "please choose from the following:" << std::endl;
      for (const auto& [key, value] : allowedImageTypes) {
        std::cerr << "* " << key << std::endl;
      }
      return EXIT_FAILURE;
    }

    // 4. Attempt to load the gif
    gd_GIF* gif = gd_open_gif(inputFilepath.c_str());
    if (!gif) {
      std::cerr << "failed to open gif: " << inputFilepath << std::endl;
      return EXIT_FAILURE;
    }

    const unsigned width = gif->width;
    const unsigned height = gif->height;

    std::cout << "width: " << width << "px" << std::endl;
    std::cout << "height: " << height << "px" << std::endl;

    // 5. load each frame from the gif file
    std::vector<std::unique_ptr<std::uint8_t[]>> textureStorages;

    while (gd_get_frame(gif)) {
      // load and commit to memory:
      auto storageRGB = std::make_unique<std::uint8_t[]>(width * height * 3u);
      gd_render_frame(gif, storageRGB.get());
      auto storageRGBA = std::make_unique<std::uint8_t[]>(width * height * 4u);

      for (unsigned row{}; row < height; row++) {
        for (unsigned col{}; col < width; col++) {
          unsigned pos = row * width + col;
          std::uint8_t* color = &(storageRGB[pos * 3u]);
          if (gd_is_bgcolor(gif, color)) {
            storageRGBA[pos * 4u + 0u] = 0x00;
            storageRGBA[pos * 4u + 1u] = 0x00;
            storageRGBA[pos * 4u + 2u] = 0x00;
            storageRGBA[pos * 4u + 3u] = 0x00;
          } else {
            storageRGBA[pos * 4u + 0u] = color[0];
            storageRGBA[pos * 4u + 1u] = color[1];
            storageRGBA[pos * 4u + 2u] = color[2];
            storageRGBA[pos * 4u + 3u] = 0xff;
          }
        }
      }

      // commit the storage to memory:
      textureStorages.push_back(std::move(storageRGBA));
      std::cout << "loaded " << textureStorages.size() << " frames" << std::endl;
    }

    gd_close_gif(gif);

    // 6. iterate over the textures and save to disk
    for (std::size_t i{}; i < textureStorages.size(); i++) {
      const auto& textureStorage = textureStorages.at(i);
      auto name = std::to_string(i).append(".").append(imageTypeStr);
      std::filesystem::path path(outputDirectory / name);
      std::cout << "saving frame " << i << " to " << path << std::endl;

      switch (imageType) {
        case ImageType::kPng: {
          const int stride_bytes = width * 4;
          stbi_write_png(path.c_str(), width, height, 4, textureStorage.get(), stride_bytes);
          break;
        }
        case ImageType::kJpg: {
          const int quality = 100;
          stbi_write_jpg(path.c_str(), width, height, 4, textureStorage.get(), quality);
          break;
        }
        case ImageType::kBmp: {
          stbi_write_bmp(path.c_str(), width, height, 4, textureStorage.get());
          break;
        }
      }
    }

    return EXIT_SUCCESS;
  } else if (cmdl[{"-d", "--decompose"}]) {
    return EXIT_SUCCESS;
  }

  std::cerr << "please provide an operation" << std::endl;
  return EXIT_FAILURE;
}