#pragma once
#include <vector>
#include <cmath>
#include "imgui.h"

// Procedural High-Fidelity 3D Demon Wings Mesh Generator & Renderer
namespace WingsMesh
{
	struct Vertex3D
	{
		float x, y, z;       // 3D coordinates relative to spine attachment
		float nx, ny, nz;    // Surface normal for 3D Phong shading
		float u, v;          // Texture / gradient coordinates
	};

	struct Triangle3D
	{
		uint16_t i0, i1, i2;
		float normal_z;
	};

	// Generate a pair of 3D Demon Bat Wings with real 3D bone volume, finger ribs, and curved membranes
	inline void GenerateDemonWings(float flap_time, float wing_scale, std::vector<Vertex3D>& out_vertices, std::vector<Triangle3D>& out_triangles)
	{
		out_vertices.clear();
		out_triangles.clear();

		float flap_cycle = std::sin(flap_time);
		float flap_spread = 0.55f + 0.45f * flap_cycle;
		float flap_sweep = std::cos(flap_time) * 0.20f;

		// Bone thicknesses
		float thick_arm = 2.2f * wing_scale;
		float thick_finger = 1.2f * wing_scale;

		for (int side = -1; side <= 1; side += 2)
		{
			float s = (float)side;
			uint16_t base_idx = (uint16_t)out_vertices.size();

			// Key Joint Centers in 3D
			// Z is up, X is lateral, Y is forward/backward
			float root_x = s * 3.5f * wing_scale;
			float root_y = -3.0f * wing_scale;
			float root_z = 0.0f;

			float j1_x = s * (18.0f + 16.0f * flap_spread) * wing_scale;
			float j1_y = (-8.0f + flap_sweep * 6.0f) * wing_scale;
			float j1_z = (12.0f + flap_cycle * 8.0f) * wing_scale;

			float j2_x = s * (38.0f + 28.0f * flap_spread) * wing_scale;
			float j2_y = (-14.0f + flap_sweep * 12.0f) * wing_scale;
			float j2_z = (18.0f + flap_cycle * 16.0f) * wing_scale;

			// 4 Finger Talon Tips in 3D
			float t1_x = s * (58.0f + 36.0f * flap_spread) * wing_scale;
			float t1_y = (-18.0f + flap_sweep * 14.0f) * wing_scale;
			float t1_z = (28.0f + flap_cycle * 22.0f) * wing_scale;

			float t2_x = s * (64.0f + 32.0f * flap_spread) * wing_scale;
			float t2_y = (-14.0f + flap_sweep * 10.0f) * wing_scale;
			float t2_z = (6.0f + flap_cycle * 18.0f) * wing_scale;

			float t3_x = s * (52.0f + 24.0f * flap_spread) * wing_scale;
			float t3_y = (-10.0f + flap_sweep * 6.0f) * wing_scale;
			float t3_z = (-12.0f + flap_cycle * 12.0f) * wing_scale;

			float t4_x = s * (34.0f + 16.0f * flap_spread) * wing_scale;
			float t4_y = (-6.0f + flap_sweep * 4.0f) * wing_scale;
			float t4_z = (-24.0f + flap_cycle * 6.0f) * wing_scale;

			// Scallop Arch Intermediates (Curved membrane sagging)
			float a1_x = (t1_x + t2_x) * 0.5f - s * 4.0f * wing_scale;
			float a1_y = (t1_y + t2_y) * 0.5f;
			float a1_z = (t1_z + t2_z) * 0.5f - 6.0f * wing_scale;

			float a2_x = (t2_x + t3_x) * 0.5f - s * 4.0f * wing_scale;
			float a2_y = (t2_y + t3_y) * 0.5f;
			float a2_z = (t2_z + t3_z) * 0.5f - 6.0f * wing_scale;

			float a3_x = (t3_x + t4_x) * 0.5f - s * 3.5f * wing_scale;
			float a3_y = (t3_y + t4_y) * 0.5f;
			float a3_z = (t3_z + t4_z) * 0.5f - 5.0f * wing_scale;

			// Vertices definition (with 3D front and back depth)
			// 0: Root, 1: Joint 1 (Elbow), 2: Joint 2 (Wrist)
			// 3: Tip 1, 4: Arch 1, 5: Tip 2, 6: Arch 2, 7: Tip 3, 8: Arch 3, 9: Tip 4
			out_vertices.push_back({ root_x, root_y, root_z, 0, 1, 0, 0.0f, 0.0f });
			out_vertices.push_back({ j1_x, j1_y, j1_z, 0, 1, 0, 0.3f, 0.2f });
			out_vertices.push_back({ j2_x, j2_y, j2_z, 0, 1, 0, 0.6f, 0.4f });

			out_vertices.push_back({ t1_x, t1_y, t1_z, 0, 0.9f, 0.4f, 1.0f, 0.0f });
			out_vertices.push_back({ a1_x, a1_y, a1_z, 0, 0.8f, 0.3f, 0.9f, 0.3f });
			out_vertices.push_back({ t2_x, t2_y, t2_z, 0, 0.9f, 0.2f, 1.0f, 0.5f });
			out_vertices.push_back({ a2_x, a2_y, a2_z, 0, 0.8f, 0.1f, 0.85f, 0.7f });
			out_vertices.push_back({ t3_x, t3_y, t3_z, 0, 0.9f, 0.0f, 0.9f, 0.9f });
			out_vertices.push_back({ a3_x, a3_y, a3_z, 0, 0.8f, -0.1f, 0.7f, 1.0f });
			out_vertices.push_back({ t4_x, t4_y, t4_z, 0, 0.9f, -0.2f, 0.5f, 1.0f });

			// Arm 3D Bone Prisms (front/back depth vertices)
			out_vertices.push_back({ j1_x, j1_y - thick_arm, j1_z, 0, -1, 0, 0.3f, 0.2f });
			out_vertices.push_back({ j2_x, j2_y - thick_arm, j2_z, 0, -1, 0, 0.6f, 0.4f });

			// Triangles for Webbing Membranes (Double-Sided)
			// Web 1: J2 -> T1 -> A1 -> T2
			out_triangles.push_back({ (uint16_t)(base_idx + 2), (uint16_t)(base_idx + 3), (uint16_t)(base_idx + 4), 1.0f });
			out_triangles.push_back({ (uint16_t)(base_idx + 2), (uint16_t)(base_idx + 4), (uint16_t)(base_idx + 5), 1.0f });

			// Web 2: J2 -> T2 -> A2 -> T3
			out_triangles.push_back({ (uint16_t)(base_idx + 2), (uint16_t)(base_idx + 5), (uint16_t)(base_idx + 6), 1.0f });
			out_triangles.push_back({ (uint16_t)(base_idx + 2), (uint16_t)(base_idx + 6), (uint16_t)(base_idx + 7), 1.0f });

			// Web 3: J2 -> T3 -> A3 -> T4
			out_triangles.push_back({ (uint16_t)(base_idx + 2), (uint16_t)(base_idx + 7), (uint16_t)(base_idx + 8), 1.0f });
			out_triangles.push_back({ (uint16_t)(base_idx + 2), (uint16_t)(base_idx + 8), (uint16_t)(base_idx + 9), 1.0f });

			// Upper Arm Membrane: Root -> J1 -> J2 -> T1
			out_triangles.push_back({ (uint16_t)(base_idx + 0), (uint16_t)(base_idx + 1), (uint16_t)(base_idx + 2), 1.0f });
			out_triangles.push_back({ (uint16_t)(base_idx + 1), (uint16_t)(base_idx + 3), (uint16_t)(base_idx + 2), 1.0f });

			// Inner Flank Membrane: Root -> J2 -> T4
			out_triangles.push_back({ (uint16_t)(base_idx + 0), (uint16_t)(base_idx + 2), (uint16_t)(base_idx + 9), 1.0f });
		}
	}
}
