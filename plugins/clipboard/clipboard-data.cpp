/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#include "clipboard-data.h"
#include <QImage>
#include <QMimeData>

namespace Kiran
{
QSharedPointer<ClipboardData> ClipboardData::createClipboardData(const QMimeData* mineData)
{
    if (mineData->hasUrls())
    {
        auto urls = mineData->urls();
        if (urls.isEmpty())
        {
            return QSharedPointer<ClipboardData>();
        }
        return QSharedPointer<ClipboardData>(new ClipboardUrlsData(urls));
    }

    if (mineData->hasText())
    {
        auto text = mineData->text();
        if (text.isEmpty())
        {
            return QSharedPointer<ClipboardData>();
        }
        return QSharedPointer<ClipboardData>(new ClipboardTextData(text));
    }

    if (mineData->hasImage())
    {
        const QImage image = qvariant_cast<QImage>(mineData->imageData());
        if (image.isNull())
        {
            return QSharedPointer<ClipboardData>();
        }
        return QSharedPointer<ClipboardData>(new ClipboardImageData(image));
    }

    return QSharedPointer<ClipboardData>();
}

ClipboardUrlsData::ClipboardUrlsData(const QList<QUrl>& urls) : m_urls(urls)
{
}

QMimeData* ClipboardUrlsData::mimeData() const
{
    auto mimeData = new QMimeData();
    mimeData->setUrls(m_urls);
    return mimeData;
}

ClipboardTextData::ClipboardTextData(const QString& text) : m_text(text)
{
}

QMimeData* ClipboardTextData::mimeData() const
{
    auto mimeData = new QMimeData();
    mimeData->setText(m_text);
    return mimeData;
}

ClipboardImageData::ClipboardImageData(const QImage& image) : m_image(image)
{
}

QMimeData* ClipboardImageData::mimeData() const
{
    auto mimeData = new QMimeData();
    mimeData->setImageData(m_image);
    return mimeData;
}
}  // namespace Kiran