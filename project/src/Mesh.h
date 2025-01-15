#pragma once
#include "pch.h"
#include "Effect.h"
#include "Matrix.h"
#include "Vertex.h"
#include "Utils.h"
#include "Texture.h"


namespace dae
{
	enum class PrimitiveTopology : uint8_t
	{
		TriangleList,
		TriangleStrip
	};

	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice, std::string const& path, std::shared_ptr<BaseEffect> pEffect)
		{
			//Initialize models
			m_Vertices = {};
			m_Indices = {};
			Utils::ParseOBJ(path, m_Vertices, m_Indices);

			m_pEffect = pEffect;
			assert(m_pEffect);

			//Create vertex layout based on vertex struct
			static constexpr uint32_t numElements{ 4 };
			D3D11_INPUT_ELEMENT_DESC layout[numElements]{};

			layout[0].SemanticName = "POSITION";
			layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			layout[0].AlignedByteOffset = 0; // float3
			layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			layout[1].SemanticName = "TEXCOORD";
			layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
			layout[1].AlignedByteOffset = 12; //float2
			layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			layout[2].SemanticName = "NORMAL";
			layout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			layout[2].AlignedByteOffset = 20; //float3
			layout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			layout[3].SemanticName = "TANGENT";
			layout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			layout[3].AlignedByteOffset = 32; //float3
			layout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

			//Create input layout
			ID3DX11EffectTechnique* pTechnique = m_pEffect->GetTechnique();

			D3DX11_PASS_DESC passDesc{};
			pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

			HRESULT hr = pDevice->CreateInputLayout(
				layout,
				numElements,
				passDesc.pIAInputSignature,
				passDesc.IAInputSignatureSize,
				&m_pInputLayout);

			if (FAILED(hr))
				assert(false && "Failed to create input layout");

			//Create vertex buffer
			D3D11_BUFFER_DESC bufferDesc{};
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size());
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA initData{};
			initData.pSysMem = m_Vertices.data();

			hr = pDevice->CreateBuffer(&bufferDesc, &initData, &m_pVertexBuffer);

			if (FAILED(hr))
				assert(false && "Failed to create vertex buffer");

			//Create index buffer
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.ByteWidth = sizeof(uint32_t) * static_cast<uint32_t>(m_Indices.size());
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			initData.pSysMem = m_Indices.data();

			hr = pDevice->CreateBuffer(&bufferDesc, &initData, &m_pIndexBuffer);
			if (FAILED(hr))
				assert(false && "Failed to create index buffer");
		}
		~Mesh()
		{
			SAFE_RELEASE(m_pInputLayout)
				SAFE_RELEASE(m_pVertexBuffer)
				SAFE_RELEASE(m_pIndexBuffer)
		}
		void Render(ID3D11DeviceContext* pDeviceContext) const
		{
			//Set primitive topology
			pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			//Set input layout
			pDeviceContext->IASetInputLayout(m_pInputLayout);

			//Set vertex buffer
			UINT constexpr stride{ sizeof(Vertex) };
			UINT constexpr offset{ 0 };
			pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

			//Set index buffer
			pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			//Draw
			D3DX11_TECHNIQUE_DESC techDesc{};
			m_pEffect->GetTechnique()->GetDesc(&techDesc);
			for (UINT p = 0; p < techDesc.Passes; ++p)
			{
				m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
				pDeviceContext->DrawIndexed(static_cast<uint32_t>(m_Indices.size()), 0, 0);
			}
		}

		// Position
		void UpdateCameraPos(Vector3 const& cameraPos)
		{
			m_pEffect->SetCameraPosition(cameraPos);
		}
		void UpdateEffectMatrices(Matrix const& viewProjectionMatrix)
		{
			m_pEffect->SetWorldViewProjectionMatrix(m_WorldMatrix * viewProjectionMatrix);
			m_pEffect->SetWorldMatrix(m_WorldMatrix);
		}

		// Movement
		void Translate(Vector3 const& t) noexcept
		{
			m_WorldMatrix = Matrix::CreateTranslation(t.x, t.y, t.z) * m_WorldMatrix;
		}
		void RotateY(float r) noexcept
		{
			m_WorldMatrix =  Matrix::CreateRotationY(r) * m_WorldMatrix;
		}

		//Some functions required for software rasterizer
		[[nodiscard]] PrimitiveTopology GetPrimitiveTopology() const noexcept
		{
			return m_PrimitiveTopology;
		}

		[[nodiscard]] std::vector<uint32_t> const& GetIndices() const noexcept
		{
			return m_Indices;
		}

		[[nodiscard]] std::vector<Vertex_Out>& GetVertices_Out_Ref() noexcept
		{
			return m_Vertices_Out;
		}

		[[nodiscard]] std::vector<Vertex> const& GetVertices() const noexcept
		{
			return m_Vertices;
		}

		[[nodiscard]] Matrix const& GetWorldMatrix() const noexcept
		{
			return m_WorldMatrix;
		}

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh&&) = delete;

	private:
		Matrix m_WorldMatrix{};

		//These don't have to be stored for the hardware rasterizer but are necessare for software.
		std::vector<Vertex> m_Vertices{};
		std::vector<Vertex_Out> m_Vertices_Out{};
		std::vector<uint32_t> m_Indices{};
		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleStrip };

		// would be shared with resource manager
		std::shared_ptr<BaseEffect> m_pEffect{};

		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11Buffer* m_pIndexBuffer{};
	};
}