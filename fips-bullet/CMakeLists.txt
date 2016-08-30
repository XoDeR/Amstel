#
# project: fips-bullet
#
if (NOT FIPS_IMPORT)
    cmake_minimum_required(VERSION 2.8)
    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")
    fips_setup(PROJECT fips-bullet)
endif()

fips_begin_lib(Bullet)
fips_src(bullet3/src/BulletCollision GROUP Collision)
fips_src(bullet3/src/BulletDynamics GROUP Dynamics)
fips_src(bullet3/src/BulletSoftBody GROUP SoftBody)
fips_src(bullet3/src/LinearMath GROUP LinearMath)
fips_end_lib()
if (MSVC)
    if (CMAKE_CL_64)
        target_compile_definitions(Bullet PRIVATE _WIN64)
        # 4359: Alignment specifier is less than actual alignment
        # 4473: 'printf' not enough arguments passed for format string
        #       (this happens in an assert check at btDantzigLCP.cpp(1568)
        target_compile_options(Bullet PUBLIC /wd4359 PRIVATE /wd4473)
    endif()
endif()
target_include_directories(Bullet PRIVATE bullet3/src)

target_include_directories(Bullet PUBLIC bullet3/src/BulletCollision/CollisionShapes)
target_include_directories(Bullet PUBLIC bullet3/src/BulletCollision/CollisionDispatch)
target_include_directories(Bullet PUBLIC bullet3/src/BulletCollision/BroadphaseCollision)
target_include_directories(Bullet PUBLIC bullet3/src/BulletDynamics/Dynamics)
target_include_directories(Bullet PUBLIC bullet3/src/BulletDynamics/ConstraintSolver)
target_include_directories(Bullet PUBLIC bullet3/src/BulletDynamics/Character)

target_include_directories(Bullet PUBLIC bullet3/src/BulletSoftBody)
target_include_directories(Bullet PUBLIC bullet3/src/LinearMath)
target_include_directories(Bullet PUBLIC bullet3/src/Bullet3Collision)
target_include_directories(Bullet PUBLIC bullet3/src/Bullet3Common)
target_include_directories(Bullet PUBLIC bullet3/src/Bullet3Dynamics)
target_include_directories(Bullet PUBLIC bullet3/src/Bullet3Serialize)

if (FIPS_CLANG)
    target_compile_options(Bullet PRIVATE -Wno-unused-variable -Wno-unused-function -Wno-unused-private-field)
endif()
if (FIPS_GCC)
    target_compile_options(Bullet PRIVATE -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wno-type-limits) 
endif()

if (NOT FIPS_IMPORT)
    fips_finish()
endif()


