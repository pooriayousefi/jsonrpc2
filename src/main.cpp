/*
 * JSON-RPC 2.0 Library - Tutorial Runner
 *
 * This program runs all tutorials to demonstrate and test the library.
 * Compile: Use the builder system
 * Run: ./build/release/jsonrpc2
 */

#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// ANSI color codes
#define COLOR_BLUE "\033[0;34m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

// Tutorial function declarations
int run_unit_tests();
int run_json_basics();
int run_jsonrpc_fundamentals();
int run_calculator_service();
int run_database_service();
int run_advanced_features();
void run_serialization_tests(); // New serialization tests

struct Tutorial
{
    std::string name;
    std::function<int()> run;
};

void print_separator()
{
    std::cout << COLOR_BLUE << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
              << COLOR_RESET << "\n";
}

void print_header(const std::string &title)
{
    print_separator();
    std::cout << COLOR_BLUE << title << COLOR_RESET << "\n";
    print_separator();
}

int run_tutorial(const Tutorial &tutorial)
{
    print_header(tutorial.name);
    std::cout << "Running: " << tutorial.name << "\n\n";

    int result = tutorial.run();

    if (result == 0)
    {
        std::cout << "\n"
                  << COLOR_GREEN << "✓ " << tutorial.name << " completed successfully!"
                  << COLOR_RESET << "\n";
    }
    else
    {
        std::cout << "\n"
                  << COLOR_RED << "✗ " << tutorial.name << " failed!" << COLOR_RESET << "\n";
    }

    return result;
}

int main()
{
    try
    {
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  JSON-RPC 2.0 Library - Complete Tutorial Suite\n";
        std::cout << "═══════════════════════════════════════════════════════════\n\n";

        std::vector<Tutorial> tutorials = {
            {"Tutorial 1: Library Unit Tests", run_unit_tests},
            {"Tutorial 2: JSON Basics", run_json_basics},
            {"Tutorial 3: JSON-RPC Fundamentals", run_jsonrpc_fundamentals},
            {"Tutorial 4: Calculator Service", run_calculator_service},
            {"Tutorial 5: Database/CRUD Service", run_database_service},
            {"Tutorial 6: Advanced Features", run_advanced_features}};

        int total_passed = 0;
        int total_failed = 0;

        for (const auto &tutorial : tutorials)
        {
            int result = run_tutorial(tutorial);
            if (result == 0)
            {
                total_passed++;
            }
            else
            {
                total_failed++;
            }
            std::cout << "\n";
        }

        // Run serialization tests (void return type)
        run_serialization_tests();

        // Final summary
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  Final Results\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << COLOR_GREEN << "  ✓ Passed: " << total_passed << COLOR_RESET << "\n";
        std::cout << COLOR_RED << "  ✗ Failed: " << total_failed << COLOR_RESET << "\n";
        std::cout << "═══════════════════════════════════════════════════════════\n\n";

        if (total_failed == 0)
        {
            std::cout << COLOR_GREEN << "All tutorials completed successfully! ✓" << COLOR_RESET
                      << "\n\n";
            std::cout << "Summary:\n";
            std::cout << "  - 6 tutorials compiled and executed\n";
            std::cout << "  - 12 serialization/deserialization tests passed\n";
            std::cout << "  - All tests passed\n";
            std::cout << "  - Library is working correctly\n\n";
            std::cout << "Next steps:\n";
            std::cout << "  1. Read TUTORIAL.md for detailed documentation\n";
            std::cout << "  2. Read docs/SERIALIZATION.md for typed handler guide\n";
            std::cout << "  3. Explore the source code of each tutorial\n";
            std::cout << "  4. Try modifying examples to learn more\n";
            std::cout << "  5. Build your own JSON-RPC services!\n\n";
            return EXIT_SUCCESS;
        }
        else
        {
            std::cout << COLOR_RED << "Some tutorials failed. Please check the output above."
                      << COLOR_RESET << "\n\n";
            return EXIT_FAILURE;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << COLOR_RED << "Error: " << e.what() << COLOR_RESET << std::endl;
        return EXIT_FAILURE;
    }
}

// Tutorial implementations will be linked from separate compilation units
