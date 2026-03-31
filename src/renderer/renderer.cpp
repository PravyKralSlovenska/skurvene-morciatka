#include "engine/renderer/renderer.hpp"

#include "engine/player/entity_manager.hpp"
#include "engine/player/entity.hpp"

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

    entities_renderer = std::make_unique<Entities_Renderer>();
    entities_renderer->init();

    text_renderer = std::make_unique<Text_Renderer>();
    text_renderer->init();

    ui_renderer = std::make_unique<UI_Renderer>();

    // ImGui
    init_imgui();
}

bool IRenderer::render_everything(bool render_world,
                                  bool render_in_game_ui,
                                  const std::function<void()> &overlay_ui)
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
    entities_renderer->set_projection(view_projection);

    if (entity_manager)
    {
        Player *player = entity_manager->get_player();
        if (player)
        {
            world_renderer->set_fog_center_world(glm::vec2(player->coords));
        }
    }
    world_renderer->set_fog_enabled(render_world);

    // Na User Interface by nemal platit zoom
    // text_renderer->set_projection(projection);

    // render renders
    if (render_world)
    {
        world_renderer->render_world_compute();

        // Render entities that are in active chunks
        if (entity_manager && world)
        {
            auto *active_chunks = world->get_active_chunks();
            if (active_chunks)
            {
                entities_renderer->render_entities_in_chunks(*active_chunks);
            }
            else
            {
                entities_renderer->render_entities();
            }
        }
    }

    // text_renderer->render_text("MISKO POZOR ZITRA! :3", {400.0f, 400.0f}, 1.0f, Color(255, 255, 255, 1.0f));
    // text_renderer->render_text(std::to_string(frame_count_display) + "FPS", {10.0f, 48.0f}, 1.0f, Color(255, 255, 255, 1));

    // ImGui UI rendering
    imgui_new_frame();
    if (render_in_game_ui)
    {
        ui_renderer->render_ui();
    }
    if (overlay_ui)
    {
        overlay_ui();
    }
    imgui_render();

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
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); // Start with a big maximized window
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

    // Sync renderer size to real window size (important when starting maximized).
    int actual_width = 0;
    int actual_height = 0;
    glfwGetWindowSize(window, &actual_width, &actual_height);
    if (actual_width > 0 && actual_height > 0)
    {
        m_window_width = static_cast<float>(actual_width);
        m_window_height = static_cast<float>(actual_height);
    }

    glfwSwapInterval(0); // Vsync 0 = off, 1 = on

    // Store windowed dimensions for fullscreen toggle
    windowed_width = static_cast<int>(m_window_width);
    windowed_height = static_cast<int>(m_window_height);
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
    cleanup_imgui();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void IRenderer::init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void IRenderer::cleanup_imgui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void IRenderer::imgui_new_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void IRenderer::imgui_render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void IRenderer::toggle_fullscreen_map()
{
    if (ui_renderer)
        ui_renderer->toggle_fullscreen_map();
}

bool IRenderer::is_fullscreen_map_open() const
{
    if (ui_renderer)
        return ui_renderer->is_fullscreen_map_open();
    return false;
}

Menu_Actions IRenderer::render_menu_screen(Menu_Screen screen,
                                           bool enter_pressed,
                                           bool escape_pressed,
                                           Menu_Options_Model &options)
{
    if (ui_renderer)
    {
        return ui_renderer->render_menu_screen(screen, enter_pressed, escape_pressed, options);
    }

    return Menu_Actions{};
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
    if (ui_renderer)
        ui_renderer->set_time_manager(time_manager);
}

void IRenderer::set_world(World *world)
{
    this->world = world;
    world_renderer->set_world(world);
    entities_renderer->set_world(world);
    if (ui_renderer)
        ui_renderer->set_world(world);
}

void IRenderer::set_camera(Camera *camera)
{
    this->camera = camera;
    if (ui_renderer)
        ui_renderer->set_camera(camera);
}

void IRenderer::set_entity_manager(Entity_Manager *entity_manager)
{
    this->entity_manager = entity_manager;
    entities_renderer->set_entity_manager(entity_manager);

    // Set up UI renderer with all the pointers it needs
    if (ui_renderer)
    {
        ui_renderer->set_entity_manager(entity_manager);
        ui_renderer->set_player(entity_manager->get_player());
    }
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