# STM32 Library Collection

Useful, header-only C++ library collection for STM32 microcontrollers. 

## Key Features

+ Modern C++ (C++23).

+ Simple Integration with CMake.

+ Provide wrapper classes over STM32 HAL drivers.

## Getting Started

1. **Install STM32CubeMX**

    Download and install STM32CubeMX from the [official website](https://www.st.com/en/development-tools/stm32cubemx.html).

2. **Install Git**

    Download and install Git from the [official website](https://git-scm.com/).

3. **Install Visual Studio Code**

    Download and install Visual Studio Code from the [official website](https://code.visualstudio.com/).

4. **Install the STM32CubeIDE for Visual Studio Code Extension**

    + Open Visual Studio Code.

    + Go to the Extensions view by clicking on the Extensions icon in the Activity Bar on the side of the window or by pressing `Ctrl+Shift+X`.

    + Search for `STM32CubeIDE for Visual Studio Code`.

    + Click `Install` on the [STM32CubeIDE for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension) extension.

5. **Generate Code Using STM32CubeMX**

    + Open STM32CubeMX.

    + Create a new project and configure your STM32 microcontroller as needed.

    + In the `Project Manager` tab, set the `Toolchain / IDE` to `CMake`.

    + Click on `Generate Code` to generate the project files.

6. **Open the Generated Project in Visual Studio Code**

    + Open Visual Studio Code.

    + Click on `File` > `Open Folder...` and select the folder where STM32CubeMX generated the project files.

    + Follow the instructions provided by the STM32CubeIDE for Visual Studio Code extension to set up the project.

    + Click on `STM32CubeIDE` extension icon > `Setup STM32Cube project(s)`, then select your STM32 Board/Device and set the `Toolchain` to `GCC`.

    + Rename the `main.c` file to `main.cpp` and update the MX_Application_Src from the CMakeLists.txt file.

    + Add the following compile flags to the `.clangd` file in the root directory of your project as shown below:

    ```
    CompileFlags:
      Add:
    - '-ferror-limit=0'
    - '-Wno-implicit-int'
    - '-std=c++23'
    - '-Wall'
    - '-Wextra'
    - '-Wfatal-errors'
    - '-Wpedantic'
    ```

7. **Add STM32 Library Collection to Your Project**

    + Add STM32 Library Collection as a submodule to your project repository by running the following command in your project directory:

    ```bash
    git submodule add git@github.com:Uydutepe-Takimi/stm32-library-collection.git External/STM32LibraryCollection
    ```

    + Initialize and update the submodule:

    ```bash
    git submodule update --init --recursive
    ```

    + Add the following lines to your top-level `CMakeLists.txt` file:

    ```cmake
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    add_subdirectory(External/STM32LibraryCollection)

    target_link_libraries(${CMAKE_PROJECT_NAME}
        STM32::LibraryCollection
    )
    ```

8. **Include the Desired Header Files in Your Code**

    + Include the desired header files from the STM32 Library Collection in your `main.cpp` or other source files. For example:

    ```cpp
    #include "STM32LibraryCollection/Gpio.hpp"
    #include "STM32LibraryCollection/Timer.hpp"
    ```

## Notes

+ To prevent your existing code from being overwritten when regenerating code, write your code between the `USER CODE BEGIN` and `USER CODE END` sections.

+ Rename `main.cpp` to `main.c` before modifying the `.ioc` file and regenerating code. After regeneration, rename the newly generated `main.c` back to `main.cpp`.

## License

Licensed under the GNU LGPL version 3. See the COPYING.LESSER file for details.
