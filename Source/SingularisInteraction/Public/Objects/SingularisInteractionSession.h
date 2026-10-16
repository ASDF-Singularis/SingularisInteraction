/* ====================================================================== *
 * SingularisInteractionSession.h                                         *
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
#include <UObject/Object.h>

#include "Types/SingularisInteractionIntentType.h"
#include "Types/SingularisInteractionSessionType.h"
#include "SingularisInteractionSession.generated.h"

/**
 * 引力奇点交互会话。
 *
 * 会话推进规则的执行单元：消费宿主供给的意图与时钟，产出会话事件与结算。
 * 会话经管线以原型配置（Instanced 内联），会话建立时派生专属实例；
 * 会话端与镜像端运行同一实现，构成"唯一时序真相 + 本地零延迟会话流"。
 * 内建族（瞬时 / 长按 / 连点 / 轴积分）覆盖常见形态，子类扩展新形态无需改动框架。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)
class SINGULARISINTERACTION_API USingularisInteractionSession : public UObject
{
	GENERATED_BODY()

#pragma region State

	/** 会话宿主对象（弱引用，失效后发射通道静默关闭；须实现 ISingularisInteractionSessionHost） */
	TWeakObjectPtr<UObject> SessionHostObject = nullptr;

#pragma endregion

public:
#pragma region API

	/**
	 * 发射会话事件。
	 *
	 * 核心事件之外经标签开放扩展；Event.Progress 为唯一连续事件，其余为离散事件。
	 * 负载为双通道：量值承载中性连续量，瞬态负荷承载会话自定的强类型结构（可空）。
	 *
	 * @param EventTag 事件标签。
	 * @param Value 量值（中性量值载体）。
	 * @param Payload 瞬态负荷（会话自定强类型结构，可空）。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互会话|API",
		meta = (DisplayName = "EmitSessionEvent")
	)
	void EmitSessionEvent(
		FGameplayTag EventTag,
		FSingularisInteractionValue Value,
		const FInstancedStruct& Payload
	);

	/**
	 * 完成会话。
	 *
	 * 达成事件收敛为 1，并下发给定瞬态负荷；无业务数据时传空结构。
	 *
	 * @param Payload 结算瞬态负荷（会话自定强类型结构，可空）。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互会话|API",
		meta = (DisplayName = "CompleteSession")
	)
	void CompleteSession(const FInstancedStruct& Payload);

	/**
	 * 取消会话。
	 *
	 * 中止事件归零，并下发给定瞬态负荷；无业务数据时传空结构。
	 *
	 * @param Payload 结算瞬态负荷（会话自定强类型结构，可空）。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互会话|API",
		meta = (DisplayName = "CancelSession")
	)
	void CancelSession(const FInstancedStruct& Payload);

	/**
	 * 绑定会话宿主（框架装配，派生实例时调用）。
	 *
	 * @param InSessionHostObject 宿主对象（弱引用保活判据；须实现 ISingularisInteractionSessionHost）。
	 */
	void BindSessionHost(UObject* InSessionHostObject);

#pragma endregion

#pragma region SPI

	/**
	 * 判定会话能否以该上下文建立。
	 *
	 * 供宿主在会话建立前调用；任一拒绝即整个会话被拒绝。
	 *
	 * @param Context 会话上下文。
	 * @return 允许建立时返回 true。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互会话|SPI",
		meta = (DisplayName = "CanBegin")
	)
	bool CanBegin(const FSingularisInteractionSessionContext& Context) const;

	/**
	 * 会话开始时驱动（宿主已广播 Begin 事件）。
	 *
	 * @param Context 会话上下文。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互会话|SPI",
		meta = (DisplayName = "Begin")
	)
	void Begin(const FSingularisInteractionSessionContext& Context);

	/**
	 * 会话期间摄入意图。
	 *
	 * @param Context 会话上下文。
	 * @param Intent 交互者提交的意图。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互会话|SPI",
		meta = (DisplayName = "ReceiveIntent")
	)
	void ReceiveIntent(
		const FSingularisInteractionSessionContext& Context,
		const FSingularisInteractionIntent& Intent
	);

	/**
	 * 会话期间逐帧推进（仅活跃会话）。
	 *
	 * @param Context 会话上下文。
	 * @param DeltaTime 帧间隔（秒）。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互会话|SPI",
		meta = (DisplayName = "Tick")
	)
	void Tick(const FSingularisInteractionSessionContext& Context, float DeltaTime);

	/**
	 * 会话收尾时驱动（达成或中止之后）。
	 *
	 * @param Context 会话上下文。
	 */
	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "引力奇点交互会话|SPI",
		meta = (DisplayName = "End")
	)
	void End(const FSingularisInteractionSessionContext& Context);

#pragma endregion
};
