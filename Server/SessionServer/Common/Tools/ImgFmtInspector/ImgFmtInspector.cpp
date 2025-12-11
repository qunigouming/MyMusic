#include "ImgFmtInspector.h"
#include "../../../LogManager.h"

const std::string ImgFmtInspector::getImageExtension(const std::string& data)
{
	ImageFormat format = detectImageFormat(data);

	switch (format) {
	case JPEG: return "jpg";
	case PNG: return "png";
	case GIF: return "gif";
	case BMP: return "bmp";
	case TIFF: return "tiff";
	case WEBP: return "webp";
	default:
		LOG(ERROR) << "Unknown image format";
		return "";
	}
}

const std::string ImgFmtInspector::getImageMimeType(const std::string& data)
{
    ImageFormat format = detectImageFormat(data);
    switch (format) {
    case JPEG: return "image/jpeg";
    case PNG: return "image/png";
    case GIF: return "image/gif";
    case BMP: return "image/bmp";
    case TIFF: return "image/tiff";
    case WEBP: return "image/webp";
    default:
        LOG(ERROR) << "Unknown image format";
        return "";
    }
}

ImgFmtInspector::ImageFormat ImgFmtInspector::detectImageFormat(const std::string& data)
{
	if (data.size() < 12) {
		return UNKNOWN;
	}

	const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.data());
	size_t size = data.size();

	// 检查JPEG (FF D8 FF)
	if (size >= 3 &&
		buffer[0] == 0xFF &&
		buffer[1] == 0xD8 &&
		buffer[2] == 0xFF) {
		return JPEG;
	}

	// 检查PNG (\x89PNG\r\n\x1a\n)
	if (size >= 8 &&
		buffer[0] == 0x89 && buffer[1] == 'P' && buffer[2] == 'N' && buffer[3] == 'G' &&
		buffer[4] == 0x0D && buffer[5] == 0x0A && buffer[6] == 0x1A && buffer[7] == 0x0A) {
		return PNG;
	}

	// 检查GIF (GIF87a or GIF89a)
	if (size >= 6 &&
		buffer[0] == 'G' && buffer[1] == 'I' && buffer[2] == 'F' &&
		buffer[3] == '8' && (buffer[4] == '7' || buffer[4] == '9') && buffer[5] == 'a') {
		return GIF;
	}

	// 检查BMP (BM)
	if (size >= 2 && buffer[0] == 'B' && buffer[1] == 'M') {
		return BMP;
	}

	// 检查TIFF
	if (size >= 4) {
		if ((buffer[0] == 'I' && buffer[1] == 'I' && buffer[2] == 0x2A && buffer[3] == 0x00) ||
			(buffer[0] == 'M' && buffer[1] == 'M' && buffer[2] == 0x00 && buffer[3] == 0x2A)) {
			return TIFF;
		}
	}

	// 检查WebP (RIFFxxxxWEBP)
	if (size >= 12 &&
		memcmp(buffer, "RIFF", 4) == 0 &&
		memcmp(buffer + 8, "WEBP", 4) == 0) {
		return WEBP;
	}
	return UNKNOWN;
}
