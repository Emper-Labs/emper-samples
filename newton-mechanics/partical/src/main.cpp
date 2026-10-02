#include "emper/EmperEngine.h"
#include "emper/modules/newton_mechanics/NewtonSystem.h"

using namespace emper::simulation;
using namespace emper::modules::newton_mechanics;


auto main() -> int{
    
    Simulation simulation;

    NewtonSystem newtonSystem(simulation.world());

    simulation.addSystem(newtonSystem);

    return 0;
}