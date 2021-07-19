#include "Camera.h"
#define WM_MOUSEMOVE


Camera::Camera()
{
}

Camera::Camera(DirectX::XMVECTOR cameraPos, DirectX::XMVECTOR cameraTarget, DirectX::XMVECTOR cameraUp, unsigned int WIDTH, unsigned int HEIGHT)
	:cameraPos(cameraPos), cameraTarget(cameraTarget), cameraUp(cameraUp), WIDTH(WIDTH), HEIGHT(HEIGHT)
{

}

DirectX::XMMATRIX Camera::getCameraProjection()
{
	return this->cameraProjection;
}

DirectX::XMVECTOR Camera::getCameraPos()
{
	return this->cameraPos;
}

void Camera::setWalkSpeed(float speed)
{
	this->walkSpeed = speed;
}

void Camera::setCameraPitchAndYawSpeed(float speed)
{
	this->cameraPitchAndYawSpeed = speed;
}


DirectX::XMMATRIX Camera::getCameraView()
{
	return this->cameraView;
}

void Camera::moveCameraWithInput()
{

	////// MOUSE MOVEMENT ///////

	auto m_mouse = mouse->GetState();

	if (m_mouse.positionMode == DirectX::Mouse::MODE_RELATIVE)
	{
		DirectX::SimpleMath::Vector3 delta = DirectX::SimpleMath::Vector3(float(m_mouse.x), float(m_mouse.y), 0.f) * ROTATION_GAIN * cameraPitchAndYawSpeed;

		pitch += delta.y;
		yaw += delta.x;

		// limit pitch to straight up or straight down
		// with a little fudge-factor to avoid gimbal lock
		float limit = DirectX::XM_PI / 2.0f - 0.01f;
		pitch = std::max<float>(-limit, pitch);
		pitch = std::min<float>(+limit, pitch);

		// keep longitude in sane range by wrapping
		if (yaw > DirectX::XM_PI)
		{
			yaw -= DirectX::XM_PI * 2.0;
		}
		else if (yaw < -DirectX::XM_PI)
		{
			yaw += DirectX::XM_PI * 2.0;
		}
	}

	mouse->SetMode(m_mouse.leftButton ? DirectX::Mouse::MODE_RELATIVE : DirectX::Mouse::MODE_ABSOLUTE);

	////// KEYBOARD ///////


	auto kb = keyboard->GetState();
	DirectX::SimpleMath::Vector3 move = DirectX::SimpleMath::Vector3::Zero;

	if (kb.Up || kb.W)
		move.z += walkSpeed;

	if (kb.Down || kb.S)
		move.z -= walkSpeed;

	if (kb.Left || kb.A)
		move.x -= walkSpeed;

	if (kb.Right || kb.D)
		move.x += walkSpeed;

	if (kb.PageUp || kb.Space)
		move.y += 1.f;

	if (kb.LeftShift || kb.X)
		move.y -= 1.f;

	

	DirectX::SimpleMath::Quaternion q = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(yaw,pitch, 0.f);

	move = DirectX::SimpleMath::Vector3::Transform(move, q);

	move *= MOVEMENT_GAIN;

	cameraPos += move;

	DirectX::SimpleMath::Vector3 halfBound = (DirectX::SimpleMath::Vector3(ROOM_BOUNDS.v) / DirectX::SimpleMath::Vector3(2.f)) - DirectX::SimpleMath::Vector3(0.1f, 0.1f, 0.1f);

	cameraPos = DirectX::SimpleMath::Vector3::Min(cameraPos, halfBound);
	cameraPos = DirectX::SimpleMath::Vector3::Max(cameraPos, -halfBound);


	/////UPDATE CAMERA/////

	this->rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0);
	this->cameraTarget = DirectX::XMVector3TransformCoord(this->defaultForward, this->rotationMatrix);
	this->cameraTarget = DirectX::XMVector3Normalize(this->cameraTarget);

	//making a temp rotation matrix for updating rotation of the camera
	DirectX::XMMATRIX rotateYTempMatrix = DirectX::XMMatrixRotationAxis({ 0,1,0 }, pitch);

	//Transform the camera rotation
	this->right = DirectX::XMVector3TransformNormal(this->defaultRight, rotateYTempMatrix);
	this->cameraUp = DirectX::XMVector3TransformNormal(this->cameraUp, rotateYTempMatrix);
	this->forward = DirectX::XMVector3TransformNormal(this->defaultForward, rotateYTempMatrix);

	//Update the position of the camera
	this->cameraPos = DirectX::XMVectorAdd(this->cameraPos, DirectX::XMVectorScale(this->right, this->move.x));
	this->cameraPos = DirectX::XMVectorAdd(this->cameraPos, DirectX::XMVectorScale(this->forward, this->move.z));
	if (this->yPosition != 0)
	{
		this->cameraPos = DirectX::XMVectorSetY(this->cameraPos, this->yPosition);
	}

	//add the position of the camera to the target vector. 
	this->cameraTarget = DirectX::XMVectorAdd(this->cameraTarget, this->cameraPos);

	//these are the things that convert from objectspace to view space to projection space.
	this->cameraView = DirectX::XMMatrixLookAtLH(this->cameraPos, this->cameraTarget, this->cameraUp);
	this->cameraProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI * 0.45, this->WIDTH / this->HEIGHT, 0.001f, 200.0f); 
}

void Camera::setWindowForMouse(HWND window)
{
	mouse->SetWindow(window);
}

void Camera::setCameraY(float y)
{
	this->yPosition = y;
}

float Camera::getGameraYPos()
{
	return this->yPosition;
}
