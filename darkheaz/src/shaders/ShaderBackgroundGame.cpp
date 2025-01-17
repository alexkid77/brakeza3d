//
// Created by eduardo on 10/05/22.
//

#include <functional>
#include <utility>
#include "ShaderBackgroundGame.h"
#include "../../../include/EngineSetup.h"
#include "../../../include/EngineBuffers.h"
#include "../../../include/Render/Logging.h"
#include "../../../include/Misc/Tools.h"
#include "../../../include/Brakeza3D.h"

ShaderBackgroundGame::ShaderBackgroundGame(
    cl_device_id deviceId,
    cl_context context,
    cl_command_queue commandQueue,
    const std::string& kernelFile
) : ShaderOpenCL(
    deviceId,
    context,
    commandQueue,
    kernelFile
) {
    this->channel1 = new Image(
    EngineSetup::get()->IMAGES_FOLDER + EngineSetup::get()->DEFAULT_SHADER_BACKGROUND_IMAGE
    );

    makeColorPalette();

    opencl_buffer_palette = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        EngineBuffers::getInstance()->sizeBuffers * sizeof(Uint32),
        palette,
        &clRet
    );
}

void ShaderBackgroundGame::makeColorPalette()
{
    palette = new uint32_t[255];
    //generate the palette
    Color colorRGB;
    for (int x = 0; x < 255; x++) {
        //use HSVtoRGB to vary the Hue of the color through the palette
        colorRGB = Tools::getColorRGB(ColorHSV(x, 1, 1));
        palette[x] = colorRGB.getColor();
    }
}

void ShaderBackgroundGame::update()
{
    Shader::update();

    executeKernelOpenCL();

    //demo01();
    //demo02();
    //demo03();
    //demo04();
}

void ShaderBackgroundGame::executeKernelOpenCL()
{
    clEnqueueWriteBuffer(
            clCommandQueue,
            opencl_buffer_video,
            CL_TRUE,
            0,
            EngineBuffers::getInstance()->sizeBuffers * sizeof(Uint32),
            &EngineBuffers::getInstance()->videoBuffer,
            0,
            nullptr,
            nullptr
    );

    clSetKernelArg(kernel, 0, sizeof(int), &EngineSetup::get()->screenWidth);
    clSetKernelArg(kernel, 1, sizeof(int), &EngineSetup::get()->screenHeight);
    clSetKernelArg(kernel, 2, sizeof(float), &Brakeza3D::get()->executionTime);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&opencl_buffer_palette);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&opencl_buffer_video);

    // Process the entire lists
    size_t global_item_size = EngineBuffers::getInstance()->sizeBuffers;
    // Divide work items into groups of 64
    size_t local_item_size = 12;

    clRet = clEnqueueNDRangeKernel(
        clCommandQueue,
        kernel,
        1,
        nullptr,
        &global_item_size,
        &local_item_size,
        0,
        nullptr,
        nullptr
    );

    clEnqueueReadBuffer(
        clCommandQueue,
        opencl_buffer_video,
        CL_TRUE,
        0,
        EngineBuffers::getInstance()->sizeBuffers * sizeof(Uint32),
        EngineBuffers::getInstance()->videoBuffer,
        0,
        nullptr,
        nullptr
    );

    if (clRet != CL_SUCCESS) {
        Logging::getInstance()->Log( "Error OpenCL kernel: " + std::to_string(clRet) );

        char buffer[1024];
        clGetProgramBuildInfo(
            program,
            clDeviceId,
            CL_PROGRAM_BUILD_LOG,
            sizeof(buffer),
            buffer,
            nullptr
        );

        if (strlen(buffer) > 0 ) {
            Logging::getInstance()->Log( buffer );
        }
    }
}

void ShaderBackgroundGame::demo01()
{
    float thickness = 0.325;
    Color green = Color::green();
    Color color;

    auto b = EngineBuffers::getInstance()->videoBuffer;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++, b++) {
            Vector2D uv(x , y);
            Vector2D st = uv / resolution;

            const float pct = st.x;
            //const float pct = pow(st.x, 0.5f);
            //const float pct = step(0.5, st.x);
            //const float pct = smoothstep(0.1,0.9,st.x);
            //const float pct = 1.0f - pow(abs(st.x), 2.5);
            //const float pct = pow(cos(PI * st.x / 2), 0.5);

            color.r = pct * 255;
            color.g = 0;
            color.b = 0;

            const float gradient = plot(st, pct, thickness);
            *b = (color * (1.0f - gradient) + green * gradient).getColor();
        }
    }
}

void ShaderBackgroundGame::demo02()
{
    Color colorA(255, 0, 0);
    Color colorB(0, 0, 255);

    Color red = Color::red();
    Color green = Color::green();
    Color blue = Color::blue();

    float thickness = 0.1;

    Vector2D st;
    Vertex3D pctColor;
    Color color;
    auto b = EngineBuffers::getInstance()->videoBuffer;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++, b++) {
            st.x = x / resolution.x;
            st.y = y / resolution.y;

            pctColor.x = smoothstep(0.0, 1.0, st.x);
            pctColor.y = sin(st.x);
            pctColor.z = pow(st.x, 2);

            color = mix(colorA, colorB, st.x);
            color = mix(color, red, plot(st, pctColor.x, thickness));
            color = mix(color, green, plot(st, pctColor.y, thickness));
            color = mix(color, blue, plot(st, pctColor.z, thickness));
            *b = color.getColor();
        }
    }
}


void ShaderBackgroundGame::demo03()
{
    Color color;
    int paletteShift = executionTime * 100.0;

    auto b = EngineBuffers::getInstance()->videoBuffer;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++, b++) {
            //int intensity = int(128.0 + (128.0 * sin(x / 8.0)));
            //int intensity = int(128.0 + (128.0 * sin(sqrt((x - w / 2.0) * (x - w / 2.0) + (y - h / 2.0) * (y - h / 2.0)) / 8.0)));
            /*int intensity = int(
                128.0 + (128.0 * sin(x / 8.0))
                + 128.0 + (128.0 * sin(y / 8.0))
            ) / 2;*/

            /*int intensity = int(
                    128.0 + (128.0 * sin(x / 16.0))
                    + 128.0 + (128.0 * sin(y / 8.0))
                    + 128.0 + (128.0 * sin((x + y) / 16.0))
                    + 128.0 + (128.0 * sin(sqrt(double(x * x + y * y)) / 8.0))
            ) / 4;*/

            int intensity = int(
                    128.0 + (128.0 * sin(x / 16.0))
                    + 128.0 + (128.0 * sin(y / 32.0))
                    + 128.0 + (128.0 * sin(sqrt(double((x - w / 2.0)* (x - w / 2.0) + (y - h / 2.0) * (y - h / 2.0))) / 8.0))
                    + 128.0 + (128.0 * sin(sqrt(double(x * x + y * y)) / 8.0))
            ) / 4;

            color = Color(palette[(intensity + paletteShift) % 256]);
            *b = color.getColor();
        }
    }
}

void ShaderBackgroundGame::demo04()
{
    float thickness = 0.425;
    Color baseColor = Color::green();
    Color red = Color::red();
    Vector2D st;
    float incX = 1 / resolution.x;
    float incY = 1 / resolution.y;

    auto b = EngineBuffers::getInstance()->videoBuffer;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++, b++) {
            const float pct = smoothstep(-1,1,sin(executionTime + st.x));
            *b = mix(baseColor, red, plot(st, pct, thickness)).getColor();
            st.x += incX;
        }
        st.y += incY;
        st.x = 0;
    }
}

float ShaderBackgroundGame::plot(Vector2D st, float thickness)
{
    return smoothstep(thickness, 0.0, abs(st.y - st.x));
}

float ShaderBackgroundGame::plot(Vector2D st, float pct, float thickness)
{
    return smoothstep(pct - thickness,pct, st.y) - smoothstep(pct,pct + thickness, st.y);
}

ShaderBackgroundGame::~ShaderBackgroundGame()
{
    delete channel1;
}
