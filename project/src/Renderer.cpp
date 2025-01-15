#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Utils.h"
#include "Effect.h"
#include <execution>

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		m_Camera.Initialize(45.f, {0.f, 0.f, 0.f}, float(m_Width) / float(m_Height));

		//Initialize DirectX pipeline
		if (InitializeDirectX() == S_OK)
		{
			m_IsDirectXInitialized = true;
			std::cout << "DirectX is initialized and ready!" << std::endl;
		}
		else
		{
			std::cout << "DirectX initialization failed!" << std::endl;
		}

		//Intialize textures
		m_pVehicleDiffuseTexture = std::make_unique<Texture>(L"Resources/vehicle_diffuse.png", m_pDevice);
		m_pVehicleGlossinessTexture = std::make_unique<Texture>(L"Resources/vehicle_gloss.png", m_pDevice);
		m_pVehicleNormalTexture = std::make_unique<Texture>(L"Resources/vehicle_normal.png", m_pDevice);
		m_pVehicleSpecularTexture = std::make_unique<Texture>(L"Resources/vehicle_specular.png", m_pDevice);

		m_pFireDiffuseTexture = std::make_unique<Texture>(L"Resources/fireFX_diffuse.png", m_pDevice);


		//Initialize effects
		m_pVehicleEffect = std::make_shared<PixelShadingEffect>(m_pDevice, L"Resources/PosCol3D.fx");
		m_pVehicleEffect->SetDiffuseTexture(m_pVehicleDiffuseTexture.get());
		m_pVehicleEffect->SetGlossinessTexture(m_pVehicleGlossinessTexture.get());
		m_pVehicleEffect->SetNormalTexture(m_pVehicleNormalTexture.get());
		m_pVehicleEffect->SetSpecularTexture(m_pVehicleSpecularTexture.get());

		m_pFireEffect = std::make_shared<BaseEffect>(m_pDevice, L"Resources/PartialCoverage3D.fx");
		m_pFireEffect->SetDiffuseTexture(m_pFireDiffuseTexture.get());

		//Initialize models
		m_Meshes.emplace_back(std::make_unique<Mesh>(m_pDevice, "Resources/vehicle.obj", m_pVehicleEffect));
		m_Meshes.emplace_back(std::make_unique<Mesh>(m_pDevice, "Resources/fireFX.obj", m_pFireEffect));
		m_Meshes[0]->Translate({ 0.f, 0.f, 50.f });
		m_Meshes[1]->Translate({ 0.f, 0.f, 50.f });
	}

	Renderer::~Renderer()
	{
		// Direct X safe release macro (call release if exists)
		SAFE_RELEASE(m_pPointSampler)
		SAFE_RELEASE(m_pLinearSampler)
		SAFE_RELEASE(m_pAnisotropicSampler)


		SAFE_RELEASE(m_pRenderTargetView)
		SAFE_RELEASE(m_pRenderTargetBuffer)

		SAFE_RELEASE(m_pDepthStencilView)
		SAFE_RELEASE(m_pDepthStencilBuffer)

		SAFE_RELEASE(m_pSwapChain)

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		SAFE_RELEASE(m_pDevice)
	}

	void Renderer::ChangeSamplerState() noexcept
	{
		if (!m_IsDirectXInitialized)
		{
			return;
		}

		//Calculate new idx
		uint8_t newId = static_cast<uint8_t>(m_SamplerState);
		++newId %= static_cast<uint8_t>(SamplerState::COUNT);
		m_SamplerState = static_cast<SamplerState>(newId);

		ID3D11SamplerState* newSampler = nullptr;	

		switch (m_SamplerState)
		{
		case SamplerState::Point: 
			newSampler = m_pPointSampler;
			std::cout << "Sampler state -> PointSampler \n";
			break;
		case SamplerState::Linear:
			newSampler = m_pLinearSampler;
			std::cout << "Sampler state -> LinearSampler \n";
			break;
		case SamplerState::Anisotropic:
			newSampler = m_pAnisotropicSampler;
			std::cout << "Sampler state -> AnisotropicSampler \n";
			break;
		default: break;
		}

		if (newSampler)
		{
			m_pDeviceContext->PSSetSamplers(0, 1, &newSampler);
		}
	}

	void Renderer::Update(Timer* pTimer)
	{
		m_Camera.Update(pTimer);

		for (auto& m : m_Meshes)
		{
			if (m_IsRotationMode)
			{
				m->RotateY(TO_RADIANS*(45.f * pTimer->GetElapsed()));
			}
			m->UpdateEffectMatrices(m_Camera.viewMatrix * m_Camera.projectionMatrix);
			m->UpdateCameraPos(m_Camera.origin);
		}
	}

	void Renderer::Render() const
	{
		//if (m_IsSofwareRasterizerMode)
		//{
		//	RenderSoftware();
		//	return;
		//}
		RenderDirectXHardware();
	}

	void Renderer::RenderDirectXHardware() const
	{
		if (!m_IsDirectXInitialized)
			return;

		// Clear RTV & DSV
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, m_DisplayUniformClearColor ? UNIFORM_COLOR : HARDWARE_COLOR);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Set pipeline + invoke drawcalls
		for (auto& m : m_Meshes)
		{
			if (!m_DisplayFireMesh && m_Meshes.size() > 1 && m == m_Meshes[1]) // Hard coded to array idx 1 currently since it's just a demo
			{
				continue;
			}
			m->Render(m_pDeviceContext);
		}

		// present backbuffer (swap)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::RenderSoftware() const
	{
		//@START
	//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);

		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
		std::fill_n(m_pBackBufferPixels, m_Width * m_Height, 0);

		//clear the background
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

		for (auto & m : m_Meshes)
		{
			//Meshes defined in world space before the transform function,
			std::vector<Vector2> vertices_screenSpace{};
			//convert each NDC coordinates to screen space / raster space
			VertexTransformationFunction(vertices_screenSpace, m.get());

			switch (m->GetPrimitiveTopology())
			{
			case PrimitiveTopology::TriangleList:
				// Use parallel execution for triangle list
				std::for_each(
					std::execution::par_unseq,  // threading execution policy - threading would be slightly better using a "custom" system but for this demo it is sufficient
					m->GetIndices().begin(), m->GetIndices().end(),
					[this, &m, &vertices_screenSpace](uint32_t v)
					{
						if (v % 3 == 0)
						{  // Only process every 3rd index - threading would be slightly better using a "custom" system but for this demo it is sufficient
							RenderTriangle(m.get(), vertices_screenSpace, v, false);
						}
					});
				break;
			case PrimitiveTopology::TriangleStrip:
				std::for_each(
					std::execution::par_unseq,  // Parallel execution policy
					m->GetIndices().begin(), m->GetIndices().end() - 2,
					[this, &m, &vertices_screenSpace](uint32_t v)
					{
						RenderTriangle(m.get(), vertices_screenSpace, v, v % 2);
					});
				break;
			default:
				break;
			}
		}

		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void Renderer::VertexTransformationFunction(std::vector<Vector2>& screenSpace, Mesh* mesh) const
	{
		//projection stage:
		//model -> world space -> world -> view space 
		auto const m{ mesh->GetWorldMatrix() * m_Camera.viewMatrix * m_Camera.projectionMatrix };

		// Prepare the output container
		screenSpace.resize(mesh->GetVertices().size());
		mesh->GetVertices_Out_Ref().resize(mesh->GetVertices().size());

		// Transform vertices in parallel
		std::transform(
			std::execution::par,  // Parallel execution policy
			mesh->GetVertices().begin(), mesh->GetVertices().end(),
			mesh->GetVertices_Out_Ref().begin(),
			[&](auto const& v) {
				Vertex_Out vOut{};
				//vOut.color = v.color;
				vOut.texcoord = v.texcoord;

				vOut.position = m.TransformPoint(v.position.ToPoint4());

				vOut.normal = mesh->GetWorldMatrix().TransformVector(v.normal);
				vOut.tangent = mesh->GetWorldMatrix().TransformVector(v.tangent);

				// View -> clipping space (NDC)
				float const inverseWComponent{ 1.f / vOut.position.w };
				vOut.position.x *= inverseWComponent;
				vOut.position.y *= inverseWComponent;
				vOut.position.z *= inverseWComponent;

				// Convert to screen space (raster space)
				float const x_screen{ (vOut.position.x + 1) * 0.5f * static_cast<float>(m_Width) }; //center of pixel
				float const y_screen{ (1 - vOut.position.y) * 0.5f * static_cast<float>(m_Height) };

				// Store screen space coordinates
				auto const idx{ &v - mesh->GetVertices().data() }; // Calculate correct idx
				screenSpace[idx] = { x_screen, y_screen };

				return vOut;
			});
	}

	void Renderer::RenderTriangle(Mesh const* m, std::vector<Vector2> const& vertices, uint32_t startVertex, bool swapVertex) const
	{

	}

	HRESULT Renderer::InitializeDirectX()
	{
		//Create device and device context
		D3D_FEATURE_LEVEL constexpr featureLevel{ D3D_FEATURE_LEVEL_11_1 };
		uint32_t createDeviceFlags{ 0 };

		#if defined(DEBUG) ||defined(_DEBUG)
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		// https://learn.microsoft.com/en-us/windows/win32/seccrypto/common-hresult-values
		HRESULT result = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			&featureLevel,
			1,
			D3D11_SDK_VERSION,
			&m_pDevice,
			nullptr,
			&m_pDeviceContext
		);

		if (FAILED(result))
		{
			return result;
		}

		// Create DXGI Factory
		IDXGIFactory1* pFactory{ };
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pFactory));
		if (FAILED(result))
		{
			return result;
		}

		// Create swap chain | "https://learn.microsoft.com/en-us/windows/win32/direct3d9/what-is-a-swap-chain-"
		// https://learn.microsoft.com/en-us/windows/win32/api/dxgi/ns-dxgi-dxgi_swap_chain_desc
		// Swap chain description
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get Hanle from the SDL backbuffer
		SDL_SysWMinfo wmInfo{};
		SDL_GetVersion(&wmInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &wmInfo);
		swapChainDesc.OutputWindow = wmInfo.info.win.window;

		// Create swapchain
		result = pFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
		{
			return result;
		}

		// Create Depth Buffer - DepthStencil & DepthStencilView
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
		{
			return result;
		}

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
		{
			return result;
		}

		// Create Render Target View
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
		{
			return result;
		}

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
		{
			return result;
		}

		//Bind the views to the pipeline
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//Set viewport
		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<float>(m_Width);
		vp.Height = static_cast<float>(m_Height);
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		m_pDeviceContext->RSSetViewports(1, &vp);


		//Initialize sampler states
		// Point sampler
		D3D11_SAMPLER_DESC pointDesc = {};
		pointDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		pointDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		pointDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		pointDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		pointDesc.MinLOD = 0.0f;
		pointDesc.MaxLOD = D3D11_FLOAT32_MAX;
		result = m_pDevice->CreateSamplerState(&pointDesc, &m_pPointSampler);
		if (FAILED(result))
		{
			return result;
		}

		// Linear sampler
		D3D11_SAMPLER_DESC linearDesc = pointDesc; // Copy base settings
		linearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		result = m_pDevice->CreateSamplerState(&linearDesc, &m_pLinearSampler);
		if (FAILED(result))
		{
			return result;
		}

		// Anisotropic sampler
		D3D11_SAMPLER_DESC anisotropicDesc = pointDesc; // Copy base settings
		anisotropicDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		anisotropicDesc.MaxAnisotropy = 16;
		result = m_pDevice->CreateSamplerState(&anisotropicDesc, &m_pAnisotropicSampler);
		if (FAILED(result))
		{
			return result;
		}

		m_pDeviceContext->PSSetSamplers(0, 1, &m_pPointSampler);

		return result;
	}
}
