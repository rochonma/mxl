### Building an image

1. Delcare environment variables
   ```sh
   USER_UID=$(id -u)
   USER_GID=$(id -g)
   ARCHITECTURE=x86_64 # or arm64 
   COMPILER=Linux-GCC-Release  # or Linux-Clang-Release
   ```
   ```sh
   mkdir -p build
   chmod 777 build
   chmod g+s build

   mkdir -p install
   chmod 777 install
   chmod g+s install
   ```
1. Build Docker image
   ```sh
   docker build \
  --build-arg BASE_IMAGE_VERSION=24.04 \
  --build-arg USER_UID=${USER_UID} \
  --build-arg USER_GID=${USER_GID} \
  -t mxl_build_container \
  -f .devcontainer/Dockerfile \
  .devcontainer
  ```
1. Creating vcpkg cache folder
   ```sh
   mkdir -p vcpkg_cache
   ```
1. Configuring Cmake
   ```sh
   docker run --mount src=$(pwd),target=/workspace/mxl,type=bind \
   -e VCPKG_BINARY_SOURCES="clear;files,/workspace/mxl/vcpkg_cache,readwrite" \
   -i mxl_build_container \
   bash -c "
   cmake -S /workspace/mxl -B /workspace/mxl/build/${COMPILER} \
     --preset ${COMPILER} \
     -DCMAKE_INSTALL_PREFIX=/workspace/mxl/install"
   ```
1. Build the project
   ```sh
   docker run --mount src=$(pwd),target=/workspace/mxl,type=bind \
   -i mxl_build_container \
   bash -c "
   cmake --build /workspace/mxl/build/${COMPILER} -t all doc install package"
   ```
1. Run the test to verify build
   ```sh
   docker run --mount src=$(pwd),target=/workspace/mxl,type=bind \
   -i mxl_build_container \
   bash -c "
   cd /workspace/mxl/build/${COMPILER} && \
   ctest --output-junit test-results.xml"
   ```