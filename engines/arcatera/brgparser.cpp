#include "graphics/surface.h"
#include "common/file.h"

namespace Arcatera {

Graphics::Surface readBrgFile(Common::File &file) {
	const auto magic1 = file.readByte();
	const auto magic2 = file.readByte();

	assert(magic1 == 'B' && magic2 == 'R');

	const auto unknown1 = file.readUint32LE();

	const auto width = file.readUint32LE();
	const auto height = file.readUint32LE();

	const auto unknown2 = file.readUint32LE();
	const auto unknown3 = file.readUint32LE();
	const auto unknown4 = file.readUint16LE();

	assert(unknown1 == 24u);
	assert(unknown2 == 0u);
	assert(unknown3 == height);
	assert(unknown4 == 0u);

	Graphics::PixelFormat rgb565(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Graphics::Surface surface;
	surface.create(width, height, rgb565);

	for (uint32 i = 0; i < height; i++) {
		for (uint32 j = 0; j < width; j++) {
			surface.setPixel(j, i, file.readUint16LE());
		}
	}

	return surface;
}

} // namespace Arcatera
