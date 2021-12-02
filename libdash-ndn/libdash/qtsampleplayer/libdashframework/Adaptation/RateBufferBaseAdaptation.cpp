#include "RateBufferBaseAdaptation.h"

using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;

#define	THROUGHPUTTRACE_SIZE	(5)
#define PREDICT_HORIZON_SIZE	(5)
#define BUFFER_SIZE (30)
using namespace std;

RateBufferBaseAdaptation::RateBufferBaseAdaptation          (IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet) :
                  AbstractAdaptationLogic   (mpd, period, adaptationSet)
{
    currentSegmentBufferfillPercent = 0;
    throughputTraceMaxCapacity = THROUGHPUTTRACE_SIZE;
    predictHorizonLength = PREDICT_HORIZON_SIZE;
    buffersize = BUFFER_SIZE;
}
RateBufferBaseAdaptation::~RateBufferBaseAdaptation         ()
{
    for ( int i=0; i < this->throughputTrace.size(); i++)
	delete this->throughputTrace[i];
}

LogicType       RateBufferBaseAdaptation::GetType               ()
{
    return adaptation::RateBufferBase;
}

void RateBufferBaseAdaptation::set_last_download_index(int index)
{

    this->last_index = index;

}

int RateBufferBaseAdaptation::get_last_download_index()
{
    
    return this->last_index;

}

IRepresentation* RateBufferBaseAdaptation::GetRepresentation()
{
/*
	predictBandwidth = 0;
	int    index = 0;
        double highest_bitrate = 0;
        double factor = 0.66 ;               
        double bufferlevel = currentSegmentBufferfillPercent*buffersize/100;

        if( bufferlevel >= 0 && bufferlevel < 4 )
            factor = 0.66;
        if( bufferlevel >= 4 && bufferlevel < 8 )
            factor = 0.88;
        if( bufferlevel >= 8 && bufferlevel < 16 )
            factor = 1.0;
        if( bufferlevel >= 16 && bufferlevel <= 31 )
            factor = 1.2;     

	for ( int i=0;i < throughputTrace.size(); i++ )
	{

	    predictBandwidth += throughputTrace.at(i)->bitRate;
	}
	predictBandwidth = predictBandwidth/throughputTrace.size();
 
        
 	for ( int i=0; i < adaptationSet->GetRepresentation().size(); i++)
	{

           if ( adaptationSet->GetRepresentation().at(i)->GetBandwidth() < predictBandwidth*factor )
	       {
                 
		 index = i;
	       }		
	}
 	
        set_last_download_index(index);
	representation = adaptationSet->GetRepresentation().at( index );
	return representation;
*/
	int    index = 0;
      //  double highest_bitrate = 0;
        double factor = 1.0 ;               
        double speed_of_last_rep = 0.0;
        double cur_download_speed = 0.0; 
        
        if ( throughputTrace.size() > 0 ) 
           cur_download_speed = this->throughputTrace.at(throughputTrace.size()-1)->bitRate;   
 
        speed_of_last_rep = adaptationSet->GetRepresentation().at(get_last_download_index())->GetBandwidth();

if (currentSegmentBufferfillPercent*50/100 < 8)
{
        double highest_bitrate = 0.0;
 	for ( int i=0; i < adaptationSet->GetRepresentation().size(); i++)
	{
double tmp = adaptationSet->GetRepresentation().at(i)->GetBandwidth();
           if ( tmp < speed_of_last_rep && tmp < cur_download_speed)
	   {
                 if(tmp > highest_bitrate)
                 {
		     index = i;
                     highest_bitrate = tmp;
                 }
	   }		
	}
}else{
        double highest_bitrate = 999999999.99;
 	for ( int i=0; i < adaptationSet->GetRepresentation().size(); i++)
	{
          double tmp = adaptationSet->GetRepresentation().at(i)->GetBandwidth();
           if ( tmp > speed_of_last_rep && tmp < cur_download_speed)
	   {
                 if(tmp < highest_bitrate)
                 {
		     index = i;
                     highest_bitrate = tmp;
                 }
	   }		
	}

}
 	
        set_last_download_index(index);
	representation = adaptationSet->GetRepresentation().at( index );
	return representation;

}
