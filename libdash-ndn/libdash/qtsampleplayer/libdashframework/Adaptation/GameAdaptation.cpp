#include "GameAdaptation.h"

using namespace std;
using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;

#define	THROUGHPUTTRACE_SIZE	(5)
//#define PREDICT_HORIZON_SIZE	(5)
#define BUFFER_SIZE (30)
const double mu = 1.0;
const double alpha = 0.05;
const double beta = 0.5;

GameAdaptation::GameAdaptation          (IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet) :
                  AbstractAdaptationLogic   (mpd, period, adaptationSet)
{
    currentSegmentBufferfillPercent = 0;
    throughputTraceMaxCapacity = THROUGHPUTTRACE_SIZE;
   // predictHorizonLength = PREDICT_HORIZON_SIZE;
    buffersize = BUFFER_SIZE;
    last_download_bitrate = 0.089283;
    averg_t = 2.0;
    var_t = 0.75;
    last_index = 0;
    currentsegmentnumber = 0;
    max_band = 5.0;
}

GameAdaptation::~GameAdaptation         ()
{
    for ( int i=0; i < this->throughputTrace.size(); i++)
	delete this->throughputTrace[i];
}

LogicType       GameAdaptation::GetType               ()
{
    return adaptation::Game;
}

void GameAdaptation::set_last_download_index(int index)
{

    this->last_index = index;

}

int GameAdaptation::get_last_download_index()
{
    
    return this->last_index;

}

void GameAdaptation::set_up(double max)
{
    if(max>max_band)
     max_band = max;
}

double GameAdaptation::get_up()
{
    return max_band;
}

void GameAdaptation::Set_t(double t)
{ 
    double sum = t;
    int count = 1;
    for(int i = 3; i>=0; i--)
    {
       m_t[i+1] = m_t[i];
       if(m_t[i] > 0)
       {
           sum += m_t[i];
           count++;
       }
    }
    m_t[0] = t;
    if(count>1)
    averg_t = sum / count;
    double sum_of_var = 0.0;
    for(int i = 0; i < 5; i++)
    {
       if(m_t[i] > 0)
          sum_of_var += (m_t[i] - averg_t)*(m_t[i] - averg_t);  
    }
    if(count > 1)
       var_t = sqrt( sum_of_var / (count-1) );  
}
double GameAdaptation::erf(double x)
{
    // constants
    double a1 =  0.254829592;
    double a2 = -0.284496736;
    double a3 =  1.421413741;
    double a4 = -1.453152027;
    double a5 =  1.061405429;
    double p  =  0.3275911;

    // Save the sign of x
    int sign = 1;
    if (x < 0)
        sign = -1;
    x = fabs(x);

    // A&S formula 7.1.26
    double t = 1.0/(1.0 + p*x);
    double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*myexp(-x*x);

    return sign*y;
}
double GameAdaptation::GetAvergT()
{
   return averg_t;
}

double GameAdaptation::GetVarT()
{
   return var_t;
}
double GameAdaptation::myexp(double x)
{
    int i,k,m = 1,t = 10;
    int xm=(int)x; 
    double sum = 1.0, e = 1.0, ef = 1.0, z = 1, sub = x - xm;
    if (xm<0) 
    {     
       xm=(-xm); 
       for(k=0;k<xm;k++)
          ef*=2.718281;
       e/=ef;
    } 
    else 
    { 
      for(k=0;k<xm;k++)
         e*=2.718281;
    }
    for(i=1;i<t;i++)
    {
        m*=i; 
        z*=sub;  
        sum+=z/m;
    }
    return sum*e;
}

double GameAdaptation::calculate_u_1(double currentrate, double up, double last_download_rate, double averg, double var, double lambda, double last_download_speed, double bufferlevel)
{
   vector<double> f;
  
   double maxb = 1;
   double adjust = 0;
   double y = 100000;
   double sum = 0;
   double step = 0.2;
   //search the table for adjust
   double x2 = (up-averg)/var;
   double x1 = (0-averg)/var;
   adjust = 0.5*var*(erf(x2/sqrt(2.0))-erf(x1/sqrt(2.0)));
   adjust = 1/adjust;
   for(double t = 0.05; t <= up; t += step)
   {

      double tmp_t = (t-averg)/var;
      double stalling =  currentrate*2/t - bufferlevel;
      if(stalling < 0)
          stalling  = 0;
   //   double temp = adjust*0.39895*myexp(-0.5*tmp_t*tmp_t)*(pow(currentrate,0.6)-alpha*abs(currentrate-last_download_rate) - beta*stalling)/(1+myexp(currentrate - lambda*maxb));
      double qoe = pow(currentrate,0.6)-alpha*abs(currentrate-last_download_rate) - beta*stalling;
      double temp = adjust*0.39895*myexp(-0.5*tmp_t*tmp_t)*qoe/(1+myexp(currentrate - lambda*t));
      f.push_back(temp);

   }
   int n = f.size()-1;
   sum = f[0] + f[n];
   for(int i = 1; i < n; i++)
   {
      if(i%2 == 0) 
	  sum += 2*f[i];
      else
          sum += 4*f[i];
   }
   return sum*step/3;
}

IRepresentation* GameAdaptation::GetRepresentation()
{
        ofstream ofile;
        ofile.open("/home/sue/game.txt",ios::out|ios::app);     
	//First Period and AdaptationSet will not change
	//Sencond MPD will not change
	predictBandwidth = 0;
        double u_1 = -10000;
        double current_u_1 = -10000.0; 
        double up = get_up();   //link limit
        //update t
        double t = 0.0;
        double currentrate = 0.0;
        double averg = 0.3, var = 0.05;
        int index = -1;
       
	for ( int i=0;i < throughputTrace.size(); i++ )
	{
	    predictBandwidth += 1.0/throughputTrace.at(i)->bitRate;
      
	}
	predictBandwidth = throughputTrace.size()/predictBandwidth;  //tiaohepingjunshu
        double last_download_speed = predictBandwidth/1000000;
        //qi(k-1)
        set_up(throughputTrace.at(throughputTrace.size()-1)->bitRate/1000000.0);
       // predictBandwidth = throughputTrace.at(throughputTrace.size()-1)->bitRate;
       // double last_download_speed = predictBandwidth/1000000.0;
        double last_download_rate = adaptationSet->GetRepresentation().at(get_last_download_index())->GetBandwidth()/1000000.0;
        ofile<<"currentsegmentnumber:"<<currentsegmentnumber<<"\tup:"<<up<<"\tlast_download_speed:"<<last_download_speed<<"\t"<<"last_download_rate:"<<last_download_rate<<endl;  
        //bufferlevel&fbuffer
        double bufferlevel = currentSegmentBufferfillPercent*buffersize/100.0;
        double bufferpercent = currentSegmentBufferfillPercent;
        double lambda = 1.0;
        if(bufferpercent < 33)
           lambda = 0.8;

        Set_t(last_download_speed);

        double averg_t = GetAvergT();
        double var_t = GetVarT();
        double x2 = (up-averg_t)/var_t;
        double x1 = (0-averg_t)/var_t;
        double t1 = 0.39894*myexp(-0.5*(pow(x1,2.0)));
        double t2 = 0.39894*myexp(-0.5*(pow(x2,2.0)));
        double t3 = 0.5*(erf(x2)- erf(x1));  
        averg = averg_t + var_t*(t1 - t2)/t3;
        double t4 = (x1*t1-x2*t2)/t3;  
        var = var_t*var_t*(1 + t4 - pow((t1-t2)/t3,2.0));
        var = sqrt(var);
        ofile<<"averg_t:"<<averg_t<<"\tvar_t:"<<var_t<<"averg:"<<averg<<"\t"<<"var:"<<var<<endl;
 	for ( int i=0; i < adaptationSet->GetRepresentation().size(); i++)
	{

           //qi(k)
           currentrate = adaptationSet->GetRepresentation().at(i)->GetBandwidth();
           currentrate /= 1000000.0;
           current_u_1 = calculate_u_1(currentrate, up, last_download_rate, averg, var, lambda, last_download_speed, bufferlevel);
 
           ofile<<"currentrate:"<<currentrate<<"  current_u_1:"<<current_u_1<<endl;
           if(current_u_1 > u_1)
           {
             u_1 = current_u_1;
             index = i;
           }
        } 
       
       if(index == -1)
       {
           if(bufferpercent < 5.0)
             index = 0;
           else
             index = get_last_download_index();
       }
        ofile<<"index:"<<index<<endl;        
        set_last_download_index(index);
       // std::cout << "index:	" << index << std::endl;
        ofile.close(); 
        currentsegmentnumber++;
	representation = adaptationSet->GetRepresentation().at( index );
	return representation;
}
