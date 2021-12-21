/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021 PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common/WindowInfo.h"
#include "GSFastList.h"
#include "GSTexture.h"
#include "GSVertex.h"
#include "GS/GSAlignedClass.h"

class HostDisplay;

enum ShaderConvert
{
	ShaderConvert_COPY = 0,
	ShaderConvert_RGBA8_TO_16_BITS,
	ShaderConvert_DATM_1,
	ShaderConvert_DATM_0,
	ShaderConvert_MOD_256,
	ShaderConvert_SCANLINE = 5,
	ShaderConvert_DIAGONAL_FILTER,
	ShaderConvert_TRANSPARENCY_FILTER,
	ShaderConvert_TRIANGULAR_FILTER,
	ShaderConvert_COMPLEX_FILTER,
	ShaderConvert_FLOAT32_TO_32_BITS = 10,
	ShaderConvert_FLOAT32_TO_RGBA8,
	ShaderConvert_FLOAT16_TO_RGB5A1,
	ShaderConvert_RGBA8_TO_FLOAT32 = 13,
	ShaderConvert_RGBA8_TO_FLOAT24,
	ShaderConvert_RGBA8_TO_FLOAT16,
	ShaderConvert_RGB5A1_TO_FLOAT16,
	ShaderConvert_RGBA_TO_8I = 17,
	ShaderConvert_YUV,
	ShaderConvert_OSD,
	ShaderConvert_Count
};

enum ChannelFetch
{
	ChannelFetch_NONE  = 0,
	ChannelFetch_RED   = 1,
	ChannelFetch_GREEN = 2,
	ChannelFetch_BLUE  = 3,
	ChannelFetch_ALPHA = 4,
	ChannelFetch_RGB   = 5,
	ChannelFetch_GXBY  = 6,
};

#pragma pack(push, 1)

class MergeConstantBuffer
{
public:
	GSVector4 BGColor;

	MergeConstantBuffer() { memset(this, 0, sizeof(*this)); }
};

class InterlaceConstantBuffer
{
public:
	GSVector2 ZrH;
	float hH;
	float _pad[1];

	InterlaceConstantBuffer() { memset(this, 0, sizeof(*this)); }
};

class ExternalFXConstantBuffer
{
public:
	GSVector2 xyFrame;
	GSVector4 rcpFrame;
	GSVector4 rcpFrameOpt;

	ExternalFXConstantBuffer() { memset(this, 0, sizeof(*this)); }
};

class FXAAConstantBuffer
{
public:
	GSVector4 rcpFrame;
	GSVector4 rcpFrameOpt;

	FXAAConstantBuffer() { memset(this, 0, sizeof(*this)); }
};

class ShadeBoostConstantBuffer
{
public:
	GSVector4 rcpFrame;
	GSVector4 rcpFrameOpt;

	ShadeBoostConstantBuffer() { memset(this, 0, sizeof(*this)); }
};

#pragma pack(pop)

enum HWBlendFlags
{
	// A couple of flag to determine the blending behavior
	BLEND_A_MAX  = 0x100, // Impossible blending uses coeff bigger than 1
	BLEND_C_CLR  = 0x200, // Clear color blending (use directly the destination color as blending factor)
	BLEND_NO_REC = 0x400, // Doesn't require sampling of the RT as a texture
	BLEND_ACCU   = 0x800, // Allow to use a mix of SW and HW blending to keep the best of the 2 worlds
};

// Determines the HW blend function for DX11/OGL
struct HWBlend
{
	u16 flags, op, src, dst;
};

struct GSHWDrawConfig
{
	enum class Topology: u8
	{
		Point,
		Line,
		Triangle,
	};
	enum class GSTopology: u8
	{
		Point,
		Line,
		Triangle,
		Sprite,
	};
	struct GSSelector
	{
		union
		{
			struct
			{
				GSTopology topology : 2;
				bool expand : 1;
				bool iip : 1;
			};
			u8 key;
		};
		GSSelector(): key(0) {}
		GSSelector(u8 k): key(k) {}
	};
	struct VSSelector
	{
		union
		{
			struct
			{
				u8 fst : 1;
				u8 tme : 1;
				u8 _free : 6;
			};
			u8 key;
		};
		VSSelector(): key(0) {}
		VSSelector(u8 k): key(k) {}
	};
	struct PSSelector
	{
		// Performance note: there are too many shader combinations
		// It might hurt the performance due to frequent toggling worse it could consume
		// a lots of memory.
		union
		{
			struct
			{
				// *** Word 1
				// Format
				u32 aem_fmt   : 2;
				u32 pal_fmt   : 2;
				u32 dfmt      : 2; // 0 → 32-bit, 1 → 24-bit, 2 → 16-bit
				u32 depth_fmt : 2; // 0 → None, 1 → 32-bit, 2 → 16-bit, 3 → RGBA
				// Alpha extension/Correction
				u32 aem : 1;
				u32 fba : 1;
				// Fog
				u32 fog : 1;
				// Flat/goround shading
				u32 iip : 1;
				// Pixel test
				u32 date : 3;
				u32 atst : 3;
				// Color sampling
				u32 fst : 1; // Investigate to do it on the VS
				u32 tfx : 3;
				u32 tcc : 1;
				u32 wms : 2;
				u32 wmt : 2;
				u32 ltf : 1;
				// Shuffle and fbmask effect
				u32 shuffle  : 1;
				u32 read_ba  : 1;
				u32 write_rg : 1;
				u32 fbmask   : 1;

				//u32 _free1:0;

				// *** Word 2
				// Blend and Colclip
				u32 blend_a : 2;
				u32 blend_b : 2;
				u32 blend_c : 2;
				u32 blend_d : 2;
				u32 clr1    : 1; // useful?
				u32 hdr     : 1;
				u32 colclip : 1;
				u32 pabe    : 1;

				// Others ways to fetch the texture
				u32 channel : 3;

				// Dithering
				u32 dither : 2;

				// Depth clamp
				u32 zclamp : 1;

				// Hack
				u32 tcoffsethack : 1;
				u32 urban_chaos_hle : 1;
				u32 tales_of_abyss_hle : 1;
				u32 tex_is_fb : 1; // Jak Shadows
				u32 automatic_lod : 1;
				u32 manual_lod : 1;
				u32 point_sampler : 1;
				u32 invalid_tex0 : 1; // Lupin the 3rd

				u32 _free2 : 6;
			};

			u64 key;
		};
		PSSelector(): key(0) {}
	};
	struct SamplerSelector
	{
		union
		{
			struct
			{
				u8 tau   : 1;
				u8 tav   : 1;
				u8 biln  : 1;
				u8 triln : 3;
				u8 aniso : 1;

				u8 _free : 1;
			};
			u8 key;
		};
		SamplerSelector(): key(0) {}
		SamplerSelector(u32 k): key(k) {}
		static SamplerSelector Point() { return SamplerSelector(); }
		static SamplerSelector Linear()
		{
			SamplerSelector out;
			out.biln = 1;
			return out;
		}
	};
	struct DepthStencilSelector
	{
		union
		{
			struct
			{
				u8 ztst : 2;
				u8 zwe  : 1;
				u8 date : 1;
				u8 date_one : 1;

				u8 _free : 3;
			};
			u8 key;
		};
		DepthStencilSelector(): key(0) {}
		DepthStencilSelector(u32 k): key(k) {}
		static DepthStencilSelector NoDepth()
		{
			DepthStencilSelector out;
			out.ztst = ZTST_ALWAYS;
			return out;
		}
	};
	struct ColorMaskSelector
	{
		union
		{
			struct
			{
				u8 wr : 1;
				u8 wg : 1;
				u8 wb : 1;
				u8 wa : 1;

				u8 _free : 4;
			};
			struct
			{
				u8 wrgba : 4;
			};
			u8 key;
		};
		ColorMaskSelector(): key(0xF) {}
		ColorMaskSelector(u32 c): key(0) { wrgba = c; }
	};
	struct alignas(16) VSConstantBuffer
	{
		GSVector2 vertex_scale;
		GSVector2 vertex_offset;
		GSVector2 texture_scale;
		GSVector2 texture_offset;
		GSVector2 point_size;
		GSVector2i max_depth;
		__fi VSConstantBuffer()
		{
			memset(this, 0, sizeof(*this));
		}
		__fi VSConstantBuffer(const VSConstantBuffer& other)
		{
			memcpy(this, &other, sizeof(*this));
		}
		__fi VSConstantBuffer& operator=(const VSConstantBuffer& other)
		{
			new (this) VSConstantBuffer(other);
			return *this;
		}
		__fi bool operator==(const VSConstantBuffer& other) const
		{
			return BitEqual(*this, other);
		}
		__fi bool operator!=(const VSConstantBuffer& other) const
		{
			return !(*this == other);
		}
		__fi bool Update(const VSConstantBuffer& other)
		{
			if (*this == other)
				return false;

			memcpy(this, &other, sizeof(*this));
			return true;
		}
	};
	struct alignas(16) PSConstantBuffer
	{
		GSVector4 FogColor_AREF;
		GSVector4 WH;
		GSVector4 TA_MaxDepth_Af;
		GSVector4i MskFix;
		GSVector4i FbMask;

		GSVector4 HalfTexel;
		GSVector4 MinMax;
		GSVector4i ChannelShuffle;
		GSVector2 TCOffsetHack;
		float pad1[2];

		GSVector4 DitherMatrix[4];

		__fi PSConstantBuffer()
		{
			memset(this, 0, sizeof(*this));
		}
		__fi PSConstantBuffer(const PSConstantBuffer& other)
		{
			memcpy(this, &other, sizeof(*this));
		}
		__fi PSConstantBuffer& operator=(const PSConstantBuffer& other)
		{
			new (this) PSConstantBuffer(other);
			return *this;
		}
		__fi bool operator==(const PSConstantBuffer& other) const
		{
			return BitEqual(*this, other);
		}
		__fi bool operator!=(const PSConstantBuffer& other) const
		{
			return !(*this == other);
		}
		__fi bool Update(const PSConstantBuffer& other)
		{
			if (*this == other)
				return false;

			memcpy(this, &other, sizeof(*this));
			return true;
		}
	};
	struct BlendState
	{
		union
		{
			struct
			{
				u8 index;
				u8 factor;
				bool is_constant     : 1;
				bool is_accumulation : 1;
				bool is_mixed_hw_sw  : 1;
			};
			u32 key;
		};
		BlendState(): key(0) {}
		BlendState(u8 index, u8 factor, bool is_constant, bool is_accumulation, bool is_mixed_hw_sw)
			: key(0)
		{
			this->index = index;
			this->factor = factor;
			this->is_constant = is_constant;
			this->is_accumulation = is_accumulation;
			this->is_mixed_hw_sw = is_mixed_hw_sw;
		}
	};
	enum class DestinationAlphaMode : u8
	{
		Off,            ///< No destination alpha test
		Stencil,        ///< Emulate using read-only stencil
		StencilOne,     ///< Emulate using read-write stencil (first write wins)
		PrimIDTracking, ///< Emulate by tracking the primitive ID of the last pixel allowed through
		Full,           ///< Full emulation (using barriers / ROV)
	};

	GSTexture* rt;        ///< Render target
	GSTexture* ds;        ///< Depth stencil
	GSTexture* tex;       ///< Source texture
	GSTexture* pal;       ///< Palette texture
	GSTexture* raw_tex;   ///< Used by channel shuffles
	GSVertex* verts;      ///< Vertices to draw
	u32* indices;         ///< Indices to draw
	u32 nverts;           ///< Number of vertices
	u32 nindices;         ///< Number of indices
	u32 indices_per_prim; ///< Number of indices that make up one primitive
	const std::vector<size_t>* drawlist; ///< For reducing barriers on sprites
	GSVector4i scissor; ///< Scissor rect
	Topology topology;  ///< Draw topology

	GSSelector gs;
	VSSelector vs;
	PSSelector ps;

	BlendState blend;
	SamplerSelector sampler;
	ColorMaskSelector colormask;
	DepthStencilSelector depth;

	bool require_one_barrier;  ///< Require texture barrier before draw (also used to requst an rt copy if texture barrier isn't supported)
	bool require_full_barrier; ///< Require texture barrier between all prims

	DestinationAlphaMode destination_alpha;
	bool datm;

	VSConstantBuffer cb_vs;
	PSConstantBuffer cb_ps;

	struct AlphaSecondPass
	{
		bool enable;
		ColorMaskSelector colormask;
		DepthStencilSelector depth;
		PSSelector ps;
		PSConstantBuffer cb_ps;
	} alpha_second_pass;
};

class GSDevice : public GSAlignedClass<32>
{
private:
	FastList<GSTexture*> m_pool;
	static std::array<HWBlend, 3*3*3*3 + 1> m_blendMap;

protected:
	enum : uint16
	{
		// HW blend factors
		SRC_COLOR,   INV_SRC_COLOR,    DST_COLOR,  INV_DST_COLOR,
		SRC1_COLOR,  INV_SRC1_COLOR,   SRC_ALPHA,  INV_SRC_ALPHA,
		DST_ALPHA,   INV_DST_ALPHA,    SRC1_ALPHA, INV_SRC1_ALPHA,
		CONST_COLOR, INV_CONST_COLOR,  CONST_ONE,  CONST_ZERO,

		// HW blend operations
		OP_ADD, OP_SUBTRACT, OP_REV_SUBTRACT
	};

	static const int m_NO_BLEND = 0;
	static const int m_MERGE_BLEND = m_blendMap.size() - 1;

	static constexpr u32 MAX_POOLED_TEXTURES = 300;

	HostDisplay* m_display;
	GSTexture* m_merge;
	GSTexture* m_weavebob;
	GSTexture* m_blend;
	GSTexture* m_target_tmp;
	GSTexture* m_current;
	struct
	{
		size_t stride, start, count, limit;
	} m_vertex;
	struct
	{
		size_t start, count, limit;
	} m_index;
	unsigned int m_frame; // for ageing the pool
	bool m_rbswapped;
	bool m_prefer_new_textures;

	virtual GSTexture* CreateSurface(int type, int w, int h, int format) = 0;
	virtual GSTexture* FetchSurface(int type, int w, int h, int format);

	virtual void DoMerge(GSTexture* sTex[3], GSVector4* sRect, GSTexture* dTex, GSVector4* dRect, const GSRegPMODE& PMODE, const GSRegEXTBUF& EXTBUF, const GSVector4& c) = 0;
	virtual void DoInterlace(GSTexture* sTex, GSTexture* dTex, int shader, bool linear, float yoffset) = 0;
	virtual void DoFXAA(GSTexture* sTex, GSTexture* dTex) {}
	virtual void DoShadeBoost(GSTexture* sTex, GSTexture* dTex) {}
	virtual void DoExternalFX(GSTexture* sTex, GSTexture* dTex) {}
	virtual uint16 ConvertBlendEnum(uint16 generic) = 0; // Convert blend factors/ops from the generic enum to DX11/OGl specific.

public:
	GSDevice();
	virtual ~GSDevice();

	__fi HostDisplay* GetDisplay() const { return m_display; }
	__fi unsigned int GetFrameNumber() const { return m_frame; }

	void Recycle(GSTexture* t);

	enum
	{
		Windowed,
		Fullscreen,
		DontCare
	};

	virtual bool Create(HostDisplay* display);
	virtual void Destroy();

	virtual void ResetAPIState();
	virtual void RestoreAPIState();

	virtual void BeginScene() {}
	virtual void DrawPrimitive() {};
	virtual void DrawIndexedPrimitive() {}
	virtual void DrawIndexedPrimitive(int offset, int count) {}
	virtual void EndScene();

	virtual bool HasDepthSparse() { return false; }
	virtual bool HasColorSparse() { return false; }

	virtual void ClearRenderTarget(GSTexture* t, const GSVector4& c) {}
	virtual void ClearRenderTarget(GSTexture* t, uint32 c) {}
	virtual void ClearDepth(GSTexture* t) {}
	virtual void ClearStencil(GSTexture* t, uint8 c) {}

	GSTexture* CreateSparseRenderTarget(int w, int h, int format = 0);
	GSTexture* CreateSparseDepthStencil(int w, int h, int format = 0);
	GSTexture* CreateRenderTarget(int w, int h, int format = 0);
	GSTexture* CreateDepthStencil(int w, int h, int format = 0);
	GSTexture* CreateTexture(int w, int h, int format = 0);
	GSTexture* CreateOffscreen(int w, int h, int format = 0);

	virtual GSTexture* CopyOffscreen(GSTexture* src, const GSVector4& sRect, int w, int h, int format = 0, int ps_shader = 0) { return NULL; }

	virtual void CopyRect(GSTexture* sTex, GSTexture* dTex, const GSVector4i& r) {}
	virtual void StretchRect(GSTexture* sTex, const GSVector4& sRect, GSTexture* dTex, const GSVector4& dRect, int shader = 0, bool linear = true) {}
	virtual void StretchRect(GSTexture* sTex, const GSVector4& sRect, GSTexture* dTex, const GSVector4& dRect, bool red, bool green, bool blue, bool alpha) {}

	void StretchRect(GSTexture* sTex, GSTexture* dTex, const GSVector4& dRect, int shader = 0, bool linear = true);

	virtual void PSSetShaderResources(GSTexture* sr0, GSTexture* sr1) {}
	virtual void PSSetShaderResource(int i, GSTexture* sRect) {}
	virtual void OMSetRenderTargets(GSTexture* rt, GSTexture* ds, const GSVector4i* scissor = NULL) {}

	__fi GSTexture* GetCurrent() { return m_current; }

	void Merge(GSTexture* sTex[3], GSVector4* sRect, GSVector4* dRect, const GSVector2i& fs, const GSRegPMODE& PMODE, const GSRegEXTBUF& EXTBUF, const GSVector4& c);
	void Interlace(const GSVector2i& ds, int field, int mode, float yoffset);
	void FXAA();
	void ShadeBoost();
	void ExternalFX();

	bool ResizeTexture(GSTexture** t, int type, int w, int h);
	bool ResizeTexture(GSTexture** t, int w, int h);
	bool ResizeTarget(GSTexture** t, int w, int h);
	bool ResizeTarget(GSTexture** t);

	bool IsRBSwapped() { return m_rbswapped; }

	void AgePool();
	void PurgePool();

	virtual void PrintMemoryUsage();

	// Convert the GS blend equations to HW specific blend factors/ops
	// Index is computed as ((((A * 3 + B) * 3) + C) * 3) + D. A, B, C, D taken from ALPHA register.
	HWBlend GetBlend(size_t index);
	uint16 GetBlendFlags(size_t index);
};

struct GSAdapter
{
	uint32 vendor;
	uint32 device;
	uint32 subsys;
	uint32 rev;

	operator std::string() const;
	bool operator==(const GSAdapter&) const;
	bool operator==(const std::string& s) const
	{
		return (std::string)*this == s;
	}
	bool operator==(const char* s) const
	{
		return (std::string)*this == s;
	}

#ifdef _WIN32
	GSAdapter(const DXGI_ADAPTER_DESC1& desc_dxgi);
#endif
#ifdef __linux__
	// TODO
#endif
};
