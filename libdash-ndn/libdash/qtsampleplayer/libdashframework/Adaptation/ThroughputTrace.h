#ifndef THROUGHPUTTRACE_H_
#define THROUGHPUTTRACE_H_

namespace libdash
{
    namespace framework
    {
        namespace adaptation
        {
	    struct ThroughputTrace
	    {
		long beginTime;
		long endTime;
		uint32_t segmentLength;   //size
		//bps
		uint32_t bitRate;
		ThroughputTrace()
		{
		   beginTime = 0;
		   endTime = 1;
		   segmentLength = 0;
		   bitRate = 0;
		} 
	    };
           
        }
    }
}


#endif /* THROUGHPUTTRACE_H_ */
