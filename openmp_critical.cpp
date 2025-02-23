#include "common.h"
#include <cmath>
#include <cstdio>
#include <stdlib.h>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

typedef struct {
    std::vector<int> particles;
    int size; // Current number of particles
} cell_t;

typedef struct {
    cell_t* cells;
    int nx, ny;
    double cell_size;
} grid_t;

static grid_t grid;

void add_to_cell(int particle_idx, particle_t& particle) {
    int cell_x = (int)(particle.x / grid.cell_size);
    int cell_y = (int)(particle.y / grid.cell_size);

    int cell_idx = cell_y * grid.nx + cell_x;
    cell_t& cell = grid.cells[cell_idx];

#pragma omp critical
    {
        if (cell.size < (int)cell.particles.size()) {
            cell.particles[cell.size] = particle_idx;
        } else {
            cell.particles.push_back(particle_idx);
        }
        cell.size++;
    }
}

void apply_force(particle_t& particle, particle_t& neighbor) {
    double dx = neighbor.x - particle.x;
    double dy = neighbor.y - particle.y;
    double r2 = dx * dx + dy * dy;

    if (r2 > cutoff * cutoff)
        return;

    r2 = fmax(r2, min_r * min_r);
    double r = sqrt(r2);

    double coef = (1 - cutoff / r) / r2 / mass;
    particle.ax += coef * dx;
    particle.ay += coef * dy;
}

void move(particle_t& p, double size) {
    p.vx += p.ax * dt;
    p.vy += p.ay * dt;
    p.x += p.vx * dt;
    p.y += p.vy * dt;

    while (p.x < 0 || p.x > size) {
        p.x = (p.x < 0) ? -p.x : 2 * size - p.x;
        p.vx = -p.vx;
    }
    while (p.y < 0 || p.y > size) {
        p.y = (p.y < 0) ? -p.y : 2 * size - p.y;
        p.vy = -p.vy;
    }
}

void init_simulation(particle_t* parts, int num_parts, double size) {
    grid.cell_size = cutoff;
    grid.nx = (int)(size / grid.cell_size) + 1;
    grid.ny = (int)(size / grid.cell_size) + 1;

    grid.cells = new cell_t[grid.nx * grid.ny];

    for (int i = 0; i < grid.nx * grid.ny; i++) {
        grid.cells[i].size = 0;
        grid.cells[i].particles.reserve(10);
    }

    // #pragma omp parallel
    //     {
    //         int num_threads = omp_get_num_threads();
    // #pragma omp master
    //         {
    // 			printf("%d threads\n", num_threads);
    //         }
    //     }
}

void simulate_one_step(particle_t* parts, int num_parts, double size) {
#pragma omp for
    for (int i = 0; i < grid.nx * grid.ny; i++) {
        grid.cells[i].size = 0;
    }

#pragma omp for
    for (int i = 0; i < num_parts; i++) {
        add_to_cell(i, parts[i]);
    }

#pragma omp for
    for (int i = 0; i < num_parts; i++) {
        parts[i].ax = parts[i].ay = 0;

        int cell_x = (int)(parts[i].x / grid.cell_size);
        int cell_y = (int)(parts[i].y / grid.cell_size);

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = cell_x + dx;
                int ny = cell_y + dy;
                if (nx < 0 || nx >= grid.nx || ny < 0 || ny >= grid.ny)
                    continue;

                cell_t& cell = grid.cells[ny * grid.nx + nx];
                for (int j = 0; j < cell.size; j++) {
                    apply_force(parts[i], parts[cell.particles[j]]);
                }
            }
        }
    }

#pragma omp for
    for (int i = 0; i < num_parts; i++) {
        move(parts[i], size);
    }
}

// Threads: 64
// Simulation Time = 0.607108 seconds for 1000 particles.
// Simulation Time = 5.71209 seconds for 10000 particles.