#pragma once

#include "Camera.h"
#include "Effect.h"
#include <memory>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	class Mesh;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		// When F2 is pressed, change to next sampler state
		void ChangeSamplerState() noexcept;

		void Update(Timer* pTimer);
		void Render() const;

	private:
		//Window
		SDL_Window* m_pWindow{};
		int m_Width{};
		int m_Height{};

		Camera m_Camera{};

		//DirectX
		bool m_IsInitialized{ false };
		ID3D11Device* m_pDevice{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext{ nullptr };
		IDXGISwapChain* m_pSwapChain{ nullptr };
		ID3D11Texture2D* m_pDepthStencilBuffer{ nullptr };
		ID3D11DepthStencilView* m_pDepthStencilView{ nullptr };
		ID3D11Resource* m_pRenderTargetBuffer{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };

		//Effects
		// would be shared with resource manager
		std::shared_ptr<BaseEffect> m_pVehicleEffect{ nullptr };
		std::shared_ptr<BaseEffect> m_pFireEffect{ nullptr };

		//Models
		std::vector<std::unique_ptr<Mesh>> m_Meshes;

		//Textures
		// would be in resource manager
		std::unique_ptr<Texture> m_pDiffuseTexture{ nullptr };
		std::unique_ptr<Texture> m_pFireDiffuseTexture{ nullptr };
		std::unique_ptr<Texture> m_pNormalTexture{ nullptr };
		std::unique_ptr<Texture> m_pGlossinessTexture{ nullptr };
		std::unique_ptr<Texture> m_pSpecularTexture{ nullptr };

		//Settings
		enum class SamplerState : uint8_t
		{
			Point = 0,
			Linear = 1,
			Anisotropic = 2,
			COUNT
		};
		SamplerState m_SamplerState{ SamplerState::Point };

		//Samplers
		ID3D11SamplerState* m_pPointSampler{ nullptr };
		ID3D11SamplerState* m_pLinearSampler{ nullptr };
		ID3D11SamplerState* m_pAnisotropicSampler{ nullptr };


		HRESULT InitializeDirectX();
	};
}
