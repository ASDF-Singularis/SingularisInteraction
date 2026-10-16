/* ====================================================================== *
 * SingularisInteractionSessionPulse.cpp                                  *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/10/13 | Author: TrifingZW                                *
 * Licensed under MIT License                                             *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   *
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 *
 * ====================================================================== */

#include "Sessions/SingularisInteractionSessionPulse.h"

#include <Math/UnrealMathUtility.h>

#include "Types/SingularisInteractionGameplayTags.h"

void USingularisInteractionSessionPulse::Begin_Implementation(const FSingularisInteractionSessionContext& Context)
{
	// 1) 会话建立即首次主张
	PulseCounter = 1;
	LastPulseElapsed = 0.0f;

	// 2) 广播首声进度
	EmitSessionEvent(
		SingularisInteraction_Event_Progress,
		FSingularisInteractionValue(static_cast<float>(PulseCounter) / PulseCount),
		FInstancedStruct()
	);
}

void USingularisInteractionSessionPulse::ReceiveIntent_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const FSingularisInteractionIntent& Intent
)
{
	// 1) 仅响应主张意图
	if (Intent.IntentTag != SingularisInteraction_Intent_Press)
		return;

	// 2) 窗口校验：自最近一次主张起算超时即中止
	if (Context.Elapsed - LastPulseElapsed > PulseWindow)
	{
		CancelSession();
		return;
	}

	// 3) 计数推进：达标即完成，否则广播进度
	++PulseCounter;
	LastPulseElapsed = Context.Elapsed;

	if (PulseCounter >= PulseCount)
	{
		CompleteSession();
		return;
	}

	EmitSessionEvent(
		SingularisInteraction_Event_Progress,
		FSingularisInteractionValue(static_cast<float>(PulseCounter) / PulseCount),
		FInstancedStruct()
	);
}

void USingularisInteractionSessionPulse::Tick_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const float DeltaTime
)
{
	// 1) 窗口超时即中止
	if (Context.Elapsed - LastPulseElapsed > PulseWindow)
		CancelSession();
}
