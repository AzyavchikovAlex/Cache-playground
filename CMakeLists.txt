cmake_minimum_required(VERSION 3.20)
project(Cache)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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