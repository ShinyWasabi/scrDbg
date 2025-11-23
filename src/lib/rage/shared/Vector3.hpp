#pragma once

namespace rage::shared
{
#pragma pack(push, 0x10)
    class alignas(16) Vector3
    {
    public:
        float x, y, z;

    public:
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

        bool operator==(const Vector3& other) const
        {
            return this->x == other.x && this->y == other.y && this->z == other.z;
        }

        template <typename T>
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

        Vector3 operator*(const float& other) const
        {
            Vector3 vec;
            vec.x = this->x * other;
            vec.y = this->y * other;
            vec.z = this->z * other;
            return vec;
        }
    };
    static_assert(sizeof(Vector3) == 0x10);
#pragma pack(pop)
};