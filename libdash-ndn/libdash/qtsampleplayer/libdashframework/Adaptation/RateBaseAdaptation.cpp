#include "RateBaseAdaptation.h"

using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;

#define	THROUGHPUTTRACE_SIZE	(5)
#define PREDICT_HORIZON_SIZE	(5)

RateBaseAdaptation::RateBaseAdaptation          (IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet) :
                  AbstractAdaptationLogic   (mpd, period, adaptationSet)
{
    currentSegmentBufferfillPercent = 0;
    throughputTraceMaxCapacity = THROUGHPUTTRACE_SIZE;
    predictHorizonLength = PREDICT_HORIZON_SIZE;
}
RateBaseAdaptation::~RateBaseAdaptation         ()
{
    for ( int i=0; i < this->throughputTrace.size(); i++)
	delete this->throughputTrace[i];
}

LogicType       RateBaseAdaptation::GetType               ()
{
    return adaptation::RateBase;
}

IRepresentation* RateBaseAdaptation::GetRepresentation()
{
	//First Period and AdaptationSet will not change
	//Senconde MPD will not change
	predictBandwidth = 0;
	int    index = 0;
	int    minValue = 0x1FFFFFFF;
	std::cout << "Current Buffer Level:	" << currentSegmentBufferfillPercent << std::endl; 
	std::cout << "Throughput Trace:BeginTime  EndTime        SegmentLength            Bitrate"<<std::endl;
	for ( int i=0;i < throughputTrace.size(); i++ )
	{
	    std::cout << std::setw(16) << throughputTrace.at(i)->beginTime  << std::setw(16) << throughputTrace.at(i)->endTime
             	      << std::setw(16) << throughputTrace.at(i)->segmentLength << std::setw(24) <<throughputTrace.at(i)->bitRate << "  bps" << std::endl; 
	    predictBandwidth += 1.0/throughputTrace.at(i)->bitRate;
	}
	predictBandwidth = throughputTrace.size()/predictBandwidth;
	std::cout << "Predict Bandwidth:	" << predictBandwidth << std::endl;
        
 	for ( int i=0; i < adaptationSet->GetRepresentation().size(); i++)
	{
	    if ( minValue > abs(adaptationSet->GetRepresentation().at(i)->GetBandwidth()-predictBandwidth ))
	       {
		 minValue = abs(adaptationSet->GetRepresentation().at(i)->GetBandwidth()-predictBandwidth);
		 index = i;
	       }		
	}	
	representation = adaptationSet->GetRepresentation().at( index );
	return representation;
}
