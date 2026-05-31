#pragma once

namespace rage
{
    class alignas(16) Vector3
    {
    public:
        float x{};
        float y{};
        float z{};

        constexpr Vector3(float x, float y, float z)
            : x(x),
              y(y),
              z(z)
        {
        }

        constexpr Vector3()
            : x(),
              y(),
              z()
        {
        }

        operator bool() const
        {
            return this->x != 0.0f || this->y != 0.0f || this->z != 0.0f;
        }

        bool operator==(const Vector3& other) const
        {
            return this->x == other.x && this->y == other.y && this->z == other.z;
        }

        bool operator!=(const Vector3& other) const
        {
            return this->x != other.x || this->y != other.y || this->z != other.z;
        }

        Vector3 operator+(const Vector3& other) const
        {
            Vector3 vec;
            vec.x = this->x + other.x;
            vec.y = this->y + other.y;
            vec.z = this->z + other.z;
            return vec;
        }

        Vector3 operator-(const Vector3& other) const
        {
            Vector3 vec;
            vec.x = this->x - other.x;
            vec.y = this->y - other.y;
            vec.z = this->z - other.z;
            return vec;
        }

        Vector3 operator*(const Vector3& other) const
        {
            Vector3 vec;
            vec.x = this->x * other.x;
            vec.y = this->y * other.y;
            vec.z = this->z * other.z;
            return vec;
        }

        Vector3 operator/(const Vector3& other) const
        {
            Vector3 vec;
            vec.x = this->x / other.x;
            vec.y = this->y / other.y;
            vec.z = this->z / other.z;
            return vec;
        }
    };
};