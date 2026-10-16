/* ====================================================================== *
 * SingularisInteractionSessionDrag.h                                     *
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
#include "SingularisInteractionSessionDrag.generated.h"

/**
 * 引力奇点拖动会话。
 *
 * 绝对跟踪型机制：消费连续值意图的带符号标量，把拖动量映射为绝对进度
 * （可正负、可停在中途）；松手广播 `Event.Settle` 定格并按配置达成。
 * 适配车门（左键按住拖动改变开门角度、松手停在任意角度）等位移映射交互。
 */
UCLASS(
	NotBlueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (DisplayName = "拖动")
)
class SINGULARISINTERACTION_API USingularisInteractionSessionDrag : public USingularisInteractionSession
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 灵敏度：每单位输入对应的进度增量 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点拖动会话",
		meta = (DisplayName = "灵敏度", ClampMin = "0.0001")
	)
	float Sensitivity = 1.0f;

	/** 松手是否即刻达成（关闭则保持会话，允许再次拖动） */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点拖动会话",
		meta = (DisplayName = "松手即达成")
	)
	bool bCompleteOnRelease = true;

	/** 拖回起点是否中止（关闭则允许停在关闭态） */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点拖动会话",
		meta = (DisplayName = "拖回即中止")
	)
	bool bCancelAtZero = true;

#pragma endregion

private:
#pragma region State

	/** 当前绝对进度 0..1 */
	float Progress = 0.0f;

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
