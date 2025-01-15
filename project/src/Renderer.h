#pragma once

#include "pch.h"

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

		void Update(Timer* pTimer);
		void Render() const;

	#pragma region RenderSettings
		// When F1 is pressed, switch between sofware and hardware rasterizer
		void ToggleRasterizerMode() noexcept
		{
			m_IsSofwareRasterizerMode = !m_IsSofwareRasterizerMode;

			if (m_IsSofwareRasterizerMode)
			{
				std::cout << "Rasterizer -> Software\n";
				return;
			}
			std::cout << "Rasterizer -> Hardware\n";
		}
		// When F2 is pressed, switch beteen rotating or not rotating the models
		void ToggleRotationMode() noexcept
		{
			m_IsRotationMode = !m_IsRotationMode;
			if (m_IsRotationMode)
			{
				std::cout << "Rotation -> Enabled\n";
				return;
			}
			std::cout << "Rotation -> Disabled\n";
		}
		// When F3 is pressed, switch beteen displaying or not displaying the fire mesh (only for the hardware rasterizer currently)
		void ToggleFireMesh() noexcept
		{
			m_DisplayFireMesh = !m_DisplayFireMesh;
			if (m_DisplayFireMesh)
			{
				std::cout << "FireMesh -> Enabled\n";
				return;
			}
			std::cout << "FireMesh -> Disabled\n";
		}
		// When F4 is pressed, change to next sampler state (only for the hardware rasterizer currently)
		void ChangeSamplerState() noexcept;
		// When F5 is pressed change to the next shading mode (only for the software rasterizer currently)
		void ChangeShadingMode() noexcept
		{
			auto curr{ static_cast<uint8_t>(m_CurrShadingMode) };
			++curr %= static_cast<uint8_t>(ShadingMode::COUNT);

			m_CurrShadingMode = static_cast<ShadingMode>(curr);

			switch (m_CurrShadingMode)
			{
			case ShadingMode::ObservedArea:
				std::cout << "Shading mode -> ObservedArea\n";
				break;
			case ShadingMode::Diffuse:
				std::cout << "Shading mode -> Diffuse\n";
				break;
			case ShadingMode::Specular:
				std::cout << "Shading mode -> Specular\n";
				break;
			case ShadingMode::Combined:
				std::cout << "Shading mode -> Combined\n";
				break;
			default: break;
			}
		}
		//When F6 is pressed, toggle the normal map (only for the software rasterizer currently)
		void ToggleNormalMap() noexcept
		{
			m_UseNormalMapping = !m_UseNormalMapping;
			if (m_UseNormalMapping)
			{
				std::cout << "Normal mapping -> Enabled\n";
				return;
			}
			std::cout << "Normal mapping -> Disabled\n";
		}
		// When F7 is pressed, toggle the depth buffer visualization (only for the software rasterizer currently)
		void ToggleDisplayDepthBuffer() noexcept
		{
			m_ShowDepthBuffer = !m_ShowDepthBuffer;
			if (m_ShowDepthBuffer)
			{
				std::cout << "Show depth buffer -> Enabled\n";
				return;
			}
			std::cout << "Show depth buffer -> Disabled\n";
		}
		// When F8 is pressed, toggle the bounding box visualization (only for the software rasterizer currently)
		void ToggleBoundingBoxes() noexcept
		{
			m_ShowBoundingBoxes = !m_ShowBoundingBoxes;
			if (m_ShowBoundingBoxes)
			{
				std::cout << "Show bounding boxes -> Enabled\n";
				return;
			}
			std::cout << "Show bounding boxes -> Disabled\n";
		}
		// When F9 is pressed, switch to the next cull mode
		void ChangeCullMode() noexcept
		{
			auto curr{ static_cast<uint8_t>(m_CurrCullMode) };
			++curr %= static_cast<uint8_t>(CullMode::COUNT);

			m_CurrCullMode = static_cast<CullMode>(curr);

			switch (m_CurrCullMode)
			{
			case CullMode::Back:
				std::cout << "Culling mode -> Back\n";
				break;
			case CullMode::Front:
				std::cout << "Culling mode -> Front\n";
				break;
			case CullMode::None:
				std::cout << "Cullings mode -> None\n";
				break;
			default: break;
			}
		}
		// When F10 is pressed, tooggle to display the uniform clear color (or not)
		void ToggleUniformClearColor() noexcept
		{
			m_DisplayUniformClearColor = !m_DisplayUniformClearColor;
			if (m_DisplayUniformClearColor)
			{
				std::cout << "Use uniform clear color -> Enabled\n";
				return;
			}
			std::cout << "Use uniform clear color -> Disabled\n";
		}
	#pragma endregion

	private:
		//Window
		SDL_Window* m_pWindow{};
		int m_Width{};
		int m_Height{};

		Camera m_Camera{};

		//Software rasiterizer
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{ nullptr };

		float* m_pDepthBufferPixels{ nullptr };


		//DirectX
		bool m_IsDirectXInitialized{ false }; // Only want to render when DirectX is properly initialized
		ID3D11Device* m_pDevice{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext{ nullptr };
		IDXGISwapChain* m_pSwapChain{ nullptr };
		ID3D11Texture2D* m_pDepthStencilBuffer{ nullptr };
		ID3D11DepthStencilView* m_pDepthStencilView{ nullptr };
		ID3D11Resource* m_pRenderTargetBuffer{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };

		//Direct X Samplers
		ID3D11SamplerState* m_pPointSampler{ nullptr };
		ID3D11SamplerState* m_pLinearSampler{ nullptr };
		ID3D11SamplerState* m_pAnisotropicSampler{ nullptr };

		//Effects
		// would be shared with resource manager
		std::shared_ptr<BaseEffect> m_pVehicleEffect{ nullptr };
		std::shared_ptr<BaseEffect> m_pFireEffect{ nullptr };

		//Models
		std::vector<std::unique_ptr<Mesh>> m_Meshes;

		//Textures
		// would be in resource manager
		std::unique_ptr<Texture> m_pVehicleDiffuseTexture{ nullptr };
		std::unique_ptr<Texture> m_pVehicleNormalTexture{ nullptr };
		std::unique_ptr<Texture> m_pVehicleGlossinessTexture{ nullptr };
		std::unique_ptr<Texture> m_pVehicleSpecularTexture{ nullptr };
		std::unique_ptr<Texture> m_pFireDiffuseTexture{ nullptr };

		//Settings
		bool m_IsSofwareRasterizerMode{ false };
		bool m_IsRotationMode{ true };
		bool m_DisplayFireMesh{ true };
		enum class SamplerState : uint8_t
		{
			Point = 0,
			Linear = 1,
			Anisotropic = 2,
			COUNT
		};
		SamplerState m_SamplerState{ SamplerState::Point };
		enum class ShadingMode : uint8_t
		{
			ObservedArea = 0,
			Diffuse = 1,
			Specular = 2,
			Combined = 3,
			COUNT
		};
		ShadingMode m_CurrShadingMode{ ShadingMode::Combined };
		bool m_UseNormalMapping{ true };
		bool m_ShowDepthBuffer{ false };
		bool m_ShowBoundingBoxes{ false };
		enum class CullMode : uint8_t
		{
			Back = 0,
			Front = 1,
			None = 2,
			COUNT
		};
		CullMode m_CurrCullMode{ CullMode::Back };
		bool m_DisplayUniformClearColor{ false };

		// Screen clear colors
		float static constexpr UNIFORM_COLOR[4] = { .1f, .1f, .1f, 1.f };
		float static constexpr HARDWARE_COLOR[4] = { .39f, .59f, .93f, 1.f };
		float static constexpr SOFTWARE_COLOR[4] = {.30f, .39f, .39f, 1.f};
		// End settings

		//Hardware
		HRESULT InitializeDirectX();
		void RenderDirectXHardware() const;

		//Software
		void RenderSoftware() const;
		void VertexTransformationFunction(std::vector<Vector2>& screenSpace, Mesh* mesh) const;
		void RenderTriangle(Mesh* m, std::vector<Vector2> const& vertices, uint32_t startVertex, bool swapVertex) const;

		[[nodiscard]] ColorRGB PixelShading(Mesh const* m, Vertex_Out const& v, Vector3 const& viewDir) const;
	};
}
