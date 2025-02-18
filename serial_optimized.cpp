#include "common.h"
#include <cmath>
#include <stdlib.h>
#include <vector>

typedef struct {
    std::vector<int> particles;
    int size;  // Current number of particles (like the original count)
} cell_t;

typedef struct {
    cell_t* cells;    // Array of cells
    int nx, ny;       // Number of cells in x and y directions
    double cell_size; // Size of each cell
} grid_t;

grid_t grid;

void add_to_cell(int particle_idx, particle_t& particle) {
    int cell_x = (int)(particle.x / grid.cell_size);
    int cell_y = (int)(particle.y / grid.cell_size);
    int cell_idx = cell_y * grid.nx + cell_x;

    cell_t& cell = grid.cells[cell_idx];
    if (cell.size < cell.particles.size()) {
        // Reuse existing space
        cell.particles[cell.size] = particle_idx;
    } else {
        // Allocate more space
        cell.particles.push_back(particle_idx);
    }
    cell.size++;
}

// Apply the force from neighbor to particle
void apply_force(particle_t& particle, particle_t& neighbor) {
    // Calculate Distance
    double dx = neighbor.x - particle.x;
    double dy = neighbor.y - particle.y;
    double r2 = dx * dx + dy * dy;

    // Check if the two particles should interact
    if (r2 > cutoff * cutoff)
        return;

    r2 = fmax(r2, min_r * min_r);
    double r = sqrt(r2);

    // Very simple short-range repulsive force
    double coef = (1 - cutoff / r) / r2 / mass;
    particle.ax += coef * dx;
    particle.ay += coef * dy;
}

// Integrate the ODE
void move(particle_t& p, double size) {
    // Slightly simplified Velocity Verlet integration
    // Conserves energy better than explicit Euler method
    p.vx += p.ax * dt;
    p.vy += p.ay * dt;
    p.x += p.vx * dt;
    p.y += p.vy * dt;

    // Bounce from walls
    while (p.x < 0 || p.x > size) {
        p.x = p.x < 0 ? -p.x : 2 * size - p.x;
        p.vx = -p.vx;
    }

    while (p.y < 0 || p.y > size) {
        p.y = p.y < 0 ? -p.y : 2 * size - p.y;
        p.vy = -p.vy;
    }
}

void init_simulation(particle_t* parts, int num_parts, double size) {
    // Initialize the grid
    grid.cell_size = cutoff;
    grid.nx = (int)(size / grid.cell_size) + 1;
    grid.ny = (int)(size / grid.cell_size) + 1;

    // Allocate cells
    grid.cells = new cell_t[grid.nx * grid.ny];

    // Initialize each cell
    for (int i = 0; i < grid.nx * grid.ny; i++) {
        grid.cells[i].size = 0;
        grid.cells[i].particles.reserve(10);  // Optional: reserve initial capacity
    }
}

void simulate_one_step(particle_t* parts, int num_parts, double size) {
    // Clear the grid
    for (int i = 0; i < grid.nx * grid.ny; i++) {
        // Just reset the size, don't clear the vector to avoid O(n) operation
        grid.cells[i].size = 0;  
    }

    // Add particles to the grid
    for (int i = 0; i < num_parts; i++) {
        add_to_cell(i, parts[i]);
    }

    // Compute forces
    for (int i = 0; i < num_parts; i++) {
        parts[i].ax = parts[i].ay = 0;

        // Get particle's cell
        int cell_x = (int)(parts[i].x / grid.cell_size);
        int cell_y = (int)(parts[i].y / grid.cell_size);

        // Check neighboring cells (including own cell)
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = cell_x + dx;
                int ny = cell_y + dy;

                // Skip out-of-bounds cells
                if (nx < 0 || nx >= grid.nx || ny < 0 || ny >= grid.ny)
                    continue;

                // Get neighboring cell
                cell_t& cell = grid.cells[ny * grid.nx + nx];

                // Check all particles in this cell
                for (int j = 0; j < cell.size; j++) {
                    apply_force(parts[i], parts[cell.particles[j]]);
                }
            }
        }
    }

    // Move particles
    for (int i = 0; i < num_parts; i++) {
        move(parts[i], size);
    }
}

// Simulation Time = 0.0574185 seconds for 1000 particles.
// Simulation Time = 0.809841 seconds for 10000 particles.
// Simulation Time = 9.33015 seconds for 100000 particles.