#include "viewer.h"
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "=== PlantGrow GUI ===" << std::endl;
    std::cout << "Interactive Tree Generation Tool" << std::endl;
    std::cout << std::endl;

    plantgrow::gui::Viewer viewer;

    if (!viewer.initialize(1600, 1000, "PlantGrow - Tree Generator")) {
        std::cerr << "Failed to initialize viewer" << std::endl;
        return 1;
    }

    std::cout << "Controls:" << std::endl;
    std::cout << "  Left Mouse: Orbit camera" << std::endl;
    std::cout << "  Right Mouse: Pan camera" << std::endl;
    std::cout << "  Scroll: Zoom" << std::endl;
    std::cout << "  R: Reset camera" << std::endl;
    std::cout << "  U: Toggle UI" << std::endl;
    std::cout << "  Space: Regenerate tree" << std::endl;
    std::cout << "  ESC: Quit" << std::endl;
    std::cout << std::endl;

    viewer.run();

    return 0;
}
