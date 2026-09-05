build:
	cmake -B build
	cmake --build build

test: build
	cd build && ctest --output-on-failure

clean:
	rm -rf build

cross-linux-x86:
	cmake -B build-linux-x86 \
		-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x86_64.cmake \
		-DBUILD_TESTING=OFF
	cmake --build build-linux-x86

cross-linux-arm64:
	cmake -B build-linux-arm64 \
		-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-arm64.cmake \
		-DBUILD_TESTING=OFF
	cmake --build build-linux-arm64

cross-macos-arm64:
	cmake -B build-macos-arm64 \
		-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/macos-arm64.cmake \
		-DBUILD_TESTING=OFF
	cmake --build build-macos-arm64

cross-macos-x86:
	cmake -B build-macos-x86 \
		-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/macos-x86.cmake \
		-DBUILD_TESTING=OFF
	cmake --build build-macos-x86

cross-windows:
	cmake -B build-windows \
		-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-x86_64.cmake \
		-DBUILD_TESTING=OFF
	cmake --build build-windows

cross-all: cross-linux-x86 cross-linux-arm64 cross-macos-arm64 cross-macos-x86 cross-windows

cross-clean:
	rm -rf build-linux-x86 build-linux-arm64 build-macos-arm64 build-macos-x86 build-windows
