/* ====================================================================== *
 * SingularisInteractionSessionType.h                                     *
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

#include "SingularisInteractionSessionType.generated.h"

class AActor;
class APawn;
class USingularisInteractionComponent;
class USingularisInteractionSession;

/**
 * 引力奇点交互会话上下文。
 *
 * 描述一次会话的身份与计时，由宿主（会话端或镜像端）组装，
 * 驱动会话的每个生命周期调用。
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionSessionContext
{
	GENERATED_BODY()

	/** 交互者（发起会话的 Actor：控制器 / Pawn / 任意对象） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Interactor = nullptr;

	/** 触发者 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APawn> Instigator = nullptr;

	/** 承载者（交互组件宿主） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Avatar = nullptr;

	/** 目标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Target = nullptr;

	/** 交互组件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USingularisInteractionComponent> InteractionComponent = nullptr;

	/** 会话对应的策略标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StrategyTag{};

	/** 会话累计时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Elapsed = 0.0f;
};

/**
 * 引力奇点交互会话事件。
 *
 * 逻辑 → 响应与表现的统一事件单元：核心事件（Begin / Progress / Complete / Cancel）
 * 由框架定义，领域事件经标签开放扩展；连续语义仅经 Event.Progress 流转。
 */
USTRUCT(BlueprintType)
struct SINGULARISINTERACTION_API FSingularisInteractionSessionEvent
{
	GENERATED_BODY()

	/** 事件标签（Singularis.Interaction.Event.*） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EventTag{};

	/** 归一化量值 0..1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Alpha = 0.0f;

	/** 附加语义标签（开放负载） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer PayloadTags{};

	/** 会话上下文 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSingularisInteractionSessionContext Context{};
};

/**
 * 引力奇点交互会话记录。
 *
 * 交互组件维护的会话内部状态，仅存在于逻辑端。
 */
struct FSingularisInteractionSessionRecord
{
	/** 交互者 */
	TWeakObjectPtr<AActor> Interactor = nullptr;

	/** 会话对应的策略标签 */
	FGameplayTag StrategyTag{};

	/** 会话专属会话实例 */
	TObjectPtr<USingularisInteractionSession> Session = nullptr;

	/** 会话累计时长（秒） */
	float Elapsed = 0.0f;
};
