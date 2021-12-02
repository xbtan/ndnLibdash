/*
+ * HTTPConnection.h
+ *****************************************************************************
+ * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
+ *
+ * Email: libdash-dev@vicky.bitmovin.net
+ *
+ * This source code and its use and distribution, is subject to the terms
+ * and conditions of the applicable license agreement.
+ *****************************************************************************/

#ifndef NDNCONNECTION_H_
#define NDNCONNECTION_H_

#include "../libdash/source/portable/Networking.h"
#include "IConnection.h"
#include "../libdash/source/metrics/HTTPTransaction.h"
#include "../libdash/source/metrics/TCPConnection.h"

#include "dash-consumer.h"

namespace libdash
{
    namespace framework
    {
        namespace input
        {
            class NDNConnection : public dash::network::IConnection
            {
                public:
                    NDNConnection          ();
                    virtual ~NDNConnection ();

                    virtual bool    PreDownload       (dash::network::IChunk *chunk);
                    virtual int     Read        (uint8_t *data, size_t len, dash::network::IChunk *chunk);
                    virtual int     Peek        (uint8_t *data, size_t len, dash::network::IChunk *chunk);
			   
 		    /*
+                     *  IDASHMetrics
+                     */
                    const std::vector<dash::metrics::ITCPConnection *>&     GetTCPConnectionList    () const;
                    const std::vector<dash::metrics::IHTTPTransaction *>&   GetHTTPTransactionList  () const;                    
		public: 
	            std::vector<dash::metrics::TCPConnection *>     tcpConnections;
                    std::vector<dash::metrics::HTTPTransaction *>   httpTransactions;
		   
		    ndn::ndndash::NdnDashConsumer m_dashConsumer;			
            };
        }
    }
}
#endif 
