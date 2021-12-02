/*QoeFairAdaptation.h
 *****************************************************************************
 * Copyright (C) 2016, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_GAMEADAPTATION_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_GAMEADAPTATION_H_

#include "AbstractAdaptationLogic.h"
#include "../MPD/AdaptationSetStream.h"
#include "../helpers/Timing.h"
//just for Unix System
#include "sys/time.h"
#include <math.h>
#include <fstream>
#include "ThroughputTrace.h"
#include <vector>
using namespace std;
namespace libdash
{
    namespace framework
    {
        namespace adaptation
        {
	    
            class GameAdaptation : public AbstractAdaptationLogic
            {
                public:
                    GameAdaptation            (dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet);
                    virtual ~GameAdaptation   ();
		    virtual dash::mpd::IRepresentation* GetRepresentation();
                    virtual LogicType               GetType             ();
                    void set_last_download_index(int index);
                    int get_last_download_index();
                    
                    double calculate_u_1(double currentrate, double up, double last_download_rate, double averg, double var, double lambda, double last_download_speed, double bufferlevel); 
                    double erf(double x);
                    double myexp(double x);
                    void Set_t(double t); 
                    double GetAvergT();
                    double GetVarT();
                    void set_up(double max);
                    double get_up();
		public:
		    uint32_t currentSegmentBufferfillPercent;
		    uint32_t throughputTraceMaxCapacity;
		    uint32_t predictHorizonLength; 
                    uint32_t buffersize;
		    std::deque<ThroughputTrace*> throughputTrace;
		    double  predictBandwidth; 
                    double last_download_bitrate;
                    double m_t[5]={0,0,0,0,0};
                    double averg_t;
                    double var_t;
                    double last_index;
                    int currentsegmentnumber;
                    double max_band;
            };
        }
    }
}

#endif /* LIBDASH_FRAMEWORK_ADAPTATION_GAMEAPTATION_H_ */

