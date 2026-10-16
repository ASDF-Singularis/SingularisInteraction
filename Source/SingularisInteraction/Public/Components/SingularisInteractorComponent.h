/* ====================================================================== *
 * SingularisInteractorComponent.h                                        *
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

#include "Interfaces/SingularisInteractionSessionHost.h"
#include "Objects/SingularisInteractionDriver.h"
#include "Objects/SingularisInteractionQueryer.h"
#include "Objects/SingularisInteractionSession.h"
#include "Types/SingularisInteractionIntentType.h"
#include "Types/SingularisInteractionSessionType.h"
#include "SingularisInteractorComponent.generated.h"

class AActor;
class APlayerController;
class APawn;
class USingularisInteractionComponent;

#pragma region 委托签名

/** 交互者锁定的交互目标发生变化时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractionTargetChangedSignature,
	USingularisInteractionComponent*,
	OldTarget,
	USingularisInteractionComponent*,
	NewTarget
);

/** 交互者本地会话流广播：本地玩家为本地会话产物，其余交互者为会话端事件转发，驱动玩家侧与 AI 观察 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractorSessionEventSignature,
	USingularisInteractionComponent*,
	Target,
	FSingularisInteractionSessionEvent,
	Event
);

#pragma endregion

/**
 * 引力奇点交互者组件。
 *
 * 任意交互主体的驱动端：挂载于玩家控制器、Pawn、AI 控制器或任意 Actor。
 * 以可插拔目标锁定（查询器或外部注入）与可插拔驱动器（增强输入或 C++ 扩展）
 * 把来源事件翻译为中性意图，经交互子系统单一入口提交；
 * 本地玩家在本地派生镜像会话并就地推进，产出零延迟的本地会话流，不消费会话端状态与事件。
 * AI 与脚本无需驱动器：直接调用组件 API 或子系统即与玩家同规结算。
 */
UCLASS(
	Blueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互者组件")
)
class SINGULARISINTERACTION_API USingularisInteractorComponent : public UActorComponent,
                                                                 public ISingularisInteractionSessionHost
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 引力奇点交互查询器：目标锁定算法；留空为外部注入目标模式（AI / 脚本） */
	UPROPERTY(
		Instanced,
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互者组件",
		meta = (DisplayName = "引力奇点交互查询器")
	)
	USingularisInteractionQueryer* InteractionQueryer = nullptr;

	/** 引力奇点交互驱动器：输入源适配（增强输入 / C++ 扩展）；AI 与脚本可留空 */
	UPROPERTY(
		Instanced,
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互者组件",
		meta = (DisplayName = "引力奇点交互驱动器")
	)
	USingularisInteractionDriver* InteractionDriver = nullptr;

#pragma endregion

#pragma region Event Dispatcher

	/** 交互者锁定的交互目标发生变化时广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互者组件|事件分发器",
		meta = (DisplayName = "交互目标变更时触发")
	)
	FOnInteractionTargetChangedSignature OnInteractionTargetChangedEvent{};

	/** 交互者本地会话流广播 */
	UPROPERTY(
		BlueprintAssignable,
		Category = "引力奇点交互者组件|事件分发器",
		meta = (DisplayName = "交互者会话事件时触发")
	)
	FOnInteractorSessionEventSignature OnInteractorSessionEvent{};

#pragma endregion

private:
#pragma region State

	/** 宿主玩家控制器（惰性解析并缓存，非本地玩家时为空） */
	TWeakObjectPtr<APlayerController> OwnerPlayerController = nullptr;

	/** 当前视线查询结果（含外部注入） */
	FSingularisInteractionQueryerResult CurrentQueryerResult{};

	/** 是否进行中的驱动 */
	bool bDriveActive = false;

	/** 进行中驱动的目标 */
	TWeakObjectPtr<USingularisInteractionComponent> DriveTarget = nullptr;

	/** 进行中驱动的策略标签 */
	FGameplayTag DriveStrategyTag{};

	/** 镜像会话实例（本地玩家就地派生并推进的会话） */
	TObjectPtr<USingularisInteractionSession> MirrorSession = nullptr;

	/** 镜像是否运行中 */
	bool bMirrorActive = false;

	/** 镜像上下文 */
	FSingularisInteractionSessionContext MirrorContext{};

	/** 镜像累计时长（秒） */
	float MirrorElapsed = 0.0f;

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractorComponent();

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
		float Alpha
	) override;
	virtual void HostCompleteSession_Implementation(USingularisInteractionSession* Session) override;
	virtual void HostCancelSession_Implementation(USingularisInteractionSession* Session) override;

#pragma endregion

#pragma region API

	/**
	 * 当前锁定的交互目标组件。
	 *
	 * @return 当前目标组件；无有效目标时返回 nullptr。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互者组件|API",
		meta = (DisplayName = "GetCurrentTarget")
	)
	USingularisInteractionComponent* GetCurrentTarget() const { return CurrentQueryerResult.InteractionComponent; }

	/**
	 * 注入或清除外部目标（外部注入目标模式）。
	 *
	 * 供 AI、脚本与测试使用；查询器模式由逐帧视线查询维护，无需调用本函数。
	 *
	 * @param NewTarget 新目标组件；传入 nullptr 清除目标。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互者组件|API",
		meta = (DisplayName = "SetTarget")
	)
	void SetTarget(USingularisInteractionComponent* NewTarget);

	/**
	 * 提交交互意图（唯一提交入口）。
	 *
	 * 本地玩家交互者就地派生并推进镜像会话、广播本地流，随后经世界子系统单一入口提交；
	 * 单驱动卫语句保证同一时刻仅驱动一个目标会话。
	 *
	 * @param Target 目标组件。
	 * @param StrategyTag 会话对应的策略标签。
	 * @param IntentTag 意图标签（Singularis.Interaction.Intent.*）。
	 * @param Value 意图量值（连续语义：布尔 / 一维 / 二维 / 三维）。
	 * @return 受理时返回 true。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互者组件|API",
		meta = (DisplayName = "SubmitInteractionIntent")
	)
	bool SubmitInteractionIntent(
		USingularisInteractionComponent* Target,
		FGameplayTag StrategyTag,
		FGameplayTag IntentTag,
		const FSingularisInteractionValue& Value
	);

	/**
	 * 主张交互（等价提交 Press 意图）。
	 *
	 * @param Target 目标组件。
	 * @param StrategyTag 会话对应的策略标签。
	 * @return 受理时返回 true。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互者组件|API",
		meta = (DisplayName = "BeginInteraction")
	)
	bool BeginInteraction(USingularisInteractionComponent* Target, FGameplayTag StrategyTag);

	/**
	 * 撤回交互（等价提交 Release 意图）。
	 *
	 * @param Target 目标组件。
	 * @param StrategyTag 会话对应的策略标签。
	 * @return 受理时返回 true。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互者组件|API",
		meta = (DisplayName = "EndInteraction")
	)
	bool EndInteraction(USingularisInteractionComponent* Target, FGameplayTag StrategyTag);

	/**
	 * 宿主玩家控制器（惰性解析并缓存）。
	 *
	 * 供驱动器解析输入装配；非玩家交互者返回 nullptr。
	 *
	 * @return 宿主玩家控制器。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互者组件|API",
		meta = (DisplayName = "GetInteractorPlayerController")
	)
	APlayerController* GetInteractorPlayerController();

	/**
	 * 是否为本地玩家交互者。
	 *
	 * @return 宿主由本地玩家控制时返回 true。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互者组件|API",
		meta = (DisplayName = "IsLocalPlayerInteractor")
	)
	bool IsLocalPlayerInteractor();

#pragma endregion

private:
#pragma region Response

	/** 目标会话事件回调：本地玩家不消费；其余交互者转发会话端事件（目标即当前锁定目标） */
	UFUNCTION()
	void HandleTargetSessionEvent(FSingularisInteractionSessionEvent Event);

#pragma endregion

#pragma region Internal Function

	/** 执行一次视线查询并刷新当前交互状态 */
	void Query();

	/** 设置查询结果（幂等） */
	void SetQueryerResult(const FSingularisInteractionQueryerResult& QueryerResult);

	/** 响应式编程：应用查询结果副作用（目标事件绑定、驱动器通知、悬浮迁移、驱动中断） */
	void ApplyQueryerResult(const FSingularisInteractionQueryerResult& OldQueryerResult);

	/** 由交互者解析触发者 Pawn */
	static APawn* ResolveInstigator(AActor* Interactor);

	/** 启动镜像会话（以原型副本派生，绑定本组件为宿主） */
	bool BeginMirror(const USingularisInteractionSession* Archetype);

	/** 向镜像会话摄入意图 */
	void FeedMirrorIntent(const FSingularisInteractionIntent& Intent);

	/** 逐帧推进镜像会话（本地玩家交互者） */
	void MirrorTick(float DeltaTime);

	/** 广播本地会话流 */
	void BroadcastInteractorSession(
		USingularisInteractionComponent* Target,
		const FGameplayTag& StrategyTag,
		const FGameplayTag& EventTag,
		float Alpha
	) const;

	/** 镜像结算：冲刷镜像并广播结算事件 */
	void EndMirror(bool bCompleted);

	/** 冲刷驱动的全部镜像状态（会话终结 / 目标迁移 / 原型不可用） */
	void FlushDrive();

#pragma endregion
};
