/* ====================================================================== *
 * SingularisInteractionIntentType.h                                      *
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
#include <GameplayTagContainer.h>

#include "Types/SingularisInteractionValueType.h"
#include "SingularisInteractionIntentType.generated.h"

/**
 * 引力奇点交互意图。
 *
 * 控制 → 逻辑的最小协议单元：交互者以标签声明交互意图并携带一个量值。
 * 核心意图（Press / Release / Value）由框架定义，领域意图经标签开放扩展；
 * 意图不携带任何输入设备语义，玩家经输入驱动器翻译，AI 与脚本直接产生。
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionIntent
{
	GENERATED_BODY()

	/** 意图标签（Singularis.Interaction.Intent.*） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag IntentTag{};

	/** 意图量值（连续语义：布尔 / 一维 / 二维 / 三维），离散意图忽略 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSingularisInteractionValue Value{};
};
