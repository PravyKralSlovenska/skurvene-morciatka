#include "engine/renderer/renderer.hpp"

IRenderer::IRenderer(float window_width, float window_height, float scale, World *world)
    : m_window_width(window_width), m_window_height(window_height), scale(scale), world(world) {}

void IRenderer::init()
{
    init_glfw();
    create_window();
    init_glad();

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
    // toto bude surovy backround background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // update camera
    update_camera_uniforms();

    // render renders
    world_renderer->render_world();

    // entities_renderer->render_entities(world->entities);
    text_renderer->render_text("MISKO POZOR ZITRA! :3", {400.0f, 400.0f}, 1.0f, Color(255, 255, 255, 1.0f));
    text_renderer->render_text(std::to_string(frame_count_display) + "FPS", {10.0f, 48.0f}, 1.0f, Color(255, 255, 255, 1));
    
    // Display current particle type
    if (controls)
    {
        std::string particle_name = "Unknown";
        Color particle_color = Color(255, 255, 255, 1.0f);
        
        switch (controls->current_particle)
        {
            case Particle_Type::SAND:
                particle_name = "Sand";
                particle_color = Color(255, 255, 0, 1.0f);  // Yellow
                break;
            case Particle_Type::WATER:
                particle_name = "Water";
                particle_color = Color(0, 150, 255, 1.0f);  // Blue
                break;
            case Particle_Type::SMOKE:
                particle_name = "Smoke";
                particle_color = Color(150, 150, 150, 1.0f);  // Gray
                break;
            default:
                break;
        }
        
        text_renderer->render_text("Current: " + particle_name, {10.0f, 75.0f}, 1.0f, particle_color);
    }

    // Help display
    if (controls && controls->show_help)
    {
        // Help overlay background (semi-transparent black)
        float help_y = 100.0f;
        float line_height = 24.0f;
        
        text_renderer->render_text("=== SKURVENE MORCIATKA - HELP ===", {10.0f, help_y}, 1.0f, Color(255, 255, 0, 1.0f));
        help_y += line_height * 1.5f;
        
        text_renderer->render_text("What can you do?", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height * 1.2f;
        
        text_renderer->render_text("CONTROLS:", {10.0f, help_y}, 1.0f, Color(0, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  WASD - Move player", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  Left Mouse - Add particles", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  1/2/3 - Select Sand/Water/Smoke", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  R - Clear world", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  H - Toggle this help", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  ESC - Exit game", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height * 1.5f;
        
        text_renderer->render_text("PARTICLES:", {10.0f, help_y}, 1.0f, Color(0, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  Sand (1) - Falls down, solid physics", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  Water (2) - Liquid physics", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height;
        text_renderer->render_text("  Smoke (3) - Gas physics, rises up", {10.0f, help_y}, 1.0f, Color(255, 255, 255, 1.0f));
        help_y += line_height * 1.5f;
        
        text_renderer->render_text("This is a physics sandbox game!", {10.0f, help_y}, 1.0f, Color(255, 255, 0, 1.0f));
        help_y += line_height;
        text_renderer->render_text("Press H again to hide help", {10.0f, help_y}, 1.0f, Color(200, 200, 200, 1.0f));
    }

    // FPS
    // mojich max fps je 60 kvoli moonitoru
    double current_time = glfwGetTime();
    frame_count++;
    if (current_time - previous_time >= 1.0)
    {
        // std::cout << "FPS: " << frame_count << '\n';
        frame_count_display = frame_count;
        frame_count = 0;
        previous_time = current_time;
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

    return 1;
}

void IRenderer::init_glfw()
{
    glfwInit();

    // glfwSwapInterval(0); // Vsync 0 = off, 1 = on
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void IRenderer::init_glad()
{
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return;
    }
}

void IRenderer::create_window()
{
    window = glfwCreateWindow(m_window_width, m_window_height, "Morciatko", nullptr, nullptr);

    if (window == nullptr)
    {
        std::cerr << "Failed to create glfw window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
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
    // In your main render loop, use bottom-up projection:
    glm::mat4 projection = glm::ortho(0.0f, m_window_width, m_window_height, 0.0f);

    // set projection to all other renderers
    text_renderer->set_projection(projection);
    world_renderer->set_projection(projection);

    render_info.push_back("ortho projection enabled");
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

void IRenderer::set_world(World *world)
{
    this->world = world;
    world_renderer->set_world(world);
}

void IRenderer::set_controls(Controls *controls)
{
    this->controls = controls;
}