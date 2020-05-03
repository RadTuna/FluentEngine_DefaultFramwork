#pragma once

// External Include
#include <vector>
#include <memory>

// Engine Include
#include "Core/Core.h"
#include "Core/ISubModule.h"
#include "RendererDefinition.h"
#include "RHI/RHIDefinition.h"


namespace Fluent
{

	class Renderer final : public ISubModule
	{
	public:
		
		explicit Renderer(const std::shared_ptr<EngineStorage>& inStorage) noexcept;
		virtual ~Renderer() noexcept;

		
		// ISubModule functions
		bool Initialize() override;
		void Update(float deltaTime) override;

		[[nodiscard]]
		bool IsInitialized() const { return mbIsInitialized; }

	private:

		// Render resource creation
		void CreateConstantBuffers();
		void CreateShaders();
		void CreateDepthStencilStates();
		void CreateRasterizerStates();
		void CreateRenderTargets();
		void CreateBlendState();
		void CreateCommandLists();

		// Update constant buffers
		void UpdateFrameBuffer(const std::shared_ptr<CommandList>& commandList);
		
		// Render passes
		void PassGBuffer(const std::shared_ptr<CommandList>& commandList);
		void PassComposition(const std::shared_ptr<CommandList>& commandList);
		
	private:

		// Core
		std::shared_ptr<Device> mDevice;
		std::shared_ptr<SwapChain> mSwapChain;

		// CommandLists // SingleThread == 1
		std::vector<std::shared_ptr<CommandList>> mCommandLists;
		
		// RenderTargets
		std::vector<std::shared_ptr<Texture2D>> mRenderTargets;

		// Shaders
		std::vector<std::shared_ptr<Shader>> mShaders;

		// Depth-stencil state
		std::vector<std::shared_ptr<DepthStencilState>> mDepthStencilStates;

		// Rasterizer state
		std::vector<std::shared_ptr<RasterizerState>> mRasterizerStates;

		// Constant buffer
		std::vector<std::shared_ptr<ConstantBuffer>> mConstantBuffers;

		// Blend state
		std::vector<std::shared_ptr<BlendState>> mBlendStates;

		// Other
		bool mbIsInitialized = false;

		bool mbIsRendering = false;
	};

}
