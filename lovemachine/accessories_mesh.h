#pragma once
#include "imgui.h"
#include <cmath>
#include <vector>

// Comprehensive High-Fidelity 3D Accessories Procedural Mesh Generator
namespace AccessoriesMesh {
struct Vertex3D {
  float x, y, z;    // Local 3D coordinates relative to anchor bone
  float nx, ny, nz; // Surface normal for lighting
  float u, v;       // Texture / parameter coords
};

struct Triangle3D {
  uint16_t i0, i1, i2;
  float normal_z;
  uint32_t color_override; // 0 = default, else ARGB
};

// 1. Authentic Asian Conical Rice Hat (Shallow Organic Bamboo Weave &
// Concentric Rings)
inline void GenerateNonLa(float hat_size, float hat_height,
                          std::vector<Vertex3D> &out_vertices,
                          std::vector<Triangle3D> &out_triangles) {
  out_vertices.clear();
  out_triangles.clear();

  float r = hat_size * 1.35f + 10.0f;
  float h = hat_height * 1.40f + 6.0f;
  int segments = 48;

  // Apex (Chóp nón) with slight curvature
  out_vertices.push_back({0.0f, 0.0f, h, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f});

  // Mid-slope curvature ring (creates natural drooping conical shape)
  float r_mid = r * 0.52f;
  float h_mid = h * 0.42f;
  for (int i = 0; i < segments; i++) {
    float angle = (float)i * (2.0f * 3.14159265f / (float)segments);
    float vx = std::cos(angle) * r_mid;
    float vy = std::sin(angle) * r_mid;
    float vz = h_mid;
    out_vertices.push_back({vx, vy, vz, std::cos(angle) * 0.7f,
                            std::sin(angle) * 0.7f, 0.7f,
                            (float)i / (float)segments, 0.5f});
  }

  // Outer Rim (Vành nón uốn cong nhẹ)
  for (int i = 0; i < segments; i++) {
    float angle = (float)i * (2.0f * 3.14159265f / (float)segments);
    float vx = std::cos(angle) * r;
    float vy = std::sin(angle) * r;
    float vz = -1.5f;
    out_vertices.push_back({vx, vy, vz, std::cos(angle) * 0.8f,
                            std::sin(angle) * 0.8f, 0.4f,
                            (float)i / (float)segments, 1.0f});
  }

  // Connect Apex to Mid-Ring
  for (int i = 0; i < segments; i++) {
    uint16_t m1 = (uint16_t)(1 + i);
    uint16_t m2 = (uint16_t)(1 + (i + 1) % segments);
    out_triangles.push_back({0, m1, m2, 1.0f, 0});
    out_triangles.push_back({0, m2, m1, -1.0f, 0});
  }

  // Connect Mid-Ring to Outer Rim
  for (int i = 0; i < segments; i++) {
    uint16_t m1 = (uint16_t)(1 + i);
    uint16_t m2 = (uint16_t)(1 + (i + 1) % segments);
    uint16_t r1 = (uint16_t)(1 + segments + i);
    uint16_t r2 = (uint16_t)(1 + segments + (i + 1) % segments);

    out_triangles.push_back({m1, r1, r2, 1.0f, 0});
    out_triangles.push_back({m1, r2, m2, 1.0f, 0});
    out_triangles.push_back({m1, r2, r1, -1.0f, 0});
    out_triangles.push_back({m1, m2, r2, -1.0f, 0});
  }
}

// 2. Radiant Celestial Holy Halo (Dual Glowing Concentric Torus Rings)
inline void GenerateHalo(float radius, float height_offset, float float_time,
                         std::vector<Vertex3D> &out_vertices,
                         std::vector<Triangle3D> &out_triangles) {
  out_vertices.clear();
  out_triangles.clear();

  int ring_pts = 32;
  int tube_pts = 6;
  float r_major = radius * 0.85f + 8.0f;
  float r_minor = 1.8f;
  float float_z =
      10.0f + height_offset * 0.4f + std::sin(float_time * 2.5f) * 1.5f;

  // Main Primary Torus Ring
  for (int i = 0; i < ring_pts; i++) {
    float u = (float)i * (2.0f * 3.14159265f / (float)ring_pts);
    float cx = std::cos(u) * r_major;
    float cy = std::sin(u) * r_major;

    for (int j = 0; j < tube_pts; j++) {
      float v = (float)j * (2.0f * 3.14159265f / (float)tube_pts);
      float vx = cx + std::cos(u) * std::cos(v) * r_minor;
      float vy = cy + std::sin(u) * std::cos(v) * r_minor;
      float vz = float_z + std::sin(v) * r_minor;

      out_vertices.push_back({vx, vy, vz, std::cos(u) * std::cos(v),
                              std::sin(u) * std::cos(v), std::sin(v),
                              (float)i / (float)ring_pts, 0});
    }
  }

  for (int i = 0; i < ring_pts; i++) {
    int next_i = (i + 1) % ring_pts;
    for (int j = 0; j < tube_pts; j++) {
      int next_j = (j + 1) % tube_pts;
      uint16_t i0 = (uint16_t)(i * tube_pts + j);
      uint16_t i1 = (uint16_t)(next_i * tube_pts + j);
      uint16_t i2 = (uint16_t)(next_i * tube_pts + next_j);
      uint16_t i3 = (uint16_t)(i * tube_pts + next_j);

      out_triangles.push_back({i0, i1, i2, 1.0f, 0xFFFFF0A0});
      out_triangles.push_back({i0, i2, i3, 1.0f, 0xFFFFD700});
    }
  }

  // Inner Secondary Concentric Ring (Golden Core)
  uint16_t inner_base = (uint16_t)out_vertices.size();
  float r_inner = r_major * 0.70f;
  float r_minor2 = 1.0f;

  for (int i = 0; i < ring_pts; i++) {
    float u = (float)i * (2.0f * 3.14159265f / (float)ring_pts);
    float cx = std::cos(u) * r_inner;
    float cy = std::sin(u) * r_inner;

    for (int j = 0; j < 4; j++) {
      float v = (float)j * (2.0f * 3.14159265f / 4.0f);
      float vx = cx + std::cos(u) * std::cos(v) * r_minor2;
      float vy = cy + std::sin(u) * std::cos(v) * r_minor2;
      float vz = float_z + std::sin(v) * r_minor2;

      out_vertices.push_back(
          {vx, vy, vz, std::cos(u), std::sin(u), 1.0f, 0, 0});
    }
  }

  for (int i = 0; i < ring_pts; i++) {
    int next_i = (i + 1) % ring_pts;
    for (int j = 0; j < 4; j++) {
      int next_j = (j + 1) % 4;
      uint16_t i0 = (uint16_t)(inner_base + i * 4 + j);
      uint16_t i1 = (uint16_t)(inner_base + next_i * 4 + j);
      uint16_t i2 = (uint16_t)(inner_base + next_i * 4 + next_j);
      uint16_t i3 = (uint16_t)(inner_base + i * 4 + next_j);

      out_triangles.push_back({i0, i1, i2, 1.0f, 0xFFFFFFFF});
      out_triangles.push_back({i0, i2, i3, 1.0f, 0xFFFFE57F});
    }
  }
}

// 3. Majestic 3D Devil Horns (Curved Gothic Ram Horns with Smooth Segments &
// Ribs)
inline void GenerateDevilHorns(float scale, std::vector<Vertex3D> &out_vertices,
                               std::vector<Triangle3D> &out_triangles) {
  out_vertices.clear();
  out_triangles.clear();

  const int num_rings = 7;
  const int pts_per_ring = 6;
  float sc = scale * 0.05f + 0.5f;

  for (int side = -1; side <= 1; side += 2) {
    float s = (float)side;
    uint16_t base_idx = (uint16_t)out_vertices.size();

    for (int r = 0; r < num_rings; r++) {
      float frac = (float)r / (float)(num_rings - 1);
      float ring_radius = (3.2f * (1.0f - frac * 0.85f)) * sc;

      float cx = s * (5.5f + std::sin(frac * 1.8f) * 6.5f) * sc;
      float cy = (2.0f - frac * 12.0f - std::sin(frac * 3.14f) * 2.0f) * sc;
      float cz = (2.0f + frac * 14.0f + std::pow(frac, 2.0f) * 4.0f) * sc;

      for (int p = 0; p < pts_per_ring; p++) {
        float angle = (float)p * (2.0f * 3.14159265f / (float)pts_per_ring);
        float px = cx + std::cos(angle) * ring_radius;
        float py = cy + std::sin(angle) * (ring_radius * 0.85f);
        float pz = cz + std::sin(angle * 2.0f) * 0.4f * sc;

        out_vertices.push_back({px, py, pz, std::cos(angle), std::sin(angle),
                                0.3f, frac, (float)p / (float)pts_per_ring});
      }
    }

    float tip_x = s * 12.2f * sc;
    float tip_y = -10.5f * sc;
    float tip_z = 21.0f * sc;
    uint16_t tip_idx = (uint16_t)out_vertices.size();
    out_vertices.push_back({tip_x, tip_y, tip_z, 0, 0, 1, 1.0f, 0.5f});

    for (int r = 0; r < num_rings - 1; r++) {
      for (int p = 0; p < pts_per_ring; p++) {
        int next_p = (p + 1) % pts_per_ring;
        uint16_t i0 = (uint16_t)(base_idx + r * pts_per_ring + p);
        uint16_t i1 = (uint16_t)(base_idx + (r + 1) * pts_per_ring + p);
        uint16_t i2 = (uint16_t)(base_idx + (r + 1) * pts_per_ring + next_p);
        uint16_t i3 = (uint16_t)(base_idx + r * pts_per_ring + next_p);

        out_triangles.push_back({i0, i1, i2, 1.0f, 0});
        out_triangles.push_back({i0, i2, i3, 1.0f, 0});
      }
    }

    for (int p = 0; p < pts_per_ring; p++) {
      int next_p = (p + 1) % pts_per_ring;
      uint16_t i0 = (uint16_t)(base_idx + (num_rings - 1) * pts_per_ring + p);
      uint16_t i1 =
          (uint16_t)(base_idx + (num_rings - 1) * pts_per_ring + next_p);
      out_triangles.push_back({i0, tip_idx, i1, 1.0f, 0});
    }
  }
}

// 4. Opulent Royal Crown 3D Mesh (8-Spired Gold Crown with Ruby Jewels)
inline void GenerateCrown(float size, float height,
                          std::vector<Vertex3D> &out_vertices,
                          std::vector<Triangle3D> &out_triangles) {
  out_vertices.clear();
  out_triangles.clear();

  int spires = 8;
  float sc = size * 0.05f + 0.5f;
  float r_base = (6.2f * sc);
  float h_base = 2.0f * sc;
  float h_spire = (height * 0.35f + 5.0f) * sc;

  for (int i = 0; i < spires * 2; i++) {
    float a = (float)i * (2.0f * 3.14159265f / (float)(spires * 2));
    out_vertices.push_back({std::cos(a) * r_base, std::sin(a) * r_base,
                            1.0f * sc, std::cos(a), std::sin(a), 0, 0, 0});
    out_vertices.push_back({std::cos(a) * (r_base * 1.05f),
                            std::sin(a) * (r_base * 1.05f), (1.0f + h_base),
                            std::cos(a), std::sin(a), 0, 0, 0});
  }

  for (int i = 0; i < spires * 2; i++) {
    int next_i = (i + 1) % (spires * 2);
    uint16_t i0 = (uint16_t)(i * 2);
    uint16_t i1 = (uint16_t)(i * 2 + 1);
    uint16_t i2 = (uint16_t)(next_i * 2 + 1);
    uint16_t i3 = (uint16_t)(next_i * 2);

    out_triangles.push_back({i0, i1, i2, 1.0f, 0});
    out_triangles.push_back({i0, i2, i3, 1.0f, 0});
  }

  uint16_t spire_start = (uint16_t)out_vertices.size();
  for (int i = 0; i < spires; i++) {
    float a = (float)(i * 2) * (2.0f * 3.14159265f / (float)(spires * 2));
    float px = std::cos(a) * (r_base * 1.15f);
    float py = std::sin(a) * (r_base * 1.15f);
    float pz = (1.0f + h_base + h_spire);

    out_vertices.push_back(
        {px, py, pz, std::cos(a), std::sin(a), 1.0f, 0.5f, 1.0f});

    uint16_t left_v = (uint16_t)(i * 4 + 1);
    uint16_t mid_v = (uint16_t)(spire_start + i);
    uint16_t right_v = (uint16_t)(((i * 2 + 2) % (spires * 2)) * 2 + 1);

    out_triangles.push_back({left_v, mid_v, right_v, 1.0f, 0});
    out_triangles.push_back({left_v, right_v, mid_v, -1.0f, 0});
  }
}

// 5. Cute Anime / Cyber Neko Cat Ears 3D Mesh
inline void GenerateCatEars(float scale, std::vector<Vertex3D> &out_vertices,
                            std::vector<Triangle3D> &out_triangles) {
  out_vertices.clear();
  out_triangles.clear();

  float sc = scale * 0.05f + 0.5f;

  for (int side = -1; side <= 1; side += 2) {
    float s = (float)side;
    uint16_t b = (uint16_t)out_vertices.size();

    out_vertices.push_back({s * 7.5f * sc, -1.5f * sc, 3.5f * sc, s * 0.7f,
                            -0.3f, 0.6f, 0.0f, 0.0f});
    out_vertices.push_back({s * 2.8f * sc, 1.0f * sc, 4.5f * sc, s * 0.2f, 0.2f,
                            0.9f, 1.0f, 0.0f});
    out_vertices.push_back(
        {s * 5.2f * sc, 3.2f * sc, 3.8f * sc, 0.0f, 0.9f, 0.4f, 0.5f, 0.0f});
    out_vertices.push_back({s * 6.8f * sc, -1.0f * sc, 13.2f * sc, s * 0.5f,
                            -0.2f, 0.8f, 0.5f, 1.0f});
    out_vertices.push_back(
        {s * 5.0f * sc, 1.8f * sc, 6.5f * sc, 0.0f, 0.9f, 0.4f, 0.5f, 0.5f});

    out_triangles.push_back(
        {(uint16_t)(b + 0), (uint16_t)(b + 3), (uint16_t)(b + 1), 1.0f, 0});
    out_triangles.push_back(
        {(uint16_t)(b + 0), (uint16_t)(b + 2), (uint16_t)(b + 3), 1.0f, 0});

    out_triangles.push_back({(uint16_t)(b + 1), (uint16_t)(b + 3),
                             (uint16_t)(b + 4), 1.0f, 0xFFFFA0C0});
    out_triangles.push_back({(uint16_t)(b + 1), (uint16_t)(b + 4),
                             (uint16_t)(b + 2), 1.0f, 0xFFFF85A5});
    out_triangles.push_back({(uint16_t)(b + 2), (uint16_t)(b + 4),
                             (uint16_t)(b + 3), 1.0f, 0xFFFFB5D0});
  }
}
} // namespace AccessoriesMesh
