#pragma once
#include "definitions.h"

struct color
{
	color() : r(255), g(255), b(255), a(255) { }

	color(int r, int g, int b, int a = 255)
	{
		this->r = max(0, min(r, 255));
		this->g = max(0, min(g, 255));
		this->b = max(0, min(b, 255));
		this->a = max(0, min(a, 255));
	}

	color operator+(int val)
	{
		this->r = min(255, r + val);
		this->g = min(255, g + val);
		this->b = min(255, b + val);
		return *this;
	}

	color operator-(int val)
	{
		this->r = max(0, r - val);
		this->g = max(0, g - val);
		this->b = max(0, b - val);
		return *this;
	}

	color operator+=(int val)
	{
		this->r = min(255, r + val);
		this->g = min(255, g + val);
		this->b = min(255, b + val);
		return *this;
	}

	dword to_d3d()
	{
		//return D3DCOLOR_ARGB(this->a, this->r, this->g, this->b);
		return D3DCOLOR_RGBA(this->r, this->g, this->b, this->a);
	}

	struct float_color {
		float rgba[4];
		float operator[](int i) const { return rgba[i]; }
		float& operator[](int i) { return rgba[i]; }
		operator float*() { return rgba; }
		operator const float*() const { return rgba; }
	};

	float_color divide() const
	{
		float_color fc;
		fc.rgba[0] = (float)(r / 255.f);
		fc.rgba[1] = (float)(g / 255.f);
		fc.rgba[2] = (float)(b / 255.f);
		fc.rgba[3] = (float)(a / 255.f);
		return fc;
	}

	color with_alpha(int alpha)
	{
		return color(r, g, b, max(0, min(alpha, 255)));
	}

	color to_alpha(int alpha, int speed)
	{
		if (abs(this->a - alpha) >= speed) this->a += (alpha > a) ? speed : -speed;
		else this->a = alpha;

		return color(r, g, b, a);
	}

	// öâåòà äëÿ ìåíþ
	static color background() { return color(18, 18, 18); } // çàäíèê
	static color outline() { return color(9, 9, 9); } // îáâîäêà ìåíþ è êîíòðîëëåðîâ
	static color closed_tab() { return color(30, 30, 30); } // çàêðûòàÿ âêëàäêà
	static color opened_tab() { return color(45, 45, 45); } // îòêðûòàÿ âêëàäêà
	static color text() { return color(150, 150, 150); } // òåêñò
	static color ptext() { return color(200, 200, 200); } // âûáðàííûé òåêñò
	static color disabled() { return color(40, 40, 40); } // ÷åêáîêñ âûêëþ÷åí
	static color enabled() { return color(152, 26, 152); } // ÷åêáîêñ âêëþ÷åí

	static color from_hsv(float h, float s, float v, int alpha = 255)
	{
		float c = v * s;
		float x = c * (1.0f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.0f));
		float m = v - c;
		float r1 = 0, g1 = 0, b1 = 0;
		if (h < 1.0f / 6.0f) { r1 = c; g1 = x; b1 = 0; }
		else if (h < 2.0f / 6.0f) { r1 = x; g1 = c; b1 = 0; }
		else if (h < 3.0f / 6.0f) { r1 = 0; g1 = c; b1 = x; }
		else if (h < 4.0f / 6.0f) { r1 = 0; g1 = x; b1 = c; }
		else if (h < 5.0f / 6.0f) { r1 = x; g1 = 0; b1 = c; }
		else { r1 = c; g1 = 0; b1 = x; }
		return color((int)((r1 + m) * 255.0f), (int)((g1 + m) * 255.0f), (int)((b1 + m) * 255.0f), alpha);
	}

	// ìîé ëþáèìûé öâåò
	static color lm() { return color(49, 124, 230); }

	int r, g, b, a;
};