/*
 *  Copyright (c) 2016 Michael Abrahams <miabraha@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_cursor_cache.h"

#include <QScreen>
#include <QWindow>
#include <QBitmap>
#include <QImage>
#include <qmath.h>
#include <QDebug>
#include <QPainter>
#include <QApplication>

#include "KoResourcePaths.h"

Q_GLOBAL_STATIC(KisCursorCache, s_instance)

// Very old cursors were saved in these byte arrays.
namespace {
    static const unsigned char select_bits[] = {
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0xff, 0xff, 0x7f,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00
    };

    static const unsigned char move_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x7e, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00,
        0x10, 0x18, 0x08, 0x18, 0x18, 0x18, 0x1c, 0x18, 0x38, 0xfe, 0xff, 0x7f,
        0xfe, 0xff, 0x7f, 0x1c, 0x18, 0x38, 0x18, 0x18, 0x18, 0x10, 0x18, 0x08,
        0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x7e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00
    };

    static const unsigned char pickerplus_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x80, 0x1e,
        0x00, 0x00, 0x40, 0x1f, 0x00, 0x00, 0xb0, 0x1f, 0x00, 0x00, 0xe8, 0x0f,
        0x00, 0x00, 0xd0, 0x07, 0x00, 0x00, 0xa8, 0x03, 0x00, 0x00, 0x64, 0x03,
        0x00, 0x00, 0x72, 0x01, 0x00, 0x00, 0xb9, 0x00, 0x00, 0x80, 0x1c, 0x00,
        0x00, 0x40, 0x0e, 0x00, 0x00, 0x20, 0x07, 0x00, 0x00, 0x90, 0x03, 0x00,
        0x00, 0xc8, 0x01, 0x01, 0x40, 0xe4, 0x00, 0x01, 0x40, 0x74, 0xc0, 0x07,
        0x40, 0x3c, 0x00, 0x01, 0x40, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00
    };

    static const unsigned char pickerminus_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x80, 0x1e,
        0x00, 0x00, 0x40, 0x1f, 0x00, 0x00, 0xb0, 0x1f, 0x00, 0x00, 0xe8, 0x0f,
        0x00, 0x00, 0xd0, 0x07, 0x00, 0x00, 0xa8, 0x03, 0x00, 0x00, 0x64, 0x03,
        0x00, 0x00, 0x72, 0x01, 0x00, 0x00, 0xb9, 0x00, 0x00, 0x80, 0x1c, 0x00,
        0x00, 0x40, 0x0e, 0x00, 0x00, 0x20, 0x07, 0x00, 0x00, 0x90, 0x03, 0x00,
        0x00, 0xc8, 0x01, 0x00, 0x40, 0xe4, 0x00, 0x00, 0x40, 0x74, 0xc0, 0x07,
        0x40, 0x3c, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00
    };

    static const unsigned char pen_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3a, 0x00, 0x00, 0x7d,
        0x00, 0x80, 0x7e, 0x00, 0x40, 0x7f, 0x00, 0xa0, 0x3f, 0x00, 0xd0, 0x1f,
        0x00, 0xe8, 0x0f, 0x00, 0xf4, 0x07, 0x00, 0xfa, 0x03, 0x00, 0xfd, 0x01,
        0x80, 0xfe, 0x00, 0x40, 0x7f, 0x00, 0xa0, 0x3f, 0x00, 0xf0, 0x1f, 0x00,
        0xd0, 0x0f, 0x00, 0x88, 0x07, 0x00, 0x88, 0x03, 0x00, 0xe4, 0x01, 0x00,
        0x7c, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    static const unsigned char brush_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x68, 0x00,
        0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0xfd, 0x00,
        0x00, 0x80, 0x7e, 0x00, 0x00, 0x40, 0x3f, 0x00, 0x00, 0xa0, 0x1f, 0x00,
        0x00, 0xd0, 0x0f, 0x00, 0x00, 0xe8, 0x07, 0x00, 0x00, 0xf4, 0x03, 0x00,
        0x00, 0xe4, 0x01, 0x00, 0x00, 0xc2, 0x00, 0x00, 0x80, 0x41, 0x00, 0x00,
        0x40, 0x32, 0x00, 0x00, 0xa0, 0x0f, 0x00, 0x00, 0xd0, 0x0f, 0x00, 0x00,
        0xd0, 0x0f, 0x00, 0x00, 0xe8, 0x07, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00,
        0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    static const unsigned char airbrush_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x68, 0x00, 0x00, 0x74,
        0x00, 0x00, 0x7a, 0xf0, 0x00, 0x3d, 0x08, 0x81, 0x1e, 0xe8, 0x41, 0x0f,
        0xe8, 0xa1, 0x07, 0xe8, 0xd1, 0x03, 0xe8, 0xe9, 0x01, 0xe8, 0xf5, 0x00,
        0xe8, 0x7b, 0x00, 0xf0, 0x33, 0x00, 0xf0, 0x23, 0x1f, 0xa0, 0x9f, 0x3f,
        0xd0, 0xff, 0x31, 0xe8, 0xf7, 0x30, 0xf4, 0x03, 0x18, 0xfc, 0x01, 0x0c,
        0xf8, 0x00, 0x06, 0x76, 0x00, 0x03, 0x36, 0x00, 0x03, 0x00, 0x00, 0x00
    };

    static const unsigned char eraser_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x1d, 0x00,
        0x00, 0x80, 0x3e, 0x00, 0x00, 0x40, 0x7f, 0x00, 0x00, 0xa0, 0xff, 0x00,
        0x00, 0xd0, 0xff, 0x00, 0x00, 0xe8, 0x7f, 0x00, 0x00, 0xf4, 0x3f, 0x00,
        0x00, 0xfe, 0x1f, 0x00, 0x00, 0xf9, 0x0f, 0x00, 0x80, 0xf2, 0x07, 0x00,
        0x40, 0xe7, 0x03, 0x00, 0xa0, 0xcf, 0x01, 0x00, 0xd0, 0x9f, 0x00, 0x00,
        0xe8, 0x7f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xf2, 0x1f, 0x00, 0x00,
        0xe2, 0x0f, 0x00, 0x00, 0xc4, 0x07, 0x00, 0x00, 0x88, 0x03, 0x00, 0x00,
        0x10, 0x01, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    static const unsigned char filler_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x28, 0x00,
        0x00, 0x54, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x85, 0x00, 0x80, 0x0a, 0x01,
        0x40, 0x11, 0x01, 0xe0, 0x00, 0x02, 0x58, 0x01, 0x04, 0x2c, 0x02, 0x04,
        0x44, 0x04, 0x08, 0x0c, 0x08, 0x18, 0x3c, 0x00, 0x14, 0x5c, 0x00, 0x0a,
        0x9c, 0x01, 0x05, 0x1c, 0x82, 0x02, 0x18, 0x4c, 0x01, 0x18, 0xb0, 0x00,
        0x08, 0x60, 0x00, 0x00, 0x00, 0x00
    };

    static const unsigned char colorChanger_bits[] = {
        0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x10, 0x01, 0x0e, 0x08, 0x02, 0x11,
        0x04, 0x82, 0x20, 0x64, 0x84, 0x20, 0x92, 0x44, 0x46, 0x12, 0x49, 0x5f,
        0x12, 0x31, 0x5f, 0x22, 0x01, 0x5f, 0xc2, 0x00, 0x4e, 0x02, 0x00, 0x40,
        0xc2, 0x00, 0x46, 0xe2, 0x01, 0x4f, 0xe4, 0x19, 0x2f, 0xe4, 0x3d, 0x2f,
        0xe8, 0x3d, 0x17, 0xd0, 0x3c, 0x10, 0x20, 0x38, 0x08, 0x40, 0x00, 0x06,
        0x80, 0x81, 0x01, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00
    };

    inline QBitmap bitmapFromData(const QSize& size, const unsigned char* data)
    {
        QBitmap result(32, 32);
        result.fill(Qt::color0);
        QPainter painter(&result);
        painter.drawPixmap(0, 0, QBitmap::fromData(size, data));
        return result;
    }

    inline QCursor fromBitmap(QSize size, const unsigned char* bits, int hotspotX, int hotspotY)
    {
        QBitmap bitmap = bitmapFromData(size, bits);
        QBitmap mask = bitmap.createHeuristicMask(false);
        return QCursor(bitmap, mask, hotspotX, hotspotY);
    }

    QCursor loadImpl(const QString &cursorName, int hotspotX, int hotspotY) {
        QImage cursorImage = QImage(":/" + cursorName);
        if (cursorImage.isNull()) {
            qWarning() << "Could not load cursor from qrc, trying filesystem" << cursorName;

            cursorImage = QImage(KoResourcePaths::findResource("kis_pics", cursorName));
            if (cursorImage.isNull()) {
                qWarning() << "Could not load cursor from filesystem" << cursorName;
                return Qt::ArrowCursor;
            }
        }

#ifdef Q_OS_WIN
        // cursor width must be multiple of 16 on Windows
        int bitmapWidth = qCeil(cursorImage.width() / 16.0) * 16;

        QBitmap bitmap(bitmapWidth, cursorImage.height());
        QBitmap mask(bitmapWidth, cursorImage.height());

        if (bitmapWidth != cursorImage.width()) {
            bitmap.clear();
            mask.clear();
        }
#else
        QBitmap bitmap(cursorImage.width(), cursorImage.height());
        QBitmap mask(cursorImage.width(), cursorImage.height());
#endif

        QPainter bitmapPainter(&bitmap);
        QPainter maskPainter(&mask);

        for (qint32 x = 0; x < cursorImage.width(); ++x) {
            for (qint32 y = 0; y < cursorImage.height(); ++y) {

                QRgb pixel = cursorImage.pixel(x, y);

                if (qAlpha(pixel) < 128) {
                    bitmapPainter.setPen(Qt::color0);
                    maskPainter.setPen(Qt::color0);
                } else {
                    maskPainter.setPen(Qt::color1);

                    if (qGray(pixel) < 128) {
                        bitmapPainter.setPen(Qt::color1);
                    } else {
                        bitmapPainter.setPen(Qt::color0);
                    }
                }

                bitmapPainter.drawPoint(x, y);
                maskPainter.drawPoint(x, y);
            }
        }

        // Seems to be a bug in QCursor? https://bugreports.qt.io/browse/QTBUG-46259
        qreal dpr;
        QWindow* focusWindow = qApp->focusWindow();
        if (focusWindow) {
            dpr = focusWindow->devicePixelRatio();
        } else {
            dpr = qApp->devicePixelRatio();
        }
        bitmap.setDevicePixelRatio(dpr);
        mask.setDevicePixelRatio(dpr);
        return QCursor(bitmap, mask, hotspotX, hotspotY);
    }

}

KisCursorCache::KisCursorCache() {}

KisCursorCache* KisCursorCache::instance()
{
    if (!s_instance.exists()) {
        s_instance->init();
        // TODO: listen for DPI change signals
    }
    return s_instance;
}

QCursor KisCursorCache::load(const QString & cursorName, int hotspotX, int hotspotY)
{
    if (cursorHash.contains(cursorName)) {
        return cursorHash[ cursorName ].second;
    }

    // Otherwise, construct the cursor
    QCursor newCursor = loadImpl(cursorName, hotspotX, hotspotY);
    cursorHash.insert(cursorName, QPair<QPoint, QCursor>(QPoint(hotspotX, hotspotY), newCursor));
    return newCursor;
}

void KisCursorCache::init()
{
    selectCursor       = fromBitmap(QSize(23, 23), select_bits, 11, 11);
    moveCursor         = fromBitmap(QSize(24, 24), move_bits, 12, 11);
    pickerPlusCursor   = fromBitmap(QSize(32, 32), pickerplus_bits, 6, 25);
    pickerMinusCursor  = fromBitmap(QSize(32, 32), pickerminus_bits, 6, 25);
    penCursor          = fromBitmap(QSize(24, 24), pen_bits, 1, 22);
    brushCursor        = fromBitmap(QSize(25, 23), brush_bits, 1, 21);
    airbrushCursor     = fromBitmap(QSize(24, 24), airbrush_bits, 1, 22);
    eraserCursor       = fromBitmap(QSize(25, 24), eraser_bits, 7, 22);
    colorChangerCursor = fromBitmap(QSize(24, 23), colorChanger_bits, 12, 10);
    fillerCursor       = fromBitmap(QSize(22, 22), filler_bits, 3, 20);
}
