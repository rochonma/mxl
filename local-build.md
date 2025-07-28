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
1. [Enable containerd image store on Docker Engine](https://docs.docker.com/engine/storage/containerd/)
   ```sh
   sudo touch /etc/docker/daemon.json && sudo chmod 666 /etc/docker/daemon.json && sudo echo '{
   "features": {
    "containerd-snapshotter": true
    }
   }' > /etc/docker/daemon.json
   ```
1. [Build lightweight images](//examples-multi-arch/build-demo-images.sh)
1. Create tar.gz file the MXL reader for x86 platform
   ```sh
   mkdir ../portable-mxl-reader
   cp ./build/Linux-Clang-Release_x86_64/lib/*.so* ../portable-mxl-reader/
   cp ./build/Linux-Clang-Release_x86_64/tools/mxl-info/mxl-info ../portable-mxl-reader/
   cp ./build/Linux-Clang-Release_x86_64/tools/mxl-gst/mxl-gst-videosink ../portable-mxl-reader/
   cp ./build/Linux-Clang-Release_x86_64/lib/tests/data/*.json ../portable-mxl-reader/
   tar czf ../portable-mxl-reader.tar.gz --directory=../portable-mxl-reader/ .
   ```
1. Manualy replace the tar.gz in the hands-on repo
1. Clean up folders that needs to be deleted (install... vcpkg_cache and build)
   ```sh
   shopt -s dotglob && rm -r ./install/* && rm -r ./install_arm64/* && rm -r install_x86_64/* && rm -r ./vcpkg_cache/* && rm -r ./build/*
   ```