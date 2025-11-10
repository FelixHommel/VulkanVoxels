#ifndef SRC_ENGINE_APPLICATION_HPP
#define SRC_ENGINE_APPLICATION_HPP

#include "Window.hpp"
#include "Pipeline.hpp"

#include <cstdint>

namespace vv
{

/// \brief The Application coordinates everything to work with each other
/// 
/// \author Felix Hommel
/// \date 11/9/2025
class Application
{
public:
    Application() = default;
    ~Application() = default;

    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    /// \brief start the Application
    void run();

    static constexpr std::uint16_t WINDOW_WIDTH{ 800 };
    static constexpr std::uint16_t WINDOW_HEIGHT{ 600 };
    static constexpr auto WINDOW_TITLE{ "VulkanVoxels" };
    static constexpr auto VERTEX_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleVert.spv" };
    static constexpr auto FRAGMENT_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleFrag.spv" };

private:
    Window m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    Pipeline m_pipeline{ VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH };
};

} // !vv

#endif // !SRC_ENGINE_APPLICATION_HPP
