# gif-utils

Utility for converting GIFs to Images, and Images to GIFs

Supported Image file formats: JPEG, PNG, BMP

## Building

To build this project you will need a version of **CMake > 3.12**, a compiler that supports at least **C++17**, and a build system (eg. **make**).

```bash
# clone and setup the repo:
git clone https://github.com/khelmka/gif-utils.git
cd gif-utils

# clone the git submodules
cd deps
git submodule update --init --recursive
cd ..

# create the build directory
mkdir build
cd build

# invoke cmake and make
cmake .. && make
```

## Usage

```bash
-c, --compose   # set operation to GIF Compose
-d, --decompose # set operation to GIF Decompose
-i, --input     # specify Input file path or directory
-o, --output    # specify Output directory or file path
-f              # specify output Format (jpeg|jpg|png|bmp)
```

## Example: Decompose GIF into PNG images
The following command

```bash
./gif-utils -d -i <path/to/file.gif> -f png -o ./out
```

will convert the gif at `path/to/file.gif` into a set of PNG images into the directory `./out`. The images will be named using the format, `n.png`, where `n` is 0-based frame number from the host GIF.

## Dependencies

The following project dependencies are provided as git submodules:
```
https://github.com/adishavit/argh.git
https://github.com/lecram/gifdec.git
https://github.com/lecram/gifenc.git
https://github.com/nothings/stb
```