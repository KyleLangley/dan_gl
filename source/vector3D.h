#pragma once
class vector3D
{
public:
	vector3D() {};
	vector3D(float _x, float _y, float _z) :x(_x), y(_y), z(_z)	{};

private:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

public:
	float data[3]{ x, y, z };
};

