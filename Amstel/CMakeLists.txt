#
# project: Amstel
#
cmake_minimum_required(VERSION 2.8)

# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

fips_setup()
fips_project(Amstel)
fips_add_subdirectory(Src)

# hack to silence some warnings
if (FIPS_CLANG)
	set_target_properties(Amstel PROPERTIES COMPILE_FLAGS "-Wno-sign-compare -Wno-deprecated-declarations")
elseif(FIPS_GCC)
	set_target_properties(Amstel PROPERTIES COMPILE_FLAGS "-Wno-sign-compare")
elseif(FIPS_MSVC)
	set_target_properties(Amstel PROPERTIES COMPILE_FLAGS "/wd4152 /wd4204 /wd4242 /wd4244 /wd4668 /wd4996 /wd4091")
endif()

fips_finish()


