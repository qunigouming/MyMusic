#ifndef __IMGFMTINSPECTOR__H__
#define __IMGFMTINSPECTOR__H__

#include <string>

class ImgFmtInspector
{
	enum ImageFormat {
		UNKNOWN = 0,
		JPEG,
        PNG,
        GIF,
        BMP,
        TIFF,
        WEBP
	};
public:
	ImgFmtInspector() = default;
	~ImgFmtInspector() = default;

	static const std::string getImageExtension(const std::string& data);
	static const std::string getImageMimeType(const std::string& data);

private:
	static ImageFormat detectImageFormat(const std::string& data);
};

#endif