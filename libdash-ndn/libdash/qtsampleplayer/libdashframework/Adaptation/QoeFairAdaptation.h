/*QoeFairAdaptation.h
 *****************************************************************************
 * Copyright (C) 2016, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_QOEFAIRADAPTATION_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_QOEFAIRADAPTATION_H_

#include "AbstractAdaptationLogic.h"
#include "../MPD/AdaptationSetStream.h"
#include "../helpers/Timing.h"
//just for Unix System
#include "sys/time.h"
#include "math.h"
#include <fstream>
#include "ThroughputTrace.h"

namespace libdash
{
    namespace framework
    {
        namespace adaptation
        {
	    
            class QoeFairAdaptation : public AbstractAdaptationLogic
            {
                public:
                    QoeFairAdaptation            (dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet);
                    virtual ~QoeFairAdaptation   ();
		    virtual dash::mpd::IRepresentation* GetRepresentation();
                    virtual LogicType               GetType             ();
                    double calculateytility(double currentrate, double last_download_speed,double bufferlevel,double last_download_rate,double cachehitrate);
                    void set_last_download_index(int index);
                    int get_last_download_index();
		public:
		    uint32_t currentSegmentBufferfillPercent;
		    uint32_t throughputTraceMaxCapacity;
		    uint32_t predictHorizonLength; 
                    uint32_t buffersize;
		    std::deque<ThroughputTrace*> throughputTrace;
		    double  predictBandwidth; 
                    double last_download_bitrate;
                    double last_index;
            };
        }
    }
}

#endif /* LIBDASH_FRAMEWORK_ADAPTATION_QOEFAIRAPTATION_H_ */

