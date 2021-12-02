ndn-das-server.cpp 

NDN 服务器节点，接受到达的兴趣包请求，并返回相应的具体数据信息；

trace-gen.cpp

带宽数据生成代码

编译方式：
g++ -std=c++0x -std=c++11 -pedantic -Wall -O0 -g -lpthread ndn-das-server.cpp -o ndn-das-server $(pkg-config --cflags --libs libndn-cxx)

g++ trace-gen.cpp -std=c++11 -o trace-gen

运行方式：
代码很短，仅仅需要输入读取视频片段文件的路径
