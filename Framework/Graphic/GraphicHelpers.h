#pragma once

namespace sce
{
	namespace Gnm
	{
		const DataFormat kDataFormatBc1Unorm_Alpha = { { { kSurfaceFormatBc1, kBufferChannelTypeUNorm, kBufferChannelX,  kBufferChannelY,  kBufferChannelZ,  kBufferChannelW } } };

		inline bool operator==(const DataFormat &formatA, const DataFormat &formatB) {
			return (0 == memcmp(&formatA, &formatB, sizeof(DataFormat)));
		}

		inline bool operator!=(const DataFormat &formatA, const DataFormat &formatB) {
			return (0 != memcmp(&formatA, &formatB, sizeof(DataFormat)));
		}
	}
}

namespace Framework
{
	inline U32 convertNumFragments(sce::Gnm::NumFragments fragments)
	{
		U32 ret = 0;
		switch (fragments)
		{
		case sce::Gnm::kNumFragments1:
			ret = 1;
			break;
		case sce::Gnm::kNumFragments2:
			ret = 2;
			break;
		case sce::Gnm::kNumFragments4:
			ret = 4;
			break;
		case sce::Gnm::kNumFragments8:
			ret = 8;
			break;
		default:
			SCE_GNM_ASSERT_MSG(false, "Not support yet");
			break;
		}
		return ret;
	}

	inline U32 convertNumSamples(sce::Gnm::NumSamples samples)
	{
		U32 ret = 0;
		switch (samples)
		{
		case sce::Gnm::kNumSamples1:
			ret = 1;
			break;
		case sce::Gnm::kNumSamples2:
			ret = 2;
			break;
		case sce::Gnm::kNumSamples4:
			ret = 4;
			break;
		case sce::Gnm::kNumSamples8:
			ret = 8;
			break;
		case sce::Gnm::kNumSamples16:
			ret = 16;
			break;
		default:
			SCE_GNM_ASSERT_MSG(false, "Not support yet");
			break;
		}
		return ret;
	}

	inline sce::Gnm::NumFragments getFragmentsFromAAType(AntiAliasingType type)
	{
		sce::Gnm::NumFragments ret = sce::Gnm::kNumFragments1;
		switch (type)
		{
		case AA_NONE:
			ret = sce::Gnm::kNumFragments1;
			break;
		case AA_EQAA_2X:
			ret = sce::Gnm::kNumFragments2;
			break;
		case AA_EQAA_4X:
			ret = sce::Gnm::kNumFragments4;
			break;
		default:
			SCE_GNM_ASSERT_MSG(false, "Not support yet");
			break;
		}
		return ret;
	}

	inline sce::Gnm::NumSamples getSamplesFromAAType(AntiAliasingType type)
	{
		sce::Gnm::NumSamples ret = sce::Gnm::kNumSamples1;
		switch (type)
		{
		case AA_NONE:
			ret = sce::Gnm::kNumSamples1;
			break;
		case AA_EQAA_2X:
			ret = sce::Gnm::kNumSamples4;
			break;
		case AA_EQAA_4X:
			ret = sce::Gnm::kNumSamples8;
			break;
		default:
			SCE_GNM_ASSERT_MSG(false, "Not support yet");
			break;
		}
		return ret;
	}

	inline sce::GpuAddress::SurfaceType getSurfaceTypeFromTextureType(sce::Gnm::TextureType type, bool isDynamic)
	{
		sce::GpuAddress::SurfaceType ret = sce::GpuAddress::kSurfaceTypeTextureFlat;
		switch (type)
		{
		case sce::Gnm::kTextureType1d:
		case sce::Gnm::kTextureType2d:
			ret = isDynamic ? sce::GpuAddress::kSurfaceTypeRwTextureFlat : sce::GpuAddress::kSurfaceTypeTextureFlat;
			break;
		case sce::Gnm::kTextureType3d:
			ret = isDynamic ? sce::GpuAddress::kSurfaceTypeRwTextureVolume : sce::GpuAddress::kSurfaceTypeTextureVolume;
			break;
		case sce::Gnm::kTextureTypeCubemap:
			ret = isDynamic ? sce::GpuAddress::kSurfaceTypeRwTextureCubemap : sce::GpuAddress::kSurfaceTypeTextureCubemap;
			break;
		default:
			SCE_GNM_ASSERT_MSG(false, "Not support yet");
			break;
		}
		return ret;
	}
}