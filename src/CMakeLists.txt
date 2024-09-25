find_package(ZLIB REQUIRED)

include(CMakeLists.unf2fs.txt)

install(TARGETS
	unf2fs
	DESTINATION bin)
