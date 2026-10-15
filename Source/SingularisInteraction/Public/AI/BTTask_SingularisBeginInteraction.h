/* ====================================================================== *
 * BTTask_SingularisBeginInteraction.h                                    *
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
#include "BTTask_SingularisBeginInteraction.generated.h"

class UBehaviorTreeComponent;
class USingularisInteractorComponent;
class USingularisInteractionComponent;

/**
 * 行为树任务：主张一次交互并潜伏等待结算。
 *
 * 从黑板解析目标，经交互者组件提交主张意图（与玩家同规、同一时钟裁决，
 * AI 无法以更短时长作弊通过长按会话）；观察交互者本地会话流：
 * 达成为成功、中止为失败；超时、交互者或目标失效为失败。
 * 挂载要求：AIController 或其 Pawn 挂有交互者组件。
 */
UCLASS(meta = (DisplayName = "Singularis 主张交互"))
class SINGULARISINTERACTION_API UBTTask_SingularisBeginInteraction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SingularisBeginInteraction();

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

	/** 潜伏超时（秒），0 为无限 */
	UPROPERTY(
		EditAnywhere,
		Category = "引力奇点交互",
		meta = (ClampMin = "0.0", ForceUnits = "s")
	)
	float TimeoutSeconds = 0.0f;

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

	/** 潜伏计时（秒） */
	float ElapsedSeconds = 0.0f;

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

	/** 解绑观察并完成潜伏任务 */
	void FinishLatent(UBehaviorTreeComponent& OwnerComp, bool bSucceeded);
};
