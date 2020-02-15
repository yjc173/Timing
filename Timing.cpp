#include "stdafx.h"
#include "Timing.h"
#include "SqliteDBBase.h"
CTiming::CTimeRecord::CTimeRecord(void)
{
}

CTiming::CTimeRecord::~CTimeRecord(void)
{
	// ��¼ʱ����Ϣ
	RecordTimeInfo();
}

void CTiming::CTimeRecord::InsertTimeInfo(const CString& strInfo, const LONGLONG& ElapsedTime)
{
	std::multimap<const CString, TimeInfo>::iterator it = m_g_mapTimeInfo.lower_bound(strInfo);
	std::multimap<const CString, TimeInfo>::iterator end = m_g_mapTimeInfo.upper_bound(strInfo);

	// ��ǰ�߳�ID
	DWORD dwThreadID = GetCurrentThreadId();

	// ����multimap,�����Ƿ�������ͬһ���̵߳���ͬʱ����Ϣ
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
		// ���������ͬһ���̵߳���ͬʱ����Ϣ,��Ӽ�ʱ�������ۼ�ʱ��
		++it->second.nTime;
		it->second.ElapsedTime += ElapsedTime;
	}
	else
	{
		// ���û������ͬһ���̵߳���ͬʱ����Ϣ,�����ʱ����Ϣ
		TimeInfo stTimeInfo;
		stTimeInfo.dwThreadId = dwThreadID;
		stTimeInfo.ElapsedTime = ElapsedTime;
		++stTimeInfo.nTime;
		m_g_mapTimeInfo.insert(std::pair<const CString, TimeInfo>(strInfo, stTimeInfo));
	}
}

void CTiming::CTimeRecord::RecordTimeInfo(void)
{
	// �������ݿ�
	CProcessLocker processLocker;
	CSqliteDataBase db(_T("D:\\Timing.db"));
	CSqliteDBStmt stmt(&db);

	stmt.Begin();

	// ����ʱ����Ϣ��
	CString strSQL = _T("CREATE TABLE IF NOT EXISTS TimeInfo(id INTEGER,\
															 time_info TEXT,\
															 time_elapsed INTEGER,\
															 times INTEGER,\
															 thread_id INTEGER,\
															 process_id INTEGER,\
															 PRIMARY KEY(id))");
	stmt.Exec(strSQL);

	// ����ʱ����Ϣͳ����ͼ
	strSQL = _T("CREATE VIEW IF NOT EXISTS 'ʱ����Ϣͳ��' AS\
							 SELECT time_info AS 'ʱ����Ϣ',\
									SUM(times) AS '���д���',\
									round(round(SUM(time_elapsed)) / 1000000,1) AS '����ʱ��(ms)',\
									round(round(SUM(time_elapsed)) / 1000000 / SUM(times),1) AS 'ƽ��ÿ������ʱ��(ms)'\
							 FROM TimeInfo\
							 GROUP BY time_info\
							 ORDER BY SUM(time_elapsed) DESC");
	stmt.Exec(strSQL);

	// �����߳�ʱ����Ϣͳ����ͼ
	strSQL = _T("CREATE VIEW IF NOT EXISTS '�߳�ʱ����Ϣͳ��' AS\
							 SELECT thread_id AS '�߳�ID',\
									time_info AS 'ʱ����Ϣ',\
									SUM(times) AS '���д���',\
									round(round(SUM(time_elapsed)) / 1000000,1) AS '����ʱ��(ms)',\
									round(round(SUM(time_elapsed)) / 1000000 / SUM(times),1) AS 'ƽ��ÿ������ʱ��(ms)'\
							 FROM TimeInfo\
							 GROUP BY thread_id,time_info\
							 ORDER BY SUM(time_elapsed) DESC");
	stmt.Exec(strSQL);

	// ��������ʱ����Ϣͳ����ͼ
	strSQL = _T("CREATE VIEW IF NOT EXISTS '����ʱ����Ϣͳ��' AS\
							 SELECT process_id AS '����ID',\
									time_info AS 'ʱ����Ϣ',\
									SUM(times) AS '���д���',\
									round(round(SUM(time_elapsed)) / 1000000,1) AS '����ʱ��(ms)',\
									round(round(SUM(time_elapsed)) / 1000000 / SUM(times),1) AS 'ƽ��ÿ������ʱ��(ms)'\
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

	// ��¼����ID
	DWORD dwProcessID = GetProcessId(GetCurrentProcess());

	// ����m_g_mapTimeInfo,��������Ϣ����¼��TimeInfo��
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
	// ��¼��ʼʱ��
	QueryPerformanceCounter(&StartingTime);
}

CTiming::~CTiming(void)
{
	// ��¼����ʱ��
	QueryPerformanceCounter(&EndingTime);

	// ��������Ƶ��
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);

	// ��¼ʱ����,������Ϊ��λ
	LARGE_INTEGER ElapsedMicroseconds;
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

	ElapsedMicroseconds.QuadPart *= 1000000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	DWORD dwThreadId = GetCurrentThreadId();

	// ���ʱ���¼
	CThreadLocker threadLoker;
	m_g_TimeRecord.InsertTimeInfo(m_strTimeInfo, ElapsedMicroseconds.QuadPart);
}

CTiming::CTimeRecord CTiming::m_g_TimeRecord;

CTiming::CThreadLocker::CCritical_Section::CCritical_Section(void)
{
	// ��ʼ���ؼ���
	InitializeCriticalSection(&m_cs);
}

CTiming::CThreadLocker::CCritical_Section::~CCritical_Section()
{
	// ɾ���ؼ���
	DeleteCriticalSection(&m_cs);
}


CTiming::CThreadLocker::CThreadLocker(void)
{
	// ����ؼ���
	EnterCriticalSection(&m_g_cs.m_cs);
}

CTiming::CThreadLocker::~CThreadLocker()
{
	// �뿪�ؼ���
	LeaveCriticalSection(&m_g_cs.m_cs);
}

CTiming::CThreadLocker::CCritical_Section CTiming::CThreadLocker::m_g_cs;

CTiming::CProcessLocker::CProcessLocker(void) :m_hMutex(NULL)
{
	// ����������,���ڽ���ͬ��
	m_hMutex = CreateMutex(NULL, TRUE, _T("TimingMutex"));
	if (NULL != m_hMutex)
	{
		WaitForSingleObject(m_hMutex, INFINITE);
	}
}

CTiming::CProcessLocker::~CProcessLocker()
{
	// �ͷŹرջ�����
	ReleaseMutex(m_hMutex);
	CloseHandle(m_hMutex);
}
