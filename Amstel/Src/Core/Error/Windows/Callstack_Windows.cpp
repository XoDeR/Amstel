// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_PLATFORM_WINDOWS

#include "Device/Log.h"
#include "Device/Windows/Headers_Windows.h"

#include <dbghelp.h>

namespace Rio
{

namespace ErrorFn
{
	void printCallstack()
	{
		SymInitialize(GetCurrentProcess(), NULL, TRUE);
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
		DWORD mtype;
		CONTEXT ctx;
		ZeroMemory(&ctx, sizeof(CONTEXT));
		ctx.ContextFlags = CONTEXT_CONTROL;
		RtlCaptureContext(&ctx);

		STACKFRAME64 stack;
		ZeroMemory(&stack, sizeof(STACKFRAME64));
#ifdef _M_IX86
		mtype = IMAGE_FILE_MACHINE_I386;
		stack.AddrPC.Offset = ctx.Eip;
		stack.AddrPC.Mode = AddrModeFlat;
		stack.AddrFrame.Offset = ctx.Ebp;
		stack.AddrFrame.Mode = AddrModeFlat;
		stack.AddrStack.Offset = ctx.Esp;
		stack.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
		mtype = IMAGE_FILE_MACHINE_AMD64;
		stack.AddrPC.Offset = ctx.Rip;
		stack.AddrPC.Mode = AddrModeFlat;
		stack.AddrFrame.Offset = ctx.Rsp;
		stack.AddrFrame.Mode = AddrModeFlat;
		stack.AddrStack.Offset = ctx.Rsp;
		stack.AddrStack.Mode = AddrModeFlat;
#endif

		DWORD ldsp = 0;
		IMAGEHLP_LINE64 line;
		ZeroMemory(&line, sizeof(IMAGEHLP_LINE64));
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		char buf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		PSYMBOL_INFO sym = (PSYMBOL_INFO)buf;
		sym->SizeOfStruct = sizeof(SYMBOL_INFO);
		sym->MaxNameLen = MAX_SYM_NAME;

		UINT num = 0;
		while (StackWalk64(mtype
				, GetCurrentProcess()
				, GetCurrentThread()
				, &stack
				, &ctx
				, NULL
				, SymFunctionTableAccess64
				, SymGetModuleBase64
				, NULL
				))
		{
			if (stack.AddrPC.Offset == 0)
			{
				break;
			}

			++num;

			BOOL res = SymGetLineFromAddr64(GetCurrentProcess()
						, stack.AddrPC.Offset
						, &ldsp
						, &line
						);
			res = res && SymFromAddr(GetCurrentProcess(), stack.AddrPC.Offset, 0, sym);

			if (res == TRUE)
			{
				RIO_LOGE("\t[%2i] %s in %s:%d", num, sym->Name, line.FileName, line.LineNumber);
			}
			else
			{
				RIO_LOGE("\t[%2i] 0x%p", num, stack.AddrPC.Offset);
			}
		}

		SymCleanup(GetCurrentProcess());
	}
} // namespace ErrorFn

} // namespace Rio

#endif // RIO_PLATFORM_WINDOWS
// Copyright (c) 2016 Volodymyr Syvochka