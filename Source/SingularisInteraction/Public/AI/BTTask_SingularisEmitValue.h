/* ====================================================================== *
 * BTTask_SingularisEmitValue.h                                           *
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
#include <BehaviorTree/BTTaskNode.h>
#include <BehaviorTree/BehaviorTreeTypes.h>

#include "Types/SingularisInteractionSessionType.h"
#include "BTTask_SingularisEmitValue.generated.h"

class UBehaviorTreeComponent;
class USingularisInteractorComponent;
class USingularisInteractionComponent;

/**
 * 行为树任务：主张一次交互并持续供值，潜伏至结算。
 *
 * 轴类会话（摇柄 / 转盘 / 蓄力）需要连续值意图，而主张任务不携带量值；
 * 本任务从黑板或常量读取量值逐帧提交，与主张任务同规（同一提交入口、同一时钟）。
 * 首次供值前可经 `BeginInteraction` 建立会话；观察交互者本地会话流：
 * 达成为成功、中止为失败。供值时长耗尽但会话仍在时按撤回处理。
 * 挂载要求：AIController 或其 Pawn 挂有交互者组件。
 */
UCLASS(meta = (DisplayName = "Singularis 供值交互"))
class SINGULARISINTERACTION_API UBTTask_SingularisEmitValue : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SingularisEmitValue();

	/** 会话对应的策略标签 */
	UPROPERTY(
		EditAnywhere,
		Category = "引力奇点交互",
		meta = (
			Categories = "Singularis.Interaction.Strategy",
			ForceSelection = "true"
		)
	)
	FGameplayTag StrategyTag{};

	/** 目标黑板键（交互组件或 Actor） */
	UPROPERTY(EditAnywhere, Category = "引力奇点交互")
	FBlackboardKeySelector TargetKey{};

	/** 供值黑板键（浮点）；未选择时使用常量量值 */
	UPROPERTY(EditAnywhere, Category = "引力奇点交互")
	FBlackboardKeySelector ValueKey{};

	/** 常量供值量值（未选择供值黑板键时生效） */
	UPROPERTY(EditAnywhere, Category = "引力奇点交互")
	float Value = 1.0f;

	/** 供值间隔（秒），0 为逐帧 */
	UPROPERTY(
		EditAnywhere,
		Category = "引力奇点交互",
		meta = (ClampMin = "0.0", ForceUnits = "s")
	)
	float EmitInterval = 0.0f;

	/** 供值时长（秒），0 为持续至会话结算 */
	UPROPERTY(
		EditAnywhere,
		Category = "引力奇点交互",
		meta = (ClampMin = "0.0", ForceUnits = "s")
	)
	float DurationSeconds = 0.0f;

	/** 供值前是否建立会话（关闭时为对既有会话续供） */
	UPROPERTY(EditAnywhere, Category = "引力奇点交互")
	bool bEstablishSession = true;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

private:
	/** 观察中的交互者组件 */
	TWeakObjectPtr<USingularisInteractorComponent> ObservedInteractor = nullptr;

	/** 进行中的目标组件 */
	TWeakObjectPtr<USingularisInteractionComponent> ActiveTarget = nullptr;

	/** 行为树组件（完成潜伏回调依据） */
	TWeakObjectPtr<UBehaviorTreeComponent> OwnerComponent = nullptr;

	/** 供值累计时长（秒） */
	float ElapsedSeconds = 0.0f;

	/** 供值节流累计（秒） */
	float EmitAccumulator = 0.0f;

	/** 潜伏是否已结算（重复结算保护） */
	bool bLatentFinished = false;

	/** 交互者会话流观察：达成为成功、中止为失败 */
	UFUNCTION()
	void HandleInteractorSessionEvent(
		USingularisInteractionComponent* Target,
		FSingularisInteractionSessionEvent Event
	);

	/** 解析交互者组件（AIController 优先，其次其 Pawn） */
	USingularisInteractorComponent* ResolveInteractorComponent(UBehaviorTreeComponent& OwnerComp) const;

	/** 解析黑板目标组件 */
	USingularisInteractionComponent* ResolveTargetComponent(UBehaviorTreeComponent& OwnerComp) const;

	/** 解析本次供值量值（黑板优先，回退常量） */
	float ResolveValue(UBehaviorTreeComponent& OwnerComp) const;

	/** 经交互者组件单一提交入口提交一次连续值意图 */
	void EmitValue(UBehaviorTreeComponent& OwnerComp);

	/** 解绑观察并完成潜伏任务 */
	void FinishLatent(UBehaviorTreeComponent& OwnerComp, bool bSucceeded);
};
