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
    static double wrapped_delta(double delta, double span);
    static double wrap(double value, double span);

    void calculate_next_velocities();
    void accumulate_force(std::size_t source, double& force_x, double& force_y) const;
    void apply_force(const Particle& particle, const Particle& other, double& force_x,
                     double& force_y) const;
    void clamp_velocity(std::size_t index, double force_x, double force_y);
    void integrate_positions();

    std::vector<Particle> particles_;
    std::vector<double> next_vx_;
    std::vector<double> next_vy_;
    double affinity_[species_count][species_count]{};
};

}  // namespace particle_life
