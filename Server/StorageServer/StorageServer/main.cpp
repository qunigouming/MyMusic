// StorageServer.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <grpcpp/grpcpp.h>

using namespace std;

int main()
{
    std::string str = "hello world";
    std::cout << "Original: " << str << std::endl;
    std::string upper_str = boost::algorithm::to_upper_copy(str);
    std::cout << "Uppercase: " << upper_str << std::endl;

    std::cout << "gRPC Version: " << grpc_version_string() << std::endl;
	return 0;
}
