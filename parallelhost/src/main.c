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

	threadData->result = system(threadData->command);			// 执行命令并保存返回值

	return 0;
}

int main(int argc, char *argv[])
{
	HANDLE *pThread;
	pThreadData threadData;
	int successfulThreadIndex = -1;

	// 假设命令行参数格式为：程序名 参数1 参数2 ...
	if (argc <= 1) {
		printf("%s %s\n%s\n", k_toolName, k_version, k_copyright);
		printf("Usage: %s <command1> <command2> ... <commandN>\n", argv[0]);
		return -1;
	}

	pThread = malloc((argc - 1)*sizeof(HANDLE));				// 根据参数分配线程句柄数量
	if (pThread == NULL)
		return -1;

	threadData = malloc((argc - 1)*sizeof(ThreadData));			// 根据参数分配线程参数数量
	if (threadData == NULL)
		return -1;

	for (int i = 0; i < argc - 1; i++)					// 创建所需的线程
	{
		threadData[i].command = argv[i+1];				// 传入线程的参数配置

		pThread[i] = CreateThread(NULL, 0, ExecuteThread, &threadData[i], 0, NULL);		// 创建线程
		if (pThread[i] == NULL)						// 线程创建失败释放内存后退出
		{
			for (int j = 0; j < i; j++)				// 释放已创建的线程句柄
			{
				CloseHandle(pThread[j]);
			}

			free(threadData);
			free(pThread);
			return -1;
		}
	}

	// 等待所有线程执行完成，或者有一个线程执行成功
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
					//printf("线程 %d 成功执行，返回值：%d\n", successfulThreadIndex, threadData[successfulThreadIndex].result);
					break;
				}
			}
		}
		if (endThreadNum >= argc - 1)
			break;
	}

	// 释放对应线程句柄
	for (int i = 0; i < argc - 1; i++)
	{
		CloseHandle(pThread[i]);
	}

	free(threadData);
	free(pThread);

	return successfulThreadIndex;
}