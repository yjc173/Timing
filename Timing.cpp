#include "stdafx.h"
#include "Timing.h"
#include "SqliteDBBase.h"
CTiming::CTimeRecord::CTimeRecord(void)
{
}

CTiming::CTimeRecord::~CTimeRecord(void)
{
	// 记录时间信息
	RecordTimeInfo();
}

void CTiming::CTimeRecord::InsertTimeInfo(const CString& strInfo, const LONGLONG& ElapsedTime)
{
	std::multimap<const CString, TimeInfo>::iterator it = m_g_mapTimeInfo.lower_bound(strInfo);
	std::multimap<const CString, TimeInfo>::iterator end = m_g_mapTimeInfo.upper_bound(strInfo);

	// 当前线程ID
	DWORD dwThreadID = GetCurrentThreadId();

	// 遍历multimap,查找是否有来自同一个线程的相同时间信息
	BOOL bFind = FALSE;
	while (end != it)
	{
		if (dwThreadID == it->second.dwThreadId)
		{
			bFind = TRUE;
			break;
		}
		else
		{
			++it;
		}
	}
	
	if (bFind)
	{
		// 如果有来自同一个线程的相同时间信息,添加计时次数并累加时间
		++it->second.nTime;
		it->second.ElapsedTime += ElapsedTime;
	}
	else
	{
		// 如果没有来自同一个线程的相同时间信息,则添加时间信息
		TimeInfo stTimeInfo;
		stTimeInfo.dwThreadId = dwThreadID;
		stTimeInfo.ElapsedTime = ElapsedTime;
		++stTimeInfo.nTime;
		m_g_mapTimeInfo.insert(std::pair<const CString, TimeInfo>(strInfo, stTimeInfo));
	}
}

void CTiming::CTimeRecord::RecordTimeInfo(void)
{
	// 创建数据库
	CProcessLocker processLocker;
	CSqliteDataBase db(_T("D:\\Timing.db"));
	CSqliteDBStmt stmt(&db);

	stmt.Begin();

	// 创建时间信息表
	CString strSQL = _T("CREATE TABLE IF NOT EXISTS TimeInfo(id INTEGER,\
															 time_info TEXT,\
															 time_elapsed INTEGER,\
															 times INTEGER,\
															 thread_id INTEGER,\
															 process_id INTEGER,\
															 PRIMARY KEY(id))");
	stmt.Exec(strSQL);

	// 创建时间信息统计视图
	strSQL = _T("CREATE VIEW IF NOT EXISTS '时间信息统计' AS\
							 SELECT time_info AS '时间信息',\
									SUM(times) AS '运行次数',\
									round(round(SUM(time_elapsed)) / 1000000,1) AS '消耗时间(ms)',\
									round(round(SUM(time_elapsed)) / 1000000 / SUM(times),1) AS '平均每次消耗时间(ms)'\
							 FROM TimeInfo\
							 GROUP BY time_info\
							 ORDER BY SUM(time_elapsed) DESC");
	stmt.Exec(strSQL);

	// 创建线程时间信息统计视图
	strSQL = _T("CREATE VIEW IF NOT EXISTS '线程时间信息统计' AS\
							 SELECT thread_id AS '线程ID',\
									time_info AS '时间信息',\
									SUM(times) AS '运行次数',\
									round(round(SUM(time_elapsed)) / 1000000,1) AS '消耗时间(ms)',\
									round(round(SUM(time_elapsed)) / 1000000 / SUM(times),1) AS '平均每次消耗时间(ms)'\
							 FROM TimeInfo\
							 GROUP BY thread_id,time_info\
							 ORDER BY SUM(time_elapsed) DESC");
	stmt.Exec(strSQL);

	// 创建进程时间信息统计视图
	strSQL = _T("CREATE VIEW IF NOT EXISTS '进程时间信息统计' AS\
							 SELECT process_id AS '进程ID',\
									time_info AS '时间信息',\
									SUM(times) AS '运行次数',\
									round(round(SUM(time_elapsed)) / 1000000,1) AS '消耗时间(ms)',\
									round(round(SUM(time_elapsed)) / 1000000 / SUM(times),1) AS '平均每次消耗时间(ms)'\
							 FROM TimeInfo\
							 GROUP BY process_id,time_info\
							 ORDER BY SUM(time_elapsed) DESC");
	stmt.Exec(strSQL);

	strSQL = _T("INSERT INTO TimeInfo(time_info, time_elapsed,times,thread_id,process_id)\
							   VALUES(:time_info,:time_elapsed,:times,:thread_id,:process_id)");
	int rc = stmt.Prepare(strSQL);
	if (SQLITE_OK != rc)
	{
		stmt.RollBack();
		return;
	}

	// 记录进程ID
	DWORD dwProcessID = GetProcessId(GetCurrentProcess());

	// 遍历m_g_mapTimeInfo,把所有信息都记录到TimeInfo表
	std::multimap<const CString, TimeInfo>::const_iterator it = m_g_mapTimeInfo.begin();
	for (; it != m_g_mapTimeInfo.end(); ++it)
	{
		stmt.Bind_Text(_T(":time_info"), it->first);
		stmt.Bind_Int(_T(":thread_id"), it->second.dwThreadId);
		stmt.Bind_Int(_T(":process_id"),dwProcessID);
		stmt.Bind_Int64(_T(":time_elapsed"), it->second.ElapsedTime);
		stmt.Bind_Int(_T(":times"), it->second.nTime);
		stmt.Step();
		stmt.Reset();
	}
	stmt.Commit();
}


CTiming::CTiming(const CString& strTimeInfo) :m_strTimeInfo(strTimeInfo)
{
	// 记录开始时间
	QueryPerformanceCounter(&StartingTime);
}

CTiming::~CTiming(void)
{
	// 记录结束时间
	QueryPerformanceCounter(&EndingTime);

	// 计数器的频率
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);

	// 记录时间间隔,以纳秒为单位
	LARGE_INTEGER ElapsedMicroseconds;
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

	ElapsedMicroseconds.QuadPart *= 1000000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	DWORD dwThreadId = GetCurrentThreadId();

	// 添加时间记录
	CThreadLocker threadLoker;
	m_g_TimeRecord.InsertTimeInfo(m_strTimeInfo, ElapsedMicroseconds.QuadPart);
}

CTiming::CTimeRecord CTiming::m_g_TimeRecord;

CTiming::CThreadLocker::CCritical_Section::CCritical_Section(void)
{
	// 初始化关键段
	InitializeCriticalSection(&m_cs);
}

CTiming::CThreadLocker::CCritical_Section::~CCritical_Section()
{
	// 删除关键段
	DeleteCriticalSection(&m_cs);
}


CTiming::CThreadLocker::CThreadLocker(void)
{
	// 进入关键段
	EnterCriticalSection(&m_g_cs.m_cs);
}

CTiming::CThreadLocker::~CThreadLocker()
{
	// 离开关键段
	LeaveCriticalSection(&m_g_cs.m_cs);
}

CTiming::CThreadLocker::CCritical_Section CTiming::CThreadLocker::m_g_cs;

CTiming::CProcessLocker::CProcessLocker(void) :m_hMutex(NULL)
{
	// 创建互斥量,用于进程同步
	m_hMutex = CreateMutex(NULL, TRUE, _T("TimingMutex"));
	if (NULL != m_hMutex)
	{
		WaitForSingleObject(m_hMutex, INFINITE);
	}
}

CTiming::CProcessLocker::~CProcessLocker()
{
	// 释放关闭互斥量
	ReleaseMutex(m_hMutex);
	CloseHandle(m_hMutex);
}
