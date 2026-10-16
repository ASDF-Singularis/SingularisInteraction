/* ====================================================================== *
 * SingularisInteractionSessionImpulse.cpp                                *
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

#include "Sessions/SingularisInteractionSessionImpulse.h"

#include <Math/UnrealMathUtility.h>

#include "Types/SingularisInteractionGameplayTags.h"

void USingularisInteractionSessionImpulse::Begin_Implementation(const FSingularisInteractionSessionContext& Context)
{
	// 1) 会话建立即首次主张：首个脉冲入账
	Impulse = FMath::Clamp(PulseStep, 0.0f, 1.0f);
	EmitSessionEvent(SingularisInteraction_Event_Progress, FSingularisInteractionValue(Impulse), FInstancedStruct());

	if (Impulse >= 1.0f)
		CompleteSession();
}

void USingularisInteractionSessionImpulse::ReceiveIntent_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const FSingularisInteractionIntent& Intent
)
{
	// 1) 仅响应主张意图：每次主张脉冲入账
	if (Intent.IntentTag != SingularisInteraction_Intent_Press)
		return;

	Impulse = FMath::Clamp(Impulse + PulseStep, 0.0f, 1.0f);
	EmitSessionEvent(SingularisInteraction_Event_Progress, FSingularisInteractionValue(Impulse), FInstancedStruct());

	if (Impulse >= 1.0f)
		CompleteSession();
}

void USingularisInteractionSessionImpulse::Tick_Implementation(
	const FSingularisInteractionSessionContext& Context,
	const float DeltaTime
)
{
	// 1) 不衰减则水位保持
	if (DecayRate <= 0.0f)
		return;

	// 2) 逐帧衰减并广播；归零按配置中止
	Impulse = FMath::Clamp(Impulse - DecayRate * DeltaTime, 0.0f, 1.0f);
	EmitSessionEvent(SingularisInteraction_Event_Progress, FSingularisInteractionValue(Impulse), FInstancedStruct());

	if (bCancelAtZero && Impulse <= 0.0f)
		CancelSession();
}
