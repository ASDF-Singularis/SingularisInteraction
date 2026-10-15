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

class USingularisInteractionSession;
class USingularisInteractionResponse;
class USingularisInteractionBehavior;

/**
 * 引力奇点交互模式。
 *
 * 注：基类查询器固定执行射线查询，不读取本枚举；Collision 模式尚未实现。
 */
UENUM(BlueprintType)
enum class ESingularisInteractionMode : uint8
{
	/** 射线查询 */
	Ray UMETA(DisplayName = "射线"),

	/** 碰撞查询 */
	Collision UMETA(DisplayName = "碰撞"),
};

/**
 * 引力奇点交互响应条目
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionResponseEntry
{
	GENERATED_BODY()

	/** 响应名称，用于编辑器展示 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ResponseName{};

	/** 响应说明 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ResponseDescription{};

	/** 响应实例 */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite)
	USingularisInteractionResponse* Response = nullptr;
};

/**
 * 引力奇点交互策略管线：会话原型与有序响应集的组合，是会话推进的唯一装配单元。
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionStrategyPipeline
{
	GENERATED_BODY()

	/** 交互会话原型：会话推进规则的执行单元，会话建立时派生专属实例 */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite)
	USingularisInteractionSession* Session = nullptr;

	/** 按数组顺序依次驱动的响应集 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (TitleProperty = "ResponseName"))
	TArray<FSingularisInteractionResponseEntry> Responses{};
};

/**
 * 引力奇点交互行为条目
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionBehaviorEntry
{
	GENERATED_BODY()

	/** 行为名称，用于编辑器展示 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BehaviorName{};

	/** 行为说明 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BehaviorDescription{};

	/** 行为实例 */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite)
	USingularisInteractionBehavior* Behavior = nullptr;
};
