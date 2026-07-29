/* ====================================================================== *
 * SingularisInteractionComponentType.h                                   *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/23 | Author: TrifingZW                                *
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

#include "SingularisInteractionComponentType.generated.h"

class USingularisInteractionStrategy;
class USingularisInteractionBehaviorStrategy;

/**
 * 引力奇点交互模式
 */
UENUM(BlueprintType)
enum class ESingularisInteractionMode : uint8
{
	Ray UMETA(DisplayName = "射线"),
	Collision UMETA(DisplayName = "碰撞"),
};

/**
 * 引力奇点交互策略条目
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionStrategyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText StrategyName{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText StrategyDescription{};

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite)
	USingularisInteractionStrategy* Strategy = nullptr;
};

/**
 * 引力奇点交互策略管线：用于包装一组有序的策略
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionStrategyPipeline
{
	GENERATED_BODY()

	// 这里使用 TArray 来存储多个策略，数组的顺序即为执行顺序
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (TitleProperty = "StrategyName"))
	TArray<FSingularisInteractionStrategyEntry> Strategies;

	// 可选：添加一个布尔值控制是否中断（例如：如果第一个策略失败，是否停止后续策略？）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuspend = true;
};

/**
 * 引力奇点交互行为策略条目
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionBehaviorStrategyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BehaviorStrategyName{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BehaviorStrategyDescription{};

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite)
	USingularisInteractionBehaviorStrategy* BehaviorStrategy = nullptr;
};
