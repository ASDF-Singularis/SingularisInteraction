/* ====================================================================== *
 * SingularisInteractionComponent.h                                       *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2025 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2025 TrifingZW. All Rights Reserved.                     *
 * Created: 2025/11/04 | Author: TrifingZW                                *
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
#include <Components/ActorComponent.h>

#if WITH_EDITOR
#include <Misc/DataValidation.h>
#endif

#include "Interfaces/SingularisInteractionSessionHost.h"
#include "Types/SingularisInteractionComponentType.h"
#include "Types/SingularisInteractionIntentType.h"
#include "Types/SingularisInteractionSessionType.h"
#include "SingularisInteractionComponent.generated.h"

class AActor;
class APawn;

#pragma region 委托签名

/** 交互启用时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionEnableSignature);

/** 交互禁用时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionDisableSignature);

/** 交互悬浮时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionHoverSignature);

/** 交互未悬浮时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionUnhoverSignature);

/** 交互会话事件广播：会话端全事件流（本地广播） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnInteractionSessionEventSignature,
	FSingularisInteractionSessionEvent,
	Event
);

#pragma endregion

/**
 * 引力奇点交互组件。
 *
 * 会话端：以极小转移核（建立 / 达成 / 中止）管理交互者会话，
 * 以意图为唯一输入、以会话为唯一推进规则、以事件流为唯一输出。
 * 事件经本地广播传播，连续语义由推进事件承载；
 * 组件不感知意图来自何种输入设备，玩家、AI 与脚本经同一入口在同一规则下结算。
 */
UCLASS(
	Blueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互组件")
)
class SINGULARISINTERACTION_API USingularisInteractionComponent : public UActorComponent,
                                                                  public ISingularisInteractionSessionHost
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 交互目标组件引用，用于在映射子系统登记本组件的归属 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "引力奇点交互组件",
		meta = (DisplayName = "交互目标", UseComponentPicker, AllowedClasses = "/Script/Engine.SceneComponent")
	)
	TArray<FComponentReference> TargetComponentReferences{};

	/** 交互管线集：按数组顺序装配的管线，标签层级匹配时首个命中者胜出 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互组件",
		meta = (DisplayName = "交互管线集")
	)
	TArray<FSingularisInteractionPipeline> InteractionPipelines{};

	/** 交互行为集，响应启用、禁用、悬浮与未悬浮状态变化 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互组件",
		meta = (DisplayName = "交互行为集")
	)
	TArray<FSingularisInteractionBehaviorEntry> InteractionBehaviors{};

#pragma endregion

#pragma region Event Dispatcher

	/** 交互启用时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互启用时触发")
	)
	FOnInteractionEnableSignature OnInteractionEnableEvent{};

	/** 交互禁用时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互禁用时触发")
	)
	FOnInteractionDisableSignature OnInteractionDisableEvent{};

	/** 交互悬浮时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互悬浮时触发")
	)
	FOnInteractionHoverSignature OnInteractionHoverEvent{};

	/** 交互未悬浮时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互未悬浮时触发")
	)
	FOnInteractionUnhoverSignature OnInteractionUnhoverEvent{};

	/** 交互会话事件广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互组件|事件分发器",
		meta = (DisplayName = "交互会话事件时触发")
	)
	FOnInteractionSessionEventSignature OnInteractionSessionEvent{};

#pragma endregion

private:
#pragma region State

	/** 当前是否启用 */
	bool bIsEnabled = true;

	/** 当前是否被悬浮 */
	bool bIsHovered = false;

	/** 活跃会话表，键为交互者 */
	TMap<TWeakObjectPtr<AActor>, FSingularisInteractionSessionRecord> ActiveSessions{};

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractionComponent();

#pragma endregion

#pragma region UObject Interface

#if WITH_EDITOR

	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

#endif

#pragma endregion

#pragma region ActorComponent Interface

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

#pragma endregion

#pragma region ISingularisInteractionSessionHost Interface

	virtual void HostEmitSessionEvent_Implementation(
		USingularisInteractionSession* Session,
		const FGameplayTag& EventTag,
		FSingularisInteractionValue Value,
		const FInstancedStruct& Payload
	) override;
	virtual void HostCompleteSession_Implementation(USingularisInteractionSession* Session) override;
	virtual void HostCancelSession_Implementation(USingularisInteractionSession* Session) override;

#pragma endregion

#pragma region API

	/**
	 * 当前是否启用。
	 *
	 * @return 启用时返回 true。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "Enabled")
	)
	bool Enabled() const { return bIsEnabled; }

	/**
	 * 当前是否被悬浮。
	 *
	 * @return 悬浮时返回 true。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "Hovered")
	)
	bool Hovered() const { return bIsHovered; }

	/** 设置启用状态。状态未变化时幂等返回，变化后广播事件并驱动行为；禁用时取消全部活跃会话 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "SetEnabled")
	)
	void SetEnabled(bool IsEnabled);

	/** 设置悬浮状态。状态未变化时幂等返回，变化后广播事件并驱动行为 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "SetHovered")
	)
	void SetHovered(bool IsHovered);

	/**
	 * 查询指定管线标签命中的首个管线的会话原型。
	 *
	 * 管线按数组顺序匹配，管线标签为触发标签自身或祖先时命中；
	 * 供交互者端派生镜像实例（本地预测）使用；无匹配管线或会话未配置时返回 nullptr。
	 *
	 * @param PipelineTag 管线标签。
	 * @return 会话原型。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互组件|API",
		meta = (DisplayName = "GetInteractionSession")
	)
	USingularisInteractionSession* GetInteractionSession(const FGameplayTag& PipelineTag) const;

#pragma endregion

#pragma region SPI

	/**
	 * 会话入口：接收交互者意图。
	 *
	 * 主导航（Press）建立会话，其余意图转发至会话实例。
	 * 经交互子系统单一入口转交，玩家、AI 与脚本共用同一通路。
	 *
	 * @param Interactor 交互者。
	 * @param PipelineTag 会话对应的管线标签。
	 * @param Intent 交互者提交的意图。
	 */
	void ReceiveInteractionIntent(
		AActor* Interactor,
		const FGameplayTag& PipelineTag,
		const FSingularisInteractionIntent& Intent
	);

#pragma endregion

private:
#pragma region Internal Function

	/** 将交互目标组件引用登记进映射子系统 */
	void RegisterInteractionMappings();

	/** 建立会话：管线匹配、开始判据、派生会话实例并驱动 Begin */
	void BeginSession(AActor* Interactor, const FGameplayTag& PipelineTag);

	/** 拒绝会话：广播取消反馈事件 */
	void RejectSession(AActor* Interactor, const FGameplayTag& PipelineTag);

	/** 结算会话：摘除记录、驱动结算事件与会话收尾，会话不存在时幂等返回 */
	void SettleSession(const TWeakObjectPtr<AActor>& Interactor, bool bCompleted);

	/** 驱动会话事件：本地广播、响应分发与全局观察 */
	void DriveSessionEvent(
		const FSingularisInteractionSessionRecord& Record,
		const FGameplayTag& EventTag,
		const FSingularisInteractionValue& Value,
		const FInstancedStruct& Payload
	);

	/** 驱动匹配管线响应消费会话事件 */
	void DriveSessionResponses(const FSingularisInteractionSessionEvent& Event);

	/** 组装会话上下文 */
	FSingularisInteractionSessionContext BuildSessionContext(
		const TWeakObjectPtr<AActor>& Interactor,
		const FGameplayTag& PipelineTag,
		float Elapsed
	);

	/** 依据会话表空非空切换组件 Tick */
	void RefreshSessionTick();

	/** 按会话实例反查所属会话的交互者 */
	TWeakObjectPtr<AActor> FindSessionInteractor(const USingularisInteractionSession* Session) const;

	/** 由交互者解析触发者 Pawn */
	static APawn* ResolveInstigator(AActor* Interactor);

#pragma endregion
};
