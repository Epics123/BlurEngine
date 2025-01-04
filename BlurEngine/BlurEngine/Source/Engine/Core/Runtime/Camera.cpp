#include "Camera.h"

namespace EngineCore
{
	
	Camera::Camera(const glm::vec3& StartPosition, const glm::vec3& StartTarget, const glm::vec3& StartUp, float Near, float Far, float StartAspectRatio)
		: Position{StartPosition}, Target{StartTarget}, UpVector{StartUp}, Orientation{glm::lookAt(Position, Target, UpVector)},
		  NearPlane{Near}, FarPlane{Far}, AspectRatio{StartAspectRatio}
	{
		Projection = glm::perspective(glm::radians(Fov), AspectRatio, NearPlane, FarPlane);
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		const glm::mat4 TranslationMat = glm::translate(glm::mat4(1.0f), -Position);
		const glm::mat4 RotationMat = glm::mat4_cast(Orientation);

		return RotationMat * TranslationMat;
	}

	glm::vec3 Camera::GetForwardVector() const
	{
		const glm::mat4 RotationMat = glm::mat4_cast(Orientation);
		return glm::vec3(RotationMat[0][2], RotationMat[1][2], RotationMat[2][2]);
	}

	glm::vec3 Camera::GetRightVector() const
	{
		const glm::mat4 RotationMat = glm::mat4_cast(Orientation);
		return glm::vec3(RotationMat[0][0], RotationMat[1][0], RotationMat[2][0]);
	}

	glm::vec3 Camera::GetUpVector() const
	{
		return glm::normalize(glm::cross(GetRightVector(), GetForwardVector()));
	}

	glm::vec3 Camera::EulerAngles() const
	{
		glm::vec3 Angles = glm::degrees(glm::eulerAngles(Orientation));
		return Angles;
	}

	void Camera::Move(const glm::vec3& Direction, float Increment)
	{
		bDirty = true;
		Position = Position + (Direction * Increment * sCameraMoveSpeed);
	}

	void Camera::SetPostion(const glm::vec3& NewPos)
	{
		bDirty = true;
		Position = NewPos;
	}

	void Camera::Rotate(const glm::vec2 Delta, float Speed)
	{
		const glm::quat DeltaQuat = glm::quat(glm::vec3(-Speed * Delta.y, Speed * Delta.x, 0.0f));
		Orientation = DeltaQuat * Orientation;
		Orientation = glm::normalize(Orientation);

		{
			bDirty = true;
			const glm::mat4 RotationMat = glm::mat4_cast(Orientation);
			const glm::vec3 Direction = -glm::vec3(RotationMat[0][2], RotationMat[1][2], RotationMat[2][2]);
			Orientation = glm::quat(glm::lookAt(Position, Position + Direction, GetUpVector()));
		}
	}

}