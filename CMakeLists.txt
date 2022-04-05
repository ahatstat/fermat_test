cmake_minimum_required(VERSION 3.11)

set(CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/libs")

project(FermatPrimeTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Test against Boost, GMP, and OpenSSL." ON)
include_directories(${CMAKE_SOURCE_DIR}/fermat)

if(BUILD_TESTS)
    if(MSVC)
      #add_compile_options(/W2)
    else()
      add_compile_options(-Wall)
    endif()

    include_directories(${CMAKE_SOURCE_DIR}/include)
    link_directories(${CMAKE_SOURCE_DIR}/libs)

    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})


    if(UNIX)
        add_definitions(-DUNIX)
    endif()

    if(WIN32)
        add_definitions(-D_WIN32_WINT=0x0601 -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
    endif()

    if (MSVC)
        set_target_properties(${BUILD_TARGET} PROPERTIES LINK_FLAGS "/PROFILE")
    endif()


    find_package(Boost)
    include_directories(${Boost_INCLUDE_DIRS}) 
    find_package(OpenMP)


    add_executable(FermatPrimeTest main.cpp fermat_prime.cpp ump_test.cpp)
    if(WIN32)
       target_link_libraries(FermatPrimeTest mpir)
    else()
       find_library(gmp gmp)
       target_link_libraries(FermatPrimeTest gmp)

    endif()
    if(OpenMP_CXX_FOUND)
        target_link_libraries(FermatPrimeTest OpenMP::OpenMP_CXX)
    endif()

    target_link_libraries(FermatPrimeTest ${OPENSSL_LIBRARIES})
endif()



