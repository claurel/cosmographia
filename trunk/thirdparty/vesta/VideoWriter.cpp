/*
 * $Revision: 621 $ $Date: 2011-08-29 13:17:00 -0700 (Mon, 29 Aug 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "VideoWriter.h"
#include "Debug.h"
#include "OGLHeaders.h"

#if defined(VESTA_VIDEO_VFW)
#include "video/VFWVideoWriter.h"
#endif

using namespace vesta;
using namespace std;


VideoWriter::VideoWriter() :
    m_hasCompressionOptionsDialog(false),
    m_isStreamOpen(false),
    m_videoWidth(0),
    m_videoHeight(0),
    m_frameRate(30.0f),
    m_imageData(NULL)
{
}


VideoWriter::~VideoWriter()
{
    if (m_imageData)
    {
        delete[] m_imageData;
    }
}


bool
VideoWriter::showCompressionOptionsDialog()
{
    if (hasCompressionOptionsDialog())
    {
        return handleCompressionOptionsDialog();
    }
    else
    {
        return false;
    }
}


bool
VideoWriter::openStream(const string& fileName,
                        int videoWidth, int videoHeight,
                        float frameRate)
{
    if (isStreamOpen())
    {
        VESTA_WARNING("VideoWriter open error: video stream is already open.");
        return false;
    }

    m_videoWidth = videoWidth;
    m_videoHeight = videoHeight;
    m_frameRate = frameRate;

    // Allocate the buffer for storing the image data
    int bytesPerRow = (m_videoWidth * 3 + 3) & ~3u;
    int imageSize = bytesPerRow * m_videoHeight;
    m_imageData = new char[imageSize];
    if (!m_imageData)
    {
        VESTA_WARNING("VideoWriter open error: not enough memory for image copy buffer.");
        return false;
    }

    m_isStreamOpen = handleOpenStream(fileName);

    return m_isStreamOpen;
}


bool
VideoWriter::closeStream()
{
    if (!isStreamOpen())
    {
        VESTA_WARNING("VideoWriter close error: video stream isn't open.");
        return false;
    }

    bool ok = handleCloseStream();
    m_isStreamOpen = false;

    if (m_imageData)
    {
        delete[] m_imageData;
    }

    return ok;
}


/** Encode a frame of video using data from the current OpenGL framebuffer.
  * The region of the framebuffer that will be saved is the rectangle
  * with the dimensions provided to openStream and the top left corner at
  * (x, y)
  */
bool
VideoWriter::encodeFrame(int x, int y)
{
    if (!isStreamOpen())
    {
        VESTA_WARNING("VideoWriter encode frame error: video stream isn't open");
        return false;
    }

#ifndef VESTA_OGLES2
    // Read image data from the frame buffer into system memory
    // NOTE: OpenGL ES requires an explicit call to resolve multisample framebuffers
    glReadPixels(x, y, m_videoWidth, m_videoHeight, GL_BGR, GL_UNSIGNED_BYTE, m_imageData);
#endif

    return handleEncodeFrame(m_imageData);
}


/** Return a list of video compressors available on this system.
  */
vector<string>
VideoWriter::availableCompressorList() const
{
    return handleAvailableCompressorList();
}


void
VideoWriter::setCompressionOptionsDialogAvailable(bool available)
{
    m_hasCompressionOptionsDialog = available;
}


VideoWriter*
VideoWriter::CreateDefaultVideoWriter()
{
#if defined(VESTA_VIDEO_VFW)
    VFWVideoWriter* vw = new VFWVideoWriter();
    if (vw)
    {
        if (!vw->initialize())
        {
            delete vw;
            vw = NULL;
        }
        return vw;
    }
#else
    VESTA_WARNING("CreateDefaultVideoWriter: no supported video framework.");
#endif

    return NULL;
}
