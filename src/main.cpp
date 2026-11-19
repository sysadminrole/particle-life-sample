#include "simulation.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::size_t parse_count(const char* value, const char* name) {
    try {
        const auto parsed = std::stoull(value);
        if (parsed == 0) {
            throw std::invalid_argument("zero");
        }
        return static_cast<std::size_t>(parsed);
    } catch (const std::exception&) {
        throw std::runtime_error(std::string(name) + " must be a positive integer");
    }
}

void step_simulation(particle_life::Simulation& simulation, std::size_t frame_count) {
    for (std::size_t frame = 0; frame < frame_count; ++frame) {
        simulation.step();
    }
}

double time_simulation(particle_life::Simulation& simulation, std::size_t frame_count) {
    const auto start = std::chrono::steady_clock::now();
    step_simulation(simulation, frame_count);
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}

void write_results(const std::string& output_path, double fps) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("could not open results file: " + output_path);
    }
    output << std::setprecision(12) << "{\"simulation_fps\":" << fps << "}\n";
}

void report_benchmark(std::size_t particle_count, std::size_t frame_count,
                      double fps, double frame_ms, std::uint64_t checksum,
                      const std::string& output_path) {
    std::cout << std::fixed << std::setprecision(3)
              << "particles=" << particle_count << " frames=" << frame_count
              << " fps=" << fps << " frame_ms=" << frame_ms
              << " checksum=" << checksum << '\n'
              << "wrote " << output_path << '\n';
}

int benchmark(std::size_t particle_count, std::size_t frame_count, const std::string& output_path) {
    particle_life::Simulation simulation(particle_count, 20260724);
    step_simulation(simulation, 5);
    const double elapsed_seconds = time_simulation(simulation, frame_count);
    const double fps = static_cast<double>(frame_count) / elapsed_seconds;
    const double frame_ms = elapsed_seconds * 1000.0 / static_cast<double>(frame_count);
    write_results(output_path, fps);
    report_benchmark(particle_count, frame_count, fps, frame_ms, simulation.checksum(), output_path);
    return 0;
}

void write_stream_header(std::size_t particle_count) {
    const std::array<char, 4> magic = {'S', 'W', 'A', 'R'};
    const auto count = static_cast<std::uint32_t>(particle_count);
    const float world_width = static_cast<float>(particle_life::Simulation::width);
    const float world_height = static_cast<float>(particle_life::Simulation::height);
    std::cout.write(magic.data(), static_cast<std::streamsize>(magic.size()));
    std::cout.write(reinterpret_cast<const char*>(&count), sizeof(count));
    std::cout.write(reinterpret_cast<const char*>(&world_width), sizeof(world_width));
    std::cout.write(reinterpret_cast<const char*>(&world_height), sizeof(world_height));
    std::cout.flush();
}

void write_stream_frame(const particle_life::Simulation& simulation, std::uint64_t frame) {
    std::cout.write(reinterpret_cast<const char*>(&frame), sizeof(frame));
    for (const auto& particle : simulation.particles()) {
        const float x = static_cast<float>(particle.x);
        const float y = static_cast<float>(particle.y);
        std::cout.write(reinterpret_cast<const char*>(&x), sizeof(x));
        std::cout.write(reinterpret_cast<const char*>(&y), sizeof(y));
        std::cout.write(reinterpret_cast<const char*>(&particle.species), sizeof(particle.species));
    }
    std::cout.flush();
}

void stream(std::size_t particle_count, std::size_t frame_count) {
    particle_life::Simulation simulation(particle_count, 20260724);
    write_stream_header(particle_count);
    step_simulation(simulation, 25);
    for (std::uint64_t frame = 0; frame < frame_count; ++frame) {
        simulation.step();
        write_stream_frame(simulation, frame);
    }
}

void usage(const char* program) {
    std::cerr << "Usage:\n"
              << "  " << program << " benchmark [particles=2200] [frames=30] [results-path]\n"
              << "  " << program << " stream [particles=3500] [frames=100000000]\n";
}

int run_benchmark(int argc, char** argv) {
    const auto particles = argc > 2 ? parse_count(argv[2], "particles") : 2200;
    const auto frames = argc > 3 ? parse_count(argv[3], "frames") : 30;
    const std::string output = argc > 4 ? argv[4] : "artemis_results.json";
    return benchmark(particles, frames, output);
}

int run_stream(int argc, char** argv) {
    const auto particles = argc > 2 ? parse_count(argv[2], "particles") : 3500;
    const auto frames = argc > 3 ? parse_count(argv[3], "frames") : 100000000;
    stream(particles, frames);
    return 0;
}

int run(int argc, char** argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }
    const std::string mode = argv[1];
    if (mode == "benchmark") {
        return run_benchmark(argc, argv);
    }
    if (mode == "stream") {
        return run_stream(argc, argv);
    }
    usage(argv[0]);
    return 2;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
