/*
 * Copyright (C) 2011 by Chris Laurel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 */

#import "VideoEncoder.h"
#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#include <QDebug>


class VideoEncoderPrivate
{
public:
    VideoEncoderPrivate() :
        m_movie(NULL),
        m_imageAttributes(NULL),
        m_width(852),
        m_height(480)
    {
    }

    bool startRecording(const QString& fileName, int width, int height)
    {
        m_width = width;
        m_height = height;

        if (!m_movie)
        {
            NSString* nsFileName = [NSString stringWithUTF8String: fileName.toUtf8().data() ];

            m_imageAttributes = [[NSMutableDictionary alloc] init];

            // Set the codec to H.264
            //   Other codecs: mp4v,
            [m_imageAttributes setObject: @"avc1" forKey: QTAddImageCodecType];
            [m_imageAttributes setObject: [NSNumber numberWithLong: codecHighQuality] forKey: QTAddImageCodecQuality];

            m_movie = [[QTMovie alloc] initToWritableFile: nsFileName error:NULL]; // initialize the output QT movie object
            return true;
        }
        else
        {
            return false;
        }
    }

    bool stopRecording()
    {
        return close();
    }

    bool encodeImage(const QImage& image)
    {
        const long framesPerSecond = 30;

        unsigned char* bitmapPlanes[4] = { NULL, NULL, NULL, NULL };
        bitmapPlanes[0] = const_cast<unsigned char*>(image.bits());

        NSBitmapImageRep* imageRep = [NSBitmapImageRep alloc];
        [imageRep
              initWithBitmapDataPlanes: bitmapPlanes
              pixelsWide: image.width()
              pixelsHigh: image.height()
              bitsPerSample: 8
              samplesPerPixel: 3
              hasAlpha: NO
              isPlanar: NO
              colorSpaceName: NSDeviceRGBColorSpace
              bytesPerRow: image.width() * 4
              bitsPerPixel: 32 ];

        NSSize imageSize = NSMakeSize(image.width(), image.height());
        NSImage* nsImage = [[NSImage alloc] initWithSize: imageSize];
        [nsImage addRepresentation: imageRep];

#ifdef DEBUG_FRAMES
        NSData *imageData;
        imageData = [imageRep TIFFRepresentation];
        [imageData writeToFile:@"test.tif" atomically:YES];
#endif // DEBUG_NSIMAGE

        [m_movie addImage: nsImage forDuration: QTMakeTime(1, framesPerSecond) withAttributes: m_imageAttributes];

        [nsImage release];

        return true;
    }

    int getWidth() const
    {
        return m_width;
    }

    int getHeight() const
    {
        return m_height;
    }

    bool close()
    {
        if (m_movie)
        {
            [m_movie updateMovieFile];
            [m_movie release];
            m_movie = NULL;
        }
        return true;
    }

private:
    QTMovie* m_movie;
    NSMutableDictionary* m_imageAttributes;
    int m_width;
    int m_height;
};


QVideoEncoder::QVideoEncoder() :
    m_impl(new VideoEncoderPrivate())
{
}


QVideoEncoder::~QVideoEncoder()
{
    delete m_impl;
}


bool
QVideoEncoder::startRecording()
{
    return true;
    //return m_impl->startRecording();
}


bool
QVideoEncoder::stopRecording()
{
    return m_impl->stopRecording();
}


bool
QVideoEncoder::encodeImage(const QImage &image)
{
    return m_impl->encodeImage(image);
}


int
QVideoEncoder::getWidth() const
{
    return m_impl->getWidth();
}


int
QVideoEncoder::getHeight() const
{
    return m_impl->getHeight();
}


bool
QVideoEncoder::close()
{
    return m_impl->close();
}


bool
QVideoEncoder::createFile(QString fileName,
                          unsigned int width,
                          unsigned int height,
                          unsigned int /* bitrate */,
                          unsigned int /* gop */)
{
    return m_impl->startRecording(fileName, width, height);
}
