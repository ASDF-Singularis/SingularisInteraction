/* ====================================================================== *
 * SingularisInteractionSessionImpulse.h                                  *
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

#pragma once

#include <CoreMinimal.h>

#include "Objects/SingularisInteractionSession.h"
#include "SingularisInteractionSessionImpulse.generated.h"

/**
 * 引力奇点脉冲会话。
 *
 * 能量蓄积型机制：每次主张注入一份脉冲，逐帧按衰减速率回落；
 * 达满即达成，归零即中止。与连点会话（窗口计数）互为补充：
 * 连点看「窗口内命中次数」，脉冲看「瞬时能量水位」，表现更连续。
 */
UCLASS(
	NotBlueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (DisplayName = "引力奇点脉冲会话")
)
class SINGULARISINTERACTION_API USingularisInteractionSessionImpulse : public USingularisInteractionSession
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 单次主张脉冲增量 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点脉冲会话",
		meta = (DisplayName = "脉冲增量", ClampMin = "0.01", ClampMax = "1.0")
	)
	float PulseStep = 0.25f;

	/** 每秒衰减量（0 为不衰减） */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点脉冲会话",
		meta = (DisplayName = "衰减速率", ClampMin = "0.0", ClampMax = "1.0")
	)
	float DecayRate = 0.5f;

	/** 归零是否中止（关闭则允许停在零水位） */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点脉冲会话",
		meta = (DisplayName = "归零即中止")
	)
	bool bCancelAtZero = true;

#pragma endregion

private:
#pragma region State

	/** 当前脉冲能量 0..1 */
	float Impulse = 0.0f;

#pragma endregion

public:
#pragma region SPI

	virtual void Begin_Implementation(const FSingularisInteractionSessionContext& Context) override;
	virtual void ReceiveIntent_Implementation(
		const FSingularisInteractionSessionContext& Context,
		const FSingularisInteractionIntent& Intent
	) override;
	virtual void Tick_Implementation(const FSingularisInteractionSessionContext& Context, float DeltaTime) override;

#pragma endregion
};
