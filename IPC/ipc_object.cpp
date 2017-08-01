
/*缺少的头文件自己添加*/



class ThreadSafeHelper{
	private:
		/*不能使用默认构造函数*/
		ThreadSafeHelper(){;}
		
		/**/
		pthread_mutex_t* mutex;
		
		
	public:
		ThreadSafeHelper( pthread_mutex_t* m){
			mutex = m;
			pthread_mutex_lock( mutex );
		}
		
		~ThreadSafeHelper(){
			pthread_mutex_unlock( mutex );
		}
};




class IpcClient{
	private:
		/*这个对象使不可复制的，故下面两个函数设为私有*/
		IpcClient(const IpcClient& other){;}
		IpcClient& operator=(const IpcClient& other){return *this;}
	
		/*消息队列*/
		struct IpcMessageQueue mMessageQueue;
		
		/*ipc对象*/
		IpcObject_t* mObj;
		
		/*路径*/
		std::string mPath;
		
		/*接口*/
		std::string mInterface;
		
		/*名字*/
		std::string mName;
		
		/*线程安全对象*/
		pthread_mutex_t mMutex;
		
	
	public:
		/*构造*/
		IpcClient(){
			mObj = NULL;
			
			IPCMQ_init( &mMessageQueue );
			
			pthread_mutex_init( &mMutex , NULL );
		}
		
		/*析构*/
		~IpcClient(){
		
			pthread_mutex_destroy( &mMutex );
		
			if( mObj )
				IPCOBJ_disconnect( mObj );
			
			if( IPCMQ_is_valid( &mMessageQueue ) )
				IPCMQ_free( &mMessageQueue );	
				
		}
		
		/*获取消息*/
		bool GetMessage( void* message , int* size ){
			unsigned int* p;
			int s;
			
			/*该对象在函数推出时自动释放*/
			ThreadSafeHelper helper(&mMutex);
			
			/*验证当前的MessageQueue中是否有效*/			
			if( !IPCMQ_is_valid( & mMessageQueue) ){
			
				/*如果无效则接收新的消息队列*/
				if( IPCMQ_init_and_recv( &mMessageQueue ， mObj ) != R_OK )
						return false;
			}
			
			/*如果消息队列中没有消息了则释放该消息队列*/
			if( B_FALSE == IPCMQ_get_message( &mMessageQueue ,&p , &s ) ){
				IPCMQ_free( &mMessageQueue );	
				return false;
			}
			
			/*成功获取消息*/
			memcpy( message , p , s );
			*size = s;
			return true;
		}
		
		
		/*发送消息*/
		bool SendMessage( void* message , int size , const std::string& targetPath  ){
			bool ret;
			
			/*该对象在函数推出时自动释放*/
			ThreadSafeHelper helper(&mMutex);
		
			/*创建一个发送队列*/
			struct IpcMessageQueue q;
			if( R_OK != IPCMQ_init_for_send( &q , targetPath.c_str() , mInterface.c_str() ) )
				return false;
			
			/*放入数据*/
			if( R_OK != IPCMQ_put_message( &q , message , size ) ){
				IPCMQ_free( &q );
				return false;
			}
			
			/*发送*/
			ret = ( R_OK == IPCMQ_send( &q ) );
			
			/*释放*/
			IPCMQ_free( &q );
			
			return ret;
		}
		
		
		/*与IPC建立连接*/
		bool Connect( const std::string& name , const std::string& interface , const std::string& path ){
			
			/*该对象在函数推出时自动释放*/
			ThreadSafeHelper helper(&mMutex);
			
			/*当前已经有一个链接建立*/
			if( NULL != mObj )
				return false;
				
			/*监听规则的字符串对象*/
			std::string rules = "type='signal',path='" + path + "'";
			/*"type='signal',path='XXXX'"*/
			
			/*链接IPC*/
			mObj = IPCOBJ_connect( name.c_str() , rules.c_str() );
			if( NULL == mObj )
				return false;
			
			/*记录一系列属性*/
			mName = name;
			mInterface = interface;
			mPath = path;
			
			return true;
		}
		
		
		/*与ipc断开连接*/
		void Disconnect(){
		
			/*该对象在函数推出时自动释放*/
			ThreadSafeHelper helper(&mMutex);
			 
			if( mObj ){
				IPCOBJ_disconnect( mObj );
				mObj = NULL;
			}
		}
		
};

