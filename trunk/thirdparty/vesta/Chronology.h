/*
 * $Revision: 562 $ $Date: 2011-02-11 12:18:04 -0800 (Fri, 11 Feb 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_CHRONOLOGY_H_
#define _VESTA_CHRONOLOGY_H_

#include "Object.h"
#include <vector>


namespace vesta
{

class Arc;

/** A Chronology represents a sequence of time contiguous arcs. Each
  * VESTA entity has a single chronology.
  */
class Chronology : public Object
{
public:
    Chronology();
    ~Chronology();

    /** Return the time at which the chronology begins. The time is in
      * seconds since J2000 TDB.
      */
    double beginning() const
    {
        return m_beginning;
    }

    void setBeginning(double t);

    /** Return the time at which the chronology ends. The time is in
      * seconds since J2000 TDB. An empty chronology will have
      * beginning() == ending().
      */
    double ending() const
    {
        return m_beginning + m_duration;
    }

    /** Get the duration of the chronology in seconds.
      */
    double duration() const
    {
        return m_duration;
    }

    Arc* activeArc(double t) const;
    Arc* firstArc() const;
    Arc* lastArc() const;
    Arc* arc(unsigned int index) const;
    unsigned int arcCount() const;

    void addArc(Arc* arc);

    /** Return true if this chronology contains any arcs, false if it does
      * not.
      */
    bool empty() const
    {
        return m_arcSequence.empty();
    }

    /** Return true if is between the beginning and ending times of this
      * chronology. The test is beginning <= t <= ending.
      *
      * \param t time in seconds since J2000 TDB
      */
    bool includesTime(double t) const
    {
        return !m_arcSequence.empty() && t >= beginning() && t <= ending();
    }

    void clearArcs();

private:
    std::vector<counted_ptr<Arc> > m_arcSequence;
    double m_beginning;
    double m_duration;
};

} // namespace

#endif // _VESTA_ARC_H_
