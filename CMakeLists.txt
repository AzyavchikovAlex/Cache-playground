cmake_minimum_required(VERSION 3.20)
project(Cache)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

add_executable(Cache
        main.cpp
        Datasets/dataset_parser.cpp
        Datasets/dataset_parser.h
        Datasets/dataset.h
        CacheImplementations/cache_interface.h
        CacheImplementations/LRUCache.h
        CacheImplementations/LFUCache.h
        CacheImplementations/adaptive_replacement_cache.h
        CacheImplementations/LIRSCache.h
)

#if(MSVC)
#    target_compile_options(Cache PRIVATE /O2 /Oi /Ot /GL)
#    target_link_options(Cache PRIVATE /LTCG)
#else()
#    target_compile_options(Cache PRIVATE -O3 -march=native -flto)
#    target_link_options(Cache PRIVATE -flto)
#endif()