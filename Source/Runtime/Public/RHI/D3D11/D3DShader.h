#pragma once

#ifdef API_GRAPHICS_D3D11

// Engine Include
#include "D3DDefinition.h"


namespace Fluent
{
	
	class D3DShader
	{
	public:

		explicit D3DShader() = default;
		virtual ~D3DShader() = default;

		[[nodiscard]]
		ID3D11VertexShader* GetVertexShader() const { return static_cast<ID3D11VertexShader*>(mShader); }

		[[nodiscard]]
		ID3D11PixelShader* GetPixelShader() const { return static_cast<ID3D11PixelShader*>(mShader); }

		[[nodiscard]]
		ID3D11ComputeShader* GetComputeShader() const { return static_cast<ID3D11ComputeShader*>(mShader); }
		
	protected:

		void* mShader = nullptr;

	};
	
}

#endif
