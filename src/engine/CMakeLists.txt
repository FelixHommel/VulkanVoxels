set(NAME "VulkanVoxelsEngine")

include (${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(${NAME}
    ./Application.cpp
    ./core/Buffer.cpp
    ./core/DescriptorPool.cpp
    ./core/DescriptorSetLayout.cpp
    ./core/DescriptorWriter.cpp
    ./core/Device.cpp
    ./core/Pipeline.cpp
    ./core/Renderer.cpp
    ./core/Swapchain.cpp
    ./core/Window.cpp
    ./renderSystems/BasicRenderSystem.cpp
    ./renderSystems/PointLightRenderSystem.cpp
    ./utility/Camera.cpp
    ./utility/Model.cpp
    ./utility/Object.cpp
    ./utility/KeyboardMovementController.cpp
    ./utility/exceptions/Exception.cpp
    ./utility/exceptions/VulkanException.cpp
    ./utility/exceptions/FileException.cpp
    ./utility/exceptions/ResourceException.cpp
    ./external/tiny_obj_loader_impl.cpp
)

target_sources(${NAME}
    PRIVATE
        FILE_SET HEADERS
        FILES
            ./Application.hpp
            ./core/Buffer.hpp
            ./core/DescriptorPool.hpp
            ./core/DescriptorSetLayout.hpp
            ./core/DescriptorWriter.hpp
            ./core/Device.hpp
            ./core/Pipeline.hpp
            ./core/Renderer.hpp
            ./core/Swapchain.hpp
            ./core/Window.hpp
            ./renderSystems/BasicRenderSystem.hpp
            ./renderSystems/PointLightRenderSystem.hpp
            ./utility/Camera.hpp
            ./utility/FrameInfo.hpp
            ./utility/Model.hpp
            ./utility/Object.hpp
            ./utility/KeyboardMovementController.hpp
            ./utility/Utils.hpp
            ./utility/exceptions/Exception.hpp
            ./utility/exceptions/VulkanException.hpp
            ./utility/exceptions/FileException.hpp
            ./utility/exceptions/ResourceException.hpp
            ./external/tiny_obj_loader.h
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
