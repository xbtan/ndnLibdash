#include "NDNConnection.h"

using namespace libdash::framework::input;

using namespace dash;
using namespace dash::network;
using namespace dash::metrics;

NDNConnection::NDNConnection()
{
}

NDNConnection::~NDNConnection()
{
}

bool    
NDNConnection::PreDownload(dash::network::IChunk *chunk)
{
  this->m_dashConsumer.resetSegBuffer();
  int pos = chunk->AbsoluteURI().find("BigBuckBunny/",0);
  std::string chunkName = chunk->AbsoluteURI().substr(pos+strlen("BigBuckBunny/"), chunk->AbsoluteURI().size()-pos-4); 
  
  try{ 
  //Get the Media Object Data,so we can deal with it.
  m_dashConsumer.getRequestNumber(chunkName);
  for (size_t i=0; i < m_dashConsumer.getSegmentNumber(); i++)
    {
      m_dashConsumer.getRequestData(chunkName, i);
    } 
 }
 catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
}

int     
NDNConnection::Read(uint8_t *data, size_t len, dash::network::IChunk *chunk)
{
  if ( m_dashConsumer.m_validateBufferSize == m_dashConsumer.m_readBufferSize )
     return 0;
  else{
     //copy the media object data to chunk
     int readLen = len < m_dashConsumer.m_validateBufferSize-m_dashConsumer.m_readBufferSize ? len : m_dashConsumer.m_validateBufferSize-m_dashConsumer.m_readBufferSize;
     memcpy(data, m_dashConsumer.m_segBuffer+m_dashConsumer.m_readBufferSize, readLen);
     m_dashConsumer.m_readBufferSize += readLen;
     return readLen;
  }
}

int     
NDNConnection::Peek(uint8_t *data, size_t len, dash::network::IChunk *chunk)
{
  return -1;
}

const std::vector<dash::metrics::ITCPConnection *>&     
NDNConnection::GetTCPConnectionList() const
{
  return (std::vector<ITCPConnection *> &) this->tcpConnections;
}

const std::vector<dash::metrics::IHTTPTransaction *>&   
NDNConnection::GetHTTPTransactionList() const
{
   return (std::vector<IHTTPTransaction *> &) this->httpTransactions;
}
