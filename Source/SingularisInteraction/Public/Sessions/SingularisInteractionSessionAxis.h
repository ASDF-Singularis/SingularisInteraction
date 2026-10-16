/* ====================================================================== *
 * SingularisInteractionSessionAxis.h                                     *
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
#include "SingularisInteractionSessionAxis.generated.h"

/**
 * 引力奇点轴积分交互会话的量值累积模式。
 */
UENUM(BlueprintType)
enum class ESingularisInteractionAxisMode : uint8
{
	/** 积分：逐次累加量值（摇柄圈数，支持正负可逆） */
	Integral UMETA(DisplayName = "积分"),

	/** 峰值：跟踪量值最大值（按压深度） */
	Peak UMETA(DisplayName = "峰值"),
};

/**
 * 引力奇点轴积分交互会话。
 *
 * 消费连续值意图（Intent.Value）累积量值，达到阈值即达成：
 * 积分模式适配摇柄 / 转盘（支持正负可逆），峰值模式适配按压 / 蓄力深度。
 */
UCLASS(
	NotBlueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (DisplayName = "轴积分")
)
class SINGULARISINTERACTION_API USingularisInteractionSessionAxis : public USingularisInteractionSession
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 量值累积模式 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点轴积分会话",
		meta = (DisplayName = "量值累积模式")
	)
	ESingularisInteractionAxisMode AxisMode = ESingularisInteractionAxisMode::Integral;

	/** 达成阈值（量值累积目标） */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点轴积分会话",
		meta = (DisplayName = "达成阈值", ClampMin = "0.01")
	)
	float Threshold = 1.0f;

#pragma endregion

private:
#pragma region State

	/** 已累积量值 */
	float AccumulatedValue = 0.0f;

#pragma endregion

public:
#pragma region SPI

	virtual void Begin_Implementation(const FSingularisInteractionSessionContext& Context) override;
	virtual void ReceiveIntent_Implementation(
		const FSingularisInteractionSessionContext& Context,
		const FSingularisInteractionIntent& Intent
	) override;

#pragma endregion
};
