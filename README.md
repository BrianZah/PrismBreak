# PrismBreak
PrismBreak is designed for exploring multidimensional Mixture Models, supporting both multivariate Gaussian and Student-t Mixture Models. The tool enables users to incrementally construct a three-dimensional viewspace for data analysis.

When a dataset is opened via the built-in file explorer, PrismBreak presents a multifaceted prism visualization. The first facet displays the mixture model for each dataset attribute. The second facet represents the Principal Component Analysis (PCA) vectors of the entire dataset. Each subsequent facet applies PCA to an individual component of the mixture model.

Users begin by selecting a basis vector, which becomes the x-axis, transitioning the prism to the second stage. Here, two-dimensional projections of the mixture model are shown, allowing the user to select the y-axis. In the third stage, the user chooses the z-axis, finalizing the three-dimensional detailed view. At this stage, individual data points are also displayed for deeper analysis.

## Video and Paper
A video demonstration of the tool's workflow will be available soon (link forthcoming). Further details are discussed in the accompanying paper (link forthcoming).

## Prequisits
PrismBreak requires CMake version 3.23 and the GNU Compiler Collection (GCC) version 12.2. Older versions may also be compatible, but they have not been tested.

## Installation
### Linux
1. Download the repository and create a build directory within it.
2. Navigate to the build directory using the command line and execute the following commands:

Generate build files:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```
Compile the executable::
```bash
cmake --build .
```
Run the program:
```bash
./main.exe
```
**Our Setup**: The tool was mainly developed on a machine running debian 12. The computer had an rtx 3070 ti and intel i7 12700k.
### Windows
1. Download the repository and create a build directory within it.
2. Navigate to the build directory using the command line and execute the following commands:

Generate build files using the Ninja Generator (see below in "Our Setup" for further details):
```bash
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release
```
Compile the executable::
```bash
cmake --build .
```
Run the program:
```bash
./main.exe
```
**Our Setup**: The tool was tested on a machine running Windows 11 with an AMD Ryzen 5800X3D and an RTX 4080 Super. We used MSYS2 (msys2.org) to set up the build environment, which includes installing Minimalist GNU for Windows (MinGW) and CMake. MSYS2 recommends using the Ninja generator. (https://www.msys2.org/docs/cmake/)

## Data
### Preprocessing
To use the provided clustering script, R (https://cran.r-project.org/) and the teigen package (DOI: https://doi.org/10.18637/jss.v083.i07) needs to be installed (https://www.rdocumentation.org/packages/teigen/versions/2.2.2). The script `scripts/Clustering.r` creates a directory containing the original `data.csv` file, along with separate subdirectories for the Gaussian Mixture Model and the t-Distribution Mixture Model. This directory can then be opened using the file manager in our tool."
### Examples
The data directory contains three real-world datasets discussed in the paper:
1. Country Data: https://www.kaggle.com/datasets/rohan0301/unsupervised-learning-on-country-data
2. Shanghai Ranking 2024: https://www.kaggle.com/datasets/computingvictor/2024-academic-ranking-of-world-universities
3. Tripadviser Reviews: https://www.kaggle.com/datasets/abdullah0a/urban-air-quality-and-health-impact-dataset
We also provide the simplex datasets used for the performance tests in our paper. A script to generate these datasets is available in the scripts directory.
## Controls
[shift]+[left mouse button] select axis / enter next stage  
[shift]+[right mouse button] enter previous stage  
[ctrl]+[left mouse button] bookmark tile, when applied on prism; focus camera on tile when applied on bookmarks list  
[ctrl]+[right mouse button] debookmark tile  
[alt]+[left mouse button] only available in third stage rotate model in box  
[A] focus left facet  
[D] focus right facet  
[R] reset view  
[esc] end progam  
[1] recompile shaders (for developers)  

## Third party libs
We used the following libraries from third parties:
- Eigen: https://eigen.tuxfamily.org/
- imgui: https://github.com/ocornut/imgui
- ImGuiFileDialog: https://github.com/aiekick/ImGuiFileDialog
- glad: https://glad.dav1d.de/
- glfw: https://www.glfw.org/
- glm: https://github.com/g-truc/glm
- stb: https://github.com/nothings/stb
- libs/pass/shader/libs/text.glsl: https://www.shadertoy.com/view/dsGXDt

## Adaptations
### Controls
Can be adopted in `libs/vis/vis/Controls.hpp` and `libs/vis/vis/src/Controlls.cpp`.
### Fontsize
Can be adopted in `libs/gui/CMakeLists.txt` by changing `CMAKE_GUIFONTSIZE`.
### Default Data Set
Can be adopted in `libs/gui/CMakeLists.txt` by changing `CMAKE_DEFAULTDATASET`.
