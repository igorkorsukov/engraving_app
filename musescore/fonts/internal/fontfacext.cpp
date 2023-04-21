#include "fontfacext.hpp"

#include <limits>
#include <algorithm>

// mu
#include "global/serialization/zipreader.h"
#include "global/stringutils.h"
#include "global/io/buffer.h"
#include "global/io/fileinfo.h"
#include "global/containers.h"

// xtz
//#include "xtz_global/runtime.hpp"

#include "log.h"

using namespace xtz::fonts;

FontFaceXT::FontFaceXT()
{
}

FontFaceXT::~FontFaceXT()
{
    delete m_zip;
}

bool FontFaceXT::load(const FaceKey& key, const mu::io::path_t& path, bool isSymbolMode)
{
    m_key = key;
    m_isSymbolMode = isSymbolMode;

    m_zip = new mu::ZipReader(path);
    if (!m_zip->exists()) {
        LOGE() << "not exists: " << path;
        return false;
    }

    // meta
    {
        mu::ByteArray metaData = m_zip->fileData("meta.txt");
        if (metaData.empty()) {
            LOGE() << "meta is empty";
            return false;
        }

        std::string meta(metaData.constChar(), metaData.size());
        std::vector<std::string> metas;
        mu::strings::split(meta, metas, "\n");

        std::string ver;
        std::string glyphs;
        for (const std::string& p : metas) {
            size_t i = p.find(':');
            if (i == std::string::npos) {
                LOGE() << "failed parse param: " << p;
                continue;
            }

            std::string name = p.substr(0, i);
            std::string valStr = p.substr(i + 1);

            if (name == "version") {
                ver = valStr;
            } else if (name == "glyphs") {
                glyphs = valStr;
            } else if (name == "leading") {
                m_leading = std::stol(valStr);
            } else if (name == "ascent") {
                m_ascent = std::stol(valStr);
            } else if (name == "descent") {
                m_descent = std::stol(valStr);
            } else if (name == "xHeight") {
                m_xHeight = std::stol(valStr);
            } else {
                LOGW() << "unknown param: " << name;
            }
        }

        LOGI() << "fxt version: " << ver << ", glyphs: " << glyphs << ", path: " << path;
    }

    // ligatures
    mu::ByteArray ligaturesData = m_zip->fileData("ligatures.txt");
    if (!ligaturesData.empty()) {
        std::string ligaturesStr(ligaturesData.constChar(), ligaturesData.size());
        std::vector<std::string> ligatureStrs;
        mu::strings::split(ligaturesStr, ligatureStrs, "\n");

        for (const std::string& str : ligatureStrs) {
            if (str.empty()) {
                continue;
            }

            size_t sepIdx = str.find('=');
            if (sepIdx == std::string::npos) {
                continue;
            }

            std::string valStr = str.substr(0, sepIdx);
            std::string keyStr = str.substr(sepIdx + 1);
            std::vector<std::string> keyStrs;
            mu::strings::split(keyStr, keyStrs, " ");

            Ligature l;
            l.first = std::stoi(valStr);
            for (const std::string& k : keyStrs) {
                if (k.empty()) {
                    continue;
                }
                l.second.push_back(std::stoi(k));
            }

            m_ligatures.push_back(l);
        }

        std::sort(m_ligatures.begin(), m_ligatures.end(), [](const Ligature& l1, const Ligature& l2) {
            return l1.second.size() > l2.second.size();
        });
    }

    return true;
}

const FaceKey& FontFaceXT::key() const
{
    return m_key;
}

bool FontFaceXT::isSymbolMode() const
{
    return m_isSymbolMode;
}

f26dot6_t FontFaceXT::leading() const
{
    return m_leading;
}

f26dot6_t FontFaceXT::ascent() const
{
    return m_ascent;
}

f26dot6_t FontFaceXT::descent() const
{
    return m_descent;
}

f26dot6_t FontFaceXT::xHeight() const
{
    return m_xHeight;
}

void FontFaceXT::applyLigatures(std::vector<char32_t>& text, const Ligatures& ls)
{
    for (const Ligature& l : ls) {
        if (l.second.size() > text.size()) {
            continue;
        }

        while (1) {
            auto it = std::search(text.begin(), text.end(), l.second.begin(), l.second.end());
            if (it != text.end()) {
                *it = l.first;
                for (size_t i = 1; i < l.second.size(); ++i) {
                    ++it;
                    *it = 0;
                }
            } else {
                break;
            }
        }
    }
}

std::vector<GlyphPos> FontFaceXT::glyphs(const char32_t* text, int text_length) const
{
    std::vector<GlyphPos> result;

    std::vector<char32_t> data(text, text + text_length);

    applyLigatures(data, m_ligatures);

    for (char32_t ch : data) {
        if (ch == 0) {
            continue;
        }

        GlyphPos p;
        p.idx = glyphIndex(ch);
        if (p.idx == 0) {
            LOGE() << "not found glyph: " << (int)ch;
        }

        p.x_advance = glyphAdvance(p.idx);

        result.push_back(std::move(p));
    }

    return result;
}

glyph_idx_t FontFaceXT::glyphIndex(char32_t ucs4) const
{
    if (!mu::contains(chars(), ucs4)) {
        return 0;
    }
    return static_cast<glyph_idx_t>(ucs4);
}

char32_t FontFaceXT::findCharCode(glyph_idx_t idx) const
{
    char32_t ch = static_cast<char32_t>(idx);
    if (!mu::contains(chars(), ch)) {
        return 0;
    }
    return ch;
}

FBBox FontFaceXT::glyphBbox(glyph_idx_t idx) const
{
    return m_isSymbolMode ? glyphData(idx).symBbox : glyphData(idx).textBbox;
}

f26dot6_t FontFaceXT::glyphAdvance(glyph_idx_t idx) const
{
    return m_isSymbolMode ? glyphData(idx).symAdvance : glyphData(idx).textAdvance;
}

const msdfgen::Shape& FontFaceXT::glyphShape(glyph_idx_t idx) const
{
    return glyphData(idx).shape;
}

const std::set<char32_t>& FontFaceXT::chars() const
{
    if (!m_chars.empty()) {
        return m_chars;
    }

    std::vector<mu::ZipReader::FileInfo> files = m_zip->fileInfoList();
    for (const mu::ZipReader::FileInfo& fi : files) {
        mu::String name = mu::io::FileInfo(fi.filePath).baseName();
        if (name.empty()) {
            continue;
        }

        if (!name.at(0).isDigit()) {
            continue;
        }

        bool ok = false;
        int code = name.toInt(&ok);
        if (ok) {
            m_chars.insert(static_cast<char32_t>(code));
        }
    }

    return m_chars;
}

const FontFaceXT::GlyphData& FontFaceXT::glyphData(glyph_idx_t idx) const
{
    auto it = m_cache.find(idx);
    if (it != m_cache.end()) {
        return it->second;
    }

    mu::ByteArray data = m_zip->fileData(std::to_string(idx));
    mu::io::Buffer buf(&data);
    buf.open(mu::io::IODevice::ReadOnly);

    std::pair<glyph_idx_t, GlyphData> v;
    v.first = idx;
    v.second.read(&buf);
    return m_cache.insert(std::move(v)).first->second;
}

namespace {
struct Package {
    struct Point
    {
        Point() = default;
        Point(Point&& o) = default;
        int16_t x = 0;
        int16_t y = 0;
    };

    struct Edge {
        Edge() = default;
        Edge(Edge&& o) = default;
        int8_t type = 0;
        std::vector<Point> points;
    };

    struct Contour {
        Contour() = default;
        Contour(Contour&& o) = default;
        std::vector<Edge> edges;
    };

    struct Metrics {
        Metrics() = default;
        Metrics(Metrics&& o) = default;
        int16_t textBbox_x = 0;
        int16_t textBbox_y = 0;
        int16_t textBbox_w = 0;
        int16_t textBbox_h = 0;
        int16_t textAdvance = 0;
        int16_t symBbox_x = 0;
        int16_t symBbox_y = 0;
        int16_t symBbox_w = 0;
        int16_t symBbox_h = 0;
        int16_t symAdvance = 0;
    };

    struct Shape {
        Shape() = default;
        Shape(Shape&& o) = default;
        int8_t inverseYAxis = 0;
        int8_t fillRule = 0;
        std::vector<Contour> contours;
    };

    Package() = default;
    Package(Package&& o) = default;
    Metrics metrics;
    Shape shape;

    void print()
    {
        std::cout << "metrics:\n";
        std::cout << "  textBbox_x: " << metrics.textBbox_x << "\n";
        std::cout << "  textBbox_y: " << metrics.textBbox_y << "\n";
        std::cout << "  textBbox_w: " << metrics.textBbox_w << "\n";
        std::cout << "  textBbox_h: " << metrics.textBbox_h << "\n";
        std::cout << "  textAdvance: " << metrics.textAdvance << "\n";

        std::cout << "  symBbox_x: " << metrics.symBbox_x << "\n";
        std::cout << "  symBbox_y: " << metrics.symBbox_y << "\n";
        std::cout << "  symBbox_w: " << metrics.symBbox_w << "\n";
        std::cout << "  symBbox_h: " << metrics.symBbox_h << "\n";
        std::cout << "  symAdvance: " << metrics.symAdvance << "\n";

        std::cout << "shape:\n";
        std::cout << "  inverseYAxis: " << int(shape.inverseYAxis) << "\n";
        std::cout << "  fillRule: " << int(shape.fillRule) << "\n";
        std::cout << "  contours:\n";
        for (const Contour& c : shape.contours) {
            std::cout << "    edges:\n";
            for (const Edge& e : c.edges) {
                std::cout << "      type: " << int(e.type) << "\n";
                std::cout << "      points:\n";
                for (const Point& p : e.points) {
                    std::cout << "        x: " << p.x << ", y: " << p.y << "\n";
                }
            }
        }
        std::cout << std::endl;
    }
};
}

template<typename T, typename V>
static void set_value(T& p, const V& v)
{
    assert(std::numeric_limits<T>::max() > v);
    p = static_cast<T>(v);
}

static void set_points(std::vector<Package::Point>& out, const msdfgen::Point2* in, int count)
{
    out.reserve(count);
    for (int i = 0; i < count; ++i) {
        Package::Point p;
        set_value(p.x, in[i].x);
        set_value(p.y, in[i].y);
        out.push_back(std::move(p));
    }
}

static void get_points(const std::vector<Package::Point>& in, msdfgen::Point2* out)
{
    for (size_t i = 0; i < in.size(); ++i) {
        const Package::Point& p = in.at(i);
        out[i].x = static_cast<double>(p.x);
        out[i].y = static_cast<double>(p.y);
    }
}

void FontFaceXT::GlyphData::write(mu::io::IODevice* d) const
{
    // Optimization size write
    if (1) {
        Package p;
        set_value(p.metrics.textBbox_x, textBbox.x());
        set_value(p.metrics.textBbox_y, textBbox.y());
        set_value(p.metrics.textBbox_w, textBbox.width());
        set_value(p.metrics.textBbox_h, textBbox.height());
        set_value(p.metrics.textAdvance, textAdvance);
        set_value(p.metrics.symBbox_x, symBbox.x());
        set_value(p.metrics.symBbox_y, symBbox.y());
        set_value(p.metrics.symBbox_w, symBbox.width());
        set_value(p.metrics.symBbox_h, symBbox.height());
        set_value(p.metrics.symAdvance, symAdvance);

        p.shape.inverseYAxis = static_cast<int8_t>(shape.inverseYAxis);
        p.shape.fillRule = static_cast<int8_t>(shape.fillRule);

        for (const msdfgen::Contour& c : shape.contours) {
            Package::Contour pc;
            for (const msdfgen::EdgeSegment& e : c.edges) {
                Package::Edge pe;
                pe.type = static_cast<int8_t>(e.actualType);
                switch (e.actualType) {
                case msdfgen::EdgeSegment::ActualType::Undefined:
                    break;
                case msdfgen::EdgeSegment::ActualType::Cubic:
                    set_points(pe.points, e.segments.cubic.p, 4);
                    break;
                case msdfgen::EdgeSegment::ActualType::Linear:
                    set_points(pe.points, e.segments.linear.p, 2);
                    break;
                case msdfgen::EdgeSegment::ActualType::Quadratic:
                    set_points(pe.points, e.segments.quadratic.p, 3);
                    break;
                }

                pc.edges.push_back(std::move(pe));
            }

            p.shape.contours.push_back(std::move(pc));
        }

        // p.print();

        d->write(reinterpret_cast<const uint8_t*>(&p.metrics), sizeof(p.metrics));
        d->write(reinterpret_cast<const uint8_t*>(&p.shape.inverseYAxis), sizeof(p.shape.inverseYAxis));
        d->write(reinterpret_cast<const uint8_t*>(&p.shape.fillRule), sizeof(p.shape.fillRule));
        uint16_t cs = static_cast<uint16_t>(p.shape.contours.size());
        d->write(reinterpret_cast<const uint8_t*>(&cs), sizeof(cs));
        for (const Package::Contour& c : p.shape.contours) {
            uint16_t es = static_cast<uint16_t>(c.edges.size());
            d->write(reinterpret_cast<const uint8_t*>(&es), sizeof(es));
            for (const Package::Edge& e : c.edges) {
                d->write(reinterpret_cast<const uint8_t*>(&e.type), sizeof(e.type));
                uint16_t ps = static_cast<uint16_t>(e.points.size());
                d->write(reinterpret_cast<const uint8_t*>(&ps), sizeof(ps));
                for (const Package::Point& p : e.points) {
                    d->write(reinterpret_cast<const uint8_t*>(&p), sizeof(p));
                }
            }
        }
    }

// Raw write
    if (0) {
        d->write(reinterpret_cast<const uint8_t*>(&textBbox), sizeof(textBbox));
        d->write(reinterpret_cast<const uint8_t*>(&textAdvance), sizeof(textAdvance));
        d->write(reinterpret_cast<const uint8_t*>(&symBbox), sizeof(symBbox));
        d->write(reinterpret_cast<const uint8_t*>(&symAdvance), sizeof(symAdvance));
        d->write(reinterpret_cast<const uint8_t*>(&shape.inverseYAxis), sizeof(shape.inverseYAxis));
        d->write(reinterpret_cast<const uint8_t*>(&shape.fillRule), sizeof(shape.fillRule));
        size_t cs = shape.contours.size();
        d->write(reinterpret_cast<const uint8_t*>(&cs), sizeof(cs));
        for (const msdfgen::Contour& c : shape.contours) {
            size_t es = c.edges.size();
            d->write(reinterpret_cast<const uint8_t*>(&es), sizeof(es));
            for (const msdfgen::EdgeSegment& e : c.edges) {
                d->write(reinterpret_cast<const uint8_t*>(&e.actualType), sizeof(e.actualType));
                switch (e.actualType) {
                case msdfgen::EdgeSegment::ActualType::Undefined:
                    break;
                case msdfgen::EdgeSegment::ActualType::Cubic:
                    d->write(reinterpret_cast<const uint8_t*>(&e.segments.cubic), sizeof(e.segments.cubic));
                    break;
                case msdfgen::EdgeSegment::ActualType::Linear:
                    d->write(reinterpret_cast<const uint8_t*>(&e.segments.linear), sizeof(e.segments.linear));
                    break;
                case msdfgen::EdgeSegment::ActualType::Quadratic:
                    d->write(reinterpret_cast<const uint8_t*>(&e.segments.quadratic), sizeof(e.segments.quadratic));
                    break;
                }
            }
        }
    }
}

void FontFaceXT::GlyphData::read(mu::io::IODevice* d)
{
    //! NOTE Data is written in little-endian
    //! x86 and ARM (by default) is little-endian, so, not need convert
    //! Just in case, I added a check and a termination,
    //! in the hope that during the tests it will be revealed if there is another endian somewhere
//    if (!xtz::runtime::IsLittleEndian()) {
//        LOGE() << "not expected CPU endian, need rework";
//        std::abort();
//    }

    // Optimization size read
    if (1) {
        Package p;
        d->read(reinterpret_cast<uint8_t*>(&p.metrics), sizeof(p.metrics));
        d->read(reinterpret_cast<uint8_t*>(&p.shape.inverseYAxis), sizeof(p.shape.inverseYAxis));
        d->read(reinterpret_cast<uint8_t*>(&p.shape.fillRule), sizeof(p.shape.fillRule));

        uint16_t cs = 0;
        d->read(reinterpret_cast<uint8_t*>(&cs), sizeof(cs));
        for (uint16_t ci = 0; ci < cs; ++ci) {
            Package::Contour c;
            uint16_t es = 0;
            d->read(reinterpret_cast<uint8_t*>(&es), sizeof(es));
            c.edges.reserve(es);
            for (uint16_t ei = 0; ei < es; ++ei) {
                Package::Edge e;
                d->read(reinterpret_cast<uint8_t*>(&e.type), sizeof(e.type));
                uint16_t ps = 0;
                d->read(reinterpret_cast<uint8_t*>(&ps), sizeof(ps));
                e.points.reserve(ps);
                for (uint16_t pi = 0; pi < ps; ++pi) {
                    Package::Point p;
                    d->read(reinterpret_cast<uint8_t*>(&p), sizeof(p));
                    e.points.push_back(std::move(p));
                }
                c.edges.push_back(std::move(e));
            }
            p.shape.contours.push_back(std::move(c));
        }

        const Package::Metrics& m = p.metrics;
        textBbox = FBBox(m.textBbox_x, m.textBbox_y, m.textBbox_w, m.textBbox_h);
        textAdvance = m.textAdvance;
        symBbox = FBBox(m.symBbox_x, m.symBbox_y, m.symBbox_w, m.symBbox_h);
        symAdvance = m.symAdvance;

        const Package::Shape& s = p.shape;
        shape.inverseYAxis = s.inverseYAxis;
        shape.fillRule = static_cast<msdfgen::FillRule>(s.fillRule);
        shape.contours.reserve(p.shape.contours.size());
        for (const Package::Contour& pc : p.shape.contours) {
            msdfgen::Contour mc;
            mc.edges.reserve(pc.edges.size());
            for (const Package::Edge& pe : pc.edges) {
                msdfgen::EdgeSegment me;
                me.actualType = static_cast<msdfgen::EdgeSegment::ActualType>(pe.type);
                switch (me.actualType) {
                case msdfgen::EdgeSegment::ActualType::Undefined:
                    break;
                case msdfgen::EdgeSegment::ActualType::Cubic:
                    get_points(pe.points, me.segments.cubic.p);
                    break;
                case msdfgen::EdgeSegment::ActualType::Linear:
                    get_points(pe.points, me.segments.linear.p);
                    break;
                case msdfgen::EdgeSegment::ActualType::Quadratic:
                    get_points(pe.points, me.segments.quadratic.p);
                    break;
                }
                mc.edges.push_back(std::move(me));
            }
            shape.contours.push_back(std::move(mc));
        }
    }

    // Raw read
    if (0) {
        d->read(reinterpret_cast<uint8_t*>(&textBbox), sizeof(textBbox));
        d->read(reinterpret_cast<uint8_t*>(&textAdvance), sizeof(textAdvance));
        d->read(reinterpret_cast<uint8_t*>(&symBbox), sizeof(symBbox));
        d->read(reinterpret_cast<uint8_t*>(&symAdvance), sizeof(symAdvance));
        d->read(reinterpret_cast<uint8_t*>(&shape.inverseYAxis), sizeof(shape.inverseYAxis));
        d->read(reinterpret_cast<uint8_t*>(&shape.fillRule), sizeof(shape.fillRule));
        size_t cs = 0;
        d->read(reinterpret_cast<uint8_t*>(&cs), sizeof(cs));
        for (size_t ci = 0; ci < cs; ++ci) {
            msdfgen::Contour c;
            size_t es = 0;
            d->read(reinterpret_cast<uint8_t*>(&es), sizeof(es));
            for (size_t ei = 0; ei < es; ++ei) {
                msdfgen::EdgeSegment e;
                d->read(reinterpret_cast<uint8_t*>(&e.actualType), sizeof(e.actualType));
                switch (e.actualType) {
                case msdfgen::EdgeSegment::ActualType::Undefined:
                    break;
                case msdfgen::EdgeSegment::ActualType::Cubic:
                    d->read(reinterpret_cast<uint8_t*>(&e.segments.cubic), sizeof(e.segments.cubic));
                    break;
                case msdfgen::EdgeSegment::ActualType::Linear:
                    d->read(reinterpret_cast<uint8_t*>(&e.segments.linear), sizeof(e.segments.linear));
                    break;
                case msdfgen::EdgeSegment::ActualType::Quadratic:
                    d->read(reinterpret_cast<uint8_t*>(&e.segments.quadratic), sizeof(e.segments.quadratic));
                    break;
                }

                c.edges.push_back(std::move(e));
            }

            shape.contours.push_back(std::move(c));
        }
    }
}
