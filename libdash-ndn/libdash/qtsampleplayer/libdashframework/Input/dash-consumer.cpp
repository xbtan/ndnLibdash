#include "dash-consumer.h"


#define MAX_SEG_SIZE                    (2*1024*1024)
#define PACKET_SEG_SIZE                 (4*1024)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace ndndash {

NdnDashConsumer :: NdnDashConsumer()
  {
    m_nSegNumber = 0;
  }
  
  size_t 
NdnDashConsumer :: getSegmentNumber()
  {
    return m_nSegNumber;
  }

  void 
NdnDashConsumer :: getRequestNumber(std::string segmentChunkName)
  {
    std::string segName = "/ustc/ndnDash/requestNumber/";
    segName += segmentChunkName;

    Interest interest(Name(segName.c_str()));
    interest.setInterestLifetime(time::milliseconds(1000));
    interest.setMustBeFresh(true);

    m_face.expressInterest(interest,
                           bind(&NdnDashConsumer::onRequestNumberArrive, this,  _1, _2),
                           bind(&NdnDashConsumer::onRequestNumberTimeout, this, _1));

    std::cout << "Sending Request Number " << interest << std::endl;

    // processEvents will block until the requested data received or timeout occurs
    m_face.processEvents();

  } 
  
  void
NdnDashConsumer :: getRequestData(std::string segmentChunkName, size_t segIndex)
  {
    std::string segName = "/ustc/ndnDash/requestData/";
    segName += segmentChunkName;
    std::ostringstream os;
    os << segIndex;
    segName += "/";
    segName += os.str();

    Interest interest(Name(segName.c_str()));
    interest.setInterestLifetime(time::milliseconds(1000));
    interest.setMustBeFresh(true);

    m_face.expressInterest(interest,
                           bind(&NdnDashConsumer::onRequestDataArrive, this,  _1, _2),
                           bind(&NdnDashConsumer::onRequsetDataTimeout, this, _1));

    std::cout << "Sending Request Data " << interest << std::endl;

    // processEvents will block until the requested data received or timeout occurs
    m_face.processEvents();
  }

  void
NdnDashConsumer :: onRequestNumberArrive(const Interest& interest, const Data& data)
  {
    memcpy(&m_nSegNumber, data.getContent().value(), data.getContent().value_size());
    std::cout << data << "segment number: " << m_nSegNumber << std::endl;
  }  
 
  void
NdnDashConsumer :: onRequestDataArrive(const Interest& interest, const Data& data)
  {
    memcpy(m_segBuffer+m_validateBufferSize, data.getContent().value(), data.getContent().value_size());
    m_validateBufferSize += data.getContent().value_size();    
    std::cout << data << std::endl;
  }

  void
NdnDashConsumer :: resetSegBuffer()
{
   m_validateBufferSize = 0;
   m_readBufferSize = 0;
   memset(m_segBuffer, 0, MAX_SEG_SIZE);
}
  void
NdnDashConsumer :: onRequestNumberTimeout(const Interest& interest)
  {
    std::cout << "Request Number Timeout " << interest << std::endl;
    m_nSegNumber = 0;
  }
 
  void
NdnDashConsumer :: onRequsetDataTimeout(const Interest& interest)
  {
    std::cout << "Request Data Timeout " << interest << std::endl;
  }

} // namespace examples
} // namespace ndn

