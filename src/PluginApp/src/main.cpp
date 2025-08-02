/**
 * @file main.cpp
 * @brief Main entry point for the plugin system demo application
 */

#include "Application.h"
#include <iostream>
#include <string>

/**
 * @brief Main entry point
 * @param argc Number of command line arguments
 * @param argv Command line arguments
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    // Set the plugin directory
    std::string pluginDir = "./";
    if (argc > 1) {
        pluginDir = argv[1];
    }
    
    // Create and run the application
    Application app(pluginDir);
    
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }
    
    int result = app.Run();
    
    app.Shutdown();
    
    return result;
}