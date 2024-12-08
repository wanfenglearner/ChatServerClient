
#include "chatclient.h"

int main(int argc,  char** argv)
{
	if (argc < 3)
	{
		std::cout << "错的的输入" << "./Client.out ip port" << std::endl;
		exit(-1);
	}

	std::string ip = argv[1];
	int port = atoi(argv[2]);

	ChatClient client(ip, port);
	if (client.connectServer())
	{
		// 读入操作信息
		while (1)
		{
			std::cout << "----------1.登录----------" << std::endl;
			std::cout << "----------2.注册----------" << std::endl;
			std::cout << "----------3.退出----------" << std::endl;
			int n;
			std::cin >> n;
			std::cin.get();

			switch (n)
			{
			case 1:
				client.login();
				break;
			case 2:
				client.reg();
				break;
			case 3:
				exit(0);
				break;
			default:
				std::cout << "无效的命令, 重新输入" << std::endl;
				break;
			}
		}


	}


	return 0;
}