#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace particle_life {

struct Particle {
    double x;
    double y;
    double vx;
    double vy;
    std::uint8_t species;
};

class Simulation {
public:
    Simulation(std::size_t particle_count, std::uint32_t seed);

    void step();

    const std::vector<Particle>& particles() const { return particles_; }
    double checksum() const;

    static constexpr double width = 160.0;
    static constexpr double height = 90.0;
    static constexpr std::size_t species_count = 5;

private:
    void accumulate_forces();
    void calculate_velocity(std::size_t index);
    void accumulate_force(const Particle& particle, std::size_t other_index, double& force_x,
                          double& force_y) const;
    double interaction_strength(const Particle& particle, const Particle& other,
                                double distance) const;
    void integrate_velocities_and_positions();
    static double wrapped_delta(double delta, double span);
    static double wrap(double value, double span);

    std::vector<Particle> particles_;
    std::vector<double> next_vx_;
    std::vector<double> next_vy_;
    double affinity_[species_count][species_count]{};
};

}  // namespace particle_life
