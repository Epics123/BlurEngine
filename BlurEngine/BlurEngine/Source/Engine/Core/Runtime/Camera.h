#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace EngineCore
{

class Camera
{
public:
	Camera(const glm::vec3& StartPosition = glm::vec3(0.0f, 2.0f, 2.0f), 
		   const glm::vec3& StartTarget = glm::vec3(0.0f), 
		   const glm::vec3& StartUp = glm::vec3(0.0f, 1.0f, 0.0f), 
		   float Near = 0.1f, float Far = 4000.0f, 
		   float StartAspectRatio = 1600.0f / 900.0f);

	glm::mat4 GetProjectionMatrix() const { return Projection; }
	glm::mat4 GetViewMatrix() const;

	glm::vec3 GetForwardVector() const;
	glm::vec3 GetRightVector() const;
	glm::vec3 GetUpVector() const;

	// Gets an Euler Angle representation of the camera's orientation in degrees
	glm::vec3 EulerAngles() const;

	bool IsDirty() const { return bDirty; }
	bool SetDirty(bool bNewDirty) { bDirty = bNewDirty; }

	void Move(const glm::vec3& Direction, float Increment);
	void SetPostion(const glm::vec3& NewPos);
	void Rotate(const glm::vec2 Delta, float Speed);

private:
	glm::vec3 Position;
	glm::vec3 Target;
	glm::vec3 UpVector;

	glm::quat Orientation;

	glm::mat4 Projection{1.0f};
	glm::mat4 Jitter{1.0f};
	glm::vec2 JitterVal;

	float NearPlane = 10.0f;
	float FarPlane = 4000.0f;
	float Fov = 70.0f;
	float AspectRatio = 1600.0f / 900.0f;

	bool bDirty = true;

	static constexpr float sCameraMoveSpeed = 0.3f;
};
}