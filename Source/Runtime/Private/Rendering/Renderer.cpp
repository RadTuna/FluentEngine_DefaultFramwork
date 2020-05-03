
// Primary Include
#include "Rendering/Renderer.h"

// Engine Include
#include "Core/EngineStorage.h"
#include "Rendering/ConstantBufferTypes.h"
#include "RHI/Device.h"
#include "RHI/SwapChain.h"
#include "RHI/ConstantBuffer.h"
#include "RHI/Shader.h"
#include "RHI/DepthStencilState.h"
#include "RHI/RasterizerState.h"
#include "RHI/Texture2D.h"
#include "RHI/PipelineState.h"
#include "RHI/BlendState.h"

// temp include
#include "Math/Vector.h"
#include "Math/Math.h"


namespace Fluent
{

	Renderer::Renderer(const std::shared_ptr<EngineStorage>& inStorage) noexcept
		: ISubModule(inStorage)
	{
	}

	Renderer::~Renderer()
	{
	}

	bool Renderer::Initialize()
	{
		mDevice = std::make_shared<Device>();
		if (!mDevice->IsInitialized())
		{
			return false;
		}

		mSwapChain = std::make_shared<SwapChain>(mDevice, mStorage->mWindowData);
		if (!mSwapChain->IsInitialized())
		{
			return false;
		}

		CreateConstantBuffers();
		CreateShaders();
		CreateDepthStencilStates();
		CreateRasterizerStates();
		CreateRenderTargets();
		CreateBlendState();
		CreateCommandLists();

		if (!mbIsInitialized)
		{
			mbIsInitialized = true;
		}

		return true;
	}

	void Renderer::Update(float deltaTime)
	{
		if (!mbIsInitialized)
		{
			return;
		}
		
		Assert(mCommandLists.size() != 0);
		std::shared_ptr<CommandList>& commandList = mCommandLists[0];


		// Start rendering
		mbIsRendering = true;

		UpdateFrameBuffer(commandList);
		PassGBuffer(commandList);
		PassComposition(commandList);
		
		mSwapChain->Present();

		mbIsRendering = false;
		// End rendering
	}

	void Renderer::CreateConstantBuffers()
	{
		mConstantBuffers.resize(ConstantBufferTypesNum);
		
		mConstantBuffers[EConstantBufferType::FrameBuffer] = std::make_shared<ConstantBuffer>(mDevice);
		mConstantBuffers[EConstantBufferType::FrameBuffer]->CreateBuffer<BufferFrame>();
	}

	void Renderer::CreateShaders()
	{
		mShaders.resize(RenderShaderTypesNum);

		mShaders[ERenderShaderType::VS_Common] = std::make_shared<Shader>(mDevice, EShaderType::Vertex);
		mShaders[ERenderShaderType::VS_Common]->Compile<VertexPos>("TempPath");

		mShaders[ERenderShaderType::PS_Common] = std::make_shared<Shader>(mDevice, EShaderType::Pixel);
		mShaders[ERenderShaderType::PS_Common]->Compile<VertexPos>("TempPath");
	}

	void Renderer::CreateDepthStencilStates()
	{
		mDepthStencilStates.resize(DepthStencilStateTypesNum);

		mDepthStencilStates[EDepthStencilStateType::Enable_Write] = std::make_shared<DepthStencilState>(mDevice, true, true, false);
		mDepthStencilStates[EDepthStencilStateType::Enable_NoWrite] = std::make_shared<DepthStencilState>(mDevice, true, false, false);
		mDepthStencilStates[EDepthStencilStateType::Disable] = std::make_shared<DepthStencilState>(mDevice, false, false, false);
	}

	void Renderer::CreateRasterizerStates()
	{
		mRasterizerStates.resize(RasterizerStateTypesNum);

		mRasterizerStates[ERasterizerStateType::CullBack_Solid] =
			std::make_shared<RasterizerState>(mDevice, ERenderCullMode::Back, ERenderFillMode::Solid, false, false, false, false);
		mRasterizerStates[ERasterizerStateType::CullBack_Solid_NoClip] =
			std::make_shared<RasterizerState>(mDevice, ERenderCullMode::Back, ERenderFillMode::Solid, true, false, false, false);
		mRasterizerStates[ERasterizerStateType::CullFront_Solid] =
			std::make_shared<RasterizerState>(mDevice, ERenderCullMode::Front, ERenderFillMode::Solid, false, false, false, false);
		mRasterizerStates[ERasterizerStateType::CullNone_Solid] =
			std::make_shared<RasterizerState>(mDevice, ERenderCullMode::None, ERenderFillMode::Solid, false, false, false, false);
		mRasterizerStates[ERasterizerStateType::CullBack_WireFrame] =
			std::make_shared<RasterizerState>(mDevice, ERenderCullMode::Back, ERenderFillMode::WireFrame, false, false, false, true);
		mRasterizerStates[ERasterizerStateType::CullFront_WireFrame] =
			std::make_shared<RasterizerState>(mDevice, ERenderCullMode::Front, ERenderFillMode::WireFrame, false, false, false, true);
		mRasterizerStates[ERasterizerStateType::CullNone_WireFrame] =
			std::make_shared<RasterizerState>(mDevice, ERenderCullMode::None, ERenderFillMode::WireFrame, false, false, false, true);
	}

	void Renderer::CreateRenderTargets()
	{
		const u32 width = mStorage->mWindowData.ScreenWidth;
		const u32 height = mStorage->mWindowData.ScreenHeight;

		if ((width / 4) == 0 || (height / 4) == 0)
		{
			return;
		}

		mRenderTargets.resize(RenderTargetTypesNum);

		mRenderTargets[ERenderTargetType::Albedo] =
			std::make_shared<Texture2D>(mDevice, width, height, EPixelFormat::R8G8B8A8_Unorm);
		mRenderTargets[ERenderTargetType::Diffuse] =
			std::make_shared<Texture2D>(mDevice, width, height, EPixelFormat::R8G8B8A8_Unorm);
		mRenderTargets[ERenderTargetType::Specular] =
			std::make_shared<Texture2D>(mDevice, width, height, EPixelFormat::R8G8B8A8_Unorm);
		mRenderTargets[ERenderTargetType::Normal] =
			std::make_shared<Texture2D>(mDevice, width, height, EPixelFormat::R16G16B16A16_Float);
		mRenderTargets[ERenderTargetType::Depth] =
			std::make_shared<Texture2D>(mDevice, width, height, EPixelFormat::D32_Float);
	}

	void Renderer::CreateBlendState()
	{
		mBlendStates.resize(BlendStateTypesNum);
		
		mBlendStates[EBlendStateType::Disable] = std::make_shared<BlendState>(mDevice, false,
			ERenderBlend::SrcAlpha, ERenderBlend::InvSrcAlpha, ERenderBlendOperation::Add,
			ERenderBlend::One, ERenderBlend::One, ERenderBlendOperation::Add);
		mBlendStates[EBlendStateType::Enable] = std::make_shared<BlendState>(mDevice, true,
			ERenderBlend::SrcAlpha, ERenderBlend::InvSrcAlpha, ERenderBlendOperation::Add,
			ERenderBlend::One, ERenderBlend::One, ERenderBlendOperation::Add);
		mBlendStates[EBlendStateType::ColorAdd] = std::make_shared<BlendState>(mDevice, true,
			ERenderBlend::One, ERenderBlend::One, ERenderBlendOperation::Add,
			ERenderBlend::One, ERenderBlend::One, ERenderBlendOperation::Add);
	}

	void Renderer::CreateCommandLists()
	{
		// temp // only single thread
		mCommandLists.emplace_back(mDevice);
	}

	void Renderer::UpdateFrameBuffer(const std::shared_ptr<CommandList>& commandList)
	{
		// Map
		BufferFrame* bufferFrame = reinterpret_cast<Fluent::BufferFrame*>(mConstantBuffers[EConstantBufferType::FrameBuffer]->Map());
		Assert(bufferFrame != nullptr);

		// temp
		const f32 cameraNear = 1.0f;
		const f32 cameraFar = 1000.0f;
		const Vector cameraPosition = Vector::SetVector4(0.0f, 0.0f, 0.0f, 0.0f);
		const Vector targetPosition = Vector::SetVector4(0.0f, 0.0f, 1.0f, 0.0f);
		const Vector upVector = Vector::SetVector4(0.0f, 1.0f, 0.0f, 0.0f);
		const Matrix viewMatrix = Matrix::CreateLookAtLH(cameraPosition, targetPosition, upVector);
		const Matrix projMatrix = Matrix::CreatePerspectiveLH(
			mStorage->mWindowData.ScreenWidth,
			mStorage->mWindowData.ScreenHeight,
			cameraNear, cameraFar);
		// temp
		
		bufferFrame->CameraNear = cameraNear;
		bufferFrame->CameraFar = cameraFar;
		bufferFrame->WorldMatrix = Matrix4x4::Identity;
		Matrix::StoreMatrix4X4(viewMatrix, &bufferFrame->ViewMatrix);
		Matrix::StoreMatrix4X4(projMatrix, &bufferFrame->ProjectionMatrix);

		// Unmap
		mConstantBuffers[EConstantBufferType::FrameBuffer]->Unmap();
	}

	void Renderer::PassGBuffer(const std::shared_ptr<CommandList>& commandList)
	{
		std::shared_ptr<Texture2D> albedoTex = mRenderTargets[ERenderTargetType::Albedo];
		std::shared_ptr<Texture2D> diffuseTex = mRenderTargets[ERenderTargetType::Diffuse];
		std::shared_ptr<Texture2D> specularTex = mRenderTargets[ERenderTargetType::Specular];
		std::shared_ptr<Texture2D> normalTex = mRenderTargets[ERenderTargetType::Normal];
		std::shared_ptr<Texture2D> depthTex = mRenderTargets[ERenderTargetType::Depth];
		
		
	}

	void Renderer::PassComposition(const std::shared_ptr<CommandList>& commandList)
	{
	}
	
}
