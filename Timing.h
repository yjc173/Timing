#pragma once
#include <map>
#include <atlstr.h>

class CTiming
{
public:
	CTiming(const CString& strTimeInfo);
	~CTiming(void);
private:
	/**
	*  @brief : 时间信息结构,用于存放时间信息
	*
	*
	*  @note
	*  @param : 
	*  @defenc: 
	*  @return: 
	*  @author: yjc
	*  @date  : 2/7/2016 15:50 
	*/
	struct TimeInfo
	{
		TimeInfo(): ElapsedTime(0), nTime(0), dwThreadId(0)
		{

		}
		DWORD dwThreadId;		// 记录线程ID
		LONGLONG ElapsedTime;	// 记录经过的时间
		int nTime;				// 记录运行的次数
	};

	/**
	*  @brief : 时间记录类,用于记录时间信息
	*
	*
	*  @note
	*  @param : 
	*  @defenc: 
	*  @return: 
	*  @author: yjc
	*  @date  : 2/7/2016 15:50 
	*/
	class CTimeRecord
	{
	public:
		CTimeRecord(void);
		~CTimeRecord(void);
	public:
		/**
		*  @brief : 用于添加时间信息
		*
		*
		*  @note
		*  @param : 
		*  @defenc: 
		*  @return: 
		*  @author: yjc
		*  @date  : 2/7/2016 15:55 
		*/
		void InsertTimeInfo(const CString& strInfo, const LONGLONG& ElapsedTime);
	private:
	/**
	*  @brief : 记录时间信息到数据库文件
	*
	*
	*  @note
	*  @param : 
	*  @defenc: 
	*  @return: 
	*  @author: yjc
	*  @date  : 2/7/2016 15:55 
	*/
		void RecordTimeInfo(void);
	private:
		std::multimap<const CString, TimeInfo> m_g_mapTimeInfo;
	};

	/**
	*  @brief : 进程锁类,用于进程间的同步
	*
	*
	*  @note
	*  @param : 
	*  @defenc: 
	*  @return: 
	*  @author: yjc
	*  @date  : 2/7/2016 15:51 
	*/
	class CProcessLocker
	{
	public:
		CProcessLocker(void);
		~CProcessLocker();
	private:
		HANDLE m_hMutex;
	};

	/**
	*  @brief : 线程锁类,用于同步线程
	*
	*
	*  @note
	*  @param : 
	*  @defenc: 
	*  @return: 
	*  @author: yjc
	*  @date  : 2/7/2016 15:52 
	*/
	class CThreadLocker
	{
	public:
		CThreadLocker(void);
		~CThreadLocker();

		/**
		*  @brief : 利用关键段来实现线程锁
		*
		*
		*  @note
		*  @param : 
		*  @defenc: 
		*  @return: 
		*  @author: yjc
		*  @date  : 2/7/2016 15:53 
		*/
		class CCritical_Section
		{
		public:
			CCritical_Section(void);
			~CCritical_Section();
		public:
			CRITICAL_SECTION m_cs;
		};
	private:
		static CCritical_Section m_g_cs;	// 用来同步进程内的各个线程
	};
private:
	LARGE_INTEGER StartingTime;		// 用来记录开始的时间
	LARGE_INTEGER EndingTime;		// 用来记录结束的时间
	CString m_strTimeInfo;			// 用来记录时间信息
private:
	static CTimeRecord m_g_TimeRecord;	// 用来记录进程中的所有时间信息
};
