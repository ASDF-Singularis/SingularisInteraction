/* ====================================================================== *
 * SingularisInteractionSessionHold.cpp                                   *
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

#include "Sessions/SingularisInteractionSessionHold.h"

#include <Math/UnrealMathUtility.h>

#include "Types/SingularisInteractionGameplayTags.h"

void USingularisInteractionSessionHold::Begin_Implementation(const FSingularisInteractionSessionContext& Context)
{
	bReleased = false;
	InternalAlpha = 0.0f;
}

void USingularisInteractionSessionHold::ReceiveIntent_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const FSingularisInteractionIntent& Intent
)
{
	// 1) 仅响应撤回意图
	if (Intent.IntentTag != SingularisInteraction_Intent_Release)
		return;

	// 2) 撤回即取消：立即中止
	if (bCancelOnRelease)
	{
		CancelSession(FInstancedStruct());
		return;
	}

	// 3) 撤回不取消：进入衰减分支
	bReleased = true;
}

void USingularisInteractionSessionHold::Tick_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const float DeltaTime
)
{
	// 1) 持续主张：以会话时钟推进，达成即完成
	if (!bReleased)
	{
		InternalAlpha = FMath::Clamp(Context.Elapsed / FMath::Max(HoldDuration, 0.01f), 0.0f, 1.0f);
		EmitSessionEvent(
			SingularisInteraction_Event_Progress,
			FSingularisInteractionValue(InternalAlpha),
			FInstancedStruct()
		);

		if (InternalAlpha >= 1.0f)
			CompleteSession(FInstancedStruct());

		return;
	}

	// 2) 已撤回：不衰减时静置，衰减时回落，归零即中止
	if (DecayRate <= 0.0f)
		return;

	InternalAlpha = FMath::Max(InternalAlpha - DecayRate * DeltaTime, 0.0f);
	EmitSessionEvent(
		SingularisInteraction_Event_Progress,
		FSingularisInteractionValue(InternalAlpha),
		FInstancedStruct()
	);

	if (InternalAlpha <= 0.0f)
		CancelSession(FInstancedStruct());
}
