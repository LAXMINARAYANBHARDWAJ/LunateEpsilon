#pragma once

#include <QString>
#include <stdexcept>

namespace LE {

enum class LocationMode {
    Keep,
    Custom
};

struct ConversionParams {
    QString inputPath;
    QString outputPath;
    QString basePath;       // Required for M3U→M3U8; optional for M3U8→M3U (custom mode)
    LocationMode locationMode = LocationMode::Keep;
};

// Pure business logic. No QWidget dependencies. Throws std::runtime_error on failure.
class Converter {
public:
    Converter() = default;
    ~Converter() = default;

    Converter(const Converter&) = delete;
    Converter& operator=(const Converter&) = delete;

    void convert(const ConversionParams& params);

private:
    void convertM3uToM3u8(const ConversionParams& params);
    void convertM3u8ToM3u(const ConversionParams& params);

    static QString normalizePath(const QString& path);
    static QString stripLeadingMusicPrefix(const QString& line);
};

} // namespace LE