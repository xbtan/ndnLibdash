#include <ndn-cxx/face.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/security/key-chain.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>

#include <map>

#define STR_REQUEST_NUMBER_PREFIX 	"/ustc/ndnDash/requestNumber/"
#define STR_REQUEST_DATA_PREFIX		"/ustc/ndnDash/requestData/"
#define PACKET_SEG_SIZE			(4*1024)

namespace ndn {

namespace ndndash{

class NdnDasServer : boost::noncopyable
{
public:
  //Default Constructor
  NdnDasServer();
  //Default Deconstructor
  ~NdnDasServer();
  //Default run
  void run();
  //inherited from Ndn; Default OnInterest Event
  virtual void onInterest(const InterestFilter& filter, const Interest& interest);

  //Default Register Fail Handle
  virtual void onRegisterFailed(const Name& prefix, const std::string& reason);

  std::string m_szContentDir;
protected:
  //Get File Size;
  long GetFileSize(std::string filename);

  //Request Segment Number
  void onRequestSegNumber(shared_ptr<Data> data, std::string fileName );
  //Request Segment Data
  void onRequestSegData(shared_ptr<Data> data, std::string fileName, uint32_t segNo);

private:
  Face m_face;
  KeyChain m_keyChain;
  std::map<std::string, long> m_mapFileSize;


};
}
}

