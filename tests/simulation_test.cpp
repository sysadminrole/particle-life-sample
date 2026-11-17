#include "simulation.hpp"

#include <cmath>
#include <iostream>

namespace {

bool close(double left, double right, double tolerance = 1.0e-12) {
    return std::abs(left - right) <= tolerance;
}

}  // namespace

void advance(particle_life::Simulation& first, particle_life::Simulation& second) {
    for (int step = 0; step < 20; ++step) {
        first.step();
        second.step();
    }
}

bool particles_are_valid(const particle_life::Simulation& first, const particle_life::Simulation& second) {
    const auto& first_particles = first.particles();
    const auto& second_particles = second.particles();
    if (first_particles.size() != 128 || second_particles.size() != 128) {
        std::cerr << "particle count changed\n";
        return false;
    }

    for (std::size_t i = 0; i < first_particles.size(); ++i) {
        const auto& left = first_particles[i];
        const auto& right = second_particles[i];
        if (!close(left.x, right.x) || !close(left.y, right.y) || !close(left.vx, right.vx) ||
            !close(left.vy, right.vy) || left.species != right.species) {
            std::cerr << "simulation is not deterministic at particle " << i << '\n';
            return false;
        }
        if (left.x < 0.0 || left.x >= particle_life::Simulation::width || left.y < 0.0 ||
            left.y >= particle_life::Simulation::height || !std::isfinite(left.vx) ||
            !std::isfinite(left.vy)) {
            std::cerr << "particle escaped simulation bounds at index " << i << '\n';
            return false;
        }
    }
    return true;
}

bool checksum_is_valid(const particle_life::Simulation& simulation, double result) {
    constexpr double expected_checksum = 610659.60219911579;
    if (!std::isfinite(result) || !close(result, expected_checksum, 1.0e-6)) {
        std::cerr.precision(17);
        std::cerr << "semantic checksum changed: expected " << expected_checksum << ", got " << result
                  << '\n';
        return false;
    }
    return true;
}

}  // namespace

int main() {
    particle_life::Simulation first(128, 42);
    particle_life::Simulation second(128, 42);
    advance(first, second);

    if (!particles_are_valid(first, second)) {
        return 1;
    }

    const double result = first.checksum();
    if (!checksum_is_valid(first, result)) {
        return 1;
    }

    std::cout.precision(17);
    std::cout << "deterministic checksum: " << result << '\n';
    return 0;
}
