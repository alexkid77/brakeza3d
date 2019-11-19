#include "../../headers/Render/Engine.h"
#include "../../headers/Render/Drawable.h"
#include "../../headers/Objects/BSPEntity3D.h"
#include <chrono>
#include <iostream>
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "LinearMath/btTransform.h"
#include "../../headers/Brakeza3D.h"
#include "../../headers/Render/Maths.h"
#include "../../headers/Render/EngineBuffers.h"
#include <OpenCL/opencl.h>
#include <SDL_image.h>

#define MAX_SOURCE_SIZE (0x100000)

Engine::Engine()
{
    IMGUI_CHECKVERSION();
    this->initOpenCL();
    this->initTiles();
    this->initCollisionManager();
}

void Engine::initOpenCL()
{
    platform_id = NULL;
    device_cpu_id = NULL;

    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms) ;
    if (ret != CL_SUCCESS) {
        printf("Unable to get platform_id");
    }

    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_cpu_id, &ret_num_devices);
    if (ret != CL_SUCCESS) {
        printf("Unable to get device_cpu_id");
    }

    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_gpu_id, &ret_num_devices);
    if (ret != CL_SUCCESS) {
        printf("Unable to get device_gpu_id");
    }

    cl_context_properties properties[3];
    properties[0] = CL_CONTEXT_PLATFORM;
    properties[1] = (cl_context_properties) platform_id;
    properties[2] = 0;

    // Create an OpenCL context
    contextCPU = clCreateContext(properties, 1, &device_cpu_id, NULL, NULL, &ret);
    contextGPU = clCreateContext(properties, 1, &device_gpu_id, NULL, NULL, &ret);

    command_queue_rasterizer = clCreateCommandQueue(contextCPU, device_cpu_id, NULL, &ret);
    command_queue_transforms = clCreateCommandQueue(contextGPU, device_gpu_id, CL_QUEUE_PROFILING_ENABLE, &ret);

    // Load the kernel source code into the array source_str
    size_t source_size;
    char *source_str = Tools::readFile("../opencl.cl", source_size);

    // Create a program from the kernel source
    programCPU = clCreateProgramWithSource(contextCPU, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    programGPU = clCreateProgramWithSource(contextGPU, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    clBuildProgram(programCPU, 1, &device_cpu_id, NULL, NULL, NULL);
    clBuildProgram(programGPU, 1, &device_gpu_id, NULL, NULL, NULL);

    // Create the OpenCL kernel
    processAllTriangles       = clCreateKernel(programCPU, "rasterizerFrameTrianglesKernel", &ret);
    processTileTriangles      = clCreateKernel(programCPU, "rasterizerFrameTileTrianglesKernel", &ret);
    processTransformTriangles = clCreateKernel(programGPU, "transformTrianglesKernel", &ret);

    opencl_buffer_depth = clCreateBuffer(contextCPU, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, EngineBuffers::getInstance()->sizeBuffers * sizeof(float), EngineBuffers::getInstance()->depthBuffer, &ret);
    opencl_buffer_video = clCreateBuffer(contextCPU, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, EngineBuffers::getInstance()->sizeBuffers * sizeof(unsigned int), EngineBuffers::getInstance()->videoBuffer, &ret);

    OpenCLInfo();
}

void Engine::handleOpenCLTriangles()
{
    int numTriangles = EngineBuffers::getInstance()->numOCLTriangles;

    opencl_buffer_triangles = clCreateBuffer(contextCPU, CL_MEM_READ_ONLY, numTriangles * sizeof(OCLTriangle), NULL, &ret);
    clEnqueueWriteBuffer(command_queue_rasterizer, opencl_buffer_triangles, CL_TRUE, 0, numTriangles * sizeof(OCLTriangle), EngineBuffers::getInstance()->OCLTrianglesBuffer, 0, NULL, NULL);

    clSetKernelArg(processAllTriangles, 0, sizeof(cl_mem), (void *)&opencl_buffer_triangles);
    clSetKernelArg(processAllTriangles, 1, sizeof(int), &EngineSetup::getInstance()->screenWidth);
    clSetKernelArg(processAllTriangles, 2, sizeof(cl_mem), (void *)&opencl_buffer_video);
    clSetKernelArg(processAllTriangles, 3, sizeof(cl_mem), (void *)&opencl_buffer_depth);

    size_t global_item_size = numTriangles; // Process the entire lists
    size_t local_item_size = 1;          // Divide work items into groups of 64

    ret = clEnqueueNDRangeKernel(command_queue_rasterizer, processAllTriangles, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    if (ret != CL_SUCCESS) {
        Logging::getInstance()->getInstance()->Log( "Error processAllTriangles: " + std::to_string(ret) );

        char buffer[1024];
        clGetProgramBuildInfo(programCPU, device_cpu_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        if (strlen(buffer) > 0 ) {
            Logging::getInstance()->getInstance()->Log( buffer );
        }
    }
    clFinish(command_queue_rasterizer);
    EngineBuffers::getInstance()->numOCLTriangles = 0;
}

void Engine::handleOpenCLTrianglesForTiles()
{
    // Create OCLTiles
    OCLTile tilesOCL[numTiles];
    for (int i = 0 ; i < numTiles ; i++) {
        tilesOCL[i].draw    = this->tiles[i].draw;
        tilesOCL[i].start_x = this->tiles[i].start_x;
        tilesOCL[i].start_y = this->tiles[i].start_y;
        tilesOCL[i].id      = this->tiles[i].id;
        tilesOCL[i].id_x    = this->tiles[i].id_x;
        tilesOCL[i].id_y    = this->tiles[i].id_y;
    }

    opencl_buffer_tiles = clCreateBuffer(contextCPU, CL_MEM_READ_ONLY, numTiles * sizeof(OCLTile), NULL, &ret);
    clEnqueueWriteBuffer(command_queue_rasterizer, opencl_buffer_tiles, CL_TRUE, 0, numTiles * sizeof(OCLTile), &tilesOCL, 0, NULL, NULL);

    for (int i = 0 ; i < numTiles ; i++) {
        if (!this->tiles[i].draw) continue;

        int numTrianglesTile = this->tiles[i].numTriangles;
        trianglesTile = new OCLTriangle[this->tiles[i].numTriangles];
        for (int j = 0 ; j < this->tiles[i].numTriangles ; j++) {
            int triangleId = this->tiles[i].triangleIds[j];
            trianglesTile[j] = EngineBuffers::getInstance()->OCLTrianglesBuffer[ triangleId ];
        }

        opencl_buffer_triangles = clCreateBuffer(contextCPU, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, this->tiles[i].numTriangles * sizeof(OCLTriangle), trianglesTile, &ret);

        clSetKernelArg(processTileTriangles, 0, sizeof(int), &this->tiles[i].id);
        clSetKernelArg(processTileTriangles, 1, sizeof(int), &EngineSetup::getInstance()->screenWidth);
        clSetKernelArg(processTileTriangles, 2, sizeof(int), &this->sizeTileWidth);
        clSetKernelArg(processTileTriangles, 3, sizeof(int), &this->sizeTileHeight);
        clSetKernelArg(processTileTriangles, 4, sizeof(cl_mem), (void *)&this->tiles[i].clBuffer);
        clSetKernelArg(processTileTriangles, 5, sizeof(cl_mem), (void *)&this->tiles[i].clBufferDepth);
        clSetKernelArg(processTileTriangles, 6, sizeof(cl_mem), (void *)&opencl_buffer_triangles);
        clSetKernelArg(processTileTriangles, 7, sizeof(cl_mem), (void *)&opencl_buffer_tiles);

        size_t global_item_size = numTrianglesTile; // Process the entire lists
        size_t local_item_size = 1;                 // Divide work items into groups of 64

        ret = clEnqueueNDRangeKernel(command_queue_rasterizer, processTileTriangles, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

        if (ret != CL_SUCCESS) {
            Logging::getInstance()->getInstance()->Log( "Error processTileTriangles: " + std::to_string(ret) );

            char buffer[1024];
            clGetProgramBuildInfo(programCPU, device_cpu_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
            if (strlen(buffer) > 0 ) {
                Logging::getInstance()->getInstance()->Log( buffer );
            }
        }
        clReleaseMemObject(opencl_buffer_triangles);
    }

    clFinish(command_queue_rasterizer);
    for (int i = 0 ; i < numTiles ; i++) {
        if (!this->tiles[i].draw) continue;
        this->dumpTileToFrameBuffer(&this->tiles[i]);
    }

    clReleaseMemObject(opencl_buffer_tiles);
    EngineBuffers::getInstance()->numOCLTriangles = 0;
}

void Engine::dumpTileToFrameBuffer(Tile *t)
{
    int start_x = t->start_x;
    int start_y = t->start_y;

    for (int buffer_y = 0; buffer_y < sizeTileHeight; buffer_y++) {
        for (int buffer_x = 0; buffer_x < sizeTileWidth; buffer_x++) {
            int index = (buffer_y * sizeTileWidth) + buffer_x;
            EngineBuffers::getInstance()->setVideoBuffer(start_x + buffer_x, start_y + buffer_y, t->buffer[index]);
        }
    }

    std::fill(t->buffer, t->buffer + (sizeTileWidth*sizeTileHeight), 0);
    std::fill(t->bufferDepth, t->bufferDepth + (sizeTileWidth*sizeTileHeight), 10000);
}

void Engine::handleOpenCLTransform()
{
    EngineBuffers::getInstance()->numOCLTriangles = 0;
    if (this->frameTriangles.size() == 0) return;

    for (int i = 0; i < this->frameTriangles.size() ; i++) {
        EngineBuffers::getInstance()->addOCLTriangle(this->frameTriangles[i]->getOpenCL());
    }

    OCLPlane planesOCL[4];
    int cont = 0;
    for (int i = EngineSetup::getInstance()->LEFT_PLANE ; i <= EngineSetup::getInstance()->BOTTOM_PLANE ; i++) {
        planesOCL[cont].A[0] = Brakeza3D::get()->getCamera()->frustum->planes[i].A.x;
        planesOCL[cont].A[1] = Brakeza3D::get()->getCamera()->frustum->planes[i].A.y;
        planesOCL[cont].A[2] = Brakeza3D::get()->getCamera()->frustum->planes[i].A.z;

        planesOCL[cont].B[0] = Brakeza3D::get()->getCamera()->frustum->planes[i].B.x;
        planesOCL[cont].B[1] = Brakeza3D::get()->getCamera()->frustum->planes[i].B.y;
        planesOCL[cont].B[2] = Brakeza3D::get()->getCamera()->frustum->planes[i].B.z;

        planesOCL[cont].C[0] = Brakeza3D::get()->getCamera()->frustum->planes[i].C.x;
        planesOCL[cont].C[1] = Brakeza3D::get()->getCamera()->frustum->planes[i].C.y;
        planesOCL[cont].C[2] = Brakeza3D::get()->getCamera()->frustum->planes[i].C.z;

        planesOCL[cont].normal[0] = Brakeza3D::get()->getCamera()->frustum->planes[i].normal.x;
        planesOCL[cont].normal[1] = Brakeza3D::get()->getCamera()->frustum->planes[i].normal.y;
        planesOCL[cont].normal[2] = Brakeza3D::get()->getCamera()->frustum->planes[i].normal.z;

        cont++;
    }

    opencl_buffer_frustum   = clCreateBuffer(contextGPU, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 4 * sizeof(OCLPlane), planesOCL, &ret);
    opencl_buffer_triangles = clCreateBuffer(contextGPU, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, EngineBuffers::getInstance()->numOCLTriangles * sizeof(OCLTriangle), EngineBuffers::getInstance()->OCLTrianglesBuffer, &ret);

    Vertex3D camPos = *Brakeza3D::get()->getCamera()->getPosition();
    M3 camRot = Brakeza3D::get()->getCamera()->getRotation();
    float pitch = camRot.getPitch();
    float yaw   = camRot.getYaw();
    float roll  = camRot.getRoll();

    clSetKernelArg(processTransformTriangles, 0, sizeof(cl_mem), (void *)&opencl_buffer_triangles);

    clSetKernelArg(processTransformTriangles, 1, sizeof(float), &camPos.x);
    clSetKernelArg(processTransformTriangles, 2, sizeof(float), &camPos.y);
    clSetKernelArg(processTransformTriangles, 3, sizeof(float), &camPos.z);

    clSetKernelArg(processTransformTriangles, 4, sizeof(float), &pitch);
    clSetKernelArg(processTransformTriangles, 5, sizeof(float), &yaw);
    clSetKernelArg(processTransformTriangles, 6, sizeof(float), &roll);

    clSetKernelArg(processTransformTriangles, 7, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNLpers.x);
    clSetKernelArg(processTransformTriangles, 8, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNLpers.y);
    clSetKernelArg(processTransformTriangles, 9, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNLpers.z);

    clSetKernelArg(processTransformTriangles, 10, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNRpers.x);
    clSetKernelArg(processTransformTriangles, 11, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNRpers.y);
    clSetKernelArg(processTransformTriangles, 12, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNRpers.z);

    clSetKernelArg(processTransformTriangles, 13, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNTpers.x);
    clSetKernelArg(processTransformTriangles, 14, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNTpers.y);
    clSetKernelArg(processTransformTriangles, 15, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNTpers.z);

    clSetKernelArg(processTransformTriangles, 16, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNBpers.x);
    clSetKernelArg(processTransformTriangles, 17, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNBpers.y);
    clSetKernelArg(processTransformTriangles, 18, sizeof(float), &Brakeza3D::get()->getCamera()->frustum->vNBpers.z);

    clSetKernelArg(processTransformTriangles, 19, sizeof(int), &EngineSetup::getInstance()->screenWidth);
    clSetKernelArg(processTransformTriangles, 20, sizeof(int), &EngineSetup::getInstance()->screenHeight);

    clSetKernelArg(processTransformTriangles, 21, sizeof(cl_mem), (void *)&opencl_buffer_frustum);

    size_t global_item_size = EngineBuffers::getInstance()->numOCLTriangles;
    size_t local_item_size = 1;

    cl_event event;
    ret = clEnqueueNDRangeKernel(command_queue_transforms, processTransformTriangles, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    if (ret != CL_SUCCESS) {
        Logging::getInstance()->getInstance()->Log( "Error processTransformTriangles: " + std::to_string(ret) );

        char buffer[1024];
        clGetProgramBuildInfo(programGPU, device_gpu_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        if (strlen(buffer) > 0 ) {
            Logging::getInstance()->getInstance()->Log( buffer );
        }
    }

    if (EngineSetup::getInstance()->OPENCL_SHOW_TIME_KERNELS) {
        clWaitForEvents(1, &event);
        clFinish(command_queue_transforms);

        cl_ulong time_start;
        cl_ulong time_end;

        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
        double nanoSeconds = time_end-time_start;

        Logging::getInstance()->Log("OpenCL processTransformTriangles kernel time is: "+ std::to_string(nanoSeconds / 1000000.0) + " milliseconds");
    } else {
        clFinish(command_queue_transforms);
    }

    // Recover transform Data
    for (int i = 0; i < this->frameTriangles.size() ; i++) {
        OCLTriangle oclt = EngineBuffers::getInstance()->OCLTrianglesBuffer[i];
        frameTriangles[i]->Ao.x = oclt.Ao_x;
        frameTriangles[i]->Ao.y = oclt.Ao_y;
        frameTriangles[i]->Ao.z = oclt.Ao_z;
        frameTriangles[i]->Bo.x = oclt.Bo_x;
        frameTriangles[i]->Bo.y = oclt.Bo_y;
        frameTriangles[i]->Bo.z = oclt.Bo_z;
        frameTriangles[i]->Co.x = oclt.Co_x;
        frameTriangles[i]->Co.y = oclt.Co_y;
        frameTriangles[i]->Co.z = oclt.Co_z;

        frameTriangles[i]->Ac.x = oclt.Ac_x;
        frameTriangles[i]->Ac.y = oclt.Ac_y;
        frameTriangles[i]->Ac.z = oclt.Ac_z;
        frameTriangles[i]->Bc.x = oclt.Bc_x;
        frameTriangles[i]->Bc.y = oclt.Bc_y;
        frameTriangles[i]->Bc.z = oclt.Bc_z;
        frameTriangles[i]->Cc.x = oclt.Cc_x;
        frameTriangles[i]->Cc.y = oclt.Cc_y;
        frameTriangles[i]->Cc.z = oclt.Cc_z;

        frameTriangles[i]->An.x = oclt.An_x;
        frameTriangles[i]->An.y = oclt.An_y;
        frameTriangles[i]->An.z = oclt.An_z;
        frameTriangles[i]->Bn.x = oclt.Bn_x;
        frameTriangles[i]->Bn.y = oclt.Bn_y;
        frameTriangles[i]->Bn.z = oclt.Bn_z;
        frameTriangles[i]->Cn.x = oclt.Cn_x;
        frameTriangles[i]->Cn.y = oclt.Cn_y;
        frameTriangles[i]->Cn.z = oclt.Cn_z;

        frameTriangles[i]->As.x = oclt.As_x;
        frameTriangles[i]->As.y = oclt.As_y;
        frameTriangles[i]->Bs.x = oclt.Bs_x;
        frameTriangles[i]->Bs.y = oclt.Bs_y;
        frameTriangles[i]->Cs.x = oclt.Cs_x;
        frameTriangles[i]->Cs.y = oclt.Cs_y;

        frameTriangles[i]->minX = oclt.minX;
        frameTriangles[i]->maxX = oclt.maxX;
        frameTriangles[i]->minY = oclt.minY;
        frameTriangles[i]->maxY = oclt.maxY;

        frameTriangles[i]->normal.x = oclt.normal[0];
        frameTriangles[i]->normal.y = oclt.normal[1];
        frameTriangles[i]->normal.z = oclt.normal[2];

        frameTriangles[i]->clipped_cl = oclt.is_clipping;
    }

    EngineBuffers::getInstance()->numOCLTriangles = 0;

}

void Engine::initTiles()
{
    if (EngineSetup::getInstance()->screenWidth % this->sizeTileWidth != 0) {
        printf("Bad sizetileWidth\r\n");
        exit(-1);
    }
    if (EngineSetup::getInstance()->screenHeight % this->sizeTileHeight != 0) {
        printf("Bad sizeTileHeight\r\n");
        exit(-1);
    }

    // Tiles Raster setup
    this->tilesWidth  = EngineSetup::getInstance()->screenWidth / this->sizeTileWidth;
    this->tilesHeight = EngineSetup::getInstance()->screenHeight / this->sizeTileHeight;
    this->numTiles = tilesWidth * tilesHeight;
    this->tilePixelsBufferSize = this->sizeTileWidth*this->sizeTileHeight;

    Logging::getInstance()->Log("Tiles: ("+std::to_string(tilesWidth)+"x"+std::to_string(tilesHeight)+"), Size: ("+std::to_string(sizeTileWidth)+"x"+std::to_string(sizeTileHeight)+") - bufferTileSize: " + std::to_string(sizeTileWidth*sizeTileHeight));

    for (int y = 0 ; y < EngineSetup::getInstance()->screenHeight; y+=this->sizeTileHeight) {
        for (int x = 0 ; x < EngineSetup::getInstance()->screenWidth; x+=this->sizeTileWidth) {

            Tile t;

            t.draw    = true;
            t.id_x    = (x/this->sizeTileWidth);
            t.id_y    = (y/this->sizeTileHeight);
            t.id      = t.id_y * this->tilesWidth + t.id_x;
            t.start_x = x;
            t.start_y = y;

            this->tiles.push_back(t);
            // Load up the vector with MyClass objects


            Logging::getInstance()->Log("Tiles: (id:" + std::to_string(t.id) + "), (offset_x: " + std::to_string(x)+", offset_y: " + std::to_string(y) + ")");
        }
    }

    // Create local buffers and openCL buffer pointers
    for (int i = 0; i < numTiles; i++) {

        this->tiles[i].buffer = new unsigned int[tilePixelsBufferSize];
        for (int j = 0; j < tilePixelsBufferSize ; j++) {
            this->tiles[i].buffer[j] = Color::red();
        }

        this->tiles[i].bufferDepth = new float[tilePixelsBufferSize];
        for (int j = 0; j < tilePixelsBufferSize ; j++) {
            this->tiles[i].bufferDepth[j] = 0;
        }

        this->tiles[i].clBuffer = clCreateBuffer(contextCPU, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, tilePixelsBufferSize * sizeof(unsigned int), this->tiles[i].buffer, &ret);
        this->tiles[i].clBufferDepth = clCreateBuffer(contextCPU, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, tilePixelsBufferSize * sizeof(float), this->tiles[i].bufferDepth, &ret);
    }
}

void Engine::initCollisionManager()
{
    Brakeza3D::get()->getCollisionManager()->setBspMap( Brakeza3D::get()->getBSP() );
    Brakeza3D::get()->getCollisionManager()->setWeaponManager( Brakeza3D::get()->getWeaponsManager() );
    Brakeza3D::get()->getCollisionManager()->setCamera( Brakeza3D::get()->getCamera() );
    Brakeza3D::get()->getCollisionManager()->setGameObjects( &Brakeza3D::get()->getSceneObjects() );
    Brakeza3D::get()->getCollisionManager()->initBulletSystem();
    Brakeza3D::get()->getCollisionManager()->setVisibleTriangles(visibleTriangles);
}

void Engine::OpenCLInfo()
{
    int i, j;
    char* value;
    size_t valueSize;
    cl_uint platformCount;
    cl_platform_id* platforms;
    cl_uint deviceCount;
    cl_device_id* devices;
    cl_uint maxComputeUnits;

    // get all platforms
    clGetPlatformIDs(0, NULL, &platformCount);
    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, NULL);

    for (i = 0; i < platformCount; i++) {

        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
        devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);

        // for each device print critical attributes
        for (j = 0; j < deviceCount; j++) {

            // print device name
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
            printf("%d. Device: %s\n", j+1, value);
            free(value);

            // print hardware device version
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
            printf(" %d.%d Hardware version: %s\n", j+1, 1, value);
            free(value);

            // print software driver version
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
            printf(" %d.%d Software version: %s\n", j+1, 2, value);
            free(value);

            // print c version supported by compiler for device
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
            printf(" %d.%d OpenCL C version: %s\n", j+1, 3, value);
            free(value);

            // print parallel compute units
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
                            sizeof(maxComputeUnits), &maxComputeUnits, NULL);
            printf(" %d.%d Parallel compute units: %d\n", j+1, 4, maxComputeUnits);
        }
        free(devices);
    }

    free(platforms);
}

void Engine::updateGUI()
{
    ImGui::NewFrame();

    //bool open = true;
    //ImGui::ShowDemoWindow(&open);

    Brakeza3D::get()->getGUIManager()->draw(
            Brakeza3D::get()->getDeltaTime(),
            finish,
            Brakeza3D::get()->getSceneObjects(),
            Brakeza3D::get()->getLightPoints(),
            Brakeza3D::get()->getCamera(),
            tiles, tilesWidth,
            visibleTriangles.size(),
            Brakeza3D::get()->getWeaponsManager()
    );

    ImGui::Render();
}

void Engine::updateWindow()
{
    Brakeza3D::get()->updateFPS();

    EngineBuffers::getInstance()->flipVideoBuffer(Brakeza3D::get()->screenSurface );

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame(Brakeza3D::get()->window);

    updateGUI();

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(Brakeza3D::get()->window);

    Brakeza3D::get()->screenTexture = SDL_CreateTextureFromSurface(Brakeza3D::get()->renderer, Brakeza3D::get()->screenSurface);
    SDL_RenderCopy(Brakeza3D::get()->renderer, Brakeza3D::get()->screenTexture, NULL, NULL);

    SDL_DestroyTexture(Brakeza3D::get()->screenTexture);
}

void Engine::onStart()
{
    Brakeza3D::get()->getCamera()->setPosition(EngineSetup::getInstance()->CameraPosition );

    Brakeza3D::get()->getCamera()->velocity.vertex1 = *Brakeza3D::get()->getCamera()->getPosition();
    Brakeza3D::get()->getCamera()->velocity.vertex2 = *Brakeza3D::get()->getCamera()->getPosition();
}

void Engine::preUpdate()
{
    // Posición inicial del vector velocidad que llevará la cámara
    Brakeza3D::get()->getCamera()->velocity.vertex1 = *Brakeza3D::get()->getCamera()->getPosition();
    Brakeza3D::get()->getCamera()->velocity.vertex2 = *Brakeza3D::get()->getCamera()->getPosition();

    // Determinamos VPS
    if (Brakeza3D::get()->getBSP()->isLoaded()) {

        int leafType = NULL;

        if (Brakeza3D::get()->getBSP()->currentLeaf != NULL) {
            leafType = Brakeza3D::get()->getBSP()->currentLeaf->type;
        }

        bspleaf_t *leaf = Brakeza3D::get()->getBSP()->FindLeaf(Brakeza3D::get()->getCamera() );
        Brakeza3D::get()->getBSP()->setVisibleSet(leaf);

        if (leafType != Brakeza3D::get()->getBSP()->currentLeaf->type) {
            if (Brakeza3D::get()->getBSP()->isCurrentLeafLiquid()) {
                Brakeza3D::get()->getCamera()->kinematicController->setFallSpeed(5);
            } else {
                Brakeza3D::get()->getCamera()->kinematicController->setFallSpeed(256);
            }
        }
    }
}

void Engine::onUpdate()
{
    // update collider forces
    Brakeza3D::get()->getCamera()->UpdateVelocity();

    // update deltaTime
    Brakeza3D::get()->updateTimer();

    // step simulation
    Vertex3D finalVelocity = Brakeza3D::get()->getCollisionManager()->stepSimulation( );

    this->resolveCollisions();

    // update camera position/rotation/frustum
    Brakeza3D::get()->getCamera()->setPosition(finalVelocity);
    Brakeza3D::get()->getCamera()->UpdateRotation();
    Brakeza3D::get()->getCamera()->UpdateFrustum();

    // Clear buffers
    EngineBuffers::getInstance()->clearDepthBuffer();
    EngineBuffers::getInstance()->clearVideoBuffer();

    /*if (EngineSetup::getInstance()->ENABLE_SHADOW_CASTING) {
        // Clear every light's shadow mapping
        this->clearLightPointsShadowMappings();

        // Fill ShadowMapping FOR every object (mesh)
        this->objects3DShadowMapping();
    }*/

    // recollect triangles
    this->getQuakeMapTriangles();
    this->getMesh3DTriangles();
    this->getSpritesTriangles();

    if (EngineSetup::getInstance()->TRANSFORMS_OPENCL) {
        this->handleOpenCLTransform();
    }
    this->hiddenSurfaceRemoval();

    if (EngineSetup::getInstance()->BASED_TILE_RENDER) {
        this->handleTrianglesToTiles();

        if (!EngineSetup::getInstance()->RASTERIZER_OPENCL) {
            this->drawTilesTriangles();
        } else {
            this->handleOpenCLTrianglesForTiles();
        }

        if (EngineSetup::getInstance()->DRAW_TILES_GRID) {
            this->drawTilesGrid();
        }
    } else {
        if (!EngineSetup::getInstance()->RASTERIZER_OPENCL) {
            this->drawFrameTriangles();
        } else {
            this->handleOpenCLTriangles();
        }
    }

    if (EngineSetup::getInstance()->BULLET_DEBUG_MODE) {
        Brakeza3D::get()->getCollisionManager()->getDynamicsWorld()->debugDrawWorld();
    }

    if (EngineSetup::getInstance()->RENDER_OBJECTS_AXIS) {
        this->drawSceneObjectsAxis();
    }

    if (EngineSetup::getInstance()->DRAW_FRUSTUM) {
        Drawable::drawFrustum(Brakeza3D::get()->getCamera()->frustum, Brakeza3D::get()->getCamera(), true, true, true);
    }

    if (EngineSetup::getInstance()->RENDER_MAIN_AXIS) {
        Drawable::drawMainAxis(Brakeza3D::get()->getCamera() );
    }

    if (Brakeza3D::get()->getBSP()->isLoaded() && Brakeza3D::get()->getBSP()->isCurrentLeafLiquid() && !EngineSetup::getInstance()->MENU_ACTIVE) {
        Brakeza3D::get()->waterShader();
    }
}

void Engine::resolveCollisions()
{

}

/*void Engine::clearLightPointsShadowMappings()
{
    for (int i = 0; i < Brakeza3D::get()->getLightPoints().size(); i++) {
        if (!Brakeza3D::get()->getLightPoints()[i]->isEnabled()) {
            continue;
        }

        Brakeza3D::get()->getLightPoints()[i]->clearShadowMappingBuffer();
    }
}*/

/*void Engine::objects3DShadowMapping()
{
    for (int i = 0; i < Brakeza3D::get()->getSceneObjects().size() ; i++) {
        if (!Brakeza3D::get()->getSceneObjects()[i]->isEnabled()) {
            continue;
        }

        Mesh3D *oMesh = dynamic_cast<Mesh3D*> (Brakeza3D::get()->getSceneObjects()[i]);
        if (oMesh != NULL) {
            for (int j = 0; j < Brakeza3D::get()->getLightPoints().size(); j++) {
                if (oMesh->isShadowCaster()) {
                    oMesh->shadowMapping(Brakeza3D::get()->getLightPoints()[j]);
                }
            }
        }
    }
}*/

void Engine::getQuakeMapTriangles()
{
    if (Brakeza3D::get()->getBSP()->isLoaded() && EngineSetup::getInstance()->RENDER_BSP_MAP) {
        Brakeza3D::get()->getBSP()->DrawVisibleLeaf(Brakeza3D::get()->getCamera() );
        if (EngineSetup::getInstance()->DRAW_BSP_HULLS) {
            Brakeza3D::get()->getBSP()->DrawHulls(Brakeza3D::get()->getCamera()  );
        }
    }
}

void Engine::getMesh3DTriangles()
{
    // draw meshes
    for (int i = 0; i < Brakeza3D::get()->getSceneObjects().size(); i++) {
        Mesh3D *oMesh = dynamic_cast<Mesh3D*> (Brakeza3D::get()->getSceneObjects()[i]);
        if (oMesh != NULL) {
            if (oMesh->isEnabled()) {
                oMesh->draw( &this->frameTriangles) ;
                if (EngineSetup::getInstance()->TEXT_ON_OBJECT3D) {
                    Tools::writeText3D(Brakeza3D::get()->renderer, Brakeza3D::get()->getCamera(), Brakeza3D::get()->fontDefault, *oMesh->getPosition(), EngineSetup::getInstance()->TEXT_3D_COLOR, oMesh->getLabel());
                }
            }
            if (EngineSetup::getInstance()->DRAW_DECAL_WIREFRAMES) {
                Decal *oDecal = dynamic_cast<Decal*> (oMesh);
                if (oDecal != NULL) {
                    oDecal->cube->draw( &this->frameTriangles );
                }
            }
        }
    }
}

void Engine::getSpritesTriangles()
{
    if (!EngineSetup::getInstance()->DRAW_SPRITES) return;

    std::vector<Object3D *>::iterator it;
    for ( it = Brakeza3D::get()->getSceneObjects().begin(); it != Brakeza3D::get()->getSceneObjects().end(); ) {
        Object3D *object = *(it);

        // Check for delete
        if (object->isRemoved()) {
            Brakeza3D::get()->getSceneObjects().erase(it);
            continue;
        } else {
            it++;
        }

        // Sprite Directional 3D
        SpriteDirectional3D *oSpriteDirectional = dynamic_cast<SpriteDirectional3D*> (object);
        if (oSpriteDirectional != NULL) {

            if (!oSpriteDirectional->isEnabled()) {
                continue;
            }

            if (!Brakeza3D::get()->getCamera()->frustum->isPointInFrustum(*oSpriteDirectional->getPosition())) {
                continue;
            }

            oSpriteDirectional->updateTrianglesCoordinates( Brakeza3D::get()->getCamera() );
            Drawable::drawBillboard(oSpriteDirectional->getBillboard(), &this->frameTriangles);

            if (EngineSetup::getInstance()->TEXT_ON_OBJECT3D) {
                Tools::writeText3D(Brakeza3D::get()->renderer, Brakeza3D::get()->getCamera(), Brakeza3D::get()->fontDefault, *oSpriteDirectional->getPosition(), EngineSetup::getInstance()->TEXT_3D_COLOR, oSpriteDirectional->getLabel());
            }
        }

        // Sprite 3D
        Sprite3D *oSprite = dynamic_cast<Sprite3D*> (object);
        if (oSprite != NULL) {
            if (!oSprite->isEnabled()) {
                continue;
            }

            oSprite->updateTrianglesCoordinatesAndTexture(Brakeza3D::get()->getCamera() );
            Drawable::drawBillboard(oSprite->getBillboard(), &frameTriangles);

            if (EngineSetup::getInstance()->TEXT_ON_OBJECT3D) {
                Tools::writeText3D(Brakeza3D::get()->renderer, Brakeza3D::get()->getCamera(), Brakeza3D::get()->fontDefault, *oSprite->getPosition(), EngineSetup::getInstance()->TEXT_3D_COLOR, oSprite->getLabel());
            }
        }
    }
}

void Engine::drawFrameTriangles()
{
    std::vector<Triangle *>::iterator it;
    for ( it = visibleTriangles.begin(); it != this->visibleTriangles.end(); it++) {
        Triangle *triangle = *(it);
        Brakeza3D::get()->processTriangle( triangle );
    }
}

void Engine::drawTilesGrid()
{
    for (int j = 0 ; j < (this->tilesWidth*this->tilesHeight) ; j++) {
        Tile *t = &this->tiles[j];
        Uint32 color = Color::white();

        if (t->numTriangles > 0) {
            color = Color::red();
        }

        Line2D top = Line2D(t->start_x, t->start_y, t->start_x+sizeTileWidth, t->start_y);
        Line2D left = Line2D(t->start_x, t->start_y, t->start_x, t->start_y+sizeTileHeight);
        Drawable::drawLine2D(top, color);
        Drawable::drawLine2D(left, color);
    }
}

void Engine::hiddenSurfaceRemoval()
{
    visibleTriangles.clear();

    for (int i = 0; i < this->frameTriangles.size() ; i++) {

        // Si la transformación es mediante openCL ya están todas hechas
        // pero si vamos por software actualizamos ObjectSpace y su normal
        // ya que es el mínimo necesario para el clipping y el faceculling

        if (!EngineSetup::getInstance()->TRANSFORMS_OPENCL) {
            this->frameTriangles[i]->updateObjectSpace();
            this->frameTriangles[i]->updateNormal();
        }

        // back face culling (needs objectSpace)
        if (EngineSetup::getInstance()->TRIANGLE_BACK_FACECULLING) {
            if (this->frameTriangles[i]->isBackFaceCulling(Brakeza3D::get()->getCamera()->getPosition() )) {
                continue;
            }
        }

        // Clipping (needs objectSpace)
        bool needClipping = false;
        if (EngineSetup::getInstance()->TRANSFORMS_OPENCL) {
            if (this->frameTriangles[i]->clipped_cl ) {
                needClipping = true;
            }
        } else {
            if (this->frameTriangles[i]->testForClipping(
                    Brakeza3D::get()->getCamera()->frustum->planes,
                    EngineSetup::getInstance()->LEFT_PLANE,
                    EngineSetup::getInstance()->BOTTOM_PLANE
            )) {
                needClipping = true;
            }
        }

        if (needClipping) {
            this->frameTriangles[i]->clipping(
                    Brakeza3D::get()->getCamera(),
                    Brakeza3D::get()->getCamera()->frustum->planes,
                    EngineSetup::getInstance()->LEFT_PLANE,
                    EngineSetup::getInstance()->BOTTOM_PLANE,
                    frameTriangles[i]->parent,
                    visibleTriangles,
                    frameTriangles[i]->isBSP
            );
            continue;
        }

        // Frustum Culling (needs objectSpace)
        if (!Brakeza3D::get()->getCamera()->frustum->isPointInFrustum(this->frameTriangles[i]->Ao) &&
            !Brakeza3D::get()->getCamera()->frustum->isPointInFrustum(this->frameTriangles[i]->Bo) &&
            !Brakeza3D::get()->getCamera()->frustum->isPointInFrustum(this->frameTriangles[i]->Co)
        ) {
            continue;
        }

        // Triangle precached data
        // Estas operaciones las hacemos después de descartar triángulos
        // para optimización en el rasterizador por software
        if (!EngineSetup::getInstance()->TRANSFORMS_OPENCL) {
            this->frameTriangles[i]->updateCameraSpace(Brakeza3D::get()->getCamera() );
            this->frameTriangles[i]->updateNDCSpace(Brakeza3D::get()->getCamera() );
            this->frameTriangles[i]->updateScreenSpace(Brakeza3D::get()->getCamera() );
            this->frameTriangles[i]->updateBoundingBox();
        }

        if (!EngineSetup::getInstance()->RASTERIZER_OPENCL) {
            this->frameTriangles[i]->updateFullArea();
            this->frameTriangles[i]->updateUVCache();
        } else {
            EngineBuffers::getInstance()->addOCLTriangle(this->frameTriangles[i]->getOpenCL());
        }

        this->frameTriangles[i]->drawed = false;
        this->visibleTriangles.push_back(this->frameTriangles[i]);
    }

    if (EngineSetup::getInstance()->DEBUG_RENDER_INFO) {
        Logging::getInstance()->Log("[DEBUG_RENDER_INFO] frameTriangles: " + std::to_string(frameTriangles.size()) + ", numVisibleTriangles: " + std::to_string(visibleTriangles.size()));
    }

    frameTriangles.clear();
}

void Engine::drawTilesTriangles()
{
    for (int i = 0 ; i < this->numTiles ; i++) {
        if (!this->tiles[i].draw) continue;

        for (int j = 0 ; j < this->tiles[i].numTriangles ; j++) {
            int triangleId = this->tiles[i].triangleIds[j];
            Tile *t = &this->tiles[i];
            Triangle *tr = this->visibleTriangles[triangleId];
            Brakeza3D::get()->softwareRasterizerForTile(
                tr,
                t->start_x,
                t->start_y,
                t->start_x+sizeTileWidth,
                t->start_y+sizeTileHeight
           );
        }
    }
}

void Engine::handleTrianglesToTiles()
{
    for (int i = 0 ; i < this->numTiles ; i++) {
        this->tiles[i].numTriangles = 0;
    }

    for (int i = 0 ; i < visibleTriangles.size() ; i++) {
        int tileStartX =  std::max((float)(this->visibleTriangles[i]->minX/this->sizeTileWidth), 0.0f);
        int tileEndX   =  std::min((float)(this->visibleTriangles[i]->maxX/this->sizeTileWidth), (float)this->tilesWidth-1);

        int tileStartY =  std::max((float)(this->visibleTriangles[i]->minY/this->sizeTileHeight), 0.0f);
        int tileEndY   =  std::min((float)(this->visibleTriangles[i]->maxY/this->sizeTileHeight), (float)this->tilesHeight-1);

        for (int y = tileStartY; y <= tileEndY ; y++) {
            for (int x = tileStartX; x <= tileEndX ; x++) {
                int tileOffset = y * tilesWidth + x;
                this->tiles[tileOffset].triangleIds[ this->tiles[tileOffset].numTriangles ] = i;
                this->tiles[tileOffset].numTriangles++;
            }
        }
    }
}

void Engine::onEnd()
{
    Brakeza3D::get()->getTimer()->stop();
}

void Engine::Close()
{
    TTF_CloseFont( Brakeza3D::get()->fontDefault );
    TTF_CloseFont( Brakeza3D::get()->fontSmall );
    TTF_CloseFont( Brakeza3D::get()->fontBig );
    TTF_CloseFont( Brakeza3D::get()->fontMedium );

    SDL_DestroyWindow(Brakeza3D::get()->window );
    SDL_Quit();

    printf("\r\nBrakeza3D exit, good bye ;)");
}


void Engine::drawSceneObjectsAxis()
{
    // draw meshes
    for (int i = 0; i < Brakeza3D::get()->getSceneObjects().size(); i++) {
        if (Brakeza3D::get()->getSceneObjects()[i]->isEnabled()) {
            Drawable::drawObject3DAxis(Brakeza3D::get()->getSceneObjects()[i], Brakeza3D::get()->getCamera(), true, true, true);
        }
    }
}