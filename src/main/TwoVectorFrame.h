// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _TWO_VECTOR_FRAME_H_
#define _TWO_VECTOR_FRAME_H_

#include <vesta/Frame.h>
#include <vesta/Entity.h>


class TwoVectorFrameDirection : public vesta::Object
{
public:
    /** Return the direction at the specified time. The returned
      * vector will not necessarily be normalized, and may in fact
      * be zero. The TwoVectorFrame class will return the identity
      * rotation when one or more directions is zero.
      */
    virtual Eigen::Vector3d direction(double tdbSec) const = 0;
};


class RelativePositionVector : public TwoVectorFrameDirection
{
public:
    RelativePositionVector(vesta::Entity* observer, vesta::Entity* target);
    ~RelativePositionVector();
    virtual Eigen::Vector3d direction(double tdbSec) const;

    vesta::Entity* observer() const
    {
        return m_observer.ptr();
    }

    vesta::Entity* target() const
    {
        return m_target.ptr();
    }

private:
    vesta::counted_ptr<vesta::Entity> m_observer;
    vesta::counted_ptr<vesta::Entity> m_target;
};


class RelativeVelocityVector : public TwoVectorFrameDirection
{
public:
    RelativeVelocityVector(vesta::Entity* observer, vesta::Entity* target);
    ~RelativeVelocityVector();
    virtual Eigen::Vector3d direction(double tdbSec) const;

    vesta::Entity* observer() const
    {
        return m_observer.ptr();
    }

    vesta::Entity* target() const
    {
        return m_target.ptr();
    }

private:
    vesta::counted_ptr<vesta::Entity> m_observer;
    vesta::counted_ptr<vesta::Entity> m_target;
};


class ConstantFrameDirection : public TwoVectorFrameDirection
{
public:
    ConstantFrameDirection(vesta::Frame* frame, const Eigen::Vector3d& vector);
    ~ConstantFrameDirection();
    virtual Eigen::Vector3d direction(double tdbSec) const;

    Eigen::Vector3d vector() const
    {
        return m_vector;
    }

    vesta::Frame* frame() const
    {
        return m_frame.ptr();
    }

private:
    vesta::counted_ptr<vesta::Frame> m_frame;
    Eigen::Vector3d m_vector;
};


class TwoVectorFrame : public vesta::Frame
{
public:
    enum Axis
    {
        PositiveX = 0,
        PositiveY = 1,
        PositiveZ = 2,
        NegativeX = 3,
        NegativeY = 4,
        NegativeZ = 5,
    };

    TwoVectorFrame(TwoVectorFrameDirection* primary, Axis primaryAxis,
                   TwoVectorFrameDirection* secondary, Axis secondaryAxis);
    ~TwoVectorFrame();

    virtual Eigen::Quaterniond orientation(double tdbSec) const;
    virtual Eigen::Vector3d angularVelocity(double tdbSec) const;

    TwoVectorFrameDirection* primaryDirection() const
    {
        return m_primary.ptr();
    }

    TwoVectorFrameDirection* secondaryDirection() const
    {
        return m_secondary.ptr();
    }

    Axis primaryAxis() const
    {
        return m_primaryAxis;
    }

    Axis secondaryAxis() const
    {
        return m_secondaryAxis;
    }

    static bool orthogonalAxes(TwoVectorFrame::Axis a, TwoVectorFrame::Axis b);

private:
    vesta::counted_ptr<TwoVectorFrameDirection> m_primary;
    vesta::counted_ptr<TwoVectorFrameDirection> m_secondary;
    Axis m_primaryAxis;
    Axis m_secondaryAxis;
    bool m_valid;
};

#endif // _TWO_VECTOR_FRAME_H_
