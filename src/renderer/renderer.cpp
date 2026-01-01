#include "engine/renderer/renderer.hpp"

IRenderer::IRenderer(float window_width, float window_height)
    : m_window_width(window_width), m_window_height(window_height) {}

void IRenderer::init()
{
    init_glfw();
    create_window();
    init_glad();

    const GLubyte *version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << '\n'; // opengl verzia 4.6

    // init vsetky render
    world_renderer = std::make_unique<World_Renderer>(world);
    world_renderer->init();

    // entities_renderer = std::make_unique<Entities_Renderer>(); // treba dat cesty do shaderov
    // entities_renderer->init();

    text_renderer = std::make_unique<Text_Renderer>();
    text_renderer->init();
}

bool IRenderer::render_everything()
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    if (width != (int)m_window_width || height != (int)m_window_height)
    {
        update_projection_on_resize();
    }

    // toto bude surovy backround background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // update camera
    update_camera_uniforms();

    // set projections
    glm::mat4 view_projection = camera->get_view_projection_matrix();
    world_renderer->set_projection(view_projection);

    // Na User Interface by nemal platit zoom
    // text_renderer->set_projection(projection);

    // render renders
    world_renderer->render_world_compute();

    // entities_renderer->render_entities(world->entities);
    // text_renderer->render_text("MISKO POZOR ZITRA! :3", {400.0f, 400.0f}, 1.0f, Color(255, 255, 255, 1.0f));
    // text_renderer->render_text(std::to_string(frame_count_display) + "FPS", {10.0f, 48.0f}, 1.0f, Color(255, 255, 255, 1));

    text_renderer->render_text(std::to_string(time_manager->get_frames_per_second()) + "FPS", {10.0, 50.0f}, 1.0f, Color(255, 255, 255, 1.0f));
    text_renderer->render_text(std::to_string(time_manager->get_updates_per_second()) + "UPS", {10.0, 100.0f}, 0.5f, Color(0, 255, 0, 1.0f));
    text_renderer->render_text(std::to_string(camera->get_zoom()) + " ZOOM", {10.0, 150.0f}, 0.5f, Color(0, 0, 255, 1.0f));
    text_renderer->render_text(std::to_string(world->get_chunks_size()) + " CHUNKS", {10.0, 200.0f}, 0.5f, Color(125, 125, 125, 1.0f));

    auto coords = camera->get_position();
    text_renderer->render_text(std::to_string(coords.x) + ' ' + std::to_string(coords.y), {10.0, 250.0f}, 0.5f, Color(255, 0, 255, 1.0f));

    if (time_manager->paused())
    {
        text_renderer->render_text("PAUSED", {400.0, 400.0f}, 1.0f, Color(255, 255, 255, 1.0f));
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

    return 1;
}

void IRenderer::init_glfw()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Make window resizable
}

void IRenderer::init_glad()
{
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return;
    }

    // Set initial viewport
    glViewport(0, 0, static_cast<int>(m_window_width), static_cast<int>(m_window_height));
}

void IRenderer::create_window()
{
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    // Create windowed window by default
    window = glfwCreateWindow(m_window_width, m_window_height, "Morciatko", nullptr, nullptr);

    if (window == nullptr)
    {
        std::cerr << "Failed to create glfw window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(0); // Vsync 0 = off, 1 = on

    // Store windowed dimensions for fullscreen toggle
    windowed_width = m_window_width;
    windowed_height = m_window_height;
    glfwGetWindowPos(window, &windowed_xpos, &windowed_ypos);
}

GLFWwindow *IRenderer::get_window()
{
    return window;
}

bool IRenderer::should_close()
{
    return glfwWindowShouldClose(window);
}

void IRenderer::update_camera_uniforms()
{
    // glm::mat4 view_projection = camera.get_view_projection_matrix();
    // unsigned int viewProjLoc = glGetUniformLocation(shader.ID, "view_projection");
    // shader.use();
    // glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, glm::value_ptr(view_projection));
    // shader.set_mat4("view_projection", view_projection);
}

void IRenderer::enable_blending()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render_info.push_back("blending enabled");
}

void IRenderer::enable_ortho_projection()
{
    glm::mat4 projection = glm::ortho(0.0f, m_window_width, m_window_height, 0.0f);

    // treba nastavit pre vsetky rendere projection
    text_renderer->set_projection(projection);
    world_renderer->set_projection(projection);

    render_info.push_back("ortho projection enabled");
}

void IRenderer::update_projection_on_resize()
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    m_window_width = static_cast<float>(width);
    m_window_height = static_cast<float>(height);

    // Update OpenGL viewport
    glViewport(0, 0, width, height);

    // Update projection matrix
    glm::mat4 projection = glm::ortho(0.0f, m_window_width, m_window_height, 0.0f);
    text_renderer->set_projection(projection);
    world_renderer->set_projection(projection);

    // Update camera window dimensions
    if (camera)
    {
        camera->set_window_dimensions(m_window_width, m_window_height);
    }
}

void IRenderer::cleanup()
{
    // text_renderer.~Text_Renderer();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void IRenderer::print_render_info()
{
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

    for (std::string info : render_info)
    {
        std::cout << info << '\n';
    }
}

void IRenderer::checkGLError(const char *operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error after " << operation << ": " << error << std::endl;
    }
}

void IRenderer::set_time_manager(Time_Manager *time_manager)
{
    this->time_manager = time_manager;
}

void IRenderer::set_world(World *world)
{
    this->world = world;
    world_renderer->set_world(world);
}

void IRenderer::set_camera(Camera *camera)
{
    this->camera = camera;
}

void IRenderer::toggle_fullscreen()
{
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if (!is_fullscreen)
    {
        // Store current windowed position and size
        glfwGetWindowPos(window, &windowed_xpos, &windowed_ypos);
        glfwGetWindowSize(window, &windowed_width, &windowed_height);

        // Switch to fullscreen
        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
        is_fullscreen = true;
    }
    else
    {
        // Switch back to windowed
        glfwSetWindowMonitor(window, nullptr, windowed_xpos, windowed_ypos, windowed_width, windowed_height, 0);
        is_fullscreen = false;
    }
}

bool IRenderer::get_fullscreen_state()
{
    return is_fullscreen;
}

void IRenderer::maximize_window()
{
    if (!is_fullscreen)
    {
        glfwMaximizeWindow(window);
    }
}