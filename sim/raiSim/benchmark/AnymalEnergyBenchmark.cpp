//
// Created by kangd on 14.05.18.
//

#include <raiSim/World_RG.hpp>

#include "AnymalEnergyBenchmark.hpp"

rai_sim::World_RG *sim;
std::vector<rai_sim::ArticulatedSystemHandle> anymals;
po::options_description desc;

void setupSimulation() {
  if(benchmark::anymal::freedrop::options.gui)
    sim = new rai_sim::World_RG(800, 600, 0.5, rai_sim::NO_BACKGROUND);
  else
    sim = new rai_sim::World_RG();

  // set erp 0
  sim->setERP(0);
}

void setupWorld() {

  // add objects
  auto checkerboard = sim->addCheckerboard(2, 100, 100, 0.1, 1, -1, rai_sim::GRID);

  // anymal
  auto anymal = sim->addArticulatedSystem(
      benchmark::anymal::freedrop::getURDFpath()
  );
  anymal->setGeneralizedCoordinate({0,
                                    0,
                                    benchmark::anymal::freedrop::params.H,
                                    1.0, 0.0, 0.0, 0.0,
                                    0.03, 0.4, -0.8,
                                    -0.03, 0.4, -0.8,
                                    0.03, -0.4, 0.8,
                                    -0.03, -0.4, 0.8});
  anymal->setGeneralizedForce(Eigen::VectorXd::Zero(anymal->getDOF()));
  anymals.push_back(anymal);

  // gravity
  sim->setGravity({0, 0, benchmark::anymal::freedrop::params.g});

  // mass
  benchmark::anymal::freedrop::params.M =
      std::accumulate( anymal->getMass().begin(), anymal->getMass().end(), 0.0);

  if(benchmark::anymal::freedrop::options.gui)
    sim->cameraFollowObject(checkerboard, {10.0, 0.0, 30.0});
}

double computeEnergy() {
  double energy = 0;
  for(int i = 0; i < anymals.size(); i++) {
    energy += anymals[i]->getEnergy({0, 0, benchmark::anymal::freedrop::params.g});
  }
  return energy;
}

double computeEnergyError(double E0) {
  // compute linear momentum
  return pow(computeEnergy() - E0, 2);
}

double simulationLoop() {

  // error list
  benchmark::anymal::freedrop::errorList.reserve(
      unsigned(benchmark::anymal::freedrop::params.T / benchmark::anymal::freedrop::options.dt));

  // init E
  double E0 = computeEnergy();

  StopWatch watch;
  watch.start();
  if(benchmark::anymal::freedrop::options.gui) {
    // gui
    for (int t = 0; t < (int) (benchmark::anymal::freedrop::params.T / benchmark::anymal::freedrop::options.dt) &&
        sim->visualizerLoop(benchmark::anymal::freedrop::options.dt, benchmark::anymal::freedrop::options.guiRealtimeFactor); t++) {

      sim->integrate1(benchmark::anymal::freedrop::options.dt);
      benchmark::anymal::freedrop::errorList.push_back(computeEnergyError(E0));
      sim->integrate2(benchmark::anymal::freedrop::options.dt);
    }
  } else {
    for (int t = 0; t < (int) (benchmark::anymal::freedrop::params.T / benchmark::anymal::freedrop::options.dt); t++) {
      sim->integrate1(benchmark::anymal::freedrop::options.dt);
      benchmark::anymal::freedrop::errorList.push_back(computeEnergyError(E0));
      sim->integrate2(benchmark::anymal::freedrop::options.dt);
    }
  }

  double time = watch.measure();
  if(benchmark::anymal::freedrop::options.csv)
    benchmark::anymal::freedrop::printCSV(
        benchmark::anymal::freedrop::getCSVpath(),
        "RAI",
        "RAI",
        time);
  return time;
}

int main(int argc, const char* argv[]) {

  benchmark::anymal::freedrop::addDescToOption(desc);
  benchmark::anymal::freedrop::getOptionsFromArg(argc, argv, desc);

  RAIINFO(
      std::endl << "=======================" << std::endl
                << "Simulator: RAI" << std::endl
                << "GUI      : " << benchmark::anymal::freedrop::options.gui << std::endl
                << "Timestep : " << benchmark::anymal::freedrop::options.dt << std::endl
                << "-----------------------"
  )

  setupSimulation();
  setupWorld();

  RAIINFO(
      std::endl << "Timer    : " << simulationLoop() << std::endl
                << "Mean Error: " << benchmark::anymal::freedrop::computeMeanError() << std::endl
                << "======================="
  )

  // show plot
  if(benchmark::anymal::freedrop::options.plot) {
    benchmark::anymal::freedrop::showPlot();
  }

  delete sim;
  return 0;
}