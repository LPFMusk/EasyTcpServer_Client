enum CMD               //枚举来定义命令
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;  //数据长度
	short cmd;         //数据命令
};
struct Login :public DataHeader
{
	//登录账号
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};
struct LoginResult :public DataHeader
{  //登录数据
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0; //自己定义，用于验证
	}
	int result;
};
struct Logout :public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};
struct LogoutResult :public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0; //自己定义，用于验证
	}
	int result;
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sockID = 0; //自己定义，用于验证
	}
	int sockID;
};