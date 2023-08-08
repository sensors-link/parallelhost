#include <stdio.h>
#include <Windows.h>

//! @brief The tool's name.
const char k_toolName[] = "parallelhost";

//! @brief Current version number for the tool.
const char k_version[] = "1.0.0";

//! @brief Copyright string.
const char k_copyright[] =
    "Copyright (c) 2023 pengxin Data Tech.\n\
All rights reserved.";

typedef struct
{
	char *command;
	int result;
}ThreadData, *pThreadData;

DWORD WINAPI ExecuteThread(LPVOID lpParam)
{
	pThreadData threadData = (pThreadData)lpParam;
	char* command = threadData->command;

	threadData->result = system(threadData->command);			// ִ��������淵��ֵ

	return 0;
}

int main(int argc, char *argv[])
{
	HANDLE *pThread;
	pThreadData threadData;
	int successfulThreadIndex = -1;

	// ���������в�����ʽΪ�������� ����1 ����2 ...
	if (argc <= 1) {
		printf("%s %s\n%s\n", k_toolName, k_version, k_copyright);
		printf("Usage: %s <command1> <command2> ... <commandN>\n", argv[0]);
		return -1;
	}

	pThread = malloc((argc - 1)*sizeof(HANDLE));				// ���ݲ��������߳̾������
	if (pThread == NULL)
		return -1;

	threadData = malloc((argc - 1)*sizeof(ThreadData));			// ���ݲ��������̲߳�������
	if (threadData == NULL)
		return -1;

	for (int i = 0; i < argc - 1; i++)					// ����������߳�
	{
		threadData[i].command = argv[i+1];				// �����̵߳Ĳ�������

		pThread[i] = CreateThread(NULL, 0, ExecuteThread, &threadData[i], 0, NULL);		// �����߳�
		if (pThread[i] == NULL)						// �̴߳���ʧ���ͷ��ڴ���˳�
		{
			for (int j = 0; j < i; j++)				// �ͷ��Ѵ������߳̾��
			{
				CloseHandle(pThread[j]);
			}

			free(threadData);
			free(pThread);
			return -1;
		}
	}

	// �ȴ������߳�ִ����ɣ�������һ���߳�ִ�гɹ�
	while (successfulThreadIndex == -1)
	{
		int endThreadNum = 0;
		for (int i = 0; i < argc - 1; i++)
		{
			DWORD waitResult = WaitForSingleObject(pThread[i], 0);
			if (waitResult == WAIT_OBJECT_0)
			{
				endThreadNum++;
				if (threadData[i].result == 0) 
				{
					successfulThreadIndex = i;
					//printf("�߳� %d �ɹ�ִ�У�����ֵ��%d\n", successfulThreadIndex, threadData[successfulThreadIndex].result);
					break;
				}
			}
		}
		if (endThreadNum >= argc - 1)
			break;
	}

	// �ͷŶ�Ӧ�߳̾��
	for (int i = 0; i < argc - 1; i++)
	{
		CloseHandle(pThread[i]);
	}

	free(threadData);
	free(pThread);

	return successfulThreadIndex;
}