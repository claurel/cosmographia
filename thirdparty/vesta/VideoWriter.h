/*
 * $Revision: 621 $ $Date: 2011-08-29 13:17:00 -0700 (Mon, 29 Aug 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_VIDEO_WRITER_H_
#define _VESTA_VIDEO_WRITER_H_

#include "Object.h"
#include <string>
#include <vector>


namespace vesta
{

/** VideoWriter is an abstract class used for encoding video and writing it
 *  to a file. Subclasses of VideoWriter implement encoding via
 *  platform-specific libraries.
 */
class VideoWriter : public Object
{
public:
    virtual ~VideoWriter();

    bool openStream(const std::string& fileName,
                    int videoWidth,
                    int videoHeight,
                    float frameRate);
    bool closeStream();

    bool encodeFrame(int x, int y);
    bool hasCompressionOptionsDialog() const
    {
        return m_hasCompressionOptionsDialog;
    }

    bool isStreamOpen() const
    {
        return m_isStreamOpen;
    }

    int videoWidth() const
    {
        return m_videoWidth;
    }

    int videoHeight() const
    {
        return m_videoHeight;
    }

    float frameRate() const
    {
        return m_frameRate;
    }

    bool showCompressionOptionsDialog();

    std::vector<std::string> availableCompressorList() const;

    static VideoWriter* CreateDefaultVideoWriter();

protected:
    VideoWriter();

    virtual bool handleOpenStream(const std::string& fileName) = 0;
    virtual bool handleCloseStream() = 0;
    virtual bool handleEncodeFrame(char* imageData) = 0;
    virtual bool handleCompressionOptionsDialog() = 0;
    virtual std::vector<std::string> handleAvailableCompressorList() const = 0;

    void setCompressionOptionsDialogAvailable(bool available);

private:
    bool m_hasCompressionOptionsDialog;
    bool m_isStreamOpen;
    int m_videoWidth;
    int m_videoHeight;
    float m_frameRate;

    char* m_imageData;
};

}

#endif // _VESTA_VIDEO_WRITER_H_
