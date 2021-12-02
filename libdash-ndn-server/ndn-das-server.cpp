#include "ndn-das-server.hpp"

#include <fstream>
#include <iostream>

namespace ndn{

namespace ndndash{

NdnDasServer::NdnDasServer()
{
  m_szContentDir = "/home/yfshao/Data/Dash/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/";
 // m_szContentDir = "/home/sue/multimediaData/";
}

NdnDasServer::~NdnDasServer()
{
   std::map<std::string, long>().swap(m_mapFileSize);
}

void
NdnDasServer::run()
{
   m_face.setInterestFilter("/ustc/ndnDash",
                             bind(&NdnDasServer::onInterest, this, _1, _2),
                             RegisterPrefixSuccessCallback(),
                             bind(&NdnDasServer::onRegisterFailed, this, _1, _2));
    m_face.processEvents();
}

void
NdnDasServer::onInterest(const InterestFilter& filter, const Interest& interest)
{
    //Out Come Interest
    std::cout << "\n<< I: " << interest << std::endl;
    std::string strRequestNumber(STR_REQUEST_NUMBER_PREFIX);
    std::string strRequestData(STR_REQUEST_DATA_PREFIX);

    shared_ptr<Data> data = make_shared<Data>();
    std::string fileName;
    uint32_t segNo = 0;

    data->setName( interest.getName() );
    //if segment number
    if ( interest.getName().toUri().compare(0, strRequestNumber.size(), strRequestNumber) == 0 )
    {
        fileName = interest.getName().toUri().substr(strRequestNumber.size());
        std::cout << "Get Request Number: " << fileName << std::endl;
     	onRequestSegNumber(data, fileName);
    }else// if segment data
    if ( interest.getName().toUri().compare(0, strRequestData.size(), strRequestData) == 0 )
    {
        fileName = interest.getName().toUri().substr(strRequestData.size());
        segNo = (uint32_t)atoi(interest.getName().toUri().substr( interest.getName().toUri().rfind('/') +1).c_str());
        std::cout << "Get Request Data: " << fileName << "\tSeg Index:" << segNo << std::endl;
        onRequestSegData(data, fileName,  segNo);
    }else
    {
     	std::cout << "<< Request can't recognition I: " << interest << std::endl;
    }
}

void
NdnDasServer::onRegisterFailed(const Name& prefix, const std::string& reason)
{
     std::cerr << "ERROR: Failed to register prefix \""
              << prefix << "\" in local hub's daemon (" << reason << ")"
              << std::endl;
     m_face.shutdown();
}

long
NdnDasServer::GetFileSize(std::string fileName)
{
    fileName = m_szContentDir + fileName;
    std::cout << "Seg Local Repo Directory: " <<  fileName << std::endl;

    // check if is already in m_fileSizes
  if (m_mapFileSize.find(fileName) != m_mapFileSize.end())
  {
    return m_mapFileSize[fileName];
  }
  // else: query disk for file size

  struct stat stat_buf;
  int rc = stat(fileName.c_str(), &stat_buf);

  if (rc == 0)
  {
    m_mapFileSize[fileName] = stat_buf.st_size;
    return stat_buf.st_size;
  }
  // else: file not found
  std::cout << "ERROR: File not found: " << fileName << std::endl;
  return -1;
}

void
NdnDasServer::onRequestSegNumber(shared_ptr<Data> data, std::string fileName)
{
   long fileSize = GetFileSize(fileName);
   size_t segNumber = (size_t)fileSize/PACKET_SEG_SIZE;
   if ( fileSize % PACKET_SEG_SIZE != 0 )
       segNumber++;

  data->setFreshnessPeriod(time::seconds(10));
  data->setContent((uint8_t*)&segNumber, sizeof(size_t));
  m_keyChain.sign(*data);
  m_face.put(*data);
}

void
NdnDasServer::onRequestSegData(shared_ptr<Data> data, std::string fileName, uint32_t seqNo)
{
  // go to pointer seqNo*PACKET_SEG_SIZE in file
  fileName = fileName.substr(0, fileName.rfind('/'));
  long fileSize = GetFileSize(fileName);
  if ( fileSize == 0 )
    return;
  fileName = m_szContentDir + fileName;
  FILE* fp = fopen(fileName.c_str(), "rb");
  fseek(fp, seqNo * PACKET_SEG_SIZE, SEEK_SET);

  size_t nCount =  fileSize < (seqNo+1)*PACKET_SEG_SIZE  ? fileSize-seqNo*PACKET_SEG_SIZE : PACKET_SEG_SIZE;
  auto buffer = make_shared< ::ndn::Buffer>(nCount);
  fread(buffer->get(), sizeof(uint8_t),  nCount, fp);
  fclose(fp);

  data->setFreshnessPeriod(time::seconds(10));
  data->setContent((uint8_t*)buffer->get(), nCount);
  m_keyChain.sign(*data);
  m_face.put(*data);
}

}//ndndash
}//ndn

int
main(int argc, char* argv[])
{
 ndn::ndndash::NdnDasServer singleServer;
 if ( argc  > 1 )
     singleServer.m_szContentDir = argv[1];

 try {
    singleServer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}


