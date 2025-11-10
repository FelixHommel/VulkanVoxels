set(NAME "VulkanVoxelsEngine")

include (${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(${NAME}
    ./Application.cpp
    ./Window.cpp
    ./Pipeline.cpp
)

target_sources(${NAME}
    PRIVATE
        FILE_SET HEADERS
        FILES
            ./utility/FileIO.hpp
            ./Application.hpp
            ./Window.hpp
            ./Pipeline.hpp
)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${NAME} PRIVATE cxx_std_23)
target_link_libraries(${NAME}
    PRIVATE
        project_warnings
    PUBLIC
        Vulkan::Vulkan
        spdlog::spdlog
        glm
        glfw
)
