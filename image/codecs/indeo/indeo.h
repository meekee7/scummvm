/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "graphics/managed_surface.h"
#include "image/codecs/codec.h"

/* Common structures, macros, and base class shared by both Indeo4 and 
 * Indeo5 decoders, derived from ffmpeg. We don't currently support Indeo5 
 * decoding, but just in case we eventually need it, this is kept as a separate
 * file like it is in ffmpeg.
 *
 * Original copyright note: * Intel Indeo 4 (IV41, IV42, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#ifndef IMAGE_CODECS_INDEO_INDEO_H
#define IMAGE_CODECS_INDEO_INDEO_H

#include "image/codecs/indeo/get_bits.h"
#include "image/codecs/indeo/vlc.h"

namespace Image {
namespace Indeo {

/**
 *  Indeo 4 frame types.
 */
enum {
    IVI4_FRAMETYPE_INTRA       = 0,
    IVI4_FRAMETYPE_INTRA1      = 1,  ///< intra frame with slightly different bitstream coding
    IVI4_FRAMETYPE_INTER       = 2,  ///< non-droppable P-frame
    IVI4_FRAMETYPE_BIDIR       = 3,  ///< bidirectional frame
    IVI4_FRAMETYPE_INTER_NOREF = 4,  ///< droppable P-frame
    IVI4_FRAMETYPE_NULL_FIRST  = 5,  ///< empty frame with no data
    IVI4_FRAMETYPE_NULL_LAST   = 6   ///< empty frame with no data
};

#define IVI_VLC_BITS 13		///< max number of bits of the ivi's huffman codes
#define IVI5_IS_PROTECTED 0x20

/**
 *  huffman codebook descriptor
 */
struct IVIHuffDesc {
    int32     num_rows;
    uint8     xbits[16];

	/*
	 *  Generate a huffman codebook from the given descriptor
	 *  and convert it into the FFmpeg VLC table.
	 *
	 *  @param[out]  vlc   where to place the generated VLC table
	 *  @param[in]   flag  flag: 1 - for static or 0 for dynamic tables
	 *  @return     result code: 0 - OK, -1 = error (invalid codebook descriptor)
	 */
	int ivi_create_huff_from_desc(VLC *vlc, int flag) const;

	/*
	 *  Compare two huffman codebook descriptors.
	 *
	 *  @param[in]  desc2  ptr to the 2nd descriptor to compare
	 *  @return         comparison result: 0 - equal, 1 - not equal
	 */
	bool ivi_huff_desc_cmp(const IVIHuffDesc *desc2) const;

	/*
	 *  Copy huffman codebook descriptors.
	 *
	 *  @param[in]   src  ptr to the source descriptor
	 */
	void ivi_huff_desc_copy(const IVIHuffDesc *src);
};

/**
 *  macroblock/block huffman table descriptor
 */
struct IVIHuffTab {
private:
	VLC_TYPE table_data[8192 * 16][2];
	VLC ivi_mb_vlc_tabs[8];  ///< static macroblock Huffman tables
	VLC ivi_blk_vlc_tabs[8]; ///< static block Huffman tables
public:
	int32	tab_sel;    /// index of one of the predefined tables
						/// or "7" for custom one
    VLC		*tab;       /// pointer to the table associated with tab_sel

    /// the following are used only when tab_sel == 7
    IVIHuffDesc cust_desc;  /// custom Huffman codebook descriptor
    VLC         cust_tab;   /// vlc table for custom codebook

	/**
	 * Constructor
	 */
	IVIHuffTab();

	int ff_ivi_dec_huff_desc(GetBits *gb, int desc_coded, int which_tab);
};

enum {
    IVI_MB_HUFF   = 0,      /// Huffman table is used for coding macroblocks
    IVI_BLK_HUFF  = 1       /// Huffman table is used for coding blocks
};


/**
 *  Common scan patterns (defined in ivi_common.c)
 */
//extern const uint8 ff_ivi_vertical_scan_8x8[64];
//extern const uint8 ff_ivi_horizontal_scan_8x8[64];
//extern const uint8 ff_ivi_direct_scan_4x4[16];


/**
 *  Declare inverse transform function types
 */
typedef void (InvTransformPtr)(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags);
typedef void (DCTransformPtr) (const int32 *in, int16 *out, uint32 pitch, int blk_size);


/**
 *  run-value (RLE) table descriptor
 */
struct RVMapDesc {
    uint8     eob_sym; ///< end of block symbol
    uint8     esc_sym; ///< escape symbol
    uint8     runtab[256];
    int8      valtab[256];
};

extern const RVMapDesc ff_ivi_rvmap_tabs[9];


/**
 *  information for Indeo macroblock (16x16, 8x8 or 4x4)
 */
struct IVIMbInfo {
    int16     xpos;
    int16     ypos;
    uint32    buf_offs; ///< address in the output buffer for this mb
    uint8     type;     ///< macroblock type: 0 - INTRA, 1 - INTER
    uint8     cbp;      ///< coded block pattern
    int8      q_delta;  ///< quant delta
    int8      mv_x;     ///< motion vector (x component)
    int8      mv_y;     ///< motion vector (y component)
    int8      b_mv_x;   ///< second motion vector (x component)
    int8      b_mv_y;   ///< second motion vector (y component)
};


/**
 *  information for Indeo tile
 */
struct IVITile {
    int         xpos;
    int         ypos;
    int         width;
    int         height;
    int         mb_size;
    int         is_empty;  ///< = 1 if this tile doesn't contain any data
    int         data_size; ///< size of the data in bytes
    int         num_MBs;   ///< number of macroblocks in this tile
    IVIMbInfo * mbs;      ///< array of macroblock descriptors
    IVIMbInfo * ref_mbs;  ///< ptr to the macroblock descriptors of the reference tile
};


/**
 *  information for Indeo wavelet band
 */
struct IVIBandDesc {
    int             plane;          ///< plane number this band belongs to
    int             band_num;       ///< band number
    int             width;
    int             height;
    int             aheight;        ///< aligned band height
    const uint8 *   data_ptr;       ///< ptr to the first byte of the band data
    int             data_size;      ///< size of the band data
    int16 *         buf;            ///< pointer to the output buffer for this band
    int16 *         ref_buf;        ///< pointer to the reference frame buffer (for motion compensation)
    int16 *         b_ref_buf;      ///< pointer to the second reference frame buffer (for motion compensation)
    int16 *         bufs[4];        ///< array of pointers to the band buffers
    int             pitch;          ///< pitch associated with the buffers above
    int             is_empty;       ///< = 1 if this band doesn't contain any data
    int             mb_size;        ///< macroblock size
    int             blk_size;       ///< block size
    int             is_halfpel;     ///< precision of the motion compensation: 0 - fullpel, 1 - halfpel
    int             inherit_mv;     ///< tells if motion vector is inherited from reference macroblock
    int             inherit_qdelta; ///< tells if quantiser delta is inherited from reference macroblock
    int             qdelta_present; ///< tells if Qdelta signal is present in the bitstream (Indeo5 only)
    int             quant_mat;      ///< dequant matrix index
    int             glob_quant;     ///< quant base for this band
    const uint8 *   scan;           ///< ptr to the scan pattern
    int             scan_size;      ///< size of the scantable

    IVIHuffTab      blk_vlc;        ///< vlc table for decoding block data

    int             num_corr;       ///< number of correction entries
    uint8           corr[61 * 2];   ///< rvmap correction pairs
    int             rvmap_sel;      ///< rvmap table selector
    RVMapDesc *     rv_map;         ///< ptr to the RLE table for this band
    int             num_tiles;      ///< number of tiles in this band
    IVITile *       tiles;          ///< array of tile descriptors
    InvTransformPtr *inv_transform;
    int             transform_size;
    DCTransformPtr  *dc_transform;
    int             is_2d_trans;    ///< 1 indicates that the two-dimensional inverse transform is used
    int32           checksum;       ///< for debug purposes
    int             checksum_present;
    int             bufsize;        ///< band buffer size in bytes
    const uint16 *  intra_base;     ///< quantization matrix for intra blocks
    const uint16 *  inter_base;     ///< quantization matrix for inter blocks
    const uint8 *   intra_scale;    ///< quantization coefficient for intra blocks
    const uint8 *   inter_scale;    ///< quantization coefficient for inter blocks

	int ivi_init_tiles(IVITile *ref_tile, int p, int b, int t_height, int t_width);
};

struct IVIPicConfig {
    uint16    pic_width;
    uint16    pic_height;
    uint16    chroma_width;
    uint16    chroma_height;
    uint16    tile_width;
    uint16    tile_height;
    uint8     luma_bands;
    uint8     chroma_bands;

	/**
	 * Compare some properties of two pictures
	 */
	bool ivi_pic_config_cmp(const IVIPicConfig &cfg2);
};

/**
 *  color plane (luma or chroma) information
 */
struct IVIPlaneDesc {
    uint16    width;
    uint16    height;
    uint8     num_bands;  ///< number of bands this plane subdivided into
    IVIBandDesc *bands;   ///< array of band descriptors

	static int ff_ivi_init_planes(IVIPlaneDesc *planes, const IVIPicConfig *cfg, bool is_indeo4);

	static int ff_ivi_init_tiles(IVIPlaneDesc *planes, int tile_width, int tile_height);

	/*
	 *  Free planes, bands and macroblocks buffers.
	 *
	 *  @param[in]  planes  pointer to the array of the plane descriptors
	 */
	static void ivi_free_buffers(IVIPlaneDesc *planes);
};

struct IVI45DecContext {
    GetBits *		gb;
    RVMapDesc       rvmap_tabs[9];   ///< local corrected copy of the static rvmap tables

    uint32          frame_num;
    int             frame_type;
    int             prev_frame_type; ///< frame type of the previous frame
    uint32          data_size;       ///< size of the frame data in bytes from picture header
    int             is_scalable;
    const uint8 *   frame_data;      ///< input frame data pointer
    int             inter_scal;      ///< signals a sequence of scalable inter frames
    uint32          frame_size;      ///< frame size in bytes
    uint32          pic_hdr_size;    ///< picture header size in bytes
    uint8           frame_flags;
    uint16          checksum;        ///< frame checksum

    IVIPicConfig    pic_conf;
    IVIPlaneDesc    planes[3];       ///< color planes

    int             buf_switch;      ///< used to switch between three buffers
    int             dst_buf;         ///< buffer index for the currently decoded frame
    int             ref_buf;         ///< inter frame reference buffer index
    int             ref2_buf;        ///< temporal storage for switching buffers
    int             b_ref_buf;       ///< second reference frame buffer index

    IVIHuffTab      mb_vlc;          ///< current macroblock table descriptor
    IVIHuffTab      blk_vlc;         ///< current block table descriptor

    uint8           rvmap_sel;
    uint8           in_imf;
    uint8           in_q;            ///< flag for explicitly stored quantiser delta
    uint8           pic_glob_quant;
    uint8           unknown1;

    uint16          gop_hdr_size;
    uint8           gop_flags;
    uint32          lock_word;

    int             show_indeo4_info;
    uint8           has_b_frames;
    uint8           has_transp;      ///< transparency mode status: 1 - enabled
    uint8           uses_tiling;
    uint8           uses_haar;
    uint8           uses_fullpel;

//    int             (*decode_pic_hdr)  (struct IVI45DecContext *ctx, AVCodecContext *avctx);
//    int             (*decode_band_hdr) (struct IVI45DecContext *ctx, IVIBandDesc *band, AVCodecContext *avctx);
//    int             (*decode_mb_info)  (struct IVI45DecContext *ctx, IVIBandDesc *band, IVITile *tile, AVCodecContext *avctx);
//    void            (*switch_buffers)  (struct IVI45DecContext *ctx);
//    int             (*is_nonnull_frame)(struct IVI45DecContext *ctx);

    int gop_invalid;
    int buf_invalid[4];

    int is_indeo4;

//    AVFrame *       p_frame;
    int             got_p_frame;
};

class IndeoDecoderBase : public Codec {
protected:
	IVI45DecContext _ctx;
	Graphics::PixelFormat _pixelFormat;
	Graphics::ManagedSurface *_surface;
protected:
	/**
	 * Returns the pixel format for the decoder's surface
	 */
	virtual Graphics::PixelFormat getPixelFormat() const { return _pixelFormat; }

	/**
	 * Decode the Indeo picture header.
	 * @returns		0 = Ok, negative number = error
	 */
	virtual int decodePictureHeader() = 0;

	/**
	 * Decodes the Indeo frame from the bit reader already 
	 * loaded into the context
	 */
	int decodeIndeoFrame();
public:
	IndeoDecoderBase(uint16 width, uint16 height);
	virtual ~IndeoDecoderBase();
};

/*------------------------------------------------------------------------*/

/**
 * Check if the given dimension of an image is valid, meaning that all
 * bytes of the image can be addressed with a signed int.
 *
 * @param w the width of the picture
 * @param h the height of the picture
 * @param log_offset the offset to sum to the log level for logging with log_ctx
 * @returns >= 0 if valid, a negative error code otherwise
*/
extern int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void *log_ctx);


} // End of namespace Indeo
} // End of namespace Image

#endif
