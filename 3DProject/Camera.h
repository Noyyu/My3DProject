#pragma once
#include "pch.h"


class Camera
{
	/////////////// Camera stuff ///////////////////

private:

	DirectX::SimpleMath::Vector3 move = DirectX::SimpleMath::Vector3::Zero;

	const DirectX::XMVECTORF32 START_POSITION = { 0.f, -1.5f, 0.f, 0.f };
	const DirectX::XMVECTORF32 ROOM_BOUNDS = { 800.f, 600.f, 1200.f, 0.f };
	float ROTATION_GAIN = 0.004f;
	float MOVEMENT_GAIN = 0.07f;

	std::unique_ptr<DirectX::Mouse> mouse = std::make_unique<DirectX::Mouse>();
	std::unique_ptr<DirectX::Keyboard> keyboard = std::make_unique<DirectX::Keyboard>();

	float yaw = 0.0f;
	float pitch = 0.0f;

	unsigned int WIDTH = 700;
	unsigned int HEIGHT = 700;

	float walkSpeed = 1.f;
	float cameraPitchAndYawSpeed = 5.f;
	float yPosition = 0;

	DirectX::XMMATRIX WorldViewProjection = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX cameraView = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX cameraProjection = DirectX::XMMatrixIdentity();

	DirectX::XMVECTOR cameraPos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR cameraTarget = { 0.0f, 0.0f, 1.0f };
	DirectX::XMVECTOR cameraUp = { 0.0f, 1.0f, 0.0f };

	DirectX::XMVECTOR defaultForward = { 0.0f, 0.0f, 1.0f };
	DirectX::XMVECTOR defaultRight = { 0.1f, 0.0f ,0.0f };
	DirectX::XMVECTOR forward = { 0.0f, 0.0f, 1.0f };
	DirectX::XMVECTOR right = { 1.0f, 0.0f, 0.0f };

	DirectX::XMMATRIX rotationMatrix;

public:
	Camera();
	Camera(DirectX::XMVECTOR cameraPos, DirectX::XMVECTOR cameraTarget, DirectX::XMVECTOR cameraUp, unsigned int WIDTH, unsigned int HEIGHT);
	~Camera();

	DirectX::XMMATRIX getCameraView();
	DirectX::XMMATRIX getCameraProjection();
	DirectX::XMVECTOR getCameraPos();

	void setWalkSpeed(float speed);
	void setCameraPitchAndYawSpeed(float speed);
	void moveCameraWithInput();
	void setWindowForMouse(HWND window);
	void setCameraY(float y);

	float getGameraYPos();

};