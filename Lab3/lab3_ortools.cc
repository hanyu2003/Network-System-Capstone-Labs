#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <limits>
#include <unordered_map>
#include "ortools/linear_solver/linear_solver.h"

using namespace operations_research;

struct Link {
  int ground_station;
  int satellite;
  double data_rate;
};

int main() {
  std::ifstream fin("BasicExample/src/network.graph");
  std::ofstream fout("BasicExample/src/network.ortools.out");

  int num_gs, num_sat, num_links;
  fin >> num_gs >> num_sat >> num_links;

  std::vector<Link> links(num_links);
  std::unordered_map<std::string, double> time_map;

  for (int i = 0; i < num_links; ++i) {
    int gs, sat;
    double rate;
    fin >> gs >> sat >> rate;
    links[i] = {gs, sat, rate};
    time_map[std::to_string(gs) + "_" + std::to_string(sat)] = 1000.0 / rate;
  }

  MPSolver solver("BipartiteAssignment", MPSolver::CBC_MIXED_INTEGER_PROGRAMMING);
  std::map<std::string, MPVariable*> x;

  for (const auto& link : links) {
    std::string key = std::to_string(link.ground_station) + "_" + std::to_string(link.satellite);
    x[key] = solver.MakeIntVar(0, 1, "x_" + key);
  }

  // 每個 ground station 只能選一個 satellite
  for (int g = 0; g < num_gs; ++g) {
    LinearExpr sum;
    for (const auto& link : links) {
      if (link.ground_station == g) {
        sum += x[std::to_string(g) + "_" + std::to_string(link.satellite)];
      }
    }
    solver.MakeRowConstraint(sum == 1);
  }

  // 每個 satellite 的 data collection time（以一個變數代表最大值）
  MPVariable* max_time = solver.MakeNumVar(0.0, MPSolver::infinity(), "max_time");

  for (int s = 0; s < num_sat; ++s) {
    LinearExpr total_time;
    for (const auto& link : links) {
      if (link.satellite == s) {
        std::string key = std::to_string(link.ground_station) + "_" + std::to_string(s);
        total_time += time_map[key] * LinearExpr(x[key]);
      }
    }
    solver.MakeRowConstraint(total_time <= max_time);
  }

  // 目標：minimize max_time
  MPObjective* const objective = solver.MutableObjective();
  objective->SetMinimization();
  objective->SetCoefficient(max_time, 1);

  // Solve!
  const MPSolver::ResultStatus result_status = solver.Solve();

  if (result_status != MPSolver::OPTIMAL) {
    std::cerr << "No optimal solution found!" << std::endl;
    return 1;
  }

  // Output
  fout << max_time->solution_value() << std::endl;

  std::vector<int> gs_to_sat(num_gs, -1);
  std::vector<double> sat_time(num_sat, 0.0);

  for (const auto& link : links) {
    std::string key = std::to_string(link.ground_station) + "_" + std::to_string(link.satellite);
    if (x[key]->solution_value() > 0.5) {
      gs_to_sat[link.ground_station] = link.satellite;
      sat_time[link.satellite] += time_map[key];
    }
  }

  for (int g = 0; g < num_gs; ++g)
    fout << g << " " << gs_to_sat[g] << std::endl;

  for (int s = 0; s < num_sat; ++s)
    fout << s << " " << sat_time[s] << std::endl;

  return 0;
}
