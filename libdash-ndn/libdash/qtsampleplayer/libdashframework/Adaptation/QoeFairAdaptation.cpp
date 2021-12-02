#include "QoeFairAdaptation.h"

using namespace std;
using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;

#define	THROUGHPUTTRACE_SIZE	(5)
#define PREDICT_HORIZON_SIZE	(5)
#define BUFFER_SIZE (30)
QoeFairAdaptation::QoeFairAdaptation          (IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet) :
                  AbstractAdaptationLogic   (mpd, period, adaptationSet)
{
    currentSegmentBufferfillPercent = 0;
    throughputTraceMaxCapacity = THROUGHPUTTRACE_SIZE;
    predictHorizonLength = PREDICT_HORIZON_SIZE;
    buffersize = BUFFER_SIZE;
    last_download_bitrate = 89283;
    last_index = 0;
}

QoeFairAdaptation::~QoeFairAdaptation         ()
{
    for ( int i=0; i < this->throughputTrace.size(); i++)
	delete this->throughputTrace[i];
}

LogicType       QoeFairAdaptation::GetType               ()
{
    return adaptation::QoeFair;
}

void QoeFairAdaptation::set_last_download_index(int index)
{

    this->last_index = index;

}

int QoeFairAdaptation::get_last_download_index()
{
    
    return this->last_index;

}

double QoeFairAdaptation::calculateytility(double currentrate, double last_download_speed,double bufferlevel,double last_download_rate,double cachehitrate)
{
  double alpha = 0.05;
  double beta = 2;
  double fcache = 0;
  double K = 0.7;
  double stallingtime = 0;
  if(2*currentrate/last_download_speed - bufferlevel > 0)
    stallingtime = 2*currentrate/last_download_speed - bufferlevel;
  double QoE = pow(currentrate,0.6) - alpha*abs(currentrate - last_download_rate) - beta*stallingtime;
  
  //cost
  /*double fbuffer = 31.0 - bufferlevel ;
  fbuffer = fbuffer/30.0;*/
  double fbuffer = 0.0;
  if(bufferlevel < 6)
     fbuffer = 1.4;
  else if(bufferlevel >= 6 && bufferlevel <= 20)
     fbuffer = 1.0;
  else
     fbuffer = 0.7;
  if(currentrate ==  cachehitrate)
    fcache = 0.6;
  else fcache = 1.0;
  //ofile<<"fbuffer"<<fbuffer<<"	fcache:"<<fcache<<"	currentrate"<<currentrate<<endl;
 // double cost = fbuffer*fcache*currentrate*currentrate/(last_download_speed*last_download_speed);
  // double cost = fcache*currentrate*currentrate/(last_download_speed*last_download_speed);
   double cost = fbuffer*fcache*exp(currentrate-last_download_speed);
  return QoE-K*cost; 
}


IRepresentation* QoeFairAdaptation::GetRepresentation()
{
        ofstream ofile;
        ofile.open("/home/sue/outmyexperiment.txt",ios::out|ios::app);     
       // ofile<<"start"<<endl;
	//First Period and AdaptationSet will not change
	//Sencond MPD will not change
	predictBandwidth = 0;
        double currentrate=0.0;
        double current_rate=0.0;
	int    index = 0;
	int    minValue = 0x1FFFFFFF;
        double utility = 0.0;
        double cachehitrate=0.0;
        double currentutility = -99999999;
	std::cout << "Current Buffer Level:	" << currentSegmentBufferfillPercent << std::endl; 
	//std::cout << "Throughput Trace:BeginTime  EndTime        SegmentLength            Bitrate"<<std::endl;
	for ( int i=0;i < throughputTrace.size(); i++ )
	{
       // std::cout << std::setw(16) << throughputTrace.at(i)->beginTime  << std::setw(16) << throughputTrace.at(i)->endTime
         //      << std::setw(16) << throughputTrace.at(i)->segmentLength << std::setw(24) <<throughputTrace.at(i)->bitRate << "  bps" << std::endl; 
	    predictBandwidth += 1.0/throughputTrace.at(i)->bitRate;
      
	}
	predictBandwidth = throughputTrace.size()/predictBandwidth;  //tiaohepingjunshu
	std::cout << "Predict Bandwidth:	" << predictBandwidth << std::endl;
        
        double last_download_rate = adaptationSet->GetRepresentation().at(get_last_download_index())->GetBandwidth()/1000000.0;

        double bufferlevel = currentSegmentBufferfillPercent*buffersize/100;
 	for ( int i=0; i < adaptationSet->GetRepresentation().size(); i++)
	{

           //qi(k)
        currentrate = adaptationSet->GetRepresentation().at(i)->GetBandwidth()/1000000.0;
      //  ofile<<"predictBandwidth:"<<predictBandwidth<<endl;
        utility = calculateytility(currentrate, predictBandwidth, bufferlevel, last_download_rate, cachehitrate);

         ofile<<" rate:"<< currentrate<<"      "<<"utility:"<<utility<<"      "<<endl;
         if(utility>currentutility)
         {
            currentutility = utility;
            index = i; 
            current_rate =  currentrate;
         }
		
	}
       // ofile<<endl;
      //  ofile<<"current_rate:"<<current_rate<<"index:"<<index<<endl;
        ofile<<"index:"<<index<<endl;
        set_last_download_index(index);
       // std::cout << "index:	" << index << std::endl;
        ofile.close(); 
	representation = adaptationSet->GetRepresentation().at( index );
	return representation;
}
