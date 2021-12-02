/*RateBaseAdaptation.h
 *****************************************************************************
 * Copyright (C) 2013, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_RATEBASEADAPTATION_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_RATEBASEADAPTATION_H_

#include "AbstractAdaptationLogic.h"
#include "../MPD/AdaptationSetStream.h"
#include "../helpers/Timing.h"
//just for Unix System
#include "sys/time.h"
#include "math.h"
#include "ThroughputTrace.h"

namespace libdash
{
    namespace framework
    {
        namespace adaptation
        {
           class RateBaseAdaptation : public AbstractAdaptationLogic
            {
                public:
                    RateBaseAdaptation            (dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet);
                    virtual ~RateBaseAdaptation   ();
		    
		    virtual dash::mpd::IRepresentation* GetRepresentation();
                    virtual LogicType               GetType             ();
		public:
		    uint32_t currentSegmentBufferfillPercent;
		    uint32_t throughputTraceMaxCapacity;
		    uint32_t predictHorizonLength; 
		    std::deque<ThroughputTrace*> throughputTrace;
		    double  predictBandwidth; 
            };
        }
    }
}

#endif /* LIBDASH_FRAMEWORK_ADAPTATION_RATEBASEAPTATION_H_ */

