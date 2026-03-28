#include "others/utils.hpp"

#include "glad/gl.h"
#include <GLFW/glfw3.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

namespace
{
    struct Log_Entry
    {
        std::string timestamp;
        std::string level;
        std::string message;
    };

    std::mutex g_log_mutex;
    std::condition_variable g_log_cv;
    std::queue<Log_Entry> g_pending_entries;
    std::vector<Log_Entry> g_entries;

    std::thread g_log_worker;
    std::atomic<bool> g_logger_running = false;
    bool g_use_background_thread = true;
    bool g_logger_initialized = false;
    std::string g_output_path = "logs/runtime.log";
    std::string g_session_output_path = "logs/runtime.log";

    std::streambuf *g_old_cout_buffer = nullptr;
    std::streambuf *g_old_cerr_buffer = nullptr;

    std::string make_timestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time_now = std::chrono::system_clock::to_time_t(now);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm local_tm{};
#ifdef _WIN32
        localtime_s(&local_tm, &time_now);
#else
        localtime_r(&time_now, &local_tm);
#endif

        std::ostringstream out;
        out << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setw(3) << std::setfill('0') << ms.count();
        return out.str();
    }

    std::string make_filename_timestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time_now = std::chrono::system_clock::to_time_t(now);

        std::tm local_tm{};
#ifdef _WIN32
        localtime_s(&local_tm, &time_now);
#else
        localtime_r(&time_now, &local_tm);
#endif

        std::ostringstream out;
        out << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S");
        return out.str();
    }

    std::string build_session_output_path(const std::string &base_output_path)
    {
        const std::filesystem::path base_path(base_output_path);
        const std::string suffix = make_filename_timestamp();

        if (base_path.has_extension())
        {
            const std::filesystem::path parent = base_path.parent_path();
            const std::string stem = base_path.stem().string();
            const std::string ext = base_path.extension().string();

            std::filesystem::path file_name = stem + "_" + suffix + ext;
            if (parent.empty())
            {
                return file_name.string();
            }
            return (parent / file_name).string();
        }

        std::filesystem::path folder = base_path;
        return (folder / ("runtime_" + suffix + ".log")).string();
    }

    void push_log_entry(const std::string &level, const std::string &message)
    {
        Log_Entry entry{make_timestamp(), level, message};

        if (g_use_background_thread && g_logger_running)
        {
            {
                std::lock_guard<std::mutex> lock(g_log_mutex);
                g_pending_entries.push(std::move(entry));
            }
            g_log_cv.notify_one();
            return;
        }

        std::lock_guard<std::mutex> lock(g_log_mutex);
        g_entries.push_back(std::move(entry));
    }

    void drain_pending_entries_to_memory()
    {
        std::lock_guard<std::mutex> lock(g_log_mutex);
        while (!g_pending_entries.empty())
        {
            g_entries.push_back(std::move(g_pending_entries.front()));
            g_pending_entries.pop();
        }
    }

    void log_worker_loop()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(g_log_mutex);
            g_log_cv.wait(lock, []
                          { return !g_pending_entries.empty() || !g_logger_running; });

            while (!g_pending_entries.empty())
            {
                g_entries.push_back(std::move(g_pending_entries.front()));
                g_pending_entries.pop();
            }

            if (!g_logger_running)
            {
                break;
            }
        }
    }

    void write_log_file()
    {
        std::filesystem::path output(g_session_output_path);
        if (output.has_parent_path())
        {
            std::error_code ec;
            std::filesystem::create_directories(output.parent_path(), ec);
        }

        std::ofstream file(output, std::ios::app);
        if (!file)
        {
            return;
        }

        file << "----- session start -----\n";
        for (const Log_Entry &entry : g_entries)
        {
            file << '[' << entry.timestamp << "] [" << entry.level << "] " << entry.message << '\n';
        }
        file << "----- session end -----\n\n";
    }

    class Timestamp_Stream_Buffer : public std::streambuf
    {
    public:
        Timestamp_Stream_Buffer(const std::string &level, bool mirror_to_console, std::streambuf *fallback_buffer)
            : level(level), mirror_to_console(mirror_to_console), fallback_buffer(fallback_buffer)
        {
        }

        void flush_pending()
        {
            flush_line();
        }

    protected:
        int overflow(int ch) override
        {
            if (ch == traits_type::eof())
            {
                return sync() == 0 ? ch : traits_type::eof();
            }

            if (ch == '\n')
            {
                flush_line();
            }
            else
            {
                buffer.push_back(static_cast<char>(ch));
            }

            return ch;
        }

        int sync() override
        {
            // std::cerr has unitbuf enabled, which triggers sync() after each insertion.
            // Flushing here would split one logical message into multiple log lines.
            return 0;
        }

    private:
        std::string level;
        bool mirror_to_console;
        std::streambuf *fallback_buffer = nullptr;
        std::string buffer;

        void flush_line()
        {
            if (buffer.empty())
            {
                return;
            }

            push_log_entry(level, buffer);

            if (mirror_to_console && fallback_buffer != nullptr)
            {
                fallback_buffer->sputn(buffer.c_str(), static_cast<std::streamsize>(buffer.size()));
                fallback_buffer->sputc('\n');
            }

            buffer.clear();
        }
    };

    std::unique_ptr<Timestamp_Stream_Buffer> g_cout_logger_buffer;
    std::unique_ptr<Timestamp_Stream_Buffer> g_cerr_logger_buffer;
} // namespace

Color::Color() {}
Color::Color(int red, int green, int blue, float alpha)
    : r(red / 255.0f), g(green / 255.0f), b(blue / 255.0f), a(alpha)
{
}

// polymorfizmus ???
Color Color::change_shade()
{
    // float shader_factor = 0.1f + (static_cast<float>(rand()) / RAND_MAX) * 0.05f;
    float shader_factor = 0.1f + (static_cast<float>(rand()) / RAND_MAX) * 0.15f;

    int red = r * 255;
    int green = g * 255;
    int blue = b * 255;

    int new_red = red * (1 - shader_factor);
    int new_green = green * (1 - shader_factor);
    int new_blue = blue * (1 - shader_factor);

    return Color(new_red, new_green, new_blue, a);
}

Color Color::change_tint()
{
    float tint_factor;

    return Color();
}

Vertex::Vertex() {}
Vertex::Vertex(float x, float y, Color color)
    : x(x), y(y), color(color) {}

/*
 * z utils.hpp
 *
 */
std::string read_file(const std::string &filepath)
{
    // std::string font_name = FileSystem::getPath();
    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

/*
 * z utils.hpp
 * Skontroluje, ci je hodnota v rozsahu
 * Pouzivam v particles.hpp
 */
bool in_world_range(int x, int y, int world_rows, int world_cols)
{
    return (x >= 0 && x < world_cols) && (y >= 0 && y < world_rows);
}

// Random::Random()
//     : gen(rand_device()) {}

std::random_device Random_Machine::rand_device;
std::mt19937 Random_Machine::gen(Random_Machine::rand_device());

int Random_Machine::get_int_from_range(int start, int end)
{
    std::uniform_int_distribution<int> distrib(start, end);
    return distrib(gen);
}

// od 0 do 1
float Random_Machine::get_float()
{
    std::uniform_real_distribution<float> distrib(0.0f, 1.0f);
    return distrib(gen);
}

/*
 * z utils.hpp
 * vrati vector
 */
std::vector<glm::ivec2> calculate_offsets(const int radius)
{
    std::vector<glm::ivec2> offsets;

    for (int i = -radius; i <= radius; i++)
        for (int j = -radius; j <= radius; j++)
        {
            float distance_from_player = std::sqrt(i * i + j * j);
            if (distance_from_player <= radius)
            {
                offsets.push_back({j, i});
            }
        }

    return offsets;
}

std::vector<glm::ivec2> calculate_offsets_square(const int radius)
{
    std::vector<glm::ivec2> offsets;
    offsets.reserve((2 * radius + 1) * (2 * radius + 1));

    for (int i = -radius; i <= radius; i++)
        for (int j = -radius; j <= radius; j++)
        {
            offsets.push_back({j, i});
        }

    return offsets;
}

std::vector<glm::ivec2> calculate_offsets_rectangle(const int width, const int height)
{
    std::vector<glm::ivec2> offsets;
    offsets.reserve(width * height);

    int new_height = height / 2 + 1;
    int new_width = width / 2 + 1;

    for (int y = -new_height; y <= new_height; y++)
        for (int x = -new_width; x <= new_width; x++)
        {
            offsets.push_back({x, y});
        }

    return offsets;
}

int hash_coords(int x, int y, int seed)
{
    return ((x * 374761393) + (y * 668265263) + seed) % 1000000007;
}

int get_index_custom(const int x, const int y, const int width)
{
    return y * width + x;
}

void Log::init(const std::string &output_path,
               bool use_background_thread,
               bool redirect_standard_streams,
               bool mirror_to_console)
{
    if (g_logger_initialized)
    {
        return;
    }

    g_output_path = output_path;
    g_session_output_path = build_session_output_path(g_output_path);
    g_use_background_thread = use_background_thread;
    g_logger_running = true;
    g_logger_initialized = true;

    if (g_use_background_thread)
    {
        g_log_worker = std::thread(log_worker_loop);
    }

    if (redirect_standard_streams)
    {
        g_old_cout_buffer = std::cout.rdbuf();
        g_old_cerr_buffer = std::cerr.rdbuf();

        g_cout_logger_buffer = std::make_unique<Timestamp_Stream_Buffer>("INFO", mirror_to_console, g_old_cout_buffer);
        g_cerr_logger_buffer = std::make_unique<Timestamp_Stream_Buffer>("ERROR", mirror_to_console, g_old_cerr_buffer);

        std::cout.rdbuf(g_cout_logger_buffer.get());
        std::cerr.rdbuf(g_cerr_logger_buffer.get());
    }

    info("logger initialized");
}

void Log::shutdown()
{
    if (!g_logger_initialized)
    {
        return;
    }

    if (g_cout_logger_buffer)
    {
        g_cout_logger_buffer->flush_pending();
    }
    if (g_cerr_logger_buffer)
    {
        g_cerr_logger_buffer->flush_pending();
    }

    if (g_old_cout_buffer != nullptr)
    {
        std::cout.rdbuf(g_old_cout_buffer);
        g_old_cout_buffer = nullptr;
    }
    if (g_old_cerr_buffer != nullptr)
    {
        std::cerr.rdbuf(g_old_cerr_buffer);
        g_old_cerr_buffer = nullptr;
    }

    g_cout_logger_buffer.reset();
    g_cerr_logger_buffer.reset();

    g_logger_running = false;
    g_log_cv.notify_all();

    if (g_log_worker.joinable())
    {
        g_log_worker.join();
    }

    drain_pending_entries_to_memory();
    write_log_file();

    g_entries.clear();
    g_logger_initialized = false;
}

void Log::info(const std::string &message)
{
    push_log_entry("INFO", message);
}

void Log::warning(const std::string &message)
{
    push_log_entry("WARN", message);
}

void Log::error(const std::string &message)
{
    push_log_entry("ERROR", message);
}

void Log::debug(const std::string &message)
{
    push_log_entry("DEBUG", message);
}
