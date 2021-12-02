#include <ndn-cxx/name.hpp>
#include <ndn-cxx/face.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>


#define MAX_SEG_SIZE                    (4*1024*1024)
#define PACKET_SEG_SIZE                 (4*1024)

namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace ndndash {

class NdnDashConsumer : noncopyable
{
public:  
  NdnDashConsumer();
  
  size_t getSegmentNumber();
  
  void 
  getRequestNumber(std::string segmentChunkName);
  
  void
  getRequestData(std::string segmentChunkName, size_t segIndex);

  void
  resetSegBuffer();
	
  uint8_t 	m_segBuffer[MAX_SEG_SIZE];
  uint32_t   	m_validateBufferSize;
  uint32_t	m_readBufferSize;  
 
private:
   
  void
  onRequestNumberArrive(const Interest& interest, const Data& data);
 
  void
  onRequestDataArrive(const Interest& interest, const Data& data);

  void
  onRequestNumberTimeout(const Interest& interest);
 
  void
  onRequsetDataTimeout(const Interest& interest);

private:
  size_t m_nSegNumber;
  Face m_face;
};

} // namespace examples
} // namespace ndn

