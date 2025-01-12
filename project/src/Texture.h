#pragma once

#include "pch.h"
#include "ColorRGB.h"
#include "Vector2.h"
#include <filesystem>

namespace dae
{
	class Texture final
	{
	public:
		Texture(std::filesystem::path const& path, ID3D11Device* pDevice)
		{
			assert(std::filesystem::exists(path));
			assert(pDevice);

			auto pSurface = IMG_Load(path.string().c_str());
			if (!pSurface)
				throw std::runtime_error("Failed to load texture from path: " + path.string());


			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = pSurface->w;
			desc.Height = pSurface->h;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = format;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA initData{};
			initData.pSysMem = pSurface->pixels;
			initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
			initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);
			
			HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
			
			if (FAILED(hr))
				throw std::runtime_error("Failed to create texture from path: " + path.string());
		
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;

			hr = pDevice->CreateShaderResourceView(m_pResource, &srvDesc, &m_pShaderResourceView);
			if (FAILED(hr))
				throw std::runtime_error("Failed to create shader resource view from path: " + path.string());
			
			SDL_FreeSurface(pSurface);
		}
		~Texture()
		{
			SAFE_RELEASE(m_pShaderResourceView)
			SAFE_RELEASE(m_pResource)
		}

		ID3D11Texture2D* GetResource() const
		{
			return m_pResource;
		}
		ID3D11ShaderResourceView* GetShaderResourceView() const
		{
			return m_pShaderResourceView;
		}

		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept = delete;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) noexcept = delete;

		//ColorRGB Sample(const Vector2& uv) const
		//{
			//D3D11_TEXTURE2D_DESC desc{};
			//m_pResource->GetDesc(&desc);

			//D3D11_MAPPED_SUBRESOURCE mappedResource{};
			//m_pResource->Map(0, D3D11_MAP_READ, 0, &mappedResource);

			//const UINT x = static_cast<UINT>(uv.x * desc.Width);
			//const UINT y = static_cast<UINT>(uv.y * desc.Height);

			//const UINT index = y * mappedResource.RowPitch + x * 4;
			//const BYTE* pPixel = static_cast<BYTE*>(mappedResource.pData) + index;

			//ColorRGB color{};
			//color.r = pPixel[0] / 255.f;
			//color.g = pPixel[1] / 255.f;
			//color.b = pPixel[2] / 255.f;

			//m_pResource->Unmap(0);

			//return color;

		// }
		

	private:
		ID3D11Texture2D* m_pResource{};
		ID3D11ShaderResourceView* m_pShaderResourceView{};
	};
}

