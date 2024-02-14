// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2015 Martin Flöser <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QFile>
#include <QStandardPaths>

#include <QCryptographicHash>

#include "c_ptr.h"
#include "drmloggingcategories.h"
#include "edid.h"

extern "C" {
#include <libdisplay-info/cta.h>
#include <libdisplay-info/edid.h>
#include <libdisplay-info/info.h>
}

namespace Aurora {

namespace Platform {

static QByteArray parsePnpId(const uint8_t *data)
{
    // Decode PNP ID from three 5 bit words packed into 2 bytes:
    //
    // | Byte |        Bit                    |
    // |      | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
    // ----------------------------------------
    // |  1   | 0)| (4| 3 | 2 | 1 | 0)| (4| 3 |
    // |      | * |    Character 1    | Char 2|
    // ----------------------------------------
    // |  2   | 2 | 1 | 0)| (4| 3 | 2 | 1 | 0)|
    // |      | Character2|      Character 3  |
    // ----------------------------------------
    const uint offset = 0x8;

    char pnpId[4];
    pnpId[0] = 'A' + ((data[offset + 0] >> 2) & 0x1f) - 1;
    pnpId[1] = 'A' + (((data[offset + 0] & 0x3) << 3) | ((data[offset + 1] >> 5) & 0x7)) - 1;
    pnpId[2] = 'A' + (data[offset + 1] & 0x1f) - 1;
    pnpId[3] = '\0';

    return QByteArray(pnpId);
}

static QByteArray parseEisaId(const uint8_t *data)
{
    for (int i = 72; i <= 108; i += 18) {
        // Skip the block if it isn't used as monitor descriptor.
        if (data[i]) {
            continue;
        }
        if (data[i + 1]) {
            continue;
        }

        // We have found the EISA ID, it's stored as ASCII.
        if (data[i + 3] == 0xfe) {
            return QByteArray(reinterpret_cast<const char *>(&data[i + 5]), 13).trimmed();
        }
    }

    // If there isn't an ASCII EISA ID descriptor, try to decode PNP ID
    return parsePnpId(data);
}

static QByteArray parseVendor(const uint8_t *data)
{
    const auto pnpId = parsePnpId(data);

    // Map to vendor name
    QFile pnpFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                         QStringLiteral("hwdata/pnp.ids")));
    if (pnpFile.exists() && pnpFile.open(QIODevice::ReadOnly)) {
        while (!pnpFile.atEnd()) {
            const auto line = pnpFile.readLine();
            if (line.startsWith(pnpId)) {
                return line.mid(4).trimmed();
            }
        }
    }

    return {};
}

Edid::Edid()
{
}

Edid::Edid(const void *data, uint32_t size)
{
    m_raw.resize(size);
    memcpy(m_raw.data(), data, size);

    const uint8_t *bytes = static_cast<const uint8_t *>(data);

    auto info = di_info_parse_edid(data, size);
    if (!info) {
        qCWarning(gLcDrm, "parsing edid failed");
        return;
    }
    const di_edid *edid = di_info_get_edid(info);
    const di_edid_vendor_product *productInfo = di_edid_get_vendor_product(edid);
    const di_edid_screen_size *screenSize = di_edid_get_screen_size(edid);

    // basic output information
    m_physicalSize = QSize(screenSize->width_cm, screenSize->height_cm) * 10;
    m_eisaId = parseEisaId(bytes);
    UniqueCPtr<char> monitorName{ di_info_get_model(info) };
    m_monitorName = QByteArray(monitorName.get());
    m_serialNumber = QByteArray::number(productInfo->serial);
    m_vendor = parseVendor(bytes);
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(m_raw);
    m_hash = QString::fromLatin1(hash.result().toHex());

    m_identifier = QByteArray(productInfo->manufacturer, 3) + " "
            + QByteArray::number(productInfo->product) + " "
            + QByteArray::number(productInfo->serial) + " "
            + QByteArray::number(productInfo->manufacture_week) + " "
            + QByteArray::number(productInfo->manufacture_year) + " "
            + QByteArray::number(productInfo->model_year);

    // colorimetry and HDR metadata
    const auto chromaticity = di_edid_get_chromaticity_coords(edid);
    if (chromaticity) {
        m_colorimetry = Core::Colorimetry{
            QVector2D{ chromaticity->red_x, chromaticity->red_y },
            QVector2D{ chromaticity->green_x, chromaticity->green_y },
            QVector2D{ chromaticity->blue_x, chromaticity->blue_y },
            QVector2D{ chromaticity->white_x, chromaticity->white_y },
        };
    } else {
        m_colorimetry.reset();
    }

    const di_edid_cta *cta = nullptr;
    const di_edid_ext *const *exts = di_edid_get_extensions(edid);
    const di_cta_hdr_static_metadata_block *hdr_static_metadata = nullptr;
    const di_cta_colorimetry_block *colorimetry = nullptr;
    for (; *exts != nullptr; exts++) {
        if ((cta = di_edid_ext_get_cta(*exts))) {
            break;
        }
    }
    if (cta) {
        const di_cta_data_block *const *blocks = di_edid_cta_get_data_blocks(cta);
        for (; *blocks != nullptr; blocks++) {
            if (!hdr_static_metadata
                && (hdr_static_metadata = di_cta_data_block_get_hdr_static_metadata(*blocks))) {
                continue;
            }
            if (!colorimetry && (colorimetry = di_cta_data_block_get_colorimetry(*blocks))) {
                continue;
            }
        }
        if (hdr_static_metadata) {
            m_hdrMetadata = HDRMetadata{
                .desiredContentMinLuminance = hdr_static_metadata->desired_content_min_luminance,
                .desiredContentMaxLuminance = hdr_static_metadata->desired_content_max_luminance > 0
                        ? std::make_optional(hdr_static_metadata->desired_content_max_luminance)
                        : std::nullopt,
                .desiredMaxFrameAverageLuminance =
                        hdr_static_metadata->desired_content_max_frame_avg_luminance > 0
                        ? std::make_optional(
                                hdr_static_metadata->desired_content_max_frame_avg_luminance)
                        : std::nullopt,
                .supportsPQ = hdr_static_metadata->eotfs->pq,
                .supportsBT2020 = colorimetry && colorimetry->bt2020_rgb,
            };
        }
    }

    m_isValid = true;
    di_info_destroy(info);
}

bool Edid::isValid() const
{
    return m_isValid;
}

QSize Edid::physicalSize() const
{
    return m_physicalSize;
}

QByteArray Edid::eisaId() const
{
    return m_eisaId;
}

QByteArray Edid::monitorName() const
{
    return m_monitorName;
}

QByteArray Edid::serialNumber() const
{
    return m_serialNumber;
}

QByteArray Edid::vendor() const
{
    return m_vendor;
}

QByteArray Edid::raw() const
{
    return m_raw;
}

QString Edid::manufacturerString() const
{
    QString manufacturer;
    if (!m_vendor.isEmpty()) {
        manufacturer = QString::fromLatin1(m_vendor);
    } else if (!m_eisaId.isEmpty()) {
        manufacturer = QString::fromLatin1(m_eisaId);
    }
    return manufacturer;
}

QString Edid::nameString() const
{
    if (!m_monitorName.isEmpty()) {
        QString m = QString::fromLatin1(m_monitorName);
        if (!m_serialNumber.isEmpty()) {
            m.append(QLatin1Char('/'));
            m.append(QString::fromLatin1(m_serialNumber));
        }
        return m;
    } else if (!m_serialNumber.isEmpty()) {
        return QString::fromLatin1(m_serialNumber);
    } else {
        return QObject::tr("unknown");
    }
}

QString Edid::hash() const
{
    return m_hash;
}

std::optional<Core::Colorimetry> Edid::colorimetry() const
{
    return m_colorimetry;
}

double Edid::desiredMinLuminance() const
{
    return m_hdrMetadata ? m_hdrMetadata->desiredContentMinLuminance : 0;
}

std::optional<double> Edid::desiredMaxFrameAverageLuminance() const
{
    return m_hdrMetadata ? m_hdrMetadata->desiredMaxFrameAverageLuminance : std::nullopt;
}

std::optional<double> Edid::desiredMaxLuminance() const
{
    return m_hdrMetadata ? m_hdrMetadata->desiredContentMaxLuminance : std::nullopt;
}

bool Edid::supportsPQ() const
{
    return m_hdrMetadata && m_hdrMetadata->supportsPQ;
}

bool Edid::supportsBT2020() const
{
    return m_hdrMetadata && m_hdrMetadata->supportsBT2020;
}

QByteArray Edid::identifier() const
{
    return m_identifier;
}

} // namespace Platform

} // namespace Aurora
