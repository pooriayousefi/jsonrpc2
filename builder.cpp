#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class BuildSystem
{
  private:
    std::string build_type_;
    std::string output_type_;

    int execute_command(const std::string &command) const
    {
        std::cout << "Executing: " << command << std::endl;
        return std::system(command.c_str());
    }

  public:
    BuildSystem() : build_type_("debug"), output_type_("executable") {}

    void set_build_type(const std::string &type) { build_type_ = type; }

    void set_output_type(const std::string &type) { output_type_ = type; }

    int build()
    {
        std::string build_dir = "build/" + build_type_;
        fs::create_directories(build_dir);

        std::vector<std::string> source_files;

        // Collect all source files from src directory
        if (fs::exists("src"))
        {
            for (const auto &entry : fs::recursive_directory_iterator("src"))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".cpp")
                {
                    source_files.push_back(entry.path().string());
                }
            }
        }

        // Collect all test files from tests directory
        std::vector<std::string> test_files;
        if (fs::exists("tests"))
        {
            for (const auto &entry : fs::directory_iterator("tests"))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".cpp")
                {
                    test_files.push_back(entry.path().string());
                }
            }
        }

        std::string compile_flags;
        std::string link_flags;
        std::string output_name;

        if (build_type_ == "debug")
        {
            compile_flags = "-g -O0 -DDEBUG";
        }
        else if (build_type_ == "release")
        {
            compile_flags = "-O3 -DNDEBUG";
        }

        // Common flags
        compile_flags += " -std=c++23 -Wall -Wextra -Wpedantic -Iinclude";

        // Add all test files to source files for main executable
        std::vector<std::string> all_sources = source_files;
        all_sources.insert(all_sources.end(), test_files.begin(), test_files.end());

        if (output_type_ == "executable")
        {
            output_name = build_dir + "/" + "jsonrpc2";
        }
        else if (output_type_ == "static")
        {
            output_name = build_dir + "/lib" + "jsonrpc2" + ".a";
            compile_flags += " -c";
        }
        else if (output_type_ == "dynamic")
        {
            output_name = build_dir + "/lib" + "jsonrpc2" + ".so";
            compile_flags += " -fPIC";
            link_flags += " -shared";
        }

        std::cout << "Building jsonrpc2 (" << build_type_ << ", " << output_type_ << ")..."
                  << std::endl;

        if (output_type_ == "static")
        {
            // Compile to object files first
            std::vector<std::string> object_files;
            for (const auto &source : all_sources)
            {
                std::string obj_file = build_dir + "/" + fs::path(source).stem().string() + ".o";
                object_files.push_back(obj_file);

                std::string compile_cmd = "g++ " + compile_flags + " " + source + " -o " + obj_file;
                if (execute_command(compile_cmd) != 0)
                {
                    return 1;
                }
            }

            // Create static library
            std::string ar_cmd = "ar rcs " + output_name;
            for (const auto &obj : object_files)
            {
                ar_cmd += " " + obj;
            }

            if (execute_command(ar_cmd) == 0)
            {
                std::cout << "Static library built: " << output_name << std::endl;
                return 0;
            }
            return 1;
        }
        else
        {
            // Build executable or dynamic library
            std::string build_cmd = "g++ " + compile_flags + " ";
            for (const auto &source : all_sources)
            {
                build_cmd += source + " ";
            }
            build_cmd += link_flags + " -o " + output_name;

            if (output_type_ == "executable")
            {
                // No need for -static flag, just regular linking
            }

            if (execute_command(build_cmd) == 0)
            {
                if (output_type_ == "executable")
                {
                    std::cout << "Executable built: " << output_name << std::endl;
                }
                else
                {
                    std::cout << "Dynamic library built: " << output_name << std::endl;
                }
                return 0;
            }
            return 1;
        }
    }
};

int main(int argc, char *argv[])
{
    try
    {
        BuildSystem builder;

        // Parse command line arguments
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--debug")
            {
                builder.set_build_type("debug");
            }
            else if (arg == "--release")
            {
                builder.set_build_type("release");
            }
            else if (arg == "--executable")
            {
                builder.set_output_type("executable");
            }
            else if (arg == "--static")
            {
                builder.set_output_type("static");
            }
            else if (arg == "--dynamic")
            {
                builder.set_output_type("dynamic");
            }
            else if (arg == "--help")
            {
                std::cout << "Usage: " << argv[0] << " [options]\n";
                std::cout << "Options:\n";
                std::cout << "  --debug          Build in debug mode\n";
                std::cout << "  --release        Build in release mode\n";
                std::cout << "  --executable     Build static executable (default)\n";
                std::cout << "  --static         Build static library\n";
                std::cout << "  --dynamic        Build dynamic library\n";
                std::cout << "  --help           Show this help message\n";
                return 0;
            }
            else
            {
                std::cerr << "Unknown option: " << arg << std::endl;
                return 1;
            }
        }

        int result = builder.build();
        if (result == 0)
        {
            std::cout << "Build completed!" << std::endl;
        }
        return result;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Build failed: " << e.what() << std::endl;
        return 1;
    }
}
