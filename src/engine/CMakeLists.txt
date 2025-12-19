set(NAME "VulkanVoxelsEngine")

include (${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(${NAME}
    ./Application.cpp
    ./core/Buffer.cpp
    ./core/ComputePipeline.cpp
    ./core/DescriptorPool.cpp
    ./core/DescriptorSetLayout.cpp
    ./core/DescriptorWriter.cpp
    ./core/Device.cpp
    ./core/GraphicsPipeline.cpp
    ./core/Renderer.cpp
    ./core/Swapchain.cpp
    ./core/Texture2D.cpp
    ./core/Window.cpp
    ./renderSystems/BasicRenderSystem.cpp
    ./renderSystems/PBRRenderSystem.cpp
    ./renderSystems/PointLightRenderSystem.cpp
    ./renderSystems/IRenderSystem.cpp
    ./renderSystems/VoxelRenderSystem.cpp
    ./utility/Camera.cpp
    ./utility/Model.cpp
    ./utility/KeyboardMovementController.cpp
    ./utility/exceptions/Exception.cpp
    ./utility/exceptions/VulkanException.cpp
    ./utility/exceptions/FileException.cpp
    ./utility/exceptions/ResourceException.cpp
    ./utility/object/Object.cpp
    ./utility/object/ObjectBuilder.cpp
    ./utility/material/Material.cpp
    ./utility/material/MaterialBuilder.cpp
    ./external/stb_image_impl.cpp
    ./external/tiny_obj_loader_impl.cpp
    ./external/vma_impl.cpp
)

target_sources(${NAME}
    PRIVATE
        FILE_SET HEADERS
        FILES
            ./Application.hpp
            ./core/Buffer.hpp
            ./core/ComputePipeline.hpp
            ./core/DescriptorPool.hpp
            ./core/DescriptorSetLayout.hpp
            ./core/DescriptorWriter.hpp
            ./core/Device.hpp
            ./core/IPipeline.hpp
            ./core/GraphicsPipeline.hpp
            ./core/Renderer.hpp
            ./core/Swapchain.hpp
            ./core/Texture2D.hpp
            ./core/Window.hpp
            ./renderSystems/BasicRenderSystem.hpp
            ./renderSystems/PBRRenderSystem.hpp
            ./renderSystems/PointLightRenderSystem.hpp
            ./renderSystems/IRenderSystem.hpp
            ./renderSystems/VoxelRenderSystem.hpp
            ./utility/Camera.hpp
            ./utility/FrameInfo.hpp
            ./utility/GLFWInputHandler.hpp
            ./utility/IInputHandler.hpp
            ./utility/Model.hpp
            ./utility/KeyboardMovementController.hpp
            ./utility/Utils.hpp
            ./utility/object/Object.hpp
            ./utility/object/ObjectBuilder.hpp
            ./utility/object/IdPool.hpp
            ./utility/object/components/Component.hpp
            ./utility/object/components/ColorComponent.hpp
            ./utility/object/components/ModelComponent.hpp
            ./utility/object/components/PointLightComponent.hpp
            ./utility/object/components/TransformComponent.hpp
            ./utility/exceptions/Exception.hpp
            ./utility/exceptions/VulkanException.hpp
            ./utility/exceptions/FileException.hpp
            ./utility/exceptions/ResourceException.hpp
            ./utility/material/Material.hpp
            ./utility/material/MaterialAlphaMode.hpp
            ./utility/material/MaterialBuilder.hpp
            ./external/stb_image.h
            ./external/tiny_obj_loader.h
)

# NOTE: Add VMA as Interface to exclude from compiler warnings that the project has
add_library(VMA INTERFACE)
target_include_directories(VMA INTERFACE ${vma_SOURCE_DIR}/include)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(${NAME} PRIVATE cxx_std_23)
target_link_libraries(${NAME}
    INTERFACE
        project_warnings
    PUBLIC
        Vulkan::Vulkan
        spdlog::spdlog
        glm
        glfw
        VMA
)
