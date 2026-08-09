#include "myapp.h"

#include <filesystem>

int main(int argc, char** argv)
{
    if (argc > 0 && argv[0])
    {
        const std::filesystem::path exePath =
            std::filesystem::absolute(argv[0]);
        std::filesystem::current_path(
            exePath.parent_path()
        );
    }

    MyApp app;

    if (!app.initialize())
        return 1;

    app.run();
    app.shutdown();

    return 0;
}