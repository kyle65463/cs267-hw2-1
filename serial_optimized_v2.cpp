#include "common.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#define MAX_PER_CELL 8

struct cell_t {
    int particles[MAX_PER_CELL];
    int size;
};

struct grid_t {
    cell_t* cells;
    int nx, ny;
    double cell_size;
};

static grid_t grid;
static std::vector<int>* neighbors_for_cell = nullptr;

inline int cell_index(int cx, int cy) { return cy * grid.nx + cx; }

void add_to_cell(int pid, const particle_t& p) {
    int cx = (int)(p.x / grid.cell_size);
    int cy = (int)(p.y / grid.cell_size);
    int cidx = cell_index(cx, cy);

    cell_t& c = grid.cells[cidx];

    if (c.size < MAX_PER_CELL) {
        c.particles[c.size] = pid;
        c.size++;
    } else {
        // Error
    }
}

// Apply force to two particles
inline void apply_force(particle_t& p1, particle_t& p2) {
    // Calculate Distance
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double r2 = dx * dx + dy * dy;

    // Check if the two particles should interact
    if (r2 > cutoff * cutoff)
        return;

    r2 = fmax(r2, min_r * min_r);
    double r = sqrt(r2);
    double coef = (1.0 - cutoff / r) / r2 / mass;

    double fx = coef * dx;
    double fy = coef * dy;
    p1.ax += fx;
    p1.ay += fy;
    p2.ax -= fx;
    p2.ay -= fy;
}

inline void move(particle_t& p, double size) {
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
    // Initialize Grid
    grid.cell_size = cutoff * 2.5;
    grid.nx = (int)(size / grid.cell_size) + 1;
    grid.ny = (int)(size / grid.cell_size) + 1;

    int nCells = grid.nx * grid.ny;
    grid.cells = new cell_t[nCells];

    for (int i = 0; i < nCells; i++) {
        grid.cells[i].size = 0;
    }

    // Precompute neighbor cells for applying forces for each cell
    neighbors_for_cell = new std::vector<int>[nCells];
    for (int cy = 0; cy < grid.ny; cy++) {
        for (int cx = 0; cx < grid.nx; cx++) {
            int i = cell_index(cx, cy);

            for (int dy = -1; dy <= 1; dy++) {
                int ny = cy + dy;
                if (ny < 0 || ny >= grid.ny)
                    continue;

                for (int dx = -1; dx <= 1; dx++) {
                    int nx = cx + dx;
                    if (nx < 0 || nx >= grid.nx)
                        continue;

                    int j = cell_index(nx, ny);
                    if (j >= i) {
                        neighbors_for_cell[i].push_back(j);
                    }
                }
            }
        }
    }

    // Debug printing
    // printf("Number of cells: %d (%d x %d)\n", nCells, grid.nx, grid.ny);
    // printf("Number of particles: %d\n", num_parts);
    // printf("Particles per cell (avg): %.2f\n", (double)num_parts / nCells);
}

void simulate_one_step(particle_t* parts, int num_parts, double size) {
    int nCells = grid.nx * grid.ny;

    // Reset cells
    for (int i = 0; i < nCells; i++) {
        grid.cells[i].size = 0;
    }

    // Add particles to cells
    for (int i = 0; i < num_parts; i++) {
        add_to_cell(i, parts[i]);
    }

    // Reset accelerations
    for (int i = 0; i < num_parts; i++) {
        parts[i].ax = 0.0;
        parts[i].ay = 0.0;
    }

    // Apply forces
    for (int i = 0; i < nCells; i++) {
        cell_t& c1 = grid.cells[i];

        for (int j : neighbors_for_cell[i]) {
            cell_t& c2 = grid.cells[j];

            if (j == i) {
                // Apply force to particles in the same cell
                for (int idx1 = 0; idx1 < c1.size; idx1++) {
                    for (int idx2 = idx1 + 1; idx2 < c1.size; idx2++) {
                        apply_force(parts[c1.particles[idx1]], parts[c1.particles[idx2]]);
                    }
                }
            } else {
                // Apply force to particles in different cells
                for (int idx1 = 0; idx1 < c1.size; idx1++) {
                    for (int idx2 = 0; idx2 < c2.size; idx2++) {
                        apply_force(parts[c1.particles[idx1]], parts[c2.particles[idx2]]);
                    }
                }
            }
        }
    }

    // Move particles
    for (int i = 0; i < num_parts; i++) {
        move(parts[i], size);
    }
}
