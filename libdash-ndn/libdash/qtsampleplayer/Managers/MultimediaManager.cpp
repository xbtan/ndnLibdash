/*
 * MultimediaManager.cpp
 *****************************************************************************
 * Copyright (C) 2013, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "MultimediaManager.h"

using namespace libdash::framework::adaptation;
using namespace libdash::framework::buffer;
using namespace sampleplayer::managers;
using namespace sampleplayer::renderer;
using namespace dash::mpd;
using namespace std;

#define SEGMENTBUFFER_SIZE 30

MultimediaManager::MultimediaManager            (QTGLRenderer *videoElement, QTAudioRenderer *audioElement) :
                   videoElement                 (videoElement),
                   audioElement                 (audioElement),
                   mpd                          (NULL),
                   period                       (NULL),
                   videoAdaptationSet           (NULL),
                   videoRepresentation          (NULL),
                   videoLogic                   (NULL),
                   videoStream                  (NULL),
                   audioAdaptationSet           (NULL),
                   audioRepresentation          (NULL),
                   audioLogic                   (NULL),
                   audioStream                  (NULL),
                   isStarted                    (false),
                   framesDisplayed              (0),
                   segmentsDownloaded           (0),
                   isVideoRendering             (false),
                   isAudioRendering             (false),
                   adaptation                   (0)
{
    InitializeCriticalSection (&this->monitorMutex);

    this->manager = CreateDashManager();
    av_register_all();
}
MultimediaManager::~MultimediaManager           () 
{
    this->Stop();
    this->ReleaseLogStream();
    this->manager->Delete();

    DeleteCriticalSection (&this->monitorMutex);
}

IMPD*   MultimediaManager::GetMPD                           ()
{
    return this->mpd;
}
bool    MultimediaManager::Init                             (const std::string& url)
{
    EnterCriticalSection(&this->monitorMutex);

    this->mpd = this->manager->Open((char *)url.c_str());

    if(this->mpd == NULL)
    {
        LeaveCriticalSection(&this->monitorMutex);
        return false;
    }
	
    this->InitLogStream();

    LeaveCriticalSection(&this->monitorMutex);
    return true;
}
void    MultimediaManager::Start                            (int adaptation)
{
    this->adaptation = adaptation;
    /* Global Start button for start must be added to interface*/
    if (this->isStarted)
        this->Stop();

    EnterCriticalSection(&this->monitorMutex);

    if (this->videoAdaptationSet && this->videoRepresentation)
    {
        this->InitVideoRendering(0);
        this->videoStream->Start();
        this->StartVideoRenderingThread();
    }

    if (this->audioAdaptationSet && this->audioRepresentation)
    {
        this->InitAudioPlayback(0);
        this->audioElement->StartPlayback();
        this->audioStream->Start();
        this->StartAudioRenderingThread();
    }

    this->isStarted = true;

    LeaveCriticalSection(&this->monitorMutex);
}
void    MultimediaManager::Stop                             ()
{
    if (!this->isStarted)
        return;

//    EnterCriticalSection(&this->monitorMutex);

    this->StopVideo();
    this->StopAudio();

    this->isStarted = false;

//    LeaveCriticalSection(&this->monitorMutex);
}
void    MultimediaManager::StopVideo                        ()
{
    if(this->isStarted && this->videoStream)
    {
        this->videoStream->Stop();
        this->StopVideoRenderingThread();

        delete this->videoStream;
        delete this->videoLogic;

        this->videoStream = NULL;
        this->videoLogic = NULL;
    }
}
void    MultimediaManager::StopAudio                        ()
{
    if (this->isStarted && this->audioStream)
    {
        this->audioStream->Stop();
        this->StopAudioRenderingThread();

        this->audioElement->StopPlayback();

        delete this->audioStream;
        delete this->audioLogic;

        this->audioStream = NULL;
        this->audioLogic = NULL;
    }
}
bool    MultimediaManager::SetVideoQuality                  (IPeriod* period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    EnterCriticalSection(&this->monitorMutex);

    this->period                = period;
    this->videoAdaptationSet    = adaptationSet;
    this->videoRepresentation   = representation;

    if (this->videoStream)
        this->videoStream->SetRepresentation(this->period, this->videoAdaptationSet, this->videoRepresentation);

    LeaveCriticalSection(&this->monitorMutex);
    return true;
}
bool    MultimediaManager::SetAudioQuality                  (IPeriod* period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    EnterCriticalSection(&this->monitorMutex);

    this->period                = period;
    this->audioAdaptationSet    = adaptationSet;
    this->audioRepresentation   = representation;

    if (this->audioStream)
        this->audioStream->SetRepresentation(this->period, this->audioAdaptationSet, this->audioRepresentation);

    LeaveCriticalSection(&this->monitorMutex);
    return true;
}
bool    MultimediaManager::SetVideoAdaptationLogic          (libdash::framework::adaptation::LogicType type)
{
    //Currently unused, always using ManualAdaptation.
    return true;
}
bool    MultimediaManager::SetAudioAdaptationLogic          (libdash::framework::adaptation::LogicType type)
{
    //Currently unused, always using ManualAdaptation.
    return true;
}
void    MultimediaManager::AttachManagerObserver            (IMultimediaManagerObserver *observer)
{
    this->managerObservers.push_back(observer);
}
void    MultimediaManager::NotifyVideoBufferObservers       (uint32_t fillstateInPercent)
{
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->OnVideoBufferStateChanged(fillstateInPercent);
}
void    MultimediaManager::NotifyVideoSegmentBufferObservers(uint32_t fillstateInPercent)
{
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->OnVideoSegmentBufferStateChanged(fillstateInPercent);
}
void    MultimediaManager::NotifyAudioSegmentBufferObservers(uint32_t fillstateInPercent)
{
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->OnAudioSegmentBufferStateChanged(fillstateInPercent);
}
void    MultimediaManager::NotifyAudioBufferObservers       (uint32_t fillstateInPercent)
{
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->OnAudioBufferStateChanged(fillstateInPercent);
}
void    MultimediaManager::InitVideoRendering               (uint32_t offset)
{
    ofstream ofile;
    ofile.open("/home/sue/outmyexperiment.txt",ios::out|ios::app);

    if( this->adaptation == 0 )
      {
         this->videoLogic = AdaptationLogicFactory::Create(libdash::framework::adaptation::QoeFair, this->mpd, this->period, this->videoAdaptationSet);
         ofile<<"QoeFair"<<endl;
      }
    else if( this->adaptation == 1 )
      {
         this->videoLogic = AdaptationLogicFactory::Create(libdash::framework::adaptation::RateBase, this->mpd, this->period, this->videoAdaptationSet);
         ofile<<"RateBase"<<endl;
      }
   else  if(this->adaptation == 2)
      {
         this->videoLogic = AdaptationLogicFactory::Create(libdash::framework::adaptation::RateBufferBase, this->mpd, this->period, this->videoAdaptationSet);
         ofile<<"RateBufferBase"<<endl;
      }
   else  
      {
         this->videoLogic = AdaptationLogicFactory::Create(libdash::framework::adaptation::Game, this->mpd, this->period, this->videoAdaptationSet);
         ofile<<"Game"<<endl;
      }
    ofile.close();
        
    this->videoStream = new MultimediaStream(sampleplayer::managers::VIDEO, this->mpd, SEGMENTBUFFER_SIZE, 2, 0);
    this->videoStream->AttachStreamObserver(this);
    this->videoStream->SetRepresentation(this->period, this->videoAdaptationSet, this->videoRepresentation);
    this->videoStream->SetPosition(offset);
}
void    MultimediaManager::InitAudioPlayback                (uint32_t offset)
{
    this->audioLogic = AdaptationLogicFactory::Create(libdash::framework::adaptation::Manual, this->mpd, this->period, this->audioAdaptationSet);

    this->audioStream = new MultimediaStream(sampleplayer::managers::AUDIO, this->mpd, SEGMENTBUFFER_SIZE, 0, 10);
    this->audioStream->AttachStreamObserver(this);
    this->audioStream->SetRepresentation(this->period, this->audioAdaptationSet, this->audioRepresentation);
    this->audioStream->SetPosition(offset);
}
void    MultimediaManager::OnSegmentDownloaded              ()
{
    this->segmentsDownloaded++;
    //there we make some decision
    printf("\nSegmentNumber:%ld\n", this->segmentsDownloaded );
    if ( this->GetMPD() == NULL )
	return;
    this->UpdateThroughputTraceAdaptationParameter();
    IRepresentation* representation = this->videoLogic->GetRepresentation();
    if ( representation != NULL )
    {
        printf("Representation:		%d\n", representation->GetBandwidth());
	this->SetVideoQuality(this->period, this->videoAdaptationSet, representation); 
	this->LogStatistics();
    }
}

void  	MultimediaManager::InitLogStream()
{	
	time_t now;
	struct tm *timeNow;
	char	strTemp[256] = "./log/Logger-";
	
	time(&now);
	timeNow = localtime(&now);
	sprintf(strTemp+strlen(strTemp), "%s.log", asctime(timeNow));	
	
	this->log.open(strTemp);
	
}

void 	MultimediaManager::ReleaseLogStream()
{
	this->log.close();
}
void    MultimediaManager::LogStatistics()
{
   	std::streambuf *coutBuf = std::cout.rdbuf();
	std::cout.rdbuf( log.rdbuf() );
	
        struct timeval tv;
        gettimeofday(&tv,NULL);
        if(this->segmentsDownloaded == 1)
        {
          std::cout<<"Time(ms)\tNode\tSegmentNumber\tSegmentRep\tSegmentExperienceBitrate(bit/s)\tBUfferLevel(s)"<<std::endl;
        }
    
	std::cout << tv.tv_sec*1000 + tv.tv_usec/1000 << "\t";
	std::cout << "Client_0\t";
	std::cout << this->segmentsDownloaded << "\t";
        std::cout << this->videoRepresentation->GetBandwidth() << "\t";  


        if ( this->adaptation == 0 )
        {
         QoeFairAdaptation* adaptationLogic = (QoeFairAdaptation*) this->videoLogic;  
       	 std::cout << adaptationLogic->predictBandwidth << "\t";	
         std::cout << adaptationLogic->currentSegmentBufferfillPercent << std::endl;
        }
        else if ( this->adaptation == 1 ) 
        {
         RateBaseAdaptation* adaptationLogic = (RateBaseAdaptation*) this->videoLogic;
       	 std::cout << adaptationLogic->predictBandwidth << "\t";	
         std::cout << adaptationLogic->currentSegmentBufferfillPercent << std::endl;        
        }
       else if ( this->adaptation == 2 ) 
       {
         RateBufferBaseAdaptation* adaptationLogic = (RateBufferBaseAdaptation*) this->videoLogic;
       	 std::cout << adaptationLogic->predictBandwidth << "\t";	
         std::cout << adaptationLogic->currentSegmentBufferfillPercent << std::endl;
        for ( int i=0;i < adaptationLogic->throughputTrace.size(); i++ )
        {
            std::cout << std::setw(16) << adaptationLogic->throughputTrace.at(i)->beginTime  << std::setw(16) << adaptationLogic->throughputTrace.at(i)->endTime
                      << std::setw(16) << adaptationLogic->throughputTrace.at(i)->segmentLength << std::setw(24) << adaptationLogic->throughputTrace.at(i)->bitRate 
		      << "  bps" << std::endl;
        }
       }
       else 
       {
         GameAdaptation* adaptationLogic = (GameAdaptation*) this->videoLogic;

       	 std::cout << adaptationLogic->predictBandwidth << "\t";	
         std::cout << adaptationLogic->currentSegmentBufferfillPercent << std::endl;
        for ( int i=0;i < adaptationLogic->throughputTrace.size(); i++ )
        {
            std::cout << std::setw(16) << adaptationLogic->throughputTrace.at(i)->beginTime  << std::setw(16) << adaptationLogic->throughputTrace.at(i)->endTime
                      << std::setw(16) << adaptationLogic->throughputTrace.at(i)->segmentLength << std::setw(24) << adaptationLogic->throughputTrace.at(i)->bitRate 
		      << "  bps" << std::endl;
        }
   /*     std::cout << std::endl;
	std::cout << "-------------------------------------------------------" << std::endl;
	std::cout << "SegmentNumber:  	        " << this->segmentsDownloaded << std::endl;
	std::cout << "Current Buffer fill ratio:     " << adaptationLogic->currentSegmentBufferfillPercent << std::endl;
        std::cout << "Throughput Trace:BeginTime  EndTime        SegmentLength            Bitrate"<<std::endl;
        for ( int i=0;i < adaptationLogic->throughputTrace.size(); i++ )
        {
            std::cout << std::setw(16) << adaptationLogic->throughputTrace.at(i)->beginTime  << std::setw(16) << adaptationLogic->throughputTrace.at(i)->endTime
                      << std::setw(16) << adaptationLogic->throughputTrace.at(i)->segmentLength << std::setw(24) << adaptationLogic->throughputTrace.at(i)->bitRate 
		      << "  bps" << std::endl;
        }
	std::cout << "Predict Bandwidth:	" << adaptationLogic->predictBandwidth << std::endl;
	std::cout << "Representation:           " << this->videoRepresentation->GetBandwidth() << std::endl;
	std::cout << "-------------------------------------------------------" << std::endl;*/
       } 
              
        log.flush();
	std::cout.rdbuf(coutBuf);		
	
}


void    MultimediaManager::UpdateThroughputTraceAdaptationParameter()
{
    
     if ( this->adaptation == 0 )
      {
         QoeFairAdaptation* adaptationLogic = (QoeFairAdaptation*) this->videoLogic;
         adaptationLogic->currentSegmentBufferfillPercent = this->videoBufferFillPercent;
    adaptationLogic->throughputTrace.push_back(this->videoStream->dashManager->receiver->GetCurrentThroughput());	
       if ( adaptationLogic->throughputTrace.size() > adaptationLogic->throughputTraceMaxCapacity ) 
       {
	  delete adaptationLogic->throughputTrace[0];
          adaptationLogic->throughputTrace.pop_front();
       }
      }
    else if ( this->adaptation == 1 )
      {
         RateBaseAdaptation* adaptationLogic = (RateBaseAdaptation*) this->videoLogic;
        adaptationLogic->currentSegmentBufferfillPercent = this->videoBufferFillPercent;
    adaptationLogic->throughputTrace.push_back(this->videoStream->dashManager->receiver->GetCurrentThroughput());	
       if ( adaptationLogic->throughputTrace.size() > adaptationLogic->throughputTraceMaxCapacity ) 
       {
	  delete adaptationLogic->throughputTrace[0];
          adaptationLogic->throughputTrace.pop_front();
       }

      }
    else if ( this->adaptation == 2 )
     {
         RateBufferBaseAdaptation* adaptationLogic = (RateBufferBaseAdaptation*) this->videoLogic;
        adaptationLogic->currentSegmentBufferfillPercent = this->videoBufferFillPercent;
    adaptationLogic->throughputTrace.push_back(this->videoStream->dashManager->receiver->GetCurrentThroughput());	
    if ( adaptationLogic->throughputTrace.size() > adaptationLogic->throughputTraceMaxCapacity ) 
       {
	  delete adaptationLogic->throughputTrace[0];
          adaptationLogic->throughputTrace.pop_front();
       }

     }
    else
     {
         GameAdaptation* adaptationLogic = (GameAdaptation*) this->videoLogic;
        adaptationLogic->currentSegmentBufferfillPercent = this->videoBufferFillPercent;
    adaptationLogic->throughputTrace.push_back(this->videoStream->dashManager->receiver->GetCurrentThroughput());	
    if ( adaptationLogic->throughputTrace.size() > adaptationLogic->throughputTraceMaxCapacity ) 
       {
	  delete adaptationLogic->throughputTrace[0];
          adaptationLogic->throughputTrace.pop_front();
       }
     }

}

void    MultimediaManager::OnSegmentBufferStateChanged    (StreamType type, uint32_t fillstateInPercent)
{
    switch (type)
    {
        case AUDIO:
            this->NotifyAudioSegmentBufferObservers(fillstateInPercent);
            break;
        case VIDEO:
            this->NotifyVideoSegmentBufferObservers(fillstateInPercent);
	    this->videoBufferFillPercent = fillstateInPercent;
            break;
        default:
            break;
    }
}
void    MultimediaManager::OnVideoBufferStateChanged      (uint32_t fillstateInPercent)
{
    this->NotifyVideoBufferObservers(fillstateInPercent);
}
void    MultimediaManager::OnAudioBufferStateChanged      (uint32_t fillstateInPercent)
{
    this->NotifyAudioBufferObservers(fillstateInPercent);
}
void    MultimediaManager::SetFrameRate                     (double framerate)
{
    this->frameRate = framerate;
}

bool    MultimediaManager::StartVideoRenderingThread        ()
{
    this->isVideoRendering = true;

    this->videoRendererHandle = CreateThreadPortable (RenderVideo, this);

    if(this->videoRendererHandle == NULL)
        return false;

    return true;
}
void    MultimediaManager::StopVideoRenderingThread         ()
{
    this->isVideoRendering = false;

    if (this->videoRendererHandle != NULL)
    {
        JoinThread(this->videoRendererHandle);
        DestroyThreadPortable(this->videoRendererHandle);
    }
}
bool    MultimediaManager::StartAudioRenderingThread        ()
{
    this->isAudioRendering = true;

    this->audioRendererHandle = CreateThreadPortable (RenderAudio, this);

    if(this->audioRendererHandle == NULL)
        return false;

    return true;
}
void    MultimediaManager::StopAudioRenderingThread         ()
{
    this->isAudioRendering = false;

    if (this->audioRendererHandle != NULL)
    {
        JoinThread(this->audioRendererHandle);
        DestroyThreadPortable(this->audioRendererHandle);
    }
}
void*   MultimediaManager::RenderVideo        (void *data)
{
    MultimediaManager *manager = (MultimediaManager*) data;

    QImage *frame = manager->videoStream->GetFrame();

    while(manager->isVideoRendering)
    {
        if (frame)
        {
            manager->videoElement->SetImage(frame);
            manager->videoElement->update();

            manager->framesDisplayed++;

            PortableSleep(1 / manager->frameRate);

            delete(frame);
        }

        frame = manager->videoStream->GetFrame();
    }

    return NULL;
}
void*   MultimediaManager::RenderAudio        (void *data)
{
    MultimediaManager *manager = (MultimediaManager*) data;

    AudioChunk *samples = manager->audioStream->GetSamples();

    while(manager->isAudioRendering)
    {
        if (samples)
        {
            manager->audioElement->WriteToBuffer(samples->Data(), samples->Length());

            PortableSleep(1 / manager->frameRate);

            delete samples;
        }

        samples = manager->audioStream->GetSamples();
    }

    return NULL;
}
