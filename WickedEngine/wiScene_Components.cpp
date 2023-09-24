#include "wiScene_Components.h"
#include "wiTextureHelper.h"
#include "wiResourceManager.h"
#include "wiPhysics.h"
#include "wiRenderer.h"
#include "wiJobSystem.h"
#include "wiSpinLock.h"
#include "wiHelper.h"
#include "wiRenderer.h"
#include "wiBacklog.h"
#include "wiTimer.h"
#include "wiUnorderedMap.h"
#include "wiLua.h"

#if __has_include("OpenImageDenoise/oidn.hpp")
#include "OpenImageDenoise/oidn.hpp"
#if OIDN_VERSION_MAJOR >= 2
#define OPEN_IMAGE_DENOISE
#pragma comment(lib,"OpenImageDenoise.lib")
// Also provide the required DLL files from OpenImageDenoise release near the exe!
#endif // OIDN_VERSION_MAJOR >= 2
#endif // __has_include("OpenImageDenoise/oidn.hpp")

using namespace wi::ecs;
using namespace wi::enums;
using namespace wi::graphics;
using namespace wi::primitive;

namespace wi::scene
{


	XMFLOAT3 TransformComponent::GetPosition() const
	{
		return *((XMFLOAT3*)&world._41);
	}
	XMFLOAT4 TransformComponent::GetRotation() const
	{
		XMFLOAT4 rotation;
		XMStoreFloat4(&rotation, GetRotationV());
		return rotation;
	}
	XMFLOAT3 TransformComponent::GetScale() const
	{
		XMFLOAT3 scale;
		XMStoreFloat3(&scale, GetScaleV());
		return scale;
	}
	XMVECTOR TransformComponent::GetPositionV() const
	{
		return XMLoadFloat3((XMFLOAT3*)&world._41);
	}
	XMVECTOR TransformComponent::GetRotationV() const
	{
		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, XMLoadFloat4x4(&world));
		return R;
	}
	XMVECTOR TransformComponent::GetScaleV() const
	{
		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, XMLoadFloat4x4(&world));
		return S;
	}
	XMMATRIX TransformComponent::GetLocalMatrix() const
	{
		XMVECTOR S_local = XMLoadFloat3(&scale_local);
		XMVECTOR R_local = XMLoadFloat4(&rotation_local);
		XMVECTOR T_local = XMLoadFloat3(&translation_local);
		return
			XMMatrixScalingFromVector(S_local) *
			XMMatrixRotationQuaternion(R_local) *
			XMMatrixTranslationFromVector(T_local);
	}
	void TransformComponent::UpdateTransform()
	{
		if (IsDirty())
		{
			SetDirty(false);

			XMStoreFloat4x4(&world, GetLocalMatrix());
		}
	}
	void TransformComponent::UpdateTransform_Parented(const TransformComponent& parent)
	{
		XMMATRIX W = GetLocalMatrix();
		XMMATRIX W_parent = XMLoadFloat4x4(&parent.world);
		W = W * W_parent;

		XMStoreFloat4x4(&world, W);
	}
	void TransformComponent::ApplyTransform()
	{
		SetDirty();

		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, XMLoadFloat4x4(&world));
		XMStoreFloat3(&scale_local, S);
		XMStoreFloat4(&rotation_local, R);
		XMStoreFloat3(&translation_local, T);
	}
	void TransformComponent::ClearTransform()
	{
		SetDirty();
		scale_local = XMFLOAT3(1, 1, 1);
		rotation_local = XMFLOAT4(0, 0, 0, 1);
		translation_local = XMFLOAT3(0, 0, 0);
	}
	void TransformComponent::Translate(const XMFLOAT3& value)
	{
		SetDirty();
		translation_local.x += value.x;
		translation_local.y += value.y;
		translation_local.z += value.z;
	}
	void TransformComponent::Translate(const XMVECTOR& value)
	{
		XMFLOAT3 translation;
		XMStoreFloat3(&translation, value);
		Translate(translation);
	}
	void TransformComponent::RotateRollPitchYaw(const XMFLOAT3& value)
	{
		SetDirty();

		// This needs to be handled a bit differently
		XMVECTOR quat = XMLoadFloat4(&rotation_local);
		XMVECTOR x = XMQuaternionRotationRollPitchYaw(value.x, 0, 0);
		XMVECTOR y = XMQuaternionRotationRollPitchYaw(0, value.y, 0);
		XMVECTOR z = XMQuaternionRotationRollPitchYaw(0, 0, value.z);

		quat = XMQuaternionMultiply(x, quat);
		quat = XMQuaternionMultiply(quat, y);
		quat = XMQuaternionMultiply(z, quat);
		quat = XMQuaternionNormalize(quat);

		XMStoreFloat4(&rotation_local, quat);
	}
	void TransformComponent::Rotate(const XMFLOAT4& quaternion)
	{
		SetDirty();

		XMVECTOR result = XMQuaternionMultiply(XMLoadFloat4(&rotation_local), XMLoadFloat4(&quaternion));
		result = XMQuaternionNormalize(result);
		XMStoreFloat4(&rotation_local, result);
	}
	void TransformComponent::Rotate(const XMVECTOR& quaternion)
	{
		XMFLOAT4 rotation;
		XMStoreFloat4(&rotation, quaternion);
		Rotate(rotation);
	}
	void TransformComponent::Scale(const XMFLOAT3& value)
	{
		SetDirty();
		scale_local.x *= value.x;
		scale_local.y *= value.y;
		scale_local.z *= value.z;
	}
	void TransformComponent::Scale(const XMVECTOR& value)
	{
		XMFLOAT3 scale;
		XMStoreFloat3(&scale, value);
		Scale(scale);
	}
	void TransformComponent::MatrixTransform(const XMFLOAT4X4& matrix)
	{
		MatrixTransform(XMLoadFloat4x4(&matrix));
	}
	void TransformComponent::MatrixTransform(const XMMATRIX& matrix)
	{
		SetDirty();

		XMVECTOR S;
		XMVECTOR R;
		XMVECTOR T;
		XMMatrixDecompose(&S, &R, &T, GetLocalMatrix() * matrix);

		XMStoreFloat3(&scale_local, S);
		XMStoreFloat4(&rotation_local, R);
		XMStoreFloat3(&translation_local, T);
	}
	void TransformComponent::Lerp(const TransformComponent& a, const TransformComponent& b, float t)
	{
		SetDirty();

		XMVECTOR aS, aR, aT;
		XMMatrixDecompose(&aS, &aR, &aT, XMLoadFloat4x4(&a.world));

		XMVECTOR bS, bR, bT;
		XMMatrixDecompose(&bS, &bR, &bT, XMLoadFloat4x4(&b.world));

		XMVECTOR S = XMVectorLerp(aS, bS, t);
		XMVECTOR R = XMQuaternionSlerp(aR, bR, t);
		XMVECTOR T = XMVectorLerp(aT, bT, t);

		XMStoreFloat3(&scale_local, S);
		XMStoreFloat4(&rotation_local, R);
		XMStoreFloat3(&translation_local, T);
	}
	void TransformComponent::CatmullRom(const TransformComponent& a, const TransformComponent& b, const TransformComponent& c, const TransformComponent& d, float t)
	{
		SetDirty();

		XMVECTOR aS, aR, aT;
		XMMatrixDecompose(&aS, &aR, &aT, XMLoadFloat4x4(&a.world));

		XMVECTOR bS, bR, bT;
		XMMatrixDecompose(&bS, &bR, &bT, XMLoadFloat4x4(&b.world));

		XMVECTOR cS, cR, cT;
		XMMatrixDecompose(&cS, &cR, &cT, XMLoadFloat4x4(&c.world));

		XMVECTOR dS, dR, dT;
		XMMatrixDecompose(&dS, &dR, &dT, XMLoadFloat4x4(&d.world));

		XMVECTOR T = XMVectorCatmullRom(aT, bT, cT, dT, t);

		XMVECTOR setupA;
		XMVECTOR setupB;
		XMVECTOR setupC;

		aR = XMQuaternionNormalize(aR);
		bR = XMQuaternionNormalize(bR);
		cR = XMQuaternionNormalize(cR);
		dR = XMQuaternionNormalize(dR);

		XMQuaternionSquadSetup(&setupA, &setupB, &setupC, aR, bR, cR, dR);
		XMVECTOR R = XMQuaternionSquad(bR, setupA, setupB, setupC, t);

		XMVECTOR S = XMVectorCatmullRom(aS, bS, cS, dS, t);

		XMStoreFloat3(&translation_local, T);
		XMStoreFloat4(&rotation_local, R);
		XMStoreFloat3(&scale_local, S);
	}

	void MaterialComponent::WriteShaderMaterial(ShaderMaterial* dest) const
	{
		ShaderMaterial material;
		material.baseColor = baseColor;
		material.emissive_r11g11b10 = wi::math::Pack_R11G11B10_FLOAT(XMFLOAT3(emissiveColor.x * emissiveColor.w, emissiveColor.y * emissiveColor.w, emissiveColor.z * emissiveColor.w));
		material.specular_r11g11b10 = wi::math::Pack_R11G11B10_FLOAT(XMFLOAT3(specularColor.x * specularColor.w, specularColor.y * specularColor.w, specularColor.z * specularColor.w));
		material.texMulAdd = texMulAdd;
		material.roughness = roughness;
		material.reflectance = reflectance;
		material.metalness = metalness;
		material.refraction = refraction;
		material.normalMapStrength = (textures[NORMALMAP].resource.IsValid() ? normalMapStrength : 0);
		material.parallaxOcclusionMapping = parallaxOcclusionMapping;
		material.displacementMapping = displacementMapping;
		XMFLOAT4 sss = subsurfaceScattering;
		sss.x *= sss.w;
		sss.y *= sss.w;
		sss.z *= sss.w;
		XMFLOAT4 sss_inv = XMFLOAT4(
			sss_inv.x = 1.0f / ((1 + sss.x) * (1 + sss.x)),
			sss_inv.y = 1.0f / ((1 + sss.y) * (1 + sss.y)),
			sss_inv.z = 1.0f / ((1 + sss.z) * (1 + sss.z)),
			sss_inv.w = 1.0f / ((1 + sss.w) * (1 + sss.w))
		);
		material.subsurfaceScattering = sss;
		material.subsurfaceScattering_inv = sss_inv;

		material.sheenColor_r11g11b10 = wi::math::Pack_R11G11B10_FLOAT(XMFLOAT3(sheenColor.x, sheenColor.y, sheenColor.z));
		material.sheenRoughness = sheenRoughness;
		material.clearcoat = clearcoat;
		material.clearcoatRoughness = clearcoatRoughness;
		material.alphaTest = 1 - alphaRef;
		material.layerMask = layerMask;
		material.transmission = transmission;
		if (shaderType == SHADERTYPE_PBR_ANISOTROPIC)
		{
			material.anisotropy_strength = wi::math::Clamp(anisotropy_strength, 0, 0.99f);
			material.anisotropy_rotation_sin = std::sin(anisotropy_rotation);
			material.anisotropy_rotation_cos = std::cos(anisotropy_rotation);
		}
		else
		{
			material.anisotropy_strength = 0;
			material.anisotropy_rotation_sin = 0;
			material.anisotropy_rotation_cos = 0;
		}
		material.stencilRef = wi::renderer::CombineStencilrefs(engineStencilRef, userStencilRef);
		material.shaderType = (uint)shaderType;
		material.userdata = userdata;

		material.options = 0;
		if (IsUsingVertexColors())
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_USE_VERTEXCOLORS;
		}
		if (IsUsingSpecularGlossinessWorkflow())
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_SPECULARGLOSSINESS_WORKFLOW;
		}
		if (IsOcclusionEnabled_Primary())
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_OCCLUSION_PRIMARY;
		}
		if (IsOcclusionEnabled_Secondary())
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_OCCLUSION_SECONDARY;
		}
		if (IsUsingWind())
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_USE_WIND;
		}
		if (IsReceiveShadow())
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_RECEIVE_SHADOW;
		}
		if (IsCastingShadow())
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_CAST_SHADOW;
		}
		if (IsDoubleSided())
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_DOUBLE_SIDED;
		}
		if (GetFilterMask() & FILTER_TRANSPARENT)
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_TRANSPARENT;
		}
		if (userBlendMode == BLENDMODE_ADDITIVE)
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_ADDITIVE;
		}
		if (shaderType == SHADERTYPE_UNLIT)
		{
			material.options |= SHADERMATERIAL_OPTION_BIT_UNLIT;
		}

		GraphicsDevice* device = wi::graphics::GetDevice();
		for (int i = 0; i < TEXTURESLOT_COUNT; ++i)
		{
			material.textures[i].uvset_lodclamp = (textures[i].uvset & 1) | (XMConvertFloatToHalf(textures[i].lod_clamp) << 1u);
			if (textures[i].resource.IsValid())
			{
				int subresource = -1;
				switch (i)
				{
				case BASECOLORMAP:
				case EMISSIVEMAP:
				case SPECULARMAP:
				case SHEENCOLORMAP:
					subresource = textures[i].resource.GetTextureSRGBSubresource();
					break;
				case SURFACEMAP:
					if (IsUsingSpecularGlossinessWorkflow())
					{
						subresource = textures[i].resource.GetTextureSRGBSubresource();
					}
					break;
				default:
					break;
				}
				material.textures[i].texture_descriptor = device->GetDescriptorIndex(textures[i].GetGPUResource(), SubresourceType::SRV, subresource);
			}
			else
			{
				material.textures[i].texture_descriptor = -1;
			}
			material.textures[i].sparse_residencymap_descriptor = textures[i].sparse_residencymap_descriptor;
			material.textures[i].sparse_feedbackmap_descriptor = textures[i].sparse_feedbackmap_descriptor;
		}

		if (sampler_descriptor < 0)
		{
			material.sampler_descriptor = device->GetDescriptorIndex(wi::renderer::GetSampler(wi::enums::SAMPLER_OBJECTSHADER));
		}
		else
		{
			material.sampler_descriptor = sampler_descriptor;
		}

		std::memcpy(dest, &material, sizeof(ShaderMaterial)); // memcpy whole structure into mapped pointer to avoid read from uncached memory
	}
	void MaterialComponent::WriteShaderTextureSlot(ShaderMaterial* dest, int slot, int descriptor)
	{
		std::memcpy(&dest->textures[slot].texture_descriptor, &descriptor, sizeof(descriptor)); // memcpy into mapped pointer to avoid read from uncached memory
	}
	void MaterialComponent::WriteTextures(const wi::graphics::GPUResource** dest, int count) const
	{
		count = std::min(count, (int)TEXTURESLOT_COUNT);
		for (int i = 0; i < count; ++i)
		{
			dest[i] = textures[i].GetGPUResource();
		}
	}
	uint32_t MaterialComponent::GetFilterMask() const
	{
		if (IsCustomShader() && customShaderID < (int)wi::renderer::GetCustomShaders().size())
		{
			auto& customShader = wi::renderer::GetCustomShaders()[customShaderID];
			return customShader.filterMask;
		}
		if (shaderType == SHADERTYPE_WATER)
		{
			return FILTER_TRANSPARENT | FILTER_WATER;
		}
		if (transmission > 0)
		{
			return FILTER_TRANSPARENT;
		}
		if (userBlendMode == BLENDMODE_OPAQUE)
		{
			return FILTER_OPAQUE;
		}
		return FILTER_TRANSPARENT;
	}
	wi::resourcemanager::Flags MaterialComponent::GetTextureSlotResourceFlags(TEXTURESLOT slot)
	{
		wi::resourcemanager::Flags flags = wi::resourcemanager::Flags::IMPORT_RETAIN_FILEDATA;
		if (!IsPreferUncompressedTexturesEnabled())
		{
			flags |= wi::resourcemanager::Flags::IMPORT_BLOCK_COMPRESSED;
		}
		switch (slot)
		{
		case NORMALMAP:
		case CLEARCOATNORMALMAP:
			flags |= wi::resourcemanager::Flags::IMPORT_NORMALMAP;
			break;
		default:
			break;
		}
		return flags;
	}
	void MaterialComponent::CreateRenderData(bool force_recreate)
	{
		if (force_recreate)
		{
			for (uint32_t slot = 0; slot < TEXTURESLOT_COUNT; ++slot)
			{
				auto& textureslot = textures[slot];
				if (textureslot.resource.IsValid())
				{
					textureslot.resource.SetOutdated();
				}
			}
		}
		for (uint32_t slot = 0; slot < TEXTURESLOT_COUNT; ++slot)
		{
			auto& textureslot = textures[slot];
			if (!textureslot.name.empty())
			{
				wi::resourcemanager::Flags flags = GetTextureSlotResourceFlags(TEXTURESLOT(slot));
				textureslot.resource = wi::resourcemanager::Load(textureslot.name, flags);
			}
		}
	}
	uint32_t MaterialComponent::GetStencilRef() const
	{
		return wi::renderer::CombineStencilrefs(engineStencilRef, userStencilRef);
	}

	void MeshComponent::DeleteRenderData()
	{
		generalBuffer = {};
		streamoutBuffer = {};
		ib = {};
		vb_pos_nor_wind = {};
		vb_tan = {};
		vb_uvs = {};
		vb_atl = {};
		vb_col = {};
		vb_bon = {};
		so_pos_nor_wind = {};
		so_tan = {};
		so_pre = {};
		BLASes.clear();
		for (MorphTarget& morph : morph_targets)
		{
			morph.offset_pos = ~0ull;
			morph.offset_nor = ~0ull;
		}
	}
	void MeshComponent::CreateRenderData()
	{
		DeleteRenderData();

		GraphicsDevice* device = wi::graphics::GetDevice();

		if (vertex_tangents.empty() && !vertex_uvset_0.empty() && !vertex_normals.empty())
		{
			// Generate tangents if not found:
			vertex_tangents.resize(vertex_positions.size());

			uint32_t first_subset = 0;
			uint32_t last_subset = 0;
			GetLODSubsetRange(0, first_subset, last_subset);
			for (uint32_t subsetIndex = first_subset; subsetIndex < last_subset; ++subsetIndex)
			{
				const MeshComponent::MeshSubset& subset = subsets[subsetIndex];
				for (size_t i = 0; i < subset.indexCount; i += 3)
				{
					const uint32_t i0 = indices[subset.indexOffset + i + 0];
					const uint32_t i1 = indices[subset.indexOffset + i + 1];
					const uint32_t i2 = indices[subset.indexOffset + i + 2];

					const XMFLOAT3 v0 = vertex_positions[i0];
					const XMFLOAT3 v1 = vertex_positions[i1];
					const XMFLOAT3 v2 = vertex_positions[i2];

					const XMFLOAT2 u0 = vertex_uvset_0[i0];
					const XMFLOAT2 u1 = vertex_uvset_0[i1];
					const XMFLOAT2 u2 = vertex_uvset_0[i2];

					const XMFLOAT3 n0 = vertex_normals[i0];
					const XMFLOAT3 n1 = vertex_normals[i1];
					const XMFLOAT3 n2 = vertex_normals[i2];

					const XMVECTOR nor0 = XMLoadFloat3(&n0);
					const XMVECTOR nor1 = XMLoadFloat3(&n1);
					const XMVECTOR nor2 = XMLoadFloat3(&n2);

					const XMVECTOR facenormal = XMVector3Normalize(nor0 + nor1 + nor2);

					const float x1 = v1.x - v0.x;
					const float x2 = v2.x - v0.x;
					const float y1 = v1.y - v0.y;
					const float y2 = v2.y - v0.y;
					const float z1 = v1.z - v0.z;
					const float z2 = v2.z - v0.z;

					const float s1 = u1.x - u0.x;
					const float s2 = u2.x - u0.x;
					const float t1 = u1.y - u0.y;
					const float t2 = u2.y - u0.y;

					const float r = 1.0f / (s1 * t2 - s2 * t1);
					const XMVECTOR sdir = XMVectorSet((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
						(t2 * z1 - t1 * z2) * r, 0);
					const XMVECTOR tdir = XMVectorSet((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
						(s1 * z2 - s2 * z1) * r, 0);

					XMVECTOR tangent;
					tangent = XMVector3Normalize(sdir - facenormal * XMVector3Dot(facenormal, sdir));
					float sign = XMVectorGetX(XMVector3Dot(XMVector3Cross(tangent, facenormal), tdir)) < 0.0f ? -1.0f : 1.0f;

					XMFLOAT3 t;
					XMStoreFloat3(&t, tangent);

					vertex_tangents[i0].x += t.x;
					vertex_tangents[i0].y += t.y;
					vertex_tangents[i0].z += t.z;
					vertex_tangents[i0].w = sign;

					vertex_tangents[i1].x += t.x;
					vertex_tangents[i1].y += t.y;
					vertex_tangents[i1].z += t.z;
					vertex_tangents[i1].w = sign;

					vertex_tangents[i2].x += t.x;
					vertex_tangents[i2].y += t.y;
					vertex_tangents[i2].z += t.z;
					vertex_tangents[i2].w = sign;
				}
			}
		}

		const size_t uv_count = std::max(vertex_uvset_0.size(), vertex_uvset_1.size());

		GPUBufferDesc bd;
		if (device->CheckCapability(GraphicsDeviceCapability::CACHE_COHERENT_UMA))
		{
			// In UMA mode, it is better to create UPLOAD buffer, this avoids one copy from UPLOAD to DEFAULT
			bd.usage = Usage::UPLOAD;
		}
		else
		{
			bd.usage = Usage::DEFAULT;
		}
		bd.bind_flags = BindFlag::VERTEX_BUFFER | BindFlag::INDEX_BUFFER | BindFlag::SHADER_RESOURCE;
		bd.misc_flags = ResourceMiscFlag::BUFFER_RAW | ResourceMiscFlag::TYPED_FORMAT_CASTING | ResourceMiscFlag::NO_DEFAULT_DESCRIPTORS;
		if (device->CheckCapability(GraphicsDeviceCapability::RAYTRACING))
		{
			bd.misc_flags |= ResourceMiscFlag::RAY_TRACING;
		}
		const uint64_t alignment = device->GetMinOffsetAlignment(&bd);
		bd.size =
			AlignTo(indices.size() * GetIndexStride(), alignment) +
			AlignTo(vertex_positions.size() * sizeof(Vertex_POS), alignment) +
			AlignTo(vertex_tangents.size() * sizeof(Vertex_TAN), alignment) +
			AlignTo(uv_count * sizeof(Vertex_UVS), alignment) +
			AlignTo(vertex_atlas.size() * sizeof(Vertex_TEX), alignment) +
			AlignTo(vertex_colors.size() * sizeof(Vertex_COL), alignment) +
			AlignTo(vertex_boneindices.size() * sizeof(Vertex_BON), alignment)
			;

		constexpr Format morph_format = Format::R16G16B16A16_FLOAT;
		constexpr size_t morph_stride = GetFormatStride(morph_format);
		for (MorphTarget& morph : morph_targets)
		{
			if (!morph.vertex_positions.empty())
			{
				bd.size += AlignTo(vertex_positions.size() * morph_stride, alignment);
			}
			if (!morph.vertex_normals.empty())
			{
				bd.size += AlignTo(vertex_normals.size() * morph_stride, alignment);
			}
		}

		auto init_callback = [&](void* dest) {
			uint8_t* buffer_data = (uint8_t*)dest;
			uint64_t buffer_offset = 0ull;

			// Create index buffer GPU data:
			if (GetIndexFormat() == IndexBufferFormat::UINT32)
			{
				ib.offset = buffer_offset;
				ib.size = indices.size() * sizeof(uint32_t);
				uint32_t* indexdata = (uint32_t*)(buffer_data + buffer_offset);
				buffer_offset += AlignTo(ib.size, alignment);
				std::memcpy(indexdata, indices.data(), ib.size);
			}
			else
			{
				ib.offset = buffer_offset;
				ib.size = indices.size() * sizeof(uint16_t);
				uint16_t* indexdata = (uint16_t*)(buffer_data + buffer_offset);
				buffer_offset += AlignTo(ib.size, alignment);
				for (size_t i = 0; i < indices.size(); ++i)
				{
					std::memcpy(indexdata + i, &indices[i], sizeof(uint16_t));
				}
			}

			XMFLOAT3 _min = XMFLOAT3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			XMFLOAT3 _max = XMFLOAT3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

			// vertexBuffer - POSITION + NORMAL + WIND:
			{
				vb_pos_nor_wind.offset = buffer_offset;
				vb_pos_nor_wind.size = vertex_positions.size() * sizeof(Vertex_POS);
				Vertex_POS* vertices = (Vertex_POS*)(buffer_data + buffer_offset);
				buffer_offset += AlignTo(vb_pos_nor_wind.size, alignment);
				for (size_t i = 0; i < vertex_positions.size(); ++i)
				{
					const XMFLOAT3& pos = vertex_positions[i];
					XMFLOAT3 nor = vertex_normals.empty() ? XMFLOAT3(1, 1, 1) : vertex_normals[i];
					XMStoreFloat3(&nor, XMVector3Normalize(XMLoadFloat3(&nor)));
					const uint8_t wind = vertex_windweights.empty() ? 0xFF : vertex_windweights[i];
					Vertex_POS vert;
					vert.FromFULL(pos, nor, wind);
					std::memcpy(vertices + i, &vert, sizeof(vert));

					_min = wi::math::Min(_min, pos);
					_max = wi::math::Max(_max, pos);
				}
			}

			aabb = AABB(_min, _max);

			// vertexBuffer - TANGENTS
			if (!vertex_tangents.empty())
			{
				vb_tan.offset = buffer_offset;
				vb_tan.size = vertex_tangents.size() * sizeof(Vertex_TAN);
				Vertex_TAN* vertices = (Vertex_TAN*)(buffer_data + buffer_offset);
				buffer_offset += AlignTo(vb_tan.size, alignment);
				for (size_t i = 0; i < vertex_tangents.size(); ++i)
				{
					Vertex_TAN vert;
					vert.FromFULL(vertex_tangents[i]);
					std::memcpy(vertices + i, &vert, sizeof(vert));
				}
			}

			// vertexBuffer - UV SETS
			if (!vertex_uvset_0.empty() || !vertex_uvset_1.empty())
			{
				const XMFLOAT2* uv0_stream = vertex_uvset_0.empty() ? vertex_uvset_1.data() : vertex_uvset_0.data();
				const XMFLOAT2* uv1_stream = vertex_uvset_1.empty() ? vertex_uvset_0.data() : vertex_uvset_1.data();

				vb_uvs.offset = buffer_offset;
				vb_uvs.size = uv_count * sizeof(Vertex_UVS);
				Vertex_UVS* vertices = (Vertex_UVS*)(buffer_data + buffer_offset);
				buffer_offset += AlignTo(vb_uvs.size, alignment);
				for (size_t i = 0; i < uv_count; ++i)
				{
					Vertex_UVS vert;
					vert.uv0.FromFULL(uv0_stream[i]);
					vert.uv1.FromFULL(uv1_stream[i]);
					std::memcpy(vertices + i, &vert, sizeof(vert));
				}
			}

			// vertexBuffer - ATLAS
			if (!vertex_atlas.empty())
			{
				vb_atl.offset = buffer_offset;
				vb_atl.size = vertex_atlas.size() * sizeof(Vertex_TEX);
				Vertex_TEX* vertices = (Vertex_TEX*)(buffer_data + buffer_offset);
				buffer_offset += AlignTo(vb_atl.size, alignment);
				for (size_t i = 0; i < vertex_atlas.size(); ++i)
				{
					Vertex_TEX vert;
					vert.FromFULL(vertex_atlas[i]);
					std::memcpy(vertices + i, &vert, sizeof(vert));
				}
			}

			// vertexBuffer - COLORS
			if (!vertex_colors.empty())
			{
				vb_col.offset = buffer_offset;
				vb_col.size = vertex_colors.size() * sizeof(Vertex_COL);
				Vertex_COL* vertices = (Vertex_COL*)(buffer_data + buffer_offset);
				buffer_offset += AlignTo(vb_col.size, alignment);
				for (size_t i = 0; i < vertex_colors.size(); ++i)
				{
					Vertex_COL vert;
					vert.color = vertex_colors[i];
					std::memcpy(vertices + i, &vert, sizeof(vert));
				}
			}

			// skinning buffers:
			if (!vertex_boneindices.empty())
			{
				vb_bon.offset = buffer_offset;
				vb_bon.size = vertex_boneindices.size() * sizeof(Vertex_BON);
				Vertex_BON* vertices = (Vertex_BON*)(buffer_data + buffer_offset);
				buffer_offset += AlignTo(vb_bon.size, alignment);
				assert(vertex_boneindices.size() == vertex_boneweights.size());
				for (size_t i = 0; i < vertex_boneindices.size(); ++i)
				{
					XMFLOAT4& wei = vertex_boneweights[i];
					// normalize bone weights
					float len = wei.x + wei.y + wei.z + wei.w;
					if (len > 0)
					{
						wei.x /= len;
						wei.y /= len;
						wei.z /= len;
						wei.w /= len;
					}
					Vertex_BON vert;
					vert.FromFULL(vertex_boneindices[i], wei);
					std::memcpy(vertices + i, &vert, sizeof(vert));
				}
			}

			// morph buffers:
			if (!morph_targets.empty())
			{
				vb_mor.offset = buffer_offset;
				for (MorphTarget& morph : morph_targets)
				{
					if (!morph.vertex_positions.empty())
					{
						morph.offset_pos = (buffer_offset - vb_mor.offset) / morph_stride;
						XMHALF4* vertices = (XMHALF4*)(buffer_data + buffer_offset);
						std::fill(vertices, vertices + vertex_positions.size(), 0);
						if (morph.sparse_indices_positions.empty())
						{
							// flat morphs:
							for (size_t i = 0; i < morph.vertex_positions.size(); ++i)
							{
								XMStoreHalf4(vertices + i, XMLoadFloat3(&morph.vertex_positions[i]));
							}
						}
						else
						{
							// sparse morphs will be flattened for GPU because they will be evaluated in skinning for every vertex:
							for (size_t i = 0; i < morph.sparse_indices_positions.size(); ++i)
							{
								const uint32_t ind = morph.sparse_indices_positions[i];
								XMStoreHalf4(vertices + ind, XMLoadFloat3(&morph.vertex_positions[i]));
							}
						}
						buffer_offset += AlignTo(morph.vertex_positions.size() * sizeof(XMHALF4), alignment);
					}
					if (!morph.vertex_normals.empty())
					{
						morph.offset_nor = (buffer_offset - vb_mor.offset) / morph_stride;
						XMHALF4* vertices = (XMHALF4*)(buffer_data + buffer_offset);
						std::fill(vertices, vertices + vertex_normals.size(), 0);
						if (morph.sparse_indices_normals.empty())
						{
							// flat morphs:
							for (size_t i = 0; i < morph.vertex_normals.size(); ++i)
							{
								XMStoreHalf4(vertices + i, XMLoadFloat3(&morph.vertex_normals[i]));
							}
						}
						else
						{
							// sparse morphs will be flattened for GPU because they will be evaluated in skinning for every vertex:
							for (size_t i = 0; i < morph.sparse_indices_normals.size(); ++i)
							{
								const uint32_t ind = morph.sparse_indices_normals[i];
								XMStoreHalf4(vertices + ind, XMLoadFloat3(&morph.vertex_normals[i]));
							}
						}
						buffer_offset += AlignTo(morph.vertex_normals.size() * sizeof(XMHALF4), alignment);
					}
				}
				vb_mor.size = buffer_offset - vb_mor.offset;
			}
		};

		bool success = device->CreateBuffer2(&bd, init_callback, &generalBuffer);
		assert(success);
		device->SetName(&generalBuffer, "MeshComponent::generalBuffer");

		assert(ib.IsValid());
		const Format ib_format = GetIndexFormat() == IndexBufferFormat::UINT32 ? Format::R32_UINT : Format::R16_UINT;
		ib.subresource_srv = device->CreateSubresource(&generalBuffer, SubresourceType::SRV, ib.offset, ib.size, &ib_format);
		ib.descriptor_srv = device->GetDescriptorIndex(&generalBuffer, SubresourceType::SRV, ib.subresource_srv);

		assert(vb_pos_nor_wind.IsValid());
		vb_pos_nor_wind.subresource_srv = device->CreateSubresource(&generalBuffer, SubresourceType::SRV, vb_pos_nor_wind.offset, vb_pos_nor_wind.size, &Vertex_POS::FORMAT);
		vb_pos_nor_wind.descriptor_srv = device->GetDescriptorIndex(&generalBuffer, SubresourceType::SRV, vb_pos_nor_wind.subresource_srv);

		if (vb_tan.IsValid())
		{
			vb_tan.subresource_srv = device->CreateSubresource(&generalBuffer, SubresourceType::SRV, vb_tan.offset, vb_tan.size, &Vertex_TAN::FORMAT);
			vb_tan.descriptor_srv = device->GetDescriptorIndex(&generalBuffer, SubresourceType::SRV, vb_tan.subresource_srv);
		}
		if (vb_uvs.IsValid())
		{
			vb_uvs.subresource_srv = device->CreateSubresource(&generalBuffer, SubresourceType::SRV, vb_uvs.offset, vb_uvs.size, &Vertex_UVS::FORMAT);
			vb_uvs.descriptor_srv = device->GetDescriptorIndex(&generalBuffer, SubresourceType::SRV, vb_uvs.subresource_srv);
		}
		if (vb_atl.IsValid())
		{
			vb_atl.subresource_srv = device->CreateSubresource(&generalBuffer, SubresourceType::SRV, vb_atl.offset, vb_atl.size, &Vertex_TEX::FORMAT);
			vb_atl.descriptor_srv = device->GetDescriptorIndex(&generalBuffer, SubresourceType::SRV, vb_atl.subresource_srv);
		}
		if (vb_col.IsValid())
		{
			vb_col.subresource_srv = device->CreateSubresource(&generalBuffer, SubresourceType::SRV, vb_col.offset, vb_col.size, &Vertex_COL::FORMAT);
			vb_col.descriptor_srv = device->GetDescriptorIndex(&generalBuffer, SubresourceType::SRV, vb_col.subresource_srv);
		}
		if (vb_bon.IsValid())
		{
			vb_bon.subresource_srv = device->CreateSubresource(&generalBuffer, SubresourceType::SRV, vb_bon.offset, vb_bon.size);
			vb_bon.descriptor_srv = device->GetDescriptorIndex(&generalBuffer, SubresourceType::SRV, vb_bon.subresource_srv);
		}
		if (vb_mor.IsValid())
		{
			vb_mor.subresource_srv = device->CreateSubresource(&generalBuffer, SubresourceType::SRV, vb_mor.offset, vb_mor.size, &morph_format);
			vb_mor.descriptor_srv = device->GetDescriptorIndex(&generalBuffer, SubresourceType::SRV, vb_mor.subresource_srv);
		}

		if (!vertex_boneindices.empty() || !morph_targets.empty())
		{
			CreateStreamoutRenderData();
		}
	}
	void MeshComponent::CreateStreamoutRenderData()
	{
		GraphicsDevice* device = wi::graphics::GetDevice();

		GPUBufferDesc desc;
		desc.usage = Usage::DEFAULT;
		desc.bind_flags = BindFlag::VERTEX_BUFFER | BindFlag::SHADER_RESOURCE | BindFlag::UNORDERED_ACCESS;
		desc.misc_flags = ResourceMiscFlag::BUFFER_RAW | ResourceMiscFlag::TYPED_FORMAT_CASTING | ResourceMiscFlag::NO_DEFAULT_DESCRIPTORS;
		if (device->CheckCapability(GraphicsDeviceCapability::RAYTRACING))
		{
			desc.misc_flags |= ResourceMiscFlag::RAY_TRACING;
		}
		const uint64_t alignment = device->GetMinOffsetAlignment(&desc);
		desc.size =
			AlignTo(vertex_positions.size() * sizeof(Vertex_POS) * 2, alignment) + // *2 because prevpos also goes into this!
			AlignTo(vertex_tangents.size() * sizeof(Vertex_TAN), alignment)
			;

		bool success = device->CreateBuffer(&desc, nullptr, &streamoutBuffer);
		assert(success);
		device->SetName(&streamoutBuffer, "MeshComponent::streamoutBuffer");

		uint64_t buffer_offset = 0ull;

		so_pos_nor_wind.offset = buffer_offset;
		so_pos_nor_wind.size = vb_pos_nor_wind.size;
		buffer_offset += AlignTo(so_pos_nor_wind.size, alignment);
		so_pos_nor_wind.subresource_srv = device->CreateSubresource(&streamoutBuffer, SubresourceType::SRV, so_pos_nor_wind.offset, so_pos_nor_wind.size, &Vertex_POS::FORMAT);
		so_pos_nor_wind.subresource_uav = device->CreateSubresource(&streamoutBuffer, SubresourceType::UAV, so_pos_nor_wind.offset, so_pos_nor_wind.size, &Vertex_POS::FORMAT);
		so_pos_nor_wind.descriptor_srv = device->GetDescriptorIndex(&streamoutBuffer, SubresourceType::SRV, so_pos_nor_wind.subresource_srv);
		so_pos_nor_wind.descriptor_uav = device->GetDescriptorIndex(&streamoutBuffer, SubresourceType::UAV, so_pos_nor_wind.subresource_uav);

		if (vb_tan.IsValid())
		{
			so_tan.offset = buffer_offset;
			so_tan.size = vb_tan.size;
			buffer_offset += AlignTo(so_tan.size, alignment);
			so_tan.subresource_srv = device->CreateSubresource(&streamoutBuffer, SubresourceType::SRV, so_tan.offset, so_tan.size, &Vertex_TAN::FORMAT);
			so_tan.subresource_uav = device->CreateSubresource(&streamoutBuffer, SubresourceType::UAV, so_tan.offset, so_tan.size, &Vertex_TAN::FORMAT);
			so_tan.descriptor_srv = device->GetDescriptorIndex(&streamoutBuffer, SubresourceType::SRV, so_tan.subresource_srv);
			so_tan.descriptor_uav = device->GetDescriptorIndex(&streamoutBuffer, SubresourceType::UAV, so_tan.subresource_uav);
		}

		so_pre.offset = buffer_offset;
		so_pre.size = vb_pos_nor_wind.size;
		buffer_offset += AlignTo(so_pre.size, alignment);
		so_pre.subresource_srv = device->CreateSubresource(&streamoutBuffer, SubresourceType::SRV, so_pre.offset, so_pre.size, &Vertex_POS::FORMAT);
		so_pre.subresource_uav = device->CreateSubresource(&streamoutBuffer, SubresourceType::UAV, so_pre.offset, so_pre.size, &Vertex_POS::FORMAT);
		so_pre.descriptor_srv = device->GetDescriptorIndex(&streamoutBuffer, SubresourceType::SRV, so_pre.subresource_srv);
		so_pre.descriptor_uav = device->GetDescriptorIndex(&streamoutBuffer, SubresourceType::UAV, so_pre.subresource_uav);
	}
	void MeshComponent::CreateRaytracingRenderData()
	{
		GraphicsDevice* device = wi::graphics::GetDevice();

		if (!device->CheckCapability(GraphicsDeviceCapability::RAYTRACING))
			return;

		BLAS_state = MeshComponent::BLAS_STATE_NEEDS_REBUILD;

		const uint32_t lod_count = GetLODCount();
		BLASes.resize(lod_count);
		for (uint32_t lod = 0; lod < lod_count; ++lod)
		{
			RaytracingAccelerationStructureDesc desc;
			desc.type = RaytracingAccelerationStructureDesc::Type::BOTTOMLEVEL;

			if (streamoutBuffer.IsValid())
			{
				desc.flags |= RaytracingAccelerationStructureDesc::FLAG_ALLOW_UPDATE;
				desc.flags |= RaytracingAccelerationStructureDesc::FLAG_PREFER_FAST_BUILD;
			}
			else
			{
				desc.flags |= RaytracingAccelerationStructureDesc::FLAG_PREFER_FAST_TRACE;
			}

			uint32_t first_subset = 0;
			uint32_t last_subset = 0;
			GetLODSubsetRange(lod, first_subset, last_subset);
			for (uint32_t subsetIndex = first_subset; subsetIndex < last_subset; ++subsetIndex)
			{
				const MeshComponent::MeshSubset& subset = subsets[subsetIndex];
				desc.bottom_level.geometries.emplace_back();
				auto& geometry = desc.bottom_level.geometries.back();
				geometry.type = RaytracingAccelerationStructureDesc::BottomLevel::Geometry::Type::TRIANGLES;
				geometry.triangles.vertex_buffer = generalBuffer;
				geometry.triangles.vertex_byte_offset = vb_pos_nor_wind.offset;
				geometry.triangles.index_buffer = generalBuffer;
				geometry.triangles.index_format = GetIndexFormat();
				geometry.triangles.index_count = subset.indexCount;
				geometry.triangles.index_offset = ib.offset / GetIndexStride() + subset.indexOffset;
				geometry.triangles.vertex_count = (uint32_t)vertex_positions.size();
				geometry.triangles.vertex_format = Format::R32G32B32_FLOAT;
				geometry.triangles.vertex_stride = sizeof(MeshComponent::Vertex_POS);
			}

			bool success = device->CreateRaytracingAccelerationStructure(&desc, &BLASes[lod]);
			assert(success);
			device->SetName(&BLASes[lod], std::string("MeshComponent::BLAS[LOD" + std::to_string(lod) + "]").c_str());
		}
	}
	void MeshComponent::BuildBVH()
	{
		bvh_leaf_aabbs.clear();
		uint32_t first_subset = 0;
		uint32_t last_subset = 0;
		GetLODSubsetRange(0, first_subset, last_subset);
		for (uint32_t subsetIndex = first_subset; subsetIndex < last_subset; ++subsetIndex)
		{
			assert(subsetIndex <= 0xFF); // must fit into 8 bits userdata packing
			const MeshComponent::MeshSubset& subset = subsets[subsetIndex];
			if (subset.indexCount == 0)
				continue;
			const uint32_t indexOffset = subset.indexOffset;
			const uint32_t triangleCount = subset.indexCount / 3;
			for (uint32_t triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
			{
				assert(triangleIndex <= 0xFFFFFF); // must fit into 24 bits userdata packing
				const uint32_t i0 = indices[indexOffset + triangleIndex * 3 + 0];
				const uint32_t i1 = indices[indexOffset + triangleIndex * 3 + 1];
				const uint32_t i2 = indices[indexOffset + triangleIndex * 3 + 2];
				const XMFLOAT3& p0 = vertex_positions[i0];
				const XMFLOAT3& p1 = vertex_positions[i1];
				const XMFLOAT3& p2 = vertex_positions[i2];
				AABB aabb = wi::primitive::AABB(wi::math::Min(p0, wi::math::Min(p1, p2)), wi::math::Max(p0, wi::math::Max(p1, p2)));
				aabb.userdata = (triangleIndex & 0xFFFFFF) | ((subsetIndex & 0xFF) << 24u);
				bvh_leaf_aabbs.push_back(aabb);
			}
		}
		bvh.Build(bvh_leaf_aabbs.data(), (uint32_t)bvh_leaf_aabbs.size());
	}
	void MeshComponent::ComputeNormals(COMPUTE_NORMALS compute)
	{
		// Start recalculating normals:

		if (compute != COMPUTE_NORMALS_SMOOTH_FAST)
		{
			// Compute hard surface normals:

			// Right now they are always computed even before smooth setting

			wi::vector<uint32_t> newIndexBuffer;
			wi::vector<XMFLOAT3> newPositionsBuffer;
			wi::vector<XMFLOAT3> newNormalsBuffer;
			wi::vector<XMFLOAT2> newUV0Buffer;
			wi::vector<XMFLOAT2> newUV1Buffer;
			wi::vector<XMFLOAT2> newAtlasBuffer;
			wi::vector<XMUINT4> newBoneIndicesBuffer;
			wi::vector<XMFLOAT4> newBoneWeightsBuffer;
			wi::vector<uint32_t> newColorsBuffer;

			for (size_t face = 0; face < indices.size() / 3; face++)
			{
				uint32_t i0 = indices[face * 3 + 0];
				uint32_t i1 = indices[face * 3 + 1];
				uint32_t i2 = indices[face * 3 + 2];

				XMFLOAT3& p0 = vertex_positions[i0];
				XMFLOAT3& p1 = vertex_positions[i1];
				XMFLOAT3& p2 = vertex_positions[i2];

				XMVECTOR U = XMLoadFloat3(&p2) - XMLoadFloat3(&p0);
				XMVECTOR V = XMLoadFloat3(&p1) - XMLoadFloat3(&p0);

				XMVECTOR N = XMVector3Cross(U, V);
				N = XMVector3Normalize(N);

				XMFLOAT3 normal;
				XMStoreFloat3(&normal, N);

				newPositionsBuffer.push_back(p0);
				newPositionsBuffer.push_back(p1);
				newPositionsBuffer.push_back(p2);

				newNormalsBuffer.push_back(normal);
				newNormalsBuffer.push_back(normal);
				newNormalsBuffer.push_back(normal);

				if (!vertex_uvset_0.empty())
				{
					newUV0Buffer.push_back(vertex_uvset_0[i0]);
					newUV0Buffer.push_back(vertex_uvset_0[i1]);
					newUV0Buffer.push_back(vertex_uvset_0[i2]);
				}

				if (!vertex_uvset_1.empty())
				{
					newUV1Buffer.push_back(vertex_uvset_1[i0]);
					newUV1Buffer.push_back(vertex_uvset_1[i1]);
					newUV1Buffer.push_back(vertex_uvset_1[i2]);
				}

				if (!vertex_atlas.empty())
				{
					newAtlasBuffer.push_back(vertex_atlas[i0]);
					newAtlasBuffer.push_back(vertex_atlas[i1]);
					newAtlasBuffer.push_back(vertex_atlas[i2]);
				}

				if (!vertex_boneindices.empty())
				{
					newBoneIndicesBuffer.push_back(vertex_boneindices[i0]);
					newBoneIndicesBuffer.push_back(vertex_boneindices[i1]);
					newBoneIndicesBuffer.push_back(vertex_boneindices[i2]);
				}

				if (!vertex_boneweights.empty())
				{
					newBoneWeightsBuffer.push_back(vertex_boneweights[i0]);
					newBoneWeightsBuffer.push_back(vertex_boneweights[i1]);
					newBoneWeightsBuffer.push_back(vertex_boneweights[i2]);
				}

				if (!vertex_colors.empty())
				{
					newColorsBuffer.push_back(vertex_colors[i0]);
					newColorsBuffer.push_back(vertex_colors[i1]);
					newColorsBuffer.push_back(vertex_colors[i2]);
				}

				newIndexBuffer.push_back(static_cast<uint32_t>(newIndexBuffer.size()));
				newIndexBuffer.push_back(static_cast<uint32_t>(newIndexBuffer.size()));
				newIndexBuffer.push_back(static_cast<uint32_t>(newIndexBuffer.size()));
			}

			// For hard surface normals, we created a new mesh in the previous loop through faces, so swap data:
			vertex_positions = newPositionsBuffer;
			vertex_normals = newNormalsBuffer;
			vertex_uvset_0 = newUV0Buffer;
			vertex_uvset_1 = newUV1Buffer;
			vertex_atlas = newAtlasBuffer;
			vertex_colors = newColorsBuffer;
			if (!vertex_boneindices.empty())
			{
				vertex_boneindices = newBoneIndicesBuffer;
			}
			if (!vertex_boneweights.empty())
			{
				vertex_boneweights = newBoneWeightsBuffer;
			}
			indices = newIndexBuffer;
		}

		switch (compute)
		{
		case MeshComponent::COMPUTE_NORMALS_HARD:
			break;

		case MeshComponent::COMPUTE_NORMALS_SMOOTH:
		{
			// Compute smooth surface normals:

			// 1.) Zero normals, they will be averaged later
			for (size_t i = 0; i < vertex_normals.size(); i++)
			{
				vertex_normals[i] = XMFLOAT3(0, 0, 0);
			}

			// 2.) Find identical vertices by POSITION, accumulate face normals
			for (size_t i = 0; i < vertex_positions.size(); i++)
			{
				XMFLOAT3& v_search_pos = vertex_positions[i];

				for (size_t ind = 0; ind < indices.size() / 3; ++ind)
				{
					uint32_t i0 = indices[ind * 3 + 0];
					uint32_t i1 = indices[ind * 3 + 1];
					uint32_t i2 = indices[ind * 3 + 2];

					XMFLOAT3& v0 = vertex_positions[i0];
					XMFLOAT3& v1 = vertex_positions[i1];
					XMFLOAT3& v2 = vertex_positions[i2];

					bool match_pos0 =
						fabs(v_search_pos.x - v0.x) < FLT_EPSILON &&
						fabs(v_search_pos.y - v0.y) < FLT_EPSILON &&
						fabs(v_search_pos.z - v0.z) < FLT_EPSILON;

					bool match_pos1 =
						fabs(v_search_pos.x - v1.x) < FLT_EPSILON &&
						fabs(v_search_pos.y - v1.y) < FLT_EPSILON &&
						fabs(v_search_pos.z - v1.z) < FLT_EPSILON;

					bool match_pos2 =
						fabs(v_search_pos.x - v2.x) < FLT_EPSILON &&
						fabs(v_search_pos.y - v2.y) < FLT_EPSILON &&
						fabs(v_search_pos.z - v2.z) < FLT_EPSILON;

					if (match_pos0 || match_pos1 || match_pos2)
					{
						XMVECTOR U = XMLoadFloat3(&v2) - XMLoadFloat3(&v0);
						XMVECTOR V = XMLoadFloat3(&v1) - XMLoadFloat3(&v0);

						XMVECTOR N = XMVector3Cross(U, V);
						N = XMVector3Normalize(N);

						XMFLOAT3 normal;
						XMStoreFloat3(&normal, N);

						vertex_normals[i].x += normal.x;
						vertex_normals[i].y += normal.y;
						vertex_normals[i].z += normal.z;
					}

				}
			}

			// 3.) Find duplicated vertices by POSITION and UV0 and UV1 and ATLAS and SUBSET and remove them:
			for (auto& subset : subsets)
			{
				for (uint32_t i = 0; i < subset.indexCount - 1; i++)
				{
					uint32_t ind0 = indices[subset.indexOffset + (uint32_t)i];
					const XMFLOAT3& p0 = vertex_positions[ind0];
					const XMFLOAT2& u00 = vertex_uvset_0.empty() ? XMFLOAT2(0, 0) : vertex_uvset_0[ind0];
					const XMFLOAT2& u10 = vertex_uvset_1.empty() ? XMFLOAT2(0, 0) : vertex_uvset_1[ind0];
					const XMFLOAT2& at0 = vertex_atlas.empty() ? XMFLOAT2(0, 0) : vertex_atlas[ind0];

					for (uint32_t j = i + 1; j < subset.indexCount; j++)
					{
						uint32_t ind1 = indices[subset.indexOffset + (uint32_t)j];

						if (ind1 == ind0)
						{
							continue;
						}

						const XMFLOAT3& p1 = vertex_positions[ind1];
						const XMFLOAT2& u01 = vertex_uvset_0.empty() ? XMFLOAT2(0, 0) : vertex_uvset_0[ind1];
						const XMFLOAT2& u11 = vertex_uvset_1.empty() ? XMFLOAT2(0, 0) : vertex_uvset_1[ind1];
						const XMFLOAT2& at1 = vertex_atlas.empty() ? XMFLOAT2(0, 0) : vertex_atlas[ind1];

						const bool duplicated_pos =
							fabs(p0.x - p1.x) < FLT_EPSILON &&
							fabs(p0.y - p1.y) < FLT_EPSILON &&
							fabs(p0.z - p1.z) < FLT_EPSILON;

						const bool duplicated_uv0 =
							fabs(u00.x - u01.x) < FLT_EPSILON &&
							fabs(u00.y - u01.y) < FLT_EPSILON;

						const bool duplicated_uv1 =
							fabs(u10.x - u11.x) < FLT_EPSILON &&
							fabs(u10.y - u11.y) < FLT_EPSILON;

						const bool duplicated_atl =
							fabs(at0.x - at1.x) < FLT_EPSILON &&
							fabs(at0.y - at1.y) < FLT_EPSILON;

						if (duplicated_pos && duplicated_uv0 && duplicated_uv1 && duplicated_atl)
						{
							// Erase vertices[ind1] because it is a duplicate:
							if (ind1 < vertex_positions.size())
							{
								vertex_positions.erase(vertex_positions.begin() + ind1);
							}
							if (ind1 < vertex_normals.size())
							{
								vertex_normals.erase(vertex_normals.begin() + ind1);
							}
							if (ind1 < vertex_uvset_0.size())
							{
								vertex_uvset_0.erase(vertex_uvset_0.begin() + ind1);
							}
							if (ind1 < vertex_uvset_1.size())
							{
								vertex_uvset_1.erase(vertex_uvset_1.begin() + ind1);
							}
							if (ind1 < vertex_atlas.size())
							{
								vertex_atlas.erase(vertex_atlas.begin() + ind1);
							}
							if (ind1 < vertex_boneindices.size())
							{
								vertex_boneindices.erase(vertex_boneindices.begin() + ind1);
							}
							if (ind1 < vertex_boneweights.size())
							{
								vertex_boneweights.erase(vertex_boneweights.begin() + ind1);
							}

							// The vertices[ind1] was removed, so each index after that needs to be updated:
							for (auto& index : indices)
							{
								if (index > ind1 && index > 0)
								{
									index--;
								}
								else if (index == ind1)
								{
									index = ind0;
								}
							}

						}

					}
				}

			}

		}
		break;

		case MeshComponent::COMPUTE_NORMALS_SMOOTH_FAST:
		{
			for (size_t i = 0; i < vertex_normals.size(); i++)
			{
				vertex_normals[i] = XMFLOAT3(0, 0, 0);
			}
			for (size_t i = 0; i < indices.size() / 3; ++i)
			{
				uint32_t index1 = indices[i * 3];
				uint32_t index2 = indices[i * 3 + 1];
				uint32_t index3 = indices[i * 3 + 2];

				XMVECTOR side1 = XMLoadFloat3(&vertex_positions[index1]) - XMLoadFloat3(&vertex_positions[index3]);
				XMVECTOR side2 = XMLoadFloat3(&vertex_positions[index1]) - XMLoadFloat3(&vertex_positions[index2]);
				XMVECTOR N = XMVector3Normalize(XMVector3Cross(side1, side2));
				XMFLOAT3 normal;
				XMStoreFloat3(&normal, N);

				vertex_normals[index1].x += normal.x;
				vertex_normals[index1].y += normal.y;
				vertex_normals[index1].z += normal.z;

				vertex_normals[index2].x += normal.x;
				vertex_normals[index2].y += normal.y;
				vertex_normals[index2].z += normal.z;

				vertex_normals[index3].x += normal.x;
				vertex_normals[index3].y += normal.y;
				vertex_normals[index3].z += normal.z;
			}
		}
		break;

		}

		vertex_tangents.clear(); // <- will be recomputed

		CreateRenderData(); // <- normals will be normalized here!
	}
	void MeshComponent::FlipCulling()
	{
		for (size_t face = 0; face < indices.size() / 3; face++)
		{
			uint32_t i0 = indices[face * 3 + 0];
			uint32_t i1 = indices[face * 3 + 1];
			uint32_t i2 = indices[face * 3 + 2];

			indices[face * 3 + 0] = i0;
			indices[face * 3 + 1] = i2;
			indices[face * 3 + 2] = i1;
		}

		CreateRenderData();
	}
	void MeshComponent::FlipNormals()
	{
		for (auto& normal : vertex_normals)
		{
			normal.x *= -1;
			normal.y *= -1;
			normal.z *= -1;
		}

		CreateRenderData();
	}
	void MeshComponent::Recenter()
	{
		XMFLOAT3 center = aabb.getCenter();

		for (auto& pos : vertex_positions)
		{
			pos.x -= center.x;
			pos.y -= center.y;
			pos.z -= center.z;
		}

		CreateRenderData();
	}
	void MeshComponent::RecenterToBottom()
	{
		XMFLOAT3 center = aabb.getCenter();
		center.y -= aabb.getHalfWidth().y;

		for (auto& pos : vertex_positions)
		{
			pos.x -= center.x;
			pos.y -= center.y;
			pos.z -= center.z;
		}

		CreateRenderData();
	}
	Sphere MeshComponent::GetBoundingSphere() const
	{
		Sphere sphere;
		sphere.center = aabb.getCenter();
		sphere.radius = aabb.getRadius();
		return sphere;
	}
	size_t MeshComponent::GetMemoryUsageCPU() const
	{
		size_t size = 
			vertex_positions.size() * sizeof(XMFLOAT3) +
			vertex_normals.size() * sizeof(XMFLOAT3) +
			vertex_tangents.size() * sizeof(XMFLOAT4) +
			vertex_uvset_0.size() * sizeof(XMFLOAT2) +
			vertex_uvset_1.size() * sizeof(XMFLOAT2) +
			vertex_boneindices.size() * sizeof(XMUINT4) +
			vertex_boneweights.size() * sizeof(XMFLOAT4) +
			vertex_atlas.size() * sizeof(XMFLOAT2) +
			vertex_colors.size() * sizeof(uint32_t) +
			vertex_windweights.size() * sizeof(uint8_t) +
			indices.size() * sizeof(uint32_t);

		for (const MorphTarget& morph : morph_targets)
		{
			size +=
				morph.vertex_positions.size() * sizeof(XMFLOAT3) +
				morph.vertex_normals.size() * sizeof(XMFLOAT3) +
				morph.sparse_indices_positions.size() * sizeof(uint32_t) +
				morph.sparse_indices_normals.size() * sizeof(uint32_t);
		}

		size += GetMemoryUsageBVH();

		return size;
	}
	size_t MeshComponent::GetMemoryUsageGPU() const
	{
		return generalBuffer.desc.size + streamoutBuffer.desc.size;
	}
	size_t MeshComponent::GetMemoryUsageBVH() const
	{
		return
			bvh.allocation.capacity() +
			bvh_leaf_aabbs.size() * sizeof(wi::primitive::AABB);
	}

	void ObjectComponent::ClearLightmap()
	{
		lightmap = Texture();
		lightmapWidth = 0;
		lightmapHeight = 0;
		lightmapIterationCount = 0;
		lightmapTextureData.clear();
		SetLightmapRenderRequest(false);
	}

	void ObjectComponent::SaveLightmap()
	{
		if (lightmap.IsValid() && has_flag(lightmap.desc.bind_flags, BindFlag::RENDER_TARGET))
		{
			SetLightmapRenderRequest(false);

			bool success = wi::helper::saveTextureToMemory(lightmap, lightmapTextureData);
			assert(success);

#ifdef OPEN_IMAGE_DENOISE
			if (success)
			{
				wi::vector<uint8_t> texturedata_dst(lightmapTextureData.size());

				size_t width = (size_t)lightmapWidth;
				size_t height = (size_t)lightmapHeight;
				{
					// https://github.com/OpenImageDenoise/oidn#c11-api-example

					// Create an Intel Open Image Denoise device
					static oidn::DeviceRef device = oidn::newDevice();
					static bool init = false;
					if (!init)
					{
						device.commit();
						init = true;
					}

					oidn::BufferRef lightmapTextureData_buffer = device.newBuffer(lightmapTextureData.size());
					oidn::BufferRef texturedata_dst_buffer = device.newBuffer(texturedata_dst.size());

					lightmapTextureData_buffer.write(0, lightmapTextureData.size(), lightmapTextureData.data());

					// Create a denoising filter
					oidn::FilterRef filter = device.newFilter("RTLightmap");
					filter.setImage("color", lightmapTextureData_buffer, oidn::Format::Float3, width, height, 0, sizeof(XMFLOAT4));
					filter.setImage("output", texturedata_dst_buffer, oidn::Format::Float3, width, height, 0, sizeof(XMFLOAT4));
					filter.commit();

					// Filter the image
					filter.execute();

					// Check for errors
					const char* errorMessage;
					auto error = device.getError(errorMessage);
					if (error != oidn::Error::None && error != oidn::Error::Cancelled)
					{
						wi::backlog::post(std::string("[OpenImageDenoise error] ") + errorMessage);
					}
					else
					{
						texturedata_dst_buffer.read(0, texturedata_dst.size(), texturedata_dst.data());
					}
				}

				lightmapTextureData = std::move(texturedata_dst); // replace old (raw) data with denoised data
			}
#endif // OPEN_IMAGE_DENOISE

			CompressLightmap();

			wi::texturehelper::CreateTexture(lightmap, lightmapTextureData.data(), lightmapWidth, lightmapHeight, lightmap.desc.format);
			wi::graphics::GetDevice()->SetName(&lightmap, "lightmap");
		}
	}
	void ObjectComponent::CompressLightmap()
	{
		if (IsLightmapDisableBlockCompression())
		{
			// Simple packing to R11G11B10_FLOAT format on CPU:
			using namespace PackedVector;
			wi::vector<uint8_t> packed_data;
			packed_data.resize(sizeof(XMFLOAT3PK) * lightmapWidth * lightmapHeight);
			XMFLOAT3PK* packed_ptr = (XMFLOAT3PK*)packed_data.data();
			XMFLOAT4* raw_ptr = (XMFLOAT4*)lightmapTextureData.data();

			uint32_t texelcount = lightmapWidth * lightmapHeight;
			for (uint32_t i = 0; i < texelcount; ++i)
			{
				XMStoreFloat3PK(packed_ptr + i, XMLoadFloat4(raw_ptr + i));
			}

			lightmapTextureData = std::move(packed_data);
			lightmap.desc.format = Format::R11G11B10_FLOAT;
			lightmap.desc.bind_flags = BindFlag::SHADER_RESOURCE;
		}
		else
		{
			// BC6 compress on GPU:
			wi::texturehelper::CreateTexture(lightmap, lightmapTextureData.data(), lightmapWidth, lightmapHeight, lightmap.desc.format);
			TextureDesc desc = lightmap.desc;
			desc.format = Format::BC6H_UF16;
			desc.bind_flags = BindFlag::SHADER_RESOURCE;
			Texture bc6tex;
			GraphicsDevice* device = GetDevice();
			device->CreateTexture(&desc, nullptr, &bc6tex);
			CommandList cmd = device->BeginCommandList();
			wi::renderer::BlockCompress(lightmap, bc6tex, cmd);
			wi::helper::saveTextureToMemory(bc6tex, lightmapTextureData); // internally waits for GPU completion
			lightmap.desc = desc;
		}
	}

	void EnvironmentProbeComponent::CreateRenderData()
	{
		if (!textureName.empty() && !resource.IsValid())
		{
			resource = wi::resourcemanager::Load(textureName);
		}
		if (resource.IsValid())
		{
			texture = resource.GetTexture();
			SetDirty(false);
			return;
		}
		resolution = wi::math::GetNextPowerOfTwo(resolution);
		if (texture.IsValid() && resolution == texture.desc.width)
			return;
		SetDirty();

		GraphicsDevice* device = wi::graphics::GetDevice();

		TextureDesc desc;
		desc.array_size = 6;
		desc.height = resolution;
		desc.width = resolution;
		desc.usage = Usage::DEFAULT;
		desc.format = Format::BC6H_UF16;
		desc.sample_count = 1; // Note that this texture is always non-MSAA, even if probe is rendered as MSAA, because this contains resolved result
		desc.bind_flags = BindFlag::SHADER_RESOURCE;
		desc.mip_levels = GetMipCount(resolution, resolution, 1, 16);
		desc.misc_flags = ResourceMiscFlag::TEXTURECUBE;
		desc.layout = ResourceState::SHADER_RESOURCE;
		device->CreateTexture(&desc, nullptr, &texture);
		device->SetName(&texture, "EnvironmentProbeComponent::texture");
	}
	void EnvironmentProbeComponent::DeleteResource()
	{
		if (resource.IsValid())
		{
			// only delete these if resource is actually valid!
			resource = {};
			texture = {};
			textureName = {};
		}
	}
	size_t EnvironmentProbeComponent::GetMemorySizeInBytes() const
	{
		return ComputeTextureMemorySizeInBytes(texture.desc);
	}

	AnimationComponent::AnimationChannel::PathDataType AnimationComponent::AnimationChannel::GetPathDataType() const
	{
		switch (path)
		{
		case wi::scene::AnimationComponent::AnimationChannel::Path::TRANSLATION:
			return PathDataType::Float3;
		case wi::scene::AnimationComponent::AnimationChannel::Path::ROTATION:
			return PathDataType::Float4;
		case wi::scene::AnimationComponent::AnimationChannel::Path::SCALE:
			return PathDataType::Float3;
		case wi::scene::AnimationComponent::AnimationChannel::Path::WEIGHTS:
			return PathDataType::Weights;

		case wi::scene::AnimationComponent::AnimationChannel::Path::LIGHT_COLOR:
			return PathDataType::Float3;
		case wi::scene::AnimationComponent::AnimationChannel::Path::LIGHT_INTENSITY:
		case wi::scene::AnimationComponent::AnimationChannel::Path::LIGHT_RANGE:
		case wi::scene::AnimationComponent::AnimationChannel::Path::LIGHT_INNERCONE:
		case wi::scene::AnimationComponent::AnimationChannel::Path::LIGHT_OUTERCONE:
			return PathDataType::Float;

		case wi::scene::AnimationComponent::AnimationChannel::Path::SOUND_PLAY:
		case wi::scene::AnimationComponent::AnimationChannel::Path::SOUND_STOP:
			return PathDataType::Event;
		case wi::scene::AnimationComponent::AnimationChannel::Path::SOUND_VOLUME:
			return PathDataType::Float;

		case wi::scene::AnimationComponent::AnimationChannel::Path::EMITTER_EMITCOUNT:
			return PathDataType::Float;

		case wi::scene::AnimationComponent::AnimationChannel::Path::CAMERA_FOV:
		case wi::scene::AnimationComponent::AnimationChannel::Path::CAMERA_FOCAL_LENGTH:
		case wi::scene::AnimationComponent::AnimationChannel::Path::CAMERA_APERTURE_SIZE:
			return PathDataType::Float;
		case wi::scene::AnimationComponent::AnimationChannel::Path::CAMERA_APERTURE_SHAPE:
			return PathDataType::Float2;

		case wi::scene::AnimationComponent::AnimationChannel::Path::SCRIPT_PLAY:
		case wi::scene::AnimationComponent::AnimationChannel::Path::SCRIPT_STOP:
			return PathDataType::Event;

		case wi::scene::AnimationComponent::AnimationChannel::Path::MATERIAL_COLOR:
		case wi::scene::AnimationComponent::AnimationChannel::Path::MATERIAL_EMISSIVE:
		case wi::scene::AnimationComponent::AnimationChannel::Path::MATERIAL_TEXMULADD:
			return PathDataType::Float4;
		case wi::scene::AnimationComponent::AnimationChannel::Path::MATERIAL_ROUGHNESS:
		case wi::scene::AnimationComponent::AnimationChannel::Path::MATERIAL_REFLECTANCE:
		case wi::scene::AnimationComponent::AnimationChannel::Path::MATERIAL_METALNESS:
			return PathDataType::Float;

		default:
			assert(0);
			break;
		}
		return PathDataType::Event;
	}

	void SoftBodyPhysicsComponent::CreateFromMesh(const MeshComponent& mesh)
	{
		vertex_positions_simulation.resize(mesh.vertex_positions.size());
		vertex_tangents_tmp.resize(mesh.vertex_tangents.size());
		vertex_tangents_simulation.resize(mesh.vertex_tangents.size());

		XMMATRIX W = XMLoadFloat4x4(&worldMatrix);
		XMFLOAT3 _min = XMFLOAT3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		XMFLOAT3 _max = XMFLOAT3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
		for (size_t i = 0; i < vertex_positions_simulation.size(); ++i)
		{
			XMFLOAT3 pos = mesh.vertex_positions[i];
			XMStoreFloat3(&pos, XMVector3Transform(XMLoadFloat3(&pos), W));
			XMFLOAT3 nor = mesh.vertex_normals.empty() ? XMFLOAT3(1, 1, 1) : mesh.vertex_normals[i];
			XMStoreFloat3(&nor, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&nor), W)));
			const uint8_t wind = mesh.vertex_windweights.empty() ? 0xFF : mesh.vertex_windweights[i];
			vertex_positions_simulation[i].FromFULL(pos, nor, wind);
			_min = wi::math::Min(_min, pos);
			_max = wi::math::Max(_max, pos);
		}
		aabb = AABB(_min, _max);

		if (physicsToGraphicsVertexMapping.empty())
		{
			// Create a mapping that maps unique vertex positions to all vertex indices that share that. Unique vertex positions will make up the physics mesh:
			wi::unordered_map<size_t, uint32_t> uniquePositions;
			graphicsToPhysicsVertexMapping.resize(mesh.vertex_positions.size());
			physicsToGraphicsVertexMapping.clear();
			weights.clear();

			for (size_t i = 0; i < mesh.vertex_positions.size(); ++i)
			{
				const XMFLOAT3& position = mesh.vertex_positions[i];

				size_t hashes[] = {
					std::hash<float>{}(position.x),
					std::hash<float>{}(position.y),
					std::hash<float>{}(position.z),
				};
				size_t vertexHash = (((hashes[0] ^ (hashes[1] << 1) >> 1) ^ (hashes[2] << 1)) >> 1);

				if (uniquePositions.count(vertexHash) == 0)
				{
					uniquePositions[vertexHash] = (uint32_t)physicsToGraphicsVertexMapping.size();
					physicsToGraphicsVertexMapping.push_back((uint32_t)i);
				}
				graphicsToPhysicsVertexMapping[i] = uniquePositions[vertexHash];
			}

			weights.resize(physicsToGraphicsVertexMapping.size());
			std::fill(weights.begin(), weights.end(), 1.0f);
		}
	}

	void CameraComponent::CreatePerspective(float newWidth, float newHeight, float newNear, float newFar, float newFOV)
	{
		zNearP = newNear;
		zFarP = newFar;
		width = newWidth;
		height = newHeight;
		fov = newFOV;

		SetCustomProjectionEnabled(false);

		UpdateCamera();
	}
	void CameraComponent::UpdateCamera()
	{
		if (!IsCustomProjectionEnabled())
		{
			XMStoreFloat4x4(&Projection, XMMatrixPerspectiveFovLH(fov, width / height, zFarP, zNearP)); // reverse zbuffer!
			Projection.m[2][0] = jitter.x;
			Projection.m[2][1] = jitter.y;
		}

		XMVECTOR _Eye = XMLoadFloat3(&Eye);
		XMVECTOR _At = XMLoadFloat3(&At);
		XMVECTOR _Up = XMLoadFloat3(&Up);

		XMMATRIX _V = XMMatrixLookToLH(_Eye, _At, _Up);
		XMStoreFloat4x4(&View, _V);

		XMMATRIX _P = XMLoadFloat4x4(&Projection);
		XMMATRIX _InvP = XMMatrixInverse(nullptr, _P);
		XMStoreFloat4x4(&InvProjection, _InvP);

		XMMATRIX _VP = XMMatrixMultiply(_V, _P);
		XMStoreFloat4x4(&View, _V);
		XMStoreFloat4x4(&VP, _VP);
		XMMATRIX _InvV = XMMatrixInverse(nullptr, _V);
		XMStoreFloat4x4(&InvView, _InvV);
		XMStoreFloat3x3(&rotationMatrix, _InvV);
		XMStoreFloat4x4(&InvVP, XMMatrixInverse(nullptr, _VP));
		XMStoreFloat4x4(&Projection, _P);
		XMStoreFloat4x4(&InvProjection, XMMatrixInverse(nullptr, _P));

		frustum.Create(_VP);
	}
	void CameraComponent::TransformCamera(const XMMATRIX& W)
	{
		XMVECTOR _Eye = XMVector3Transform(XMVectorSet(0, 0, 0, 1), W);
		XMVECTOR _At = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), W));
		XMVECTOR _Up = XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), W));

		XMMATRIX _V = XMMatrixLookToLH(_Eye, _At, _Up);
		XMStoreFloat4x4(&View, _V);

		XMStoreFloat3x3(&rotationMatrix, XMMatrixInverse(nullptr, _V));

		XMStoreFloat3(&Eye, _Eye);
		XMStoreFloat3(&At, _At);
		XMStoreFloat3(&Up, _Up);
	}
	void CameraComponent::Reflect(const XMFLOAT4& plane)
	{
		XMVECTOR _Eye = XMLoadFloat3(&Eye);
		XMVECTOR _At = XMLoadFloat3(&At);
		XMVECTOR _Up = XMLoadFloat3(&Up);
		XMMATRIX _Ref = XMMatrixReflect(XMLoadFloat4(&plane));

		// reverse clipping if behind clip plane ("if underwater")
		clipPlane = plane;
		float d = XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&clipPlane), _Eye));
		if (d < 0)
		{
			clipPlane.x *= -1;
			clipPlane.y *= -1;
			clipPlane.z *= -1;
			clipPlane.w *= -1;
		}

		_Eye = XMVector3Transform(_Eye, _Ref);
		_At = XMVector3TransformNormal(_At, _Ref);
		_Up = XMVector3TransformNormal(_Up, _Ref);

		XMStoreFloat3(&Eye, _Eye);
		XMStoreFloat3(&At, _At);
		XMStoreFloat3(&Up, _Up);

		UpdateCamera();
	}
	void CameraComponent::Lerp(const CameraComponent& a, const CameraComponent& b, float t)
	{
		SetDirty();

		width = wi::math::Lerp(a.width, b.width, t);
		height = wi::math::Lerp(a.height, b.height, t);
		zNearP = wi::math::Lerp(a.zNearP, b.zNearP, t);
		zFarP = wi::math::Lerp(a.zFarP, b.zFarP, t);
		fov = wi::math::Lerp(a.fov, b.fov, t);
		focal_length = wi::math::Lerp(a.focal_length, b.focal_length, t);
		aperture_size = wi::math::Lerp(a.aperture_size, b.aperture_size, t);
		aperture_shape = wi::math::Lerp(a.aperture_shape, b.aperture_shape, t);
	}

	void ScriptComponent::CreateFromFile(const std::string& filename)
	{
		this->filename = filename;
		resource = wi::resourcemanager::Load(filename, wi::resourcemanager::Flags::IMPORT_RETAIN_FILEDATA);
		script.clear(); // will be created on first Update()
	}

	void SoundComponent::Play()
	{
		_flags |= PLAYING;
		wi::audio::Play(&soundinstance);
	}
	void SoundComponent::Stop()
	{
		_flags &= ~PLAYING;
		wi::audio::Stop(&soundinstance);
	}

}
