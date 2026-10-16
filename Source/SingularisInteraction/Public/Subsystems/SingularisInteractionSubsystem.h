/* ====================================================================== *
 * SingularisInteractionSubsystem.h                                      *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/07/25 | Author: TrifingZW                                *
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
#include <Subsystems/WorldSubsystem.h>

#include "Types/SingularisInteractionIntentType.h"
#include "Types/SingularisInteractionSessionType.h"
#include "SingularisInteractionSubsystem.generated.h"

class AActor;
class UPrimitiveComponent;
class USingularisInteractionComponent;

#pragma region 委托签名

/** 世界级交互会话事件广播：全局观察与遥测 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWorldInteractionSessionEventSignature,
	USingularisInteractionComponent*,
	Target,
	FSingularisInteractionSessionEvent,
	Event
);

#pragma endregion

/**
 * 引力奇点交互子系统。
 *
 * 世界级交互中枢：登记可交互碰撞组件到交互组件的映射，供查询器命中后反查；
 * 并以单一入口承接全部交互意图——玩家经交互者组件提交、AI 与脚本直接调用，
 * 三者共用同一通路，是交互唯一数据源（SSOT）的控制面入口。
 */
UCLASS(
	NotBlueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (DisplayName = "引力奇点交互子系统")
)
class SINGULARISINTERACTION_API USingularisInteractionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
#pragma region Event Dispatcher

	/** 世界级交互会话事件广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互子系统|事件分发器",
		meta = (DisplayName = "世界会话事件时触发")
	)
	FOnWorldInteractionSessionEventSignature OnWorldInteractionSessionEvent{};

#pragma endregion

private:
#pragma region State

	/** 碰撞组件到交互组件的弱引用映射 */
	TMap<TWeakObjectPtr<UPrimitiveComponent>, TWeakObjectPtr<USingularisInteractionComponent>> Map{};

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractionSubsystem();

#pragma endregion

#pragma region Subsystem Interface

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma endregion

#pragma region API

	/**
	 * 提交交互意图（唯一入口）。
	 *
	 * 玩家交互者经交互者组件调用，AI、行为树与脚本直接调用。
	 *
	 * @param Interactor 交互者。
	 * @param TargetInteractionComponent 目标交互组件。
	 * @param PipelineTag 会话对应的管线标签。
	 * @param Intent 交互者提交的意图。
	 * @return 受理时返回 true。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互子系统|API",
		meta = (DisplayName = "提交交互意图")
	)
	bool SubmitInteractionIntent(
		AActor* Interactor,
		USingularisInteractionComponent* TargetInteractionComponent,
		FGameplayTag PipelineTag,
		FSingularisInteractionIntent Intent
	);

	/**
	 * 登记碰撞组件与交互组件的映射关系。
	 *
	 * 重复登记同一碰撞组件时覆盖既有映射。
	 *
	 * @param PrimitiveComponent 可交互的碰撞组件。
	 * @param InteractionComponent 承载交互语义的交互组件。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互子系统|API",
		meta = (DisplayName = "注册映射")
	)
	void RegisterMapping(
		UPrimitiveComponent* PrimitiveComponent,
		USingularisInteractionComponent* InteractionComponent
	);

	/**
	 * 注销碰撞组件的映射关系。
	 *
	 * @param PrimitiveComponent 待注销的碰撞组件。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互子系统|API",
		meta = (DisplayName = "注销映射")
	)
	void UnregisterMapping(UPrimitiveComponent* PrimitiveComponent);

	/**
	 * 反查碰撞组件登记的交互组件。
	 *
	 * @param PrimitiveComponent 命中的碰撞组件。
	 * @return 已登记的交互组件；未登记或弱引用失效时返回 nullptr。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互子系统|API",
		meta = (DisplayName = "映射组件")
	)
	USingularisInteractionComponent* MappingComponent(UPrimitiveComponent* PrimitiveComponent);

	/**
	 * 通知世界级会话事件。
	 *
	 * 由交互组件在驱动路径上调用。
	 *
	 * @param Target 事件归属的交互组件。
	 * @param Event 会话事件。
	 */
	void NotifyWorldSessionEvent(
		USingularisInteractionComponent* Target,
		const FSingularisInteractionSessionEvent& Event
	);

#pragma endregion
};
