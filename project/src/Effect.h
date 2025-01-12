#pragma once

#include "pch.h"
#include "Texture.h"
#include "Vertex.h"
#include "Matrix.h"

namespace dae
{
	class BaseEffect
	{
	public:
		BaseEffect(ID3D11Device* pDevice, std::wstring const& assetFile)
		{
			assert(pDevice);
			m_pEffect = LoadEffect(pDevice, assetFile);
			assert(m_pEffect);

			//m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
			m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
			if (!m_pTechnique->IsValid())
			{
				std::wcout << L"Effect::Effect() > GetTechniqueByName() failed" << std::endl;
			}

			m_pWorldViewProjection = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
			if (!m_pWorldViewProjection->IsValid())
			{
				std::wcout << L"m_pWorldViewProjection not valid" << std::endl;
			}
			m_pWorldMatrix = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
			if (!m_pWorldMatrix->IsValid())
			{
				std::wcout << L"m_pWorldMatrix not valid" << std::endl;
			}
			m_pCameraPosition = m_pEffect->GetVariableByName("gCameraPosition")->AsVector();
			if (!m_pCameraPosition->IsValid())
			{
				std::wcout << L"m_pCameraPosition not valid" << std::endl;
			}

			//Textures
			m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
			if (!m_pDiffuseMapVariable->IsValid())
			{
				std::wcout << L"Effect variable 'gDiffuseMap' not valid\n";
			}
		}
		virtual ~BaseEffect()
		{
			SAFE_RELEASE(m_pEffect)
		}

		BaseEffect(const BaseEffect&) = delete;
		BaseEffect(BaseEffect&&) noexcept = delete;
		BaseEffect& operator=(const BaseEffect&) = delete;
		BaseEffect& operator=(BaseEffect&&) noexcept = delete;

		ID3DX11Effect* GetEffect() const
		{
			return m_pEffect;
		}

		ID3DX11EffectTechnique* GetTechnique() const
		{
			return m_pTechnique;
		}

		void SetWorldMatrix(const Matrix& m)
		{
			assert(m_pWorldMatrix);
			m_pWorldMatrix->SetMatrix(reinterpret_cast<const float*>(&m));
		}
		void SetWorldViewProjectionMatrix(const Matrix& m)
		{
			assert(m_pWorldViewProjection);
			m_pWorldViewProjection->SetMatrix(reinterpret_cast<const float*>(&m));
		}
		void SetCameraPosition(const Vector3& pos)
		{
			assert(m_pCameraPosition);
			float vec[3];
			vec[0] = pos.x;
			vec[1] = pos.y;
			vec[2] = pos.z;
			//reinterpret_cast<const float*>(&pos)
			m_pCameraPosition->SetFloatVector(vec);
		}

		//Textures
		void SetDiffuseTexture(Texture* pDiffuseTexture)
		{
			assert(m_pDiffuseMapVariable);
			if (m_pDiffuseMapVariable)
			{
				assert(pDiffuseTexture);
				m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
			}
		}
		virtual void SetGlossinessTexture(Texture*) {}
		virtual void SetSpecularTexture(Texture*) {}
		virtual void SetNormalTexture(Texture*){}

	protected:
		ID3DX11Effect* m_pEffect{ nullptr };
		ID3DX11EffectTechnique* m_pTechnique{ nullptr };

		ID3DX11EffectMatrixVariable* m_pWorldMatrix{ nullptr };
		ID3DX11EffectMatrixVariable* m_pWorldViewProjection{ nullptr };
		ID3DX11EffectVectorVariable* m_pCameraPosition{ nullptr };

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	private: 
		ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, std::wstring const& assetFile)
		{
			HRESULT hr{ S_FALSE };
			ID3D10Blob* pErrorBlob{ nullptr };
			ID3DX11Effect* pEffect{ nullptr };

			DWORD shaderFlags = 0;

			#if defined(DEBUG) || defined(_DEBUG)
				shaderFlags |= D3DCOMPILE_DEBUG;
				shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
			#endif

			hr = D3DX11CompileEffectFromFile(assetFile.c_str(),
				nullptr,
				nullptr,
				shaderFlags,
				0,
				pDevice,
				&pEffect,
				&pErrorBlob);

			if (FAILED(hr))
			{
				if (pErrorBlob != nullptr)
				{
					const char* pErrors = static_cast<const char*>(pErrorBlob->GetBufferPointer());

					std::wstringstream ss;
					for (unsigned i = 0; i < pErrorBlob->GetBufferSize(); ++i)
					{
						ss << pErrors[i];
					}

					OutputDebugStringW(ss.str().c_str());
					pErrorBlob->Release();
					pErrorBlob = nullptr;

					std::wcout << ss.str() << std::endl;
				}
				else
				{
					std::wstringstream ss;
					ss << "EffectLoader: Failed to create effect from file: " << assetFile;
					std::wcout << ss.str() << std::endl;
					return nullptr;
				}
			}
			return pEffect;
		}

	};

	class PixelShadingEffect final : public BaseEffect
	{
	public:
		PixelShadingEffect(ID3D11Device* pDevice, std::wstring const& assetFile):
			BaseEffect{ pDevice, assetFile }
		{
			m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
			if (!m_pNormalMapVariable->IsValid())
			{
				std::wcout << L"Effect variable 'gNormalMap' not valid\n";
			}
			m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
			if (!m_pGlossinessMapVariable->IsValid())
			{
				std::wcout << L"Effect variable 'gGlossinessMap' not valid\n";
			}
			m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
			if (!m_pSpecularMapVariable->IsValid())
			{
				std::wcout << L"Effect variable 'gSpecularMap' not valid\n";
			}
		}
		~PixelShadingEffect()
		{
			SAFE_RELEASE(m_pDiffuseMapVariable)
			SAFE_RELEASE(m_pNormalMapVariable)
			SAFE_RELEASE(m_pSpecularMapVariable)
			SAFE_RELEASE(m_pGlossinessMapVariable)

			BaseEffect::~BaseEffect();
		}

		//Textures
		void SetGlossinessTexture(Texture* pGlossinessTexture) override
		{
			assert(m_pGlossinessMapVariable);
			if (m_pGlossinessMapVariable)
			{
				assert(pGlossinessTexture);
				m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetShaderResourceView());
			}
		}
		void SetSpecularTexture(Texture* pSpecularTexture) override
		{
			assert(m_pSpecularMapVariable);
			if (m_pSpecularMapVariable)
			{
				assert(pSpecularTexture);
				m_pSpecularMapVariable->SetResource(pSpecularTexture->GetShaderResourceView());
			}
		}
		void SetNormalTexture(Texture* pNormalTexture) override
		{
			assert(m_pNormalMapVariable);
			if (m_pNormalMapVariable)
			{
				assert(pNormalTexture);
				m_pNormalMapVariable->SetResource(pNormalTexture->GetShaderResourceView());
			}
		}

		PixelShadingEffect(const PixelShadingEffect&) = delete;
		PixelShadingEffect(PixelShadingEffect&&) noexcept = delete;
		PixelShadingEffect& operator=(const PixelShadingEffect&) = delete;
		PixelShadingEffect& operator=(PixelShadingEffect&&) noexcept = delete;

	private:
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

	};

}