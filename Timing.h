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
	*  @brief : ʱ����Ϣ�ṹ,���ڴ��ʱ����Ϣ
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
		DWORD dwThreadId;		// ��¼�߳�ID
		LONGLONG ElapsedTime;	// ��¼������ʱ��
		int nTime;				// ��¼���еĴ���
	};

	/**
	*  @brief : ʱ���¼��,���ڼ�¼ʱ����Ϣ
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
		*  @brief : �������ʱ����Ϣ
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
	*  @brief : ��¼ʱ����Ϣ�����ݿ��ļ�
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
	*  @brief : ��������,���ڽ��̼��ͬ��
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
	*  @brief : �߳�����,����ͬ���߳�
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
		*  @brief : ���ùؼ�����ʵ���߳���
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
		static CCritical_Section m_g_cs;	// ����ͬ�������ڵĸ����߳�
	};
private:
	LARGE_INTEGER StartingTime;		// ������¼��ʼ��ʱ��
	LARGE_INTEGER EndingTime;		// ������¼������ʱ��
	CString m_strTimeInfo;			// ������¼ʱ����Ϣ
private:
	static CTimeRecord m_g_TimeRecord;	// ������¼�����е�����ʱ����Ϣ
};
