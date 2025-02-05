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

#include <QImage>
#include <QSharedPointer>
#include <QUrl>

class QMimeData;

namespace Kiran
{
class ClipboardData : public QObject
{
    Q_OBJECT

public:
    static QSharedPointer<ClipboardData> createClipboardData(const QMimeData* mineData);

    virtual QMimeData* mimeData() const = 0;
};

class ClipboardUrlsData : public ClipboardData
{
    Q_OBJECT
public:
    ClipboardUrlsData(const QList<QUrl>& urls);

    virtual QMimeData* mimeData() const;

private:
    QList<QUrl> m_urls;
};

class ClipboardTextData : public ClipboardData
{
    Q_OBJECT
public:
    ClipboardTextData(const QString& text);

    virtual QMimeData* mimeData() const;

private:
    QString m_text;
};

class ClipboardImageData : public ClipboardData
{
    Q_OBJECT
public:
    ClipboardImageData(const QImage& image);

    virtual QMimeData* mimeData() const;

private:
    QImage m_image;
};
}  // namespace Kiran