#pragma once

namespace sce
{
	namespace Gnm
	{
		const DataFormat kDataFormatBc1Unorm_Alpha = { { { kSurfaceFormatBc1, kBufferChannelTypeUNorm, kBufferChannelX,  kBufferChannelY,  kBufferChannelZ,  kBufferChannelW } } };
		const DataFormat kDataFormatB10G10R10A2UnormSrgb = { { { kSurfaceFormat2_10_10_10,  kTextureChannelTypeSrgb, kTextureChannelZ,  kTextureChannelY,  kTextureChannelX,  kTextureChannelW, 0 } } };
		
		inline bool operator==(const DataFormat &formatA, const DataFormat &formatB) {
			// return (0 == memcmp(&formatA, &formatB, sizeof(DataFormat)));
			return (formatA.m_asInt == formatB.m_asInt);
		}

		inline bool operator!=(const DataFormat &formatA, const DataFormat &formatB) {
			// return (0 != memcmp(&formatA, &formatB, sizeof(DataFormat)));
			return (formatA.m_asInt != formatB.m_asInt);
		}
	}
}

namespace Framework
{
	class EopEventQueue
	{
		SceKernelEqueue mEqueue;
		const char *mName;
	public:
		EopEventQueue(const char *name);
		~EopEventQueue();
		void waitForEvent();
	};

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

	inline SceVideoOutPixelFormat getVideoOutFormat(sce::Gnm::DataFormat format)
	{
		SceVideoOutPixelFormat ret = SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB;
		if (format == sce::Gnm::kDataFormatB8G8R8A8UnormSrgb)
		{
			//Format lining 8 bit unsigned int elements from LSB in the B, G, R, A order
			//The system handles buffers as rendered in a curve with gamma correction based on sRGB, and does not perform gamma correction during video output (however, allows adjustment using a gamma adjustment value)
			ret = SCE_VIDEO_OUT_PIXEL_FORMAT_A8R8G8B8_SRGB;
		}
		else if (format == sce::Gnm::kDataFormatR8G8B8A8UnormSrgb)
		{
			//Format lining 8 bit unsigned int elements from LSB in the R, G, B, A order
			//The system handles buffers as rendered in a curve with gamma correction based on sRGB, and does not perform gamma correction during video output (however, allows adjustment using a gamma adjustment value)
			ret = SCE_VIDEO_OUT_PIXEL_FORMAT_A8B8G8R8_SRGB;
		}
		else if (format == sce::Gnm::kDataFormatB10G10R10A2Unorm)
		{
			//Format lining unsigned int elements from LSB in the B 10 bit, G 10 bit, R 10 bit, A 2 bit order
			//The system handles buffers as rendered in a curve with gamma correction based on sRGB, and does not perform gamma correction during video output (however, allows adjustment using a gamma adjustment value)
			ret = SCE_VIDEO_OUT_PIXEL_FORMAT_A2R10G10B10;
		}
		else if (format == sce::Gnm::kDataFormatB10G10R10A2UnormSrgb)
		{
			//Format lining unsigned int elements from LSB in the B 10 bit, G 10 bit, R 10 bit, A 2 bit order
			//The system handles buffers as rendered in a curve with gamma correction based on sRGB, and does not perform gamma correction during video output (however, allows adjustment using a gamma adjustment value)
			ret = SCE_VIDEO_OUT_PIXEL_FORMAT_A2R10G10B10_SRGB;
		}
		else if (format == sce::Gnm::kDataFormatB16G16R16A16Float)
		{
			//Format lining 16 bit float elements from LSB in the B, G, R, A order
			//The system handles buffers as linearly rendered, and performs gamma correction during video output
			ret = SCE_VIDEO_OUT_PIXEL_FORMAT_A16R16G16B16_FLOAT;
		}
		else
		{
			SCE_GNM_ASSERT_MSG(false, "Not support yet");
		}
		return ret;
	}

	inline SceVideoOutTilingMode getVideoOutTileMode(sce::Gnm::TileMode tileMode)
	{
		SceVideoOutTilingMode ret = SCE_VIDEO_OUT_TILING_MODE_TILE;
		switch (tileMode)
		{
		case sce::Gnm::kTileModeDisplay_2dThin:
			ret = SCE_VIDEO_OUT_TILING_MODE_TILE; // Tile mode
			break;
		case sce::Gnm::kTileModeDisplay_LinearAligned:
			ret = SCE_VIDEO_OUT_TILING_MODE_LINEAR; // Linear mode
			break;
		default:
			SCE_GNM_ASSERT_MSG(false, "Not support yet");
			break;
		}
		return ret;
	}
// 
// 	inline sce::GpuAddress::SurfaceType getSurfaceTypeFromTextureType(sce::Gnm::TextureType type, bool isDynamic)
// 	{
// 		sce::GpuAddress::SurfaceType ret = sce::GpuAddress::kSurfaceTypeTextureFlat;
// 		switch (type)
// 		{
// 		case sce::Gnm::kTextureType1d:
// 		case sce::Gnm::kTextureType2d:
// 			ret = isDynamic ? sce::GpuAddress::kSurfaceTypeRwTextureFlat : sce::GpuAddress::kSurfaceTypeTextureFlat;
// 			break;
// 		case sce::Gnm::kTextureType3d:
// 			ret = isDynamic ? sce::GpuAddress::kSurfaceTypeRwTextureVolume : sce::GpuAddress::kSurfaceTypeTextureVolume;
// 			break;
// 		case sce::Gnm::kTextureTypeCubemap:
// 			ret = isDynamic ? sce::GpuAddress::kSurfaceTypeRwTextureCubemap : sce::GpuAddress::kSurfaceTypeTextureCubemap;
// 			break;
// 		default:
// 			SCE_GNM_ASSERT_MSG(false, "Not support yet");
// 			break;
// 		}
// 		return ret;
// 	}
}
